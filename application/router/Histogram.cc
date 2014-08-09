#include "Histogram.h"

namespace VENTOS {

Histogram::Histogram()
{
    average = 0;
}

multiset<int>::iterator Histogram::insert(int d)
{
    average = ((average * data.size()) + d)/ (data.size() + 1);
    return data.insert(d);
}

double Histogram::percentBelow(int d)//exclusive
{
    int count = 0;
    for(multiset<int>::iterator it = data.begin(); it != data.end() && (*it) < d; it++)
    {
        count++;
    }
    return (double)count / (double)data.size();
}

double Histogram::percentAbove(int d)//inclusive
{
    return 1.0 - percentBelow(d);
}

double Histogram::percentBetween(int low, int high)//includes low, doesn't include high
{
    if(low == high)
        return percentAbove(low) - percentAbove(high + 1);
    return percentAbove(low) - percentAbove(high);
}

double Histogram::percentAt(int d)
{
    int count = 0;

    //cout << endl;
    //for(multiset<int>::iterator it = data.begin(); it != data.end(); it++)
    //    cout << *it << " ";
    //cout << endl;

    for(multiset<int>::iterator it = data.begin(); it != data.end() && (*it) <= d; it++)
    {
        if(*it == d)
            count++;
    }
    double ret = (double)count / (double)data.size();
    //cout << "percentAt returning " << ret;
    return ret;
}

std::ostream& operator<<(ostream& os, const Histogram& h)
{
    os << h.data.size() << endl;
    for(multiset<int>::iterator it = h.data.begin(); it != h.data.end(); it++)
        os << *it << " ";
    return os;
}

}
