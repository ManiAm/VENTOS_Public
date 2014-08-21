#ifndef HISTOGRAM_H
    #define HISTOGRAM_H

#include <vector>
#include <map>
#include <iostream>

using namespace std;

namespace VENTOS {

class Histogram
{
public:
    map<int, int> data;
    int count;
    double average;
    int minimum;

    void insert(int d);
    double percentAt(int d);
    friend ostream& operator<<(ostream& os, Histogram& h);

    Histogram();
};
}

#endif
