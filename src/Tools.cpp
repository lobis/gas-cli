
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

    bool similar(double a, double b, double eps) {
        // use the same definition of similar as Garfield
        const double difference_abs = abs(a - b);
        const double sum_abs = abs(a) + abs(b);
        return difference_abs <= max(eps * sum_abs, 1E-20);
    }

    void removeSimilarElements(vector<double>& values, double eps) {
        sort(values.begin(), values.end());

        auto areSimilar = [&eps](double a, double b) {
            return similar(a, b, eps);
        };

        values.erase(unique(values.begin(), values.end(), areSimilar), values.end());

        sort(values.begin(), values.end());
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
