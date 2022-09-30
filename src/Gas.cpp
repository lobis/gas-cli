//
// Created by lobis on 28/09/2022.
//

#include "Gas.h"

#include "Tools.h"

#include <algorithm>
#include <iostream>
#include <nlohmann/json.hpp>
#include <regex>
#include <string>
#include <thread>
#include <vector>

using namespace std;
using namespace Garfield;

namespace fs = std::filesystem;

Gas::Gas() : gas(make_unique<MediumMagboltz>()) {}

Gas::Gas(const string& gasFilepath) : gas(make_unique<MediumMagboltz>()) { gas->LoadGasFile(gasFilepath); }

Gas::Gas(const vector<string>& _components,
         const vector<double>& _fractions) {
    vector<std::string> components = _components;
    vector<double> fractions = _fractions;

    if (components.empty()) {
        cerr << "cannot initialize gas with empty components" << endl;
        exit(1);
    }
    if (!components.empty() && fractions.size() == components.size() - 1) {
        double sum = 0;
        for (const auto& value: fractions) { sum += value; }
        // fractions can be numbers between 0 and 1 or %
        if (sum > 1.0) {
            fractions.emplace_back(100.0 - sum);
        } else {
            fractions.emplace_back(1.0 - sum);
        }
    }
    if (components.size() != fractions.size()) {
        cerr << "number of component names and fractions mismatch" << endl;
        exit(1);
    }

    // TODO: Throw when negative number as fraction, remove elements when fraction is zero
    const unsigned int componentsLimit = 6;
    if (components.size() > componentsLimit) {
        cerr << "cannot initialize gas more than 6 components" << endl;
        exit(1);
    }

    double fractionSum = 0;
    for (const auto& value: fractions) { fractionSum += value; }
    // normalize so the sum is 1
    for (auto& value: fractions) { value /= fractionSum; }

    while (components.size() < componentsLimit) {
        components.emplace_back("");
        fractions.emplace_back(0);
    }

    gas = make_unique<MediumMagboltz>(components[0], fractions[0], components[1], fractions[1], components[2], fractions[2], components[3], fractions[3], components[4], fractions[4], components[5], fractions[5]);
}

void Gas::Generate(vector<double> electricFieldValues, unsigned int numberOfCollisions, bool verbose) {
    sort(electricFieldValues.begin(), electricFieldValues.end());

    // TODO: remove very close E field values
    gas->SetFieldGrid(electricFieldValues, {0.0}, {HalfPi});

    gas->GenerateGasTable(int(numberOfCollisions), verbose);
}

void Gas::Write(const string& filename) const {
    gas->WriteGasFile(filename);
}

std::string Gas::GetName() const {
    // Ar/iC4H10 (97.7/2.3) -> Ar2p3iC4H10
    const auto components = GetComponents();

    string name;
    name += components.first[0];
    for (unsigned int i = 1; i < components.first.size(); i++) {
        string fractionString = to_string(components.second[i] * 100);
        name += tools::cleanNumberString(fractionString) + components.first[i];
    }
    return name;
}

std::string Gas::GetGarfieldName() const {
    return gas->GetName();
}

std::pair<std::vector<std::string>, std::vector<double>> Gas::GetComponents() const {

    std::vector<std::pair<std::string, double>> components(gas->GetNumberOfComponents());
    for (unsigned int i = 0; i < components.size(); i++) {
        gas->GetComponent(i, components[i].first, components[i].second);
    }

    // sort them in descending order of gas fractions
    sort(components.begin(), components.end(), [](auto& left, auto& right) {
        return left.second > right.second;
    });

    std::vector<std::string> names(gas->GetNumberOfComponents());
    std::vector<double> fractions(gas->GetNumberOfComponents());
    for (unsigned int i = 0; i < components.size(); i++) {
        names[i] = components[i].first;
        fractions[i] = components[i].second;
    }

    return {names, fractions};
}

constexpr double torrToBar = 0.001333223684;
double Gas::GetPressure() const {
    return gas->GetPressure() * torrToBar;
}

void Gas::SetPressure(double pressureInBar) {
    gas->SetPressure(pressureInBar / torrToBar);
}

double Gas::GetTemperature() const {
    return gas->GetTemperature() - ZeroCelsius;
}

void Gas::SetTemperature(double temperatureInCelsius) {
    gas->SetTemperature(temperatureInCelsius + ZeroCelsius);
}

double Gas::GetElectronDriftVelocity(double electricField) const {
    double vx, vy, vz;
    gas->ElectronVelocity(0, 0, -electricField, 0, 0, 0, vx, vy, vz);
    return vz;
}

pair<double, double> Gas::GetElectronDiffusion(double electricField) const {
    double longitudinal, transversal;
    gas->ElectronDiffusion(0, 0, -electricField, 0, 0, 0, longitudinal, transversal);
    return {longitudinal, transversal};
}

double Gas::GetElectronTransversalDiffusion(double electricField) const {
    return GetElectronDiffusion(electricField).second;
}

double Gas::GetElectronLongitudinalDiffusion(double electricField) const {
    return GetElectronDiffusion(electricField).first;
}

double Gas::GetElectronTownsend(double electricField) const {
    double townsend;
    gas->ElectronTownsend(0, 0, -electricField, 0, 0, 0, townsend);
    return townsend;
}

