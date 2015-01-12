#include "Histogram.h"

namespace VENTOS {

Histogram::Histogram()
{
    average = 0;
    count = 0;
    minimum = 100000;
}

void Histogram::insert(int d, int LaneCostsMode)
{
    if(LaneCostsMode == 1)
    {
    average = ((average * data.size()) + d)/ (data.size() + 1);
    count++;
    if(d < minimum)
        minimum = d;

    if(data.find(d) == data.end())
        data[d] = 1;
    else
        data[d]++;
    }
    else if(LaneCostsMode == 2)
    {
        average = average * 0.8 + (double)d * .2;
    }
}

double Histogram::percentAt(int d)
{
    return (double)data[d] / (double)count;
}

std::ostream& operator<<(ostream& os, Histogram& h)
{
    os << h.data.size() << endl;
    for(map<int, int>::iterator it = h.data.begin(); it != h.data.end(); it++)
        os << it->first << " ";
    return os;
}

}
