
#include <filesystem>
#include <gtest/gtest.h>

#include "Gas.h"

namespace fs = std::filesystem;

using namespace std;

TEST(Gas, defaultValues) {
    Gas gas;
    EXPECT_DOUBLE_EQ(gas.GetTemperature(), 20.0);
    EXPECT_DOUBLE_EQ(gas.GetPressure(), 1.01324999984); // 1 atm
}
