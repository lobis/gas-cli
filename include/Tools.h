
#pragma once

#include <algorithm>
#include <cmath>
#include <iostream>
#include <regex>

namespace tools {
    template<typename T>
    std::vector<T> linspace(T start, T end, unsigned int points) {
        std::vector<T> result(points);
        float step = (end - start) / (points - 1);
        unsigned int i = 0;
        for (auto& value: result) {
            value = start + step * T(i++);
        }
        return result;
    }

    template<typename T>
    std::vector<T> logspace(T start, T end, unsigned int points) {
        std::vector<T> result = linspace<T>(log10(start), log10(end), points);
        std::transform(result.begin(), result.end(),
                       result.begin(),
                       [](double x) { return pow(10, x); });
        return result;
    }

    std::string cleanNumberString(const std::string& s) {
        // "2.3000" -> "2p3", "1.0000" -> "1"
        std::string result(s);
        result = std::regex_replace(result, std::regex("[0]+$"), "");   // remove trailing zeros
        result = std::regex_replace(result, std::regex("[\\.]+$"), ""); // remove trailing '.' (if zeros have been removed)
        result = std::regex_replace(result, std::regex("\\."), "p");    // replace '.' by 'p'
        return result;
    }

    std::string numberToCleanNumberString(double d) {
        return cleanNumberString(std::to_string(round(d * 1000) / 1000));
    }

    void sortVectorForCompute(std::vector<double>& values) {
        std::sort(values.begin(), values.end());
        values.erase(std::unique(values.begin(), values.end()), values.end());

        std::vector<double> toProcess(values);
        values.clear();

        while (!toProcess.empty()) {
            unsigned int indexToRemove = 0;
            double minDistanceMax = 0;
            unsigned int minDistanceRepeatMax = 0;
            for (unsigned int i = 0; i < toProcess.size(); i++) {
                double minDistance = std::numeric_limits<double>::max();
                unsigned int minDistanceRepeat = 0;
                for (const auto& valueProcessed: values) {
                    double diff = std::abs(toProcess[i] - valueProcessed);
                    if (diff < minDistance) {
                        minDistance = diff;
                    }
                    if (diff == minDistance) {
                        minDistanceRepeat++;
                    }
                }
                if (minDistance > minDistanceMax || (minDistance == minDistanceMax && minDistanceRepeat > minDistanceRepeatMax)) {
                    minDistanceMax = minDistance;
                    minDistanceRepeatMax = minDistanceRepeat;
                    indexToRemove = i;
                }
            }

            values.push_back(toProcess[indexToRemove]);
            toProcess.erase(toProcess.begin() + indexToRemove);
        }
    }

    void removeSimilarElements(std::vector<double>& values, double absoluteTolerance = 0.05) {
        std::sort(values.begin(), values.end());

        auto similar = [&absoluteTolerance](double a, double b) {
            return std::abs(a - b) <= absoluteTolerance;
        };

        values.erase(std::unique(values.begin(), values.end(), similar), values.end());

        std::sort(values.begin(), values.end());
    }

    double getDefaultToleranceForRemoval(const std::vector<double>& values) {
        double min = *std::min_element(values.begin(), values.end());
        double max = *std::max_element(values.begin(), values.end());
        return (max - min) / 20000;
    }

} // namespace tools
