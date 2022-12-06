
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

    /// Remove trailing zeros and trailing '.' from a string representation of a number ("2.3000" -> "2.3", "1.0000" -> "1", etc.)
    std::string cleanNumberString(const std::string& s);

    std::string numberToCleanNumberString(double d);

    void sortVectorForCompute(std::vector<double>& values);

    void removeSimilarElements(std::vector<double>& values, double eps = 1E-3);

    bool similar(double a, double b, double eps = 1E-3);

    /// Write contents (string) to file
    void writeToFile(const std::string& filename, const std::string& content);

    /// Compress files using tar (tar needs to be available to work)
    void tar(const std::string& filename, const std::vector<std::string>& files);
} // namespace tools
