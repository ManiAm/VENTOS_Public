#ifndef HYPERTREE_H
#define HYPERTREE_H

#include <map>
#include <string>

using namespace std;

namespace VENTOS {

class Hypertree
{
public:
    map<string, double> label;
    map<string, string> transition;

    Hypertree();

};

}

#endif
