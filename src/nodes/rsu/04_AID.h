/****************************************************************************/
/// @file    AID.h
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

#ifndef APPLRSUAID_H_
#define APPLRSUAID_H_

#include <eigen3/Eigen/Dense>
#include "nodes/rsu/03_Classify.h"

namespace VENTOS {

class ApplRSUAID : public ApplRSUCLASSIFY
{
private:
    typedef ApplRSUCLASSIFY super;

    bool enableAID;
    bool printIncidentDetection;

    static Eigen::MatrixXi tableCount;
    static Eigen::MatrixXd tableProb;

public:
    ~ApplRSUAID();
    virtual void initialize(int stage);
    virtual void finish();
    virtual void handleSelfMsg(omnetpp::cMessage* msg);

protected:
    void virtual executeEachTimeStep();

    virtual void onBeaconVehicle(BeaconVehicle*);
    virtual void onBeaconBicycle(BeaconBicycle*);
    virtual void onBeaconPedestrian(BeaconPedestrian*);
    virtual void onBeaconRSU(BeaconRSU*);
    virtual void onLanechange(LaneChangeMsg*);

private:
    void incidentDetectionToFile();
};

}

#endif
