
#pragma once

#include <algorithm>

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
} // namespace tools