double Gas::GetElectronAttachment(double electricField) const {
    double attachment;
    gas->ElectronAttachment(0, 0, -electricField, 0, 0, 0, attachment);
    return attachment;
}

std::vector<double> Gas::GetTableElectricField() const {
    vector<double> electricField, magneticField, angle;
    gas->GetFieldGrid(electricField, magneticField, angle);
    // sort electric field in case it's not ordered
    sort(electricField.begin(), electricField.end());
    return electricField;
}

std::string Gas::GetGasPropertiesJson() const {
    nlohmann::json j;

    j["name"] = GetName();
    j["temperature"] = GetTemperature();
    j["pressure"] = GetPressure();

    const auto components = GetComponents();
    j["components"]["labels"] = components.first;
    j["components"]["fractions"] = components.second;

    const auto electricField = GetTableElectricField();
    j["electric_field"] = electricField;

    vector<double> electronDriftVelocity(electricField.size());
    std::transform(electricField.begin(), electricField.end(),
                   electronDriftVelocity.begin(),
                   [this](double e) { return GetElectronDriftVelocity(e); });
    j["electron_drift_velocity"] = electronDriftVelocity;

    vector<double> electronTransversalDiffusion(electricField.size());
    std::transform(electricField.begin(), electricField.end(),
                   electronTransversalDiffusion.begin(),
                   [this](double e) { return GetElectronTransversalDiffusion(e); });
    j["electron_transversal_diffusion"] = electronTransversalDiffusion;

    vector<double> electronLongitudinalDiffusion(electricField.size());
    std::transform(electricField.begin(), electricField.end(),
                   electronLongitudinalDiffusion.begin(),
                   [this](double e) { return GetElectronLongitudinalDiffusion(e); });
    j["electron_longitudinal_diffusion"] = electronLongitudinalDiffusion;

    /*
    vector<double> electronTownsend(electricField.size());
    std::transform(electricField.begin(), electricField.end(),
                   electronTownsend.begin(),
                   [this](double e) { return GetElectronTownsend(e); });
    j["electron_townsend"] = electronTownsend;

    vector<double> electronAttachment(electricField.size());
    std::transform(electricField.begin(), electricField.end(),
                   electronAttachment.begin(),
                   [this](double e) { return GetElectronAttachment(e); });
    j["electron_attachment"] = electronAttachment;
    */

    return j.dump(4);
}

bool Gas::Merge(const string& gasFile, bool replaceOld) {
    return gas->MergeGasFile(gasFile, replaceOld);
}

/*
 * Not working for some reason...
void Gas::GenerateMT(unsigned int nThreads) {
    const auto electricFieldLinear = linspace<double>(0, 1000, 2);
    // const auto electricFieldLog = logspace<double>(1, 2000, 2);
    vector<double> electricField(electricFieldLinear);
    // electricField.insert(electricField.end(), electricFieldLog.begin(), electricFieldLog.end());
    sort(electricField.begin(), electricField.end());

gas->SetFieldGrid(electricField, {0.0}, {HalfPi});

// split electric field into chunks for each thread to process
std::vector<std::vector<double>> electricFieldChunks(nThreads);
for (unsigned int i = 0; i < electricField.size(); i++) {
    electricFieldChunks[i % nThreads].emplace_back(electricField[i]);
}
electricFieldChunks.erase(std::remove_if(electricFieldChunks.begin(), electricFieldChunks.end(), [](const vector<double>& v) { return v.empty(); }), electricFieldChunks.end());

std::vector<thread> threads;
std::mutex gasMutex;
for (unsigned int i = 0; i < nThreads; i++) {
    const auto& electricFieldChunk = electricFieldChunks[i];
    if (electricFieldChunk.empty()) {
        // if more threads than E field values exists some will be empty
        continue;
    }
    threads.emplace_back([this, &gasMutex](unsigned int threadId, const vector<double>& electricFieldChunk) {
        // copy gas since copy constructor is deleted
        gasMutex.lock();
        basic_string<char> gas1, gas2, gas3, gas4, gas5, gas6;
        double f1, f2, f3, f4, f5, f6;
        gas->GetComposition(gas1, f1, gas2, f2, gas3, f3, gas4, f4, gas5, f5, gas6, f6);
        MediumMagboltz gasThread(gas1, f1, gas2, f2, gas3, f3, gas4, f4, gas5, f5, gas6, f6);
        gasThread.SetPressure(gas->GetPressure());
        gasThread.SetTemperature(gas->GetTemperature());

        gasThread.SetFieldGrid(electricFieldChunk, {0.0}, {HalfPi});

        gasMutex.unlock();

        std::string filename = string(std::tmpnam(nullptr)) + "-" + to_string(threadId) + ".gas";
        this_thread::sleep_for(1.0s);

        const int numCollisions = 10;
        gasThread.GenerateGasTable(numCollisions, true); // This is the only expensive operation
        gasThread.WriteGasFile(filename);

        gasMutex.lock();

        cout << "thread: " << threadId << " " << filename << endl;
        for (const auto& value: electricFieldChunk) {
            cout << "\t- " << value << endl;
        }

        gas->MergeGasFile(filename, true);
        fs::remove(filename);

        gasMutex.unlock();
    },
                         i, electricFieldChunk);
}

for (auto& t: threads) {
    t.join();
}
}
*/