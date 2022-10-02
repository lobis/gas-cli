
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
