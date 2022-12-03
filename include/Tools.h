
#pragma once

#include <algorithm>
#include <cmath>
#include <iostream>
#include <regex>

namespace tools {
    template<typename T>
    inline std::vector<T> linspace(T start, T end, unsigned int points) {
        std::vector<T> result(points);
        float step = (end - start) / (points - 1);
        unsigned int i = 0;
        for (auto& value: result) {
            value = start + step * T(i++);
        }
        return result;
    }

    template<typename T>
    inline std::vector<T> logspace(T start, T end, unsigned int points) {
        std::vector<T> result = linspace<T>(log10(start), log10(end), points);
        std::transform(result.begin(), result.end(),
                       result.begin(),
                       [](double x) { return pow(10, x); });
        return result;
    }

    std::string cleanNumberString(const std::string& s);

    std::string numberToCleanNumberString(double d);

    void sortVectorForCompute(std::vector<double>& values);

    void removeSimilarElements(std::vector<double>& values, double absoluteTolerance = 0.05);

    double getDefaultToleranceForRemoval(const std::vector<double>& values);

    void writeToFile(const std::string& filename, const std::string& content);
} // namespace tools
