/****************************************************************************/
/// @file    AddEntity.h
/// @author  Mani Amoozadeh <maniam@ucdavis.edu>
/// @author  second author name
/// @date    August 2013
///
/****************************************************************************/
// VENTOS, Vehicular Network Open Simulator; see http:?
// Copyright (C) 2013-2015
/****************************************************************************/
//
// This file is part of VENTOS.
// VENTOS is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#ifndef ADDENTITY
#define ADDENTITY

#include "BaseApplLayer.h"
#include "TraCI_Commands.h"

// un-defining ev!
// why? http://stackoverflow.com/questions/24103469/cant-include-the-boost-filesystem-header
#undef ev
#include "boost/filesystem.hpp"
#define ev  (*cSimulation::getActiveEnvir())

namespace VENTOS {

class AddEntity : public BaseApplLayer
{
public:
    virtual ~AddEntity();
    virtual void initialize(int stage);
    virtual void handleMessage(cMessage *msg);
    virtual void finish();
    virtual void receiveSignal(cComponent *, simsignal_t, long);

private:
    void Add();

    void Scenario1();
    void Scenario5();
    void Scenario6();
    void Scenario7();
    void Scenario8();
    void Scenario9();
    void Scenario10();
    void Scenario11();
    void Scenario12();

private:
    // NED variables
    TraCI_Commands *TraCI;  // pointer to the TraCI module
    double terminate;

    bool on;
    int mode;
    int totalVehicles;
    std::string vehiclesType;
    int distribution;
    int interval;
    double lambda;

    int plnSize;
    double plnSpace;

    double overlap;
    bool vehMultiClass;
    bool bike;

    // class variables
    simsignal_t Signal_executeFirstTS;
    boost::filesystem::path SUMO_FullPath;

    std::vector<double> vehRouteDistribution;
    std::vector<double> vehClassDistribution;
    std::vector<double> bikeRouteDistribution;
};

}

#endif
