
#pragma once

#include <memory>
#include <string>
#include <vector>

#include "Garfield/MediumMagboltz.hh"
#include "nlohmann/json.hpp"

class Gas {

protected:
    std::unique_ptr<Garfield::MediumMagboltz> gas;

public:
    Gas();
    Gas(const std::string& gasFilepath);
    Gas(const std::vector<std::string>& components,
        const std::vector<double>& fractions);

    std::string GetName() const;
    std::string GetGarfieldName() const;

    std::pair<std::vector<std::string>, std::vector<double>> GetComponents() const;
    double GetTemperature() const;
    double GetPressure() const;
    std::vector<double> GetTableElectricField() const;
    inline std::vector<double> GetElectricFieldValues() const { return GetTableElectricField(); }

    double GetElectronDriftVelocity(double electricField) const;
    std::pair<double, double> GetElectronDiffusion(double electricField) const;
    double GetElectronTransversalDiffusion(double electricField) const;
    double GetElectronLongitudinalDiffusion(double electricField) const;
    double GetElectronTownsend(double electricField) const;
    double GetElectronAttachment(double electricField) const;

    void SetPressure(double pressureInBar);
    void SetTemperature(double temperatureInCelsius);

    void Generate(std::vector<double> electricFieldValues, unsigned int numberOfCollisions = 10, bool verbose = false);
    void Write(const std::string& filename) const;
    bool Merge(const std::string& gasFile, bool replaceOld = false);

    nlohmann::json GetGasPropertiesJson(const std::vector<double>& electricField = {}) const;
};
