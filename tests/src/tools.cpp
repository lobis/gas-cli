
#include <filesystem>
#include <gtest/gtest.h>

#include "Tools.h"

namespace fs = std::filesystem;

using namespace std;
using namespace tools;

TEST(Tools, linspace) {
    auto result = linspace<double>(0.0, 1.0, 5);
    ASSERT_EQ(result.size(), 5);
    ASSERT_DOUBLE_EQ(result[0], 0);
    ASSERT_DOUBLE_EQ(result[1], 0.25);
    ASSERT_DOUBLE_EQ(result[2], 0.5);
    ASSERT_DOUBLE_EQ(result[3], 0.75);
    ASSERT_DOUBLE_EQ(result[4], 1);
}

TEST(Tools, logspace) {
    auto result = logspace<double>(1.0, 100.0, 5);
    ASSERT_EQ(result.size(), 5);
    ASSERT_DOUBLE_EQ(result[0], 1);
    ASSERT_DOUBLE_EQ(result[1], 3.1622776601683795);
    ASSERT_DOUBLE_EQ(result[2], 10);
    ASSERT_DOUBLE_EQ(result[3], 31.622776601683793);
    ASSERT_DOUBLE_EQ(result[4], 100);
}

TEST(Tools, sortVectorForCompute) {
    vector<double> values = {0, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
    sortVectorForCompute(values);
    ASSERT_EQ(values, vector<double>({0, 100, 50, 70, 30, 60, 40, 80, 90, 20, 10}));
}

TEST(Tools, removeSimilarElements) {
    vector<double> values = {0, 10, 10, 20, 20, 30, 20, 30};
    removeSimilarElements(values);
    ASSERT_EQ(values, vector<double>({0, 10, 20, 30}));
}

TEST(Tools, removeSimilarElementsTolerance) {
    // Using this number of starting values and default tolerance we get 200 points
    auto values = logspace<double>(0.1, 1000.0, 107);
    for (const auto& value: linspace<double>(0, 1000, 110)) {
        values.push_back(value);
    }
    std::sort(values.begin(), values.end());

    const double tolerance = getDefaultToleranceForRemoval(values);

    ASSERT_NEAR(tolerance, 0.05, 0.0001);

    ASSERT_EQ(values.size(), 217);

    // We should only remove last element (1000.0) since it appears in both sequences
    removeSimilarElements(values);

    ASSERT_EQ(values.size(), 200);

    ASSERT_NEAR(values.back(), 1000.0, 0.0005);
    ASSERT_NEAR(values.front(), 0.0, 0.0005);
}
