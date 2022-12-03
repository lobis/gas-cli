
#include "Gas.h"

#include "Tools.h"

#include "Garfield/FundamentalConstants.hh"
#include "nlohmann/json.hpp"

#include <algorithm>
#include <iostream>
#include <string>
#include <thread>

using namespace std;
using namespace Garfield;

Gas::Gas() : gas(make_unique<MediumMagboltz>()) {}

Gas::Gas(const string& gasFilepath) : gas(make_unique<MediumMagboltz>()) {
    if (!gas->LoadGasFile(gasFilepath)) {
        cerr << "gas file not found: " << gasFilepath << endl;
        exit(1);
    }
}

Gas::Gas(std::vector<std::pair<std::string, double>> components) {

    if (components.empty()) {
        cerr << "Error: cannot initialize gas with empty components" << endl;
        exit(1);
    }

    constexpr unsigned int componentsLimit = 6;
    if (components.size() > componentsLimit) {
        cerr << "Error: cannot initialize gas more than 6 components" << endl;
        exit(1);
    }

    // sort components by fraction
    sort(components.begin(), components.end(), [](const auto& a, const auto& b) { return a.second > b.second; });

    double fractionSum = 0;
    for (const auto& [name, fraction]: components) { fractionSum += fraction; }
    // normalize by total fraction
    for (auto& [name, fraction]: components) { fraction /= fractionSum; }

    // add empty components to match size of componentsLimit
    for (int i = components.size(); i < componentsLimit; i++) {
        components.emplace_back("", 0);
    }

    gas = make_unique<MediumMagboltz>(components[0].first, components[0].second,
                                      components[1].first, components[1].second,
                                      components[2].first, components[2].second,
                                      components[3].first, components[3].second,
                                      components[4].first, components[4].second,
                                      components[5].first, components[5].second);
}

void Gas::Generate(vector<double> electricFieldValues, unsigned int numberOfCollisions, bool verbose) {
    sort(electricFieldValues.begin(), electricFieldValues.end());

    // TODO: remove very close E field values
    gas->SetFieldGrid(electricFieldValues, {0.0}, {HalfPi});

    gas->EnableThermalMotion();
    // gas->EnablePenningTransfer()

    gas->GenerateGasTable(int(numberOfCollisions), verbose);
}

void Gas::Write(const string& filename) const {
    gas->WriteGasFile(filename);
}

string Gas::GetName() const {
    // C2H6/CF4/Ne/H2O (9.99/9.99/79.92/0.1) -> Ne_79.92-C2H6_9.99-CF4_9.99-H2O_0.1
    const auto components = GetComponents();

    string name;
    for (unsigned int i = 0; i < components.first.size(); i++) {
        if (i > 0) {
            name += '-';
        }
        string fractionString = to_string(components.second[i] * 100);
        name += components.first[i] + '_' + tools::cleanNumberString(fractionString);
    }
    return name;
}

string Gas::GetGarfieldName() const {
    return gas->GetName();
}

pair<vector<string>, vector<double>> Gas::GetComponents() const {

    vector<pair<string, double>> components(gas->GetNumberOfComponents());
    for (unsigned int i = 0; i < components.size(); i++) {
        gas->GetComponent(i, components[i].first, components[i].second);
    }

    // sort them in descending order of gas fractions
    sort(components.begin(), components.end(), [](auto& left, auto& right) {
        if (left.second == right.second) {
            // sort by name (e.g. "Ar" < "Ne")
            return left.first < right.first;
        }
        return left.second > right.second;
    });

    vector<string> names(gas->GetNumberOfComponents());
    vector<double> fractions(gas->GetNumberOfComponents());
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
    // returns electron drift velocity in cm/us (garfield unit is cm/ns)
    double vx, vy, vz;
    gas->ElectronVelocity(0, 0, -electricField, 0, 0, 0, vx, vy, vz);
    return vz * 1.E3;
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

vector<double> Gas::GetTableElectricField() const {
    vector<double> electricField, magneticField, angle;
    gas->GetFieldGrid(electricField, magneticField, angle);
    // sort electric field in case it's not ordered
    sort(electricField.begin(), electricField.end());
    return electricField;
}

nlohmann::json Gas::GetGasPropertiesJson(const vector<double>& electricFieldMaybeEmpty) const {
    nlohmann::json j;

    j["name"] = GetName();
    j["temperature"] = GetTemperature();
    j["pressure"] = GetPressure();

    const auto components = GetComponents();
    j["components"]["labels"] = components.first;
    j["components"]["fractions"] = components.second;

    // if electricField is empty, use the table electric field
    const auto& electricField = electricFieldMaybeEmpty.empty() ? GetTableElectricField() : electricFieldMaybeEmpty;

    j["electric_field"] = electricField;

    vector<double> electronDriftVelocity(electricField.size());
    transform(electricField.begin(), electricField.end(),
              electronDriftVelocity.begin(),
              [this](double e) { return GetElectronDriftVelocity(e); });
    if (any_of(electronDriftVelocity.begin(), electronDriftVelocity.end(), [](double e) { return e != 0; })) {
        j["electron_drift_velocity"] = electronDriftVelocity;
    }

    vector<double> electronTransversalDiffusion(electricField.size());
    transform(electricField.begin(), electricField.end(),
              electronTransversalDiffusion.begin(),
              [this](double e) { return GetElectronTransversalDiffusion(e); });
    if (any_of(electronTransversalDiffusion.begin(), electronTransversalDiffusion.end(), [](double e) { return e != 0; })) {
        j["electron_transversal_diffusion"] = electronTransversalDiffusion;
    }

    vector<double> electronLongitudinalDiffusion(electricField.size());
    transform(electricField.begin(), electricField.end(),
              electronLongitudinalDiffusion.begin(),
              [this](double e) { return GetElectronLongitudinalDiffusion(e); });
    if (any_of(electronLongitudinalDiffusion.begin(), electronLongitudinalDiffusion.end(), [](double e) { return e != 0; })) {
        j["electron_longitudinal_diffusion"] = electronLongitudinalDiffusion;
    }

    vector<double> electronTownsend(electricField.size());
    transform(electricField.begin(), electricField.end(),
              electronTownsend.begin(),
              [this](double e) { return GetElectronTownsend(e); });
    if (any_of(electronTownsend.begin(), electronTownsend.end(), [](double e) { return e != 0; })) {
        j["electron_townsend"] = electronTownsend;
    }

    vector<double> electronAttachment(electricField.size());
    transform(electricField.begin(), electricField.end(),
              electronAttachment.begin(),
              [this](double e) { return GetElectronAttachment(e); });
    if (any_of(electronAttachment.begin(), electronAttachment.end(), [](double e) { return e != 0; })) {
        j["electron_attachment"] = electronAttachment;
    }

    return j;
}

bool Gas::Merge(const string& gasFile, bool replaceOld) {
    return gas->MergeGasFile(gasFile, replaceOld);
}
