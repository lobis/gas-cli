#pragma once

#include "Garfield/FundamentalConstants.hh"
#include "Garfield/MediumMagboltz.hh"

#include <memory>

class Gas {
protected:
    std::unique_ptr<Garfield::MediumMagboltz> gas;

public:
    Gas();
    Gas(const std::string& gasFilepath);
    Gas(const std::vector<std::string>& components,
        const std::vector<double>& fractions);

    std::string GetName() const;
    std::pair<std::vector<std::string>, std::vector<double>> GetComponents() const;
    double GetTemperature() const;
    double GetPressure() const;
    std::vector<double> GetTableElectricField() const;

    double GetElectronDriftVelocity(double electricField) const;
    std::pair<double, double> GetElectronDiffusion(double electricField) const;
    double GetElectronTransversalDiffusion(double electricField) const;
    double GetElectronLongitudinalDiffusion(double electricField) const;
    double GetElectronTownsend(double electricField) const;
    double GetElectronAttachment(double electricField) const;

    void SetPressure(double pressureInBar);
    void SetTemperature(double temperatureInCelsius);

    void Generate(std::vector<double> electricFieldValues, unsigned int numberOfCollisions = 10);
    void Write(const std::string& filename) const;

    std::string GetGasPropertiesJson() const;
};

namespace tools {
    template<typename T>
    std::vector<T> linspace(T start, T end, unsigned int points);

    template<typename T>
    std::vector<T> logspace(T start, T end, unsigned int points);
} // namespace tools