
#include <iostream>
#include <regex>

#include "CLI/App.hpp"
#include "CLI/Config.hpp"
#include "CLI/Formatter.hpp"

#include "Gas.h"
#include "Tools.h"

using namespace std;

int main(int argc, char** argv) {

    CLI::App app{"Gas CLI"};

    string gasFilenameInput;
    string gasFilenameOutput;

    CLI::App* read = app.add_subcommand("read", "Read from a gas file properties such as drift velocity of diffusion coefficients and generate a JSON file with the results");
    read->add_option("-g,--gas,-i,--input", gasFilenameInput, "Garfield gas file (.gas) read from")->required();
    string gasReadOutputJsonFilepath;
    read->add_option("-o,--output,--json", gasReadOutputJsonFilepath, "Location to save gas properties as json file");

    CLI::App* generate = app.add_subcommand("generate", "Generate a Garfield gas file using command line parameters");
    generate->add_option("-g,--gas,-o,--output", gasFilenameOutput, "Garfield gas file (.gas) to save output into")->required();
    vector<string> generateGasComponentsString;
    generate->add_option("--components", generateGasComponentsString, "Garfield gas components to use in the gas file. It should be of the form of 'component1', 'fraction1', 'component2', 'fraction2', ... up to 6 components")->required()->expected(1, 12);
    double pressure = 1.0, temperature = 20.0;
    generate->add_option("--pressure", pressure, "Gas pressure in bar");
    generate->add_option("--temperature", temperature, "Gas temperature in Celsius");
    unsigned int numberOfCollisions = 10;
    generate->add_option("--collisions", numberOfCollisions, "Number of collisions to simulate (defaults to 10)");
    bool generateVerbose = false;
    generate->add_flag("--verbose", generateVerbose, "Garfield verbosity");
    vector<double> generateGasElectricFieldValues;
    generate->add_option("--electric-field,--field,--efield,-E", generateGasElectricFieldValues, "Gas electric field values in V/cm");
    vector<double> generateGasElectricFieldLinearOptions;
    generate->add_option("--electric-field-linear,--field-lin,--efield-lin,--E-lin", generateGasElectricFieldLinearOptions, "Use linearly spaced electric field values (start, end, number)")->expected(3);
    vector<double> generateGasElectricFieldLogOptions;
    generate->add_option("--electric-field-log,--field-log,--efield-log,--E-log", generateGasElectricFieldLogOptions, "Use logarithmically spaced electric field values (start, end, number)")->expected(3);

    CLI::App* merge = app.add_subcommand("merge", "Merge multiple Garfield gas files into one");
    merge->add_option("-g,--gas,-o,--output", gasFilenameOutput, "Garfield gas file (.gas) to save output into")->required();
    vector<string> mergeGasInputFilenames;
    merge->add_option("-i,--input", mergeGasInputFilenames, "Garfield gas file (.gas) to merge into the output")->required();

    app.require_subcommand(1);

    CLI11_PARSE(app, argc, argv);

    if (!gasFilenameOutput.empty()) {
        // absolute path
        gasFilenameOutput = std::filesystem::weakly_canonical(gasFilenameOutput);
    }

    const auto subcommand = app.get_subcommands().back();

    const string subcommandName = subcommand->get_name();
    if (subcommandName == "read") {
        Gas gas(gasFilenameInput);
        cout << gas.GetGasPropertiesJson() << endl;
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
        sort(eField.begin(), eField.end());

        gas.Generate(eField, numberOfCollisions, generateVerbose);

        gas.Write(gasFilenameOutput);

        cout << "gas file saved to '" << gasFilenameOutput << "'" << endl;

    } else if (subcommandName == "merge") {
        // TODO
    }
}