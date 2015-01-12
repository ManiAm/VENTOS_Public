#ifndef VEHICLE_H
#define VEHICLE_H

#include <iostream>

using namespace std;

namespace VENTOS {

class Vehicle
{
public:
    string id;
    string type;
    string origin;
    string destination;
    double depart;

    Vehicle(string id, string type, string origin, string destination, double depart);
};

}

#endif
