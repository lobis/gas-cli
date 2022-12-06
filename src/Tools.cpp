
#include "Tools.h"

#include <fstream>

using namespace std;

namespace tools {
    string cleanNumberString(const string& s) {
        // "2.3000" -> "2.3", "1.0000" -> "1"
        string result(s);
        result = regex_replace(result, regex("[0]+$"), "");   // remove trailing zeros
        result = regex_replace(result, regex("[\\.]+$"), ""); // remove trailing '.' (if zeros have been removed)
        return result;
    }

    string numberToCleanNumberString(double d) {
        return cleanNumberString(to_string(round(d * 1000) / 1000));
    }

    void sortVectorForCompute(vector<double>& values) {
        sort(values.begin(), values.end());
        values.erase(unique(values.begin(), values.end()), values.end());

        vector<double> toProcess(values);
        values.clear();

        while (!toProcess.empty()) {
            unsigned int indexToRemove = 0;
            double minDistanceMax = 0;
            unsigned int minDistanceRepeatMax = 0;
            for (unsigned int i = 0; i < toProcess.size(); i++) {
                double minDistance = numeric_limits<double>::max();
                unsigned int minDistanceRepeat = 0;
                for (const auto& valueProcessed: values) {
                    double diff = abs(toProcess[i] - valueProcessed);
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

    void removeSimilarElements(vector<double>& values, double absoluteTolerance) {
        sort(values.begin(), values.end());

        auto similar = [&absoluteTolerance](double a, double b) {
            return abs(a - b) <= absoluteTolerance;
        };

        values.erase(unique(values.begin(), values.end(), similar), values.end());

        sort(values.begin(), values.end());
    }

    double getDefaultToleranceForRemoval(const vector<double>& values) {
        double min = *min_element(values.begin(), values.end());
        double max = *max_element(values.begin(), values.end());
        return (max - min) / 20000;
    }

    void writeToFile(const string& filename, const string& content) {
        ofstream file(filename);
        file << content;
        file.close();
    }

    void tar(const std::string& filename, const std::vector<std::string>& files) {
        string command = "tar -czf " + filename;
        for (const auto& file: files) {
            command += " " + file;
        }
        const int result = system(command.c_str());
    }

} // namespace tools
