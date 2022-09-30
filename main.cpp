
#include <iostream>
#include <regex>

#include "CLI/App.hpp"
#include "CLI/Config.hpp"
#include "CLI/Formatter.hpp"

#include "Gas.h"

using namespace std;

int main(int argc, char** argv) {

    CLI::App app{"Gas CLI"};

    string gasFilenameInput;
    string gasFilenameOutput;

    CLI::App* read = app.add_subcommand("read", "Read from a gas file to read properties such as drift velocity of diffusion coefficients");
    read->add_option("-g,--gas,-i,--input", gasFilenameInput, "Garfield gas file (.gas) read from")->required();

    CLI::App* generate = app.add_subcommand("generate", "Generate a Garfield gas file using the configuration");
    generate->add_option("-g,--gas,-o,--output", gasFilenameOutput, "Garfield gas file (.gas) to save output into")->required();
    vector<string> generateGasComponentsString;
    generate->add_option("--components", generateGasComponentsString, "Garfield gas components to use in the gas file. It should be of the for of 'component1', 'fraction1', 'component2', 'fraction2', ... up to 6 components")->required();
    double pressure = 1.0, temperature = 20.0;
    generate->add_option("--pressure", pressure, "Gas pressure in bar");
    generate->add_option("--temperature", temperature, "Gas temperature in C");

    CLI::App* merge = app.add_subcommand("merge", "Merge multiple Garfield gas files into one");
    merge->add_option("-g,--gas,-o,--output", gasFilenameOutput, "Garfield gas file (.gas) to save output into")->required();
    vector<string> mergeGasInputFilenames;
    merge->add_option("-i,--input", mergeGasInputFilenames, "Garfield gas file (.gas) to merge into the output")->required();

    app.require_subcommand(1);

    CLI11_PARSE(app, argc, argv);

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

    } else if (subcommandName == "merge") {
        // TODO
    }

    /*
        double quencherFraction = 2.3;
        Gas gas({"Ar", "C4H10"}, {100 - quencherFraction, quencherFraction});
        gas.SetPressure(1.4);

        // gas.Generate();
        // gas.Write("/tmp/mt.gas");

        gas = Gas("/root/long.gas");

        cout << gas.GetGasPropertiesJson() << endl;

    */
}