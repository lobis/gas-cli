
#include <iostream>
#include <regex>

#include "CLI/App.hpp"
#include "CLI/Config.hpp"
#include "CLI/Formatter.hpp"

#include "Gas.h"
#include "Tools.h"

using namespace std;

namespace fs = std::filesystem;

int main(int argc, char** argv) {

    CLI::App app{"Gas CLI (https://github.com/lobis/gas-cli)"};

    fs::path gasFilenameInput;
    fs::path gasFilenameOutput;
    fs::path outputDirectory;
    fs::path gasPropertiesJsonFilename;

    CLI::App* read = app.add_subcommand("read", "Read from a gas file properties such as drift velocity of diffusion coefficients and generate a JSON file with the results");
    read->add_option("-g,--gas,-i,--input", gasFilenameInput, "Garfield gas file (.gas) read from")->required();
    read->add_option("-o,--output,--json", gasPropertiesJsonFilename, "Location to save gas properties as json file. If location not specified it will auto generate it")->expected(0, 1);
    read->add_option("--dir,--output-dir,--output-directory", outputDirectory, "Directory to save json file into")->expected(1);

    CLI::App* generate = app.add_subcommand("generate", "Generate a Garfield gas file using command line parameters");
    generate->add_option("-g,--gas,-o,--output", gasFilenameOutput, "Garfield gas file (.gas) to save output into");
    generate->add_option("--dir,--output-dir,--output-directory", outputDirectory, "Directory to save gas file into")->expected(1);
    generate->add_option("--json", gasPropertiesJsonFilename, "Location to save gas properties as json file. If location not specified it will auto generate it")->expected(0, 1);
    vector<string> generateGasComponentsString;
    generate->add_option("--components,--mixture", generateGasComponentsString, "Garfield gas components to use in the gas file. It should be of the form of 'component1', 'fraction1', 'component2', 'fraction2', ... up to 6 components")->required()->expected(1, 12);
    double pressure = 1.0, temperature = 20.0;
    generate->add_option("--pressure", pressure, "Gas pressure in bar");
    generate->add_option("--temperature,--temp", temperature, "Gas temperature in Celsius");
    unsigned int numberOfCollisions = 10;
    generate->add_option("--collisions,--ncoll,--nColl", numberOfCollisions, "Number of collisions to simulate (defaults to 10)");
    bool generateVerbose = false;
    generate->add_flag("-v,--verbose", generateVerbose, "Garfield verbosity");
    bool generateProgress = true;
    generate->add_flag("--progress,!--no-progress", generateProgress, "Save progress periodically to output file (defaults to true)");
    bool generatePrint = false;
    generate->add_flag("--print", generatePrint, "Print gas properties to stdout after generating gas file (defaults to false)");

    vector<double> subcommandGasElectricFieldValues;
    vector<double> subcommandGasElectricFieldLinearOptions;
    vector<double> subcommandGasElectricFieldLogOptions;
    for (CLI::App* subcommand: {read, generate}) {
        subcommand->add_option("--electric-field,--field,--efield,-E,-e", subcommandGasElectricFieldValues, "Gas electric field values in V/cm");
        subcommand->add_option("--electric-field-linear,--electric-field-lin,--field-lin,--efield-lin,--E-lin,--e-lin", subcommandGasElectricFieldLinearOptions, "Use linearly spaced electric field values (start, end, number)")->expected(3);
        subcommand->add_option("--electric-field-log,--electric-field-log,--field-log,--efield-log,--E-log,--e-log", subcommandGasElectricFieldLogOptions, "Use logarithmically spaced electric field values (start, end, number)")->expected(3);
    }

    bool generateTestOnly = false;
    generate->add_flag("--test", generateTestOnly, "Do not run generation (used to test input parameters)");

    CLI::App* merge = app.add_subcommand("merge", "Merge multiple Garfield gas files into one");
    merge->add_option("-g,--gas,-o,--output", gasFilenameOutput, "Garfield gas file (.gas) to save output into")->required();
    vector<fs::path> mergeGasInputFilenames;
    merge->add_option("-i,--input", mergeGasInputFilenames, "Garfield gas file (.gas) to merge into the output. In case of overlaps, first file of list will take precedence")->required()->expected(2, numeric_limits<int>::max());
    merge->add_option("--dir,--output-dir,--output-directory", outputDirectory, "Directory to save merged gas file into")->expected(1);
    bool mergeVerbose = false;
    merge->add_flag("-v,--verbose", mergeVerbose, "Merge verbosity");
    bool mergeCompressOutput = false;
    merge->add_flag("--tar,--compress", mergeCompressOutput, "Compress output gas file using tar");

    app.require_subcommand(1);

    CLI11_PARSE(app, argc, argv);

    if (gasFilenameOutput.is_absolute() && !outputDirectory.empty()) {
        cerr << "Cannot specify an output directory and an absolute path as output file" << endl;
        return 1;
    }
    if (outputDirectory.empty()) {
        outputDirectory = fs::current_path();
    }

    const auto subcommand = app.get_subcommands().back();
    const string subcommandName = subcommand->get_name();

    // generate electric field values from user options
    auto& eField = subcommandGasElectricFieldValues;
    {
        if (!subcommandGasElectricFieldLinearOptions.empty()) {
            const vector<double> electricFieldLinear = tools::linspace<double>(subcommandGasElectricFieldLinearOptions[0], subcommandGasElectricFieldLinearOptions[1], static_cast<unsigned int>(subcommandGasElectricFieldLinearOptions[2]));
            eField.insert(eField.end(), electricFieldLinear.begin(), electricFieldLinear.end());
        }
        if (!subcommandGasElectricFieldLogOptions.empty()) {
            const vector<double> electricFieldLog = tools::logspace<double>(subcommandGasElectricFieldLogOptions[0], subcommandGasElectricFieldLogOptions[1], static_cast<unsigned int>(subcommandGasElectricFieldLogOptions[2]));
            eField.insert(eField.end(), electricFieldLog.begin(), electricFieldLog.end());
        }
        if (!eField.empty()) {
            sort(eField.begin(), eField.end());
            cout << "Electric field values (V/cm):";
            for (const auto& e: eField) {
                cout << " " << e;
            }
            cout << endl;
        }
    }

    if (subcommandName == "read") {
        cout << "Reading gas properties from file: " << gasFilenameInput << endl;
        Gas gas(gasFilenameInput);

        // if user specified electric field values, those values will be used, otherwise the gas file will be read for the electric field values
        const auto gasProperties = gas.GetGasPropertiesJson(eField);

        if (read->get_option("--json")->empty()) {
            // print electric field info
            const auto& eFieldValues = gasProperties["electric_field"];
            cout << "Number of electric field values: " << eFieldValues.size() << endl;
            cout << gasProperties.dump(4) << endl;
        } else {
            if (gasPropertiesJsonFilename.empty()) {
                gasPropertiesJsonFilename = string(gasFilenameInput.filename()) + ".json";
            }
            gasPropertiesJsonFilename = outputDirectory / gasPropertiesJsonFilename;

            cout << "Gas properties json will be saved to " << gasPropertiesJsonFilename << endl;
            tools::writeToFile(gasPropertiesJsonFilename, gasProperties.dump());
        }
    } else if (subcommandName == "generate") {
        vector<pair<string, double>> gasComponents;
        {
            vector<string> components;
            vector<double> fractions;

            regex rgx("^[0-9]+([.][0-9]+)?"); // only positive (decimal) numbers
            for (const auto& componentString: generateGasComponentsString) {
                std::smatch matches;
                if (std::regex_search(componentString, matches, rgx)) {
                    fractions.push_back(stod(componentString));
                } else {
                    components.push_back(componentString);
                }
            }

            if (!components.empty() && fractions.size() == components.size() - 1) {
                double sum = 0;
                for (const auto& value: fractions) { sum += value; }
                const double inferredPercentage = 100.0 - sum;
                fractions.emplace_back(inferredPercentage);
                cerr << "Warning: Inferred fraction of " << inferredPercentage << "% for component " << components.back() << endl;
            }

            if (components.size() != fractions.size()) {
                cerr << "Error parsing components: number of component names and fractions mismatch" << endl;
                exit(1);
            }

            // do not allow negative fractions
            for (int i = 0; i < fractions.size(); ++i) {
                if (fractions[i] < 0) {
                    cerr << "Error: Fraction of component " << components[i] << " cannot be negative: " << fractions[i] << endl;
                    exit(1);
                }
            }

            for (size_t i = 0; i < components.size(); ++i) {
                // if component fraction is zero, do not add it to the gas
                if (fractions[i] == 0) {
                    cerr << "Warning: Component " << components[i] << " has zero fraction, will not be added to gas" << endl;
                    continue;
                }
                gasComponents.emplace_back(components[i], fractions[i]);
            }
        }

        tools::removeSimilarElements(eField);
        if (eField.empty()) {
            cerr << "No electric field values provided (--help)" << endl;
            return 1;
        }

        // validation is handled in constructor
        Gas gas(gasComponents);

        gas.SetPressure(pressure);
        gas.SetTemperature(temperature);

        if (gasFilenameOutput.empty()) {
            string name = gas.GetName();
            name += "-T" + tools::numberToCleanNumberString(temperature) + "C";
            name += "-P" + tools::numberToCleanNumberString(pressure) + "bar";
            name += "-nColl" + to_string(numberOfCollisions);
            name += "-E" + (eField.size() == 1 ? tools::numberToCleanNumberString(eField.front()) : tools::numberToCleanNumberString(eField.front()) + "t" + tools::numberToCleanNumberString(eField.back())) + "Vcm";
            name += "-nE" + to_string(eField.size());
            if (!subcommandGasElectricFieldLinearOptions.empty()) {
                unsigned int nLin = subcommandGasElectricFieldLinearOptions[2];
                if (nLin == eField.size()) {
                    name += "lin";
                } else {
                    name += "lin" + to_string(nLin);
                }
            }
            if (!subcommandGasElectricFieldLogOptions.empty()) {
                unsigned int nLog = subcommandGasElectricFieldLogOptions[2];
                if (nLog == eField.size()) {
                    name += "log";
                } else {
                    name += "log" + to_string(nLog);
                }
            }
            name += ".gas";
            gasFilenameOutput = name;
        }

        if (!gasFilenameOutput.is_absolute()) {
            gasFilenameOutput = outputDirectory / gasFilenameOutput;
        }

        cout << "Gas file will be saved to " << gasFilenameOutput << endl;

        if (generateTestOnly) {
            cout << "Test only, no gas file will be generated" << endl;
            return 0;
        }

        {
            // create empty file
            fs::remove(gasFilenameOutput);
            ofstream ofs(gasFilenameOutput);
            ofs.close();

            if (!fs::exists(gasFilenameOutput)) {
                cerr << "Gas file '" << gasFilenameOutput << "' could not be created" << endl;
                return 1;
            }
        }

        if (!generateProgress) {
            gas.Generate(eField, numberOfCollisions, generateVerbose);
            gas.Write(gasFilenameOutput);
        } else {
            tools::sortVectorForCompute(eField);
            for (int i = 0; i < eField.size(); i++) {
                gas.Generate({eField[i]}, numberOfCollisions, generateVerbose);
                if (i > 0) {
                    gas.Merge(gasFilenameOutput);
                }
                cout << "Progress: " << i + 1 << "/" << eField.size() << endl;
                gas.Write(gasFilenameOutput);
            }
        }

        cout << "Gas file saved to " << gasFilenameOutput << endl;

        if (!generate->get_option("--json")->empty()) {
            // user specified to also save gas properties as json
            if (gasPropertiesJsonFilename.empty()) {
                gasPropertiesJsonFilename = string(gasFilenameOutput.filename()) + ".json";
            }
            gasPropertiesJsonFilename = outputDirectory / gasPropertiesJsonFilename;

            cout << "Gas properties json will be saved to " << gasPropertiesJsonFilename << endl;
            tools::writeToFile(gasPropertiesJsonFilename, gas.GetGasPropertiesJson().dump());
        }

        if (generatePrint) {
            cout << gas.GetGasPropertiesJson() << endl;
        }

    } else if (subcommandName == "merge") {
        if (!gasFilenameOutput.is_absolute()) {
            gasFilenameOutput = outputDirectory / gasFilenameOutput;
        }

        cout << "Gas file will be saved to " << gasFilenameOutput << endl;

        // check if any file is empty and remove them
        {
            vector<fs::path> emptyGasFiles;
            for (const auto& filename: mergeGasInputFilenames) {
                if (fs::is_empty(filename)) {
                    emptyGasFiles.push_back(filename);
                }
            }
            // remove all empty files
            for (const auto& filename: emptyGasFiles) {
                cerr << "Warning: Gas file '" << filename << "' is empty and will be ignored" << endl;
                mergeGasInputFilenames.erase(std::remove(mergeGasInputFilenames.begin(), mergeGasInputFilenames.end(), filename), mergeGasInputFilenames.end());
            }

            if (mergeGasInputFilenames.size() < 2) {
                cerr << "At least two gas files are required for merging" << endl;
                return 1;
            }
        }

        cout << "Merging " << mergeGasInputFilenames.size() << " gas files:" << endl;
        for (const auto& filename: mergeGasInputFilenames) {
            cout << "    - " << filename << endl;
        }

        auto gas = Gas(mergeGasInputFilenames[0]);
        for (unsigned int i = 1; i < mergeGasInputFilenames.size(); i++) {
            const auto& toMerge = mergeGasInputFilenames[i];
            cout << "Merging " << toMerge << endl;

            if (mergeVerbose) {
                {
                    const auto values = Gas(toMerge).GetTableElectricField();
                    cout << "Electric field values (V/cm) for file to merge (" << values.size() << "):";
                    for (const auto& value: values) {
                        cout << " " << value;
                    }
                    cout << endl;
                }

                {
                    const auto values = gas.GetTableElectricField();
                    cout << "Electric field values (V/cm) for base file (" << values.size() << "):";
                    for (const auto& value: gas.GetTableElectricField()) {
                        cout << " " << value;
                    }
                    cout << endl;
                }
            }

            bool mergeOk = gas.Merge(toMerge);
            if (!mergeOk) {
                cerr << "Error merging gas files" << endl;
                return 1;
            }
        }

        gas.Write(gasFilenameOutput);

        if (mergeVerbose) {
            const auto values = gas.GetTableElectricField();
            cout << "Electric field values (V/cm) for final merge file (" << values.size() << "):";
            for (const auto& value: values) {
                cout << " " << value;
            }
            cout << endl;
        }

        cout << "Gas file saved to " << gasFilenameOutput << endl;

        if (mergeCompressOutput) {
            const fs::path gasFilenameOutputTar = gasFilenameOutput.string() + ".tar.gz";
            cout << "Compressing gas file to " << gasFilenameOutputTar << endl;
            // change into output directory to avoid absolute paths in tar
            const auto currentPath = fs::current_path();
            fs::current_path(gasFilenameOutput.parent_path());
            tools::tar(gasFilenameOutputTar, {gasFilenameOutput.filename()});
            // return to original path
            fs::current_path(currentPath);
        }
    }
}
