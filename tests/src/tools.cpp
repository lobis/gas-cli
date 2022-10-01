
#include <filesystem>
#include <gtest/gtest.h>

#include "Gas.h"
#include "Tools.h"

namespace fs = std::filesystem;

using namespace std;

TEST(Tools, linspace) {
    auto result = tools::linspace<double>(0.0, 1.0, 5);
    ASSERT_EQ(result.size(), 5);
    ASSERT_DOUBLE_EQ(result[0], 0);
    ASSERT_DOUBLE_EQ(result[1], 0.25);
    ASSERT_DOUBLE_EQ(result[2], 0.5);
    ASSERT_DOUBLE_EQ(result[3], 0.75);
    ASSERT_DOUBLE_EQ(result[4], 1);
}

TEST(Tools, logspace) {
    auto result = tools::logspace<double>(1.0, 100.0, 5);
    ASSERT_EQ(result.size(), 5);
    ASSERT_DOUBLE_EQ(result[0], 1);
    ASSERT_DOUBLE_EQ(result[1], 3.1622776601683795);
    ASSERT_DOUBLE_EQ(result[2], 10);
    ASSERT_DOUBLE_EQ(result[3], 31.622776601683793);
    ASSERT_DOUBLE_EQ(result[4], 100);
}
