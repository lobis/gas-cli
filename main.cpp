
#include <iostream>

#include "Gas.h"

using namespace std;

int main() {
    double quencherFraction = 2.3;
    Gas gas({"Ar", "C4H10"}, {100 - quencherFraction, quencherFraction});

    cout << gas.GetGasPropertiesJson() << endl;
}