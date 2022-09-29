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

    double GetTemperature() const;
    double GetPressure() const;

    double GetElectronDriftVelocity(double electricField) const;
    std::pair<double, double> GetElectronDiffusion(double electricField) const;
    double GetElectronTransversalDiffusion(double electricField) const;
    double GetElectronLongitudinalDiffusion(double electricField) const;
    double GetElectronTownsend(double electricField) const;
    double GetElectronAttachment(double electricField) const;

    void SetPressure(double pressureInBar);
    void SetTemperature(double temperatureInCelsius);

    void Generate();
    void Write(const std::string& filename) const;
    std::string GetDriftTable() const;
};
