/****************************************************************************/
/// @file    Manager.h
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

#ifndef ApplVMANAGER_H
#define ApplVMANAGER_H

#include "nodes/vehicle/yourCode.h"

namespace VENTOS {

class ApplVManager : public ApplVYourCode
{
public:
    bool jamming = false;

private:
    typedef ApplVYourCode super;

    // NED variables (packet loss ratio)
    double dropStartTime;
    double plr;

    carFollowingModel_t carFollowingModelId;

    bool record_beacon_stat;

    long BeaconVehCount = 0;
    long BeaconVehDropped = 0;
    long BeaconBikeCount = 0;
    long BeaconPedCount = 0;
    long BeaconRSUCount = 0;
    long PlatoonCount = 0;

public:
    ~ApplVManager();
    virtual void initialize(int stage);
    virtual void finish();
    virtual void receiveSignal(omnetpp::cComponent* source, omnetpp::simsignal_t signalID, omnetpp::cObject* obj, cObject* details);

protected:
    virtual void handleLowerMsg(omnetpp::cMessage*);
    virtual void handleSelfMsg(omnetpp::cMessage*);
    virtual void handlePositionUpdate(omnetpp::cObject*);

    virtual void onBeaconVehicle(BeaconVehicle*);

    void onMessageType(omnetpp::cMessage*);
    bool dropBeacon(double time, double plr);

    /**
     * Returns the amount of CO2 emissions in grams/second, calculated for an average Car
     * @param v speed in m/s
     * @param a acceleration in m/s^2
     * @returns emission in g/s
     */
    double calculateCO2emission(double v, double a);
};

}

#endif
