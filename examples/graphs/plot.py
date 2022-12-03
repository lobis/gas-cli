
import numpy as np
import matplotlib.pyplot as plt
import argparse
import json

parser = argparse.ArgumentParser(description="plot gas graphs")

parser.add_argument("filename")

args = parser.parse_args()

filename = args.filename

print(f"selected filename: {filename}")

with open(filename) as f:
    data = json.load(f)

x = data["electric_field"]
drift_velocity = data["electron_drift_velocity"]
diffusion_transversal = data["electron_transversal_diffusion"]
diffusion_longitudinal = data["electron_longitudinal_diffusion"]
electron_townsend = data["electron_townsend"]

fig, axs = plt.subplots(2, 2)

axs[0, 0].plot(x, drift_velocity, ".")
axs[0, 0].set_title("Electron drift velocity")
axs[0, 0].set_ylabel("Drift velocity (cm/μs)")

axs[1, 0].plot(x, electron_townsend, '.')
axs[1, 0].set_title("Townsend coefficient")
axs[1, 0].set_ylabel("Townsend coefficient")

axs[0, 1].plot(x, diffusion_transversal, ".")
axs[0, 1].set_title("Transversal diffusion coefficient")
axs[0, 1].set_ylabel("Transversal diffusion coefficient (sqrt(cm))")

axs[1, 1].plot(x, diffusion_longitudinal, '.')
axs[1, 1].set_title("Longitudinal diffusion coefficient")
axs[1, 1].set_ylabel("Transversal diffusion coefficient (sqrt(cm))")

for ax in axs.flat:
    ax.set(xlabel="Electric field (V/cm)")

plt.show()
