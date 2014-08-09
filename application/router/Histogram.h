#ifndef HISTOGRAM_H
    #define HISTOGRAM_H

#include <vector>
#include <set>
#include <iostream>

using namespace std;

namespace VENTOS {

class Histogram
{
protected:

public:
    multiset<int> data;
    double average;
    multiset<int>::iterator insert(int d);
    double percentBelow(int d);
    double percentAbove(int d);
    double percentBetween(int low, int high);
    double percentAt(int d);
    friend ostream& operator<<(ostream& os, const Histogram& h);

    Histogram();
};
}

#endif
