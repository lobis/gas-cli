
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
    Gas(std::vector<std::pair<std::string, double>> components);

    std::string GetName() const;
    std::string GetGarfieldName() const;

    std::pair<std::vector<std::string>, std::vector<double>> GetComponents() const;

    /// Temperature in Celsius
    double GetTemperature() const;
    /// Pressure in bar
    double GetPressure() const;
    /// Electric field in V/cm
    std::vector<double> GetTableElectricField() const;
    inline std::vector<double> GetElectricFieldValues() const { return GetTableElectricField(); }

    /// Drift velocity in cm/us
    double GetElectronDriftVelocity(double electricField) const;

    /// Diffusion in cm^(1/2) (more details in https://root-forum.cern.ch/t/unit-of-diffusion-coefficients-not-clear/45671)
    std::pair<double, double> GetElectronDiffusion(double electricField) const;
    double GetElectronTransversalDiffusion(double electricField) const;
    double GetElectronLongitudinalDiffusion(double electricField) const;

    /// Townsend coefficient (cm-1)
    double GetElectronTownsend(double electricField) const;
    /// Attachment coefficient (cm-1)
    double GetElectronAttachment(double electricField) const;

    void SetPressure(double pressureInBar);
    void SetTemperature(double temperatureInCelsius);

    void Generate(std::vector<double> electricFieldValues, unsigned int numberOfCollisions = 10, bool verbose = false);
    void Write(const std::string& filename) const;
    bool Merge(const std::string& gasFile, bool replaceOld = false);

    nlohmann::json GetGasPropertiesJson(const std::vector<double>& electricField = {}) const;
};
