
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
    vector<double> readGasElectricFieldValues;
    read->add_option("--electric-field,--field,--efield,-E", readGasElectricFieldValues, "Optional electric field values (V/cm) to read properties for. Warning: if the values are not in the gas file they will be interpolated and the results may not be accurate.");

    CLI::App* generate = app.add_subcommand("generate", "Generate a Garfield gas file using command line parameters");
    generate->add_option("-g,--gas,-o,--output", gasFilenameOutput, "Garfield gas file (.gas) to save output into");
    generate->add_option("--dir,--output-dir,--output-directory", outputDirectory, "Directory to save gas file into")->expected(1);
    generate->add_option("--json", gasPropertiesJsonFilename, "Location to save gas properties as json file. If location not specified it will auto generate it")->expected(0, 1);
    vector<string> generateGasComponentsString;
    generate->add_option("--components", generateGasComponentsString, "Garfield gas components to use in the gas file. It should be of the form of 'component1', 'fraction1', 'component2', 'fraction2', ... up to 6 components")->required()->expected(1, 12);
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
    vector<double> generateGasElectricFieldValues;
    generate->add_option("--electric-field,--field,--efield,-E", generateGasElectricFieldValues, "Gas electric field values in V/cm");
    vector<double> generateGasElectricFieldLinearOptions;
    generate->add_option("--electric-field-linear,--electric-field-lin,--field-lin,--efield-lin,--E-lin", generateGasElectricFieldLinearOptions, "Use linearly spaced electric field values (start, end, number)")->expected(3);
    vector<double> generateGasElectricFieldLogOptions;
    generate->add_option("--electric-field-log,--electric-field-log,--field-log,--efield-log,--E-log", generateGasElectricFieldLogOptions, "Use logarithmically spaced electric field values (start, end, number)")->expected(3);

    CLI::App* merge = app.add_subcommand("merge", "Merge multiple Garfield gas files into one");
    merge->add_option("-g,--gas,-o,--output", gasFilenameOutput, "Garfield gas file (.gas) to save output into")->required();
    vector<fs::path> mergeGasInputFilenames;
    merge->add_option("-i,--input", mergeGasInputFilenames, "Garfield gas file (.gas) to merge into the output. In case of overlaps, first file of list will take precedence")->required()->expected(2, numeric_limits<int>::max());
    merge->add_option("--dir,--output-dir,--output-directory", outputDirectory, "Directory to save merged gas file into")->expected(1);
    bool mergeVerbose = false;
    merge->add_flag("-v,--verbose", mergeVerbose, "Merge verbosity");

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
    if (subcommandName == "read") {
        cout << "Reading gas properties from file: " << gasFilenameInput << endl;
        Gas gas(gasFilenameInput);

        // if user specified electric field values, those values will be used, otherwise the gas file will be read for the electric field values
        if (!readGasElectricFieldValues.empty()) {
            sort(readGasElectricFieldValues.begin(), readGasElectricFieldValues.end());
        }
        const auto gasProperties = gas.GetGasPropertiesJson(readGasElectricFieldValues);

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
        vector<string> gasComponentNames;
        vector<double> gasComponentFractions;

        regex rgx("^[0-9]+([.][0-9]+)?"); // only positive (decimal) numbers
        for (const auto& componentString: generateGasComponentsString) {
            std::smatch matches;
            if (std::regex_search(componentString, matches, rgx)) {
                gasComponentFractions.push_back(stod(componentString));
            } else {
                gasComponentNames.push_back(componentString);
            }
        }

        // validation is handled in constructor
        Gas gas(gasComponentNames, gasComponentFractions);

        gas.SetPressure(pressure);
        gas.SetTemperature(temperature);

        // electric field
        auto& eField = generateGasElectricFieldValues;
        if (!generateGasElectricFieldLinearOptions.empty()) {
            const vector<double> electricFieldLinear = tools::linspace<double>(generateGasElectricFieldLinearOptions[0], generateGasElectricFieldLinearOptions[1], static_cast<unsigned int>(generateGasElectricFieldLinearOptions[2]));
            eField.insert(eField.end(), electricFieldLinear.begin(), electricFieldLinear.end());
        }
        if (!generateGasElectricFieldLogOptions.empty()) {
            const vector<double> electricFieldLog = tools::logspace<double>(generateGasElectricFieldLogOptions[0], generateGasElectricFieldLogOptions[1], static_cast<unsigned int>(generateGasElectricFieldLogOptions[2]));
            eField.insert(eField.end(), electricFieldLog.begin(), electricFieldLog.end());
        }
        if (eField.empty()) {
            cerr << "No electric field values provided (--help)" << endl;
            return 1;
        }

        tools::removeSimilarElements(eField, tools::getDefaultToleranceForRemoval(eField));

        cout << "Electric field values (V/cm):";
        for (const auto& e: eField) {
            cout << " " << e;
        }
        cout << endl;

        if (gasFilenameOutput.empty()) {
            string name = gas.GetName();
            name += "-T" + tools::numberToCleanNumberString(temperature) + "C";
            name += "-P" + tools::numberToCleanNumberString(pressure) + "bar";
            name += "-nColl" + to_string(numberOfCollisions);
            name += "-E" + (eField.size() == 1 ? tools::numberToCleanNumberString(eField.front()) : tools::numberToCleanNumberString(eField.front()) + "t" + tools::numberToCleanNumberString(eField.back())) + "Vcm";
            name += "-nE" + to_string(eField.size());
            if (!generateGasElectricFieldLinearOptions.empty()) {
                unsigned int nLin = generateGasElectricFieldLinearOptions[2];
                if (nLin == eField.size()) {
                    name += "lin";
                } else {
                    name += "lin" + to_string(nLin);
                }
            }
            if (!generateGasElectricFieldLogOptions.empty()) {
                unsigned int nLog = generateGasElectricFieldLogOptions[2];
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
                cout << "Warning: Gas file '" << filename << "' is empty and will be ignored" << endl;
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

        for (unsigned int i = 0; i < mergeGasInputFilenames.size() - 1; i++) {
            auto gas = i == 0 ? Gas(mergeGasInputFilenames[0]) : Gas(gasFilenameOutput);

            const auto& toMerge = mergeGasInputFilenames[i + 1];
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

            gas.Write(gasFilenameOutput);
        }

        if (mergeVerbose) {
            const auto values = Gas(gasFilenameOutput).GetTableElectricField();
            cout << "Electric field values (V/cm) for final merge file (" << values.size() << "):";
            for (const auto& value: values) {
                cout << " " << value;
            }
            cout << endl;
        }

        cout << "Gas file saved to " << gasFilenameOutput << endl;
    }
}
