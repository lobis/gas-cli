import random
import os
import subprocess
import numpy as np
from pathlib import Path

gas_cli = "/afs/desy.de/user/l/lobis/.local/bin/gas-cli"
storage_dir = Path("/afs/desy.de/user/l/lobis/gas-simulations/dag")


def get_efield_points(n: int):
    def similar(a, b):
        return abs(a - b) < max(1e-3 * (abs(a) + abs(b)), 1e-20)

    j = 0
    while True:
        result = []
        for x in sorted(
                np.concatenate(
                    (
                            np.logspace(0, 4, n // 2 + (j + 1) // 2),
                            np.linspace(1, 10000, n // 2 + j // 2),
                    )
                )
        ):
            if not any(similar(i, x) for i in result):
                result.append(x)
        if len(result) >= n:
            return result
        j += 1


e_field = get_efield_points(200)

runs = [
    ("C4H10 0.5 Ar", e_field, 1),
]

print(runs)

# randomly shuffle runs (optional)
random.shuffle(runs)

print("Number of runs:", len(runs))


def get_names_fractions(component_strings):
    names = []
    fractions = []
    split = component_strings.split()
    for i in range(len(split)):
        if i % 2 == 0:
            names.append(split[i])
        else:
            fractions.append(float(split[i]))
    if len(names) == len(fractions) + 1:
        fractions.append(100 - sum(fractions))
    if len(names) != len(fractions):
        raise Exception("Names and fractions do not match")

    if len(names) == 1:
        return f"{names[0]}"
    # sort by fractions, if fractions are equal sort by name
    names_fractions = list(zip(names, fractions))
    names_fractions.sort(key=lambda x: (-x[1], x[0]), reverse=False)

    # concatenate name and fraction with _ as separator
    # remove trailing zeros of fraction except for the last 0 after .
    def clean_number(x):
        s = f"{x:.3f}".rstrip("0")
        if s.endswith("."):
            s += "0"
        return s

    names_fractions = ["_".join([name, clean_number(fraction)])
                       for name, fraction in names_fractions]
    return "-".join(names_fractions)


for (components, e_field_values, e_field_batch_size) in runs:
    # divide e_field_values into batches of size e_field_batch_size
    e_field_batches = [e_field_values[i:i + e_field_batch_size]
                       for i in range(0, len(e_field_values), e_field_batch_size)]

    output_dir = storage_dir / get_names_fractions(components)

    data_dir = output_dir / "data"
    data_dir.mkdir(parents=True, exist_ok=True)

    print(f"Data will be stored in {data_dir}")

    jobs_dir = output_dir / "jobs"
    subs_dir = jobs_dir / "submissions"
    logs_dir = jobs_dir / "logs"
    error_dir = jobs_dir / "error"
    jobs_output_dir = jobs_dir / "output"

    subs_dir.mkdir(parents=True, exist_ok=True)
    logs_dir.mkdir(parents=True, exist_ok=True)
    error_dir.mkdir(parents=True, exist_ok=True)
    jobs_output_dir.mkdir(parents=True, exist_ok=True)

    env = ""
    for key, value in os.environ.items():
        if '\n' in value:
            continue
        env += f"export {key}='{value}'\n"

    sub_files = []
    output_files = []

    for i, e_field_batch in enumerate(e_field_batches):
        output_file = f"{(str(data_dir))}/{i}.gas"
        output_files.append(output_file)
        # if output file exists and is not empty, skip
        if os.path.exists(output_file) and os.path.getsize(output_file) > 0:
            print(f"Skipping {output_file}")
            continue

        command = f"{gas_cli} generate --components {components} --efield {' '.join([f'{e:0.4f}' for e in e_field_batch])} --collisions 10 --output {output_file}"
        script_content = f"""{command}"""

        name = f"{str(subs_dir)}/script_{i}.sh"
        name_sub = f"{str(subs_dir)}/job_{i}.sub"

        submission_file_content = f"""
    executable   = {name}
    arguments    =
    getenv       = True

    output       = {str(jobs_output_dir)}/output_{i}
    error        = {str(error_dir)}/error_{i}
    log          = {str(logs_dir)}/log_{i}

    request_cpus   = 1

    +RequestRuntime = 7200

    should_transfer_files = yes

    queue    
    """

        with open(name, "w") as f:
            f.write(script_content)

        with open(name_sub, "w") as f:
            f.write(submission_file_content)

        print(command)
        sub_files.append(name_sub)

    print("Total number of jobs:", len(sub_files))

    # merge job
    command = f"""
{gas_cli} merge -i {' '.join(output_files)} --tar -o {str(output_dir)}/{get_names_fractions(components)}.gas
mv {str(output_dir)}/{get_names_fractions(components)}.gas.tar.gz {storage_dir}
    """
    script_content = f"""{command}"""

    name = f"{str(subs_dir)}/script_merge.sh"
    name_sub = f"{str(subs_dir)}/job_merge.sub"

    submission_file_content = f"""
    executable   = {name}
    arguments    =
    getenv       = True

    output       = {str(jobs_output_dir)}/output_merge
    error        = {str(error_dir)}/error_merge
    log          = {str(logs_dir)}/log_merge

    request_cpus   = 1

    should_transfer_files = yes

    queue    
    """

    with open(name, "w") as f:
        f.write(script_content)

    with open(name_sub, "w") as f:
        f.write(submission_file_content)

    jobs = "\n".join([f"JOB job_{i} {sub_file}" for i,
    sub_file in enumerate(sub_files)])
    dag_submission_content = f"""
{jobs}

JOB job_merge {name_sub}
PARENT {" ".join([f"job_{i}" for i in range(len(sub_files))])} CHILD job_merge
"""
    name_dag_file = f"{str(subs_dir)}/dag"

    with open(name_dag_file, "w") as f:
        f.write(dag_submission_content)

    print(f"Submitting {name_dag_file}")
    subprocess.run(["condor_submit_dag", name_dag_file])

    print("Finished!")
