
#pragma once

#include <algorithm>
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
        std::vector<T> result = linspace(log10(start), log10(end), points);
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
        return cleanNumberString(std::to_string(floor(d * 1000) / 1000));
    }

} // namespace tools
