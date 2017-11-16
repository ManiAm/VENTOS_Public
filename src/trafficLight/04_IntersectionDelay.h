/****************************************************************************/
/// @file    IntersectionDelay.h
/// @author  Mani Amoozadeh <maniam@ucdavis.edu>
/// @date    Jul 2015
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

#ifndef INTERSECTIONDELAY_H
#define INTERSECTIONDELAY_H

#include "trafficLight/03_IntersectionDemand.h"

namespace VENTOS {

struct delayEntry_t
{
    std::string TLid;
    std::string vehType;
    bool onIncomingLane;
    std::string lastLane;
    double intersectionEntrance;
    bool crossed;
    double crossedTime;
    double oldSpeed;
    double stoppingDelayThreshold;
    double startDeccel;
    double startStopping;
    double startAccel;
    double endDelay;

    // delay components
    double decelDelay;
    double waitingDelay;
    double accelDelay;
    double totalDelay;

    // temporary buffers for storing last speed/accel/signal
    boost::circular_buffer<std::pair<double,double>> lastSpeeds;
    boost::circular_buffer<std::pair<double,double>> lastSpeeds2;
    boost::circular_buffer<std::pair<double,double>> lastAccels;
    boost::circular_buffer<char> lastSignals;
};


class IntersectionDelay : public IntersectionDemand
{
private:
    typedef IntersectionDemand super;

    bool record_intersectionDelay_stat;
    double speedThreshold_veh = 0;
    double speedThreshold_bike = 0;
    double deccelDelayThreshold = 0;

    omnetpp::simsignal_t Signal_arrived_vehs;

    // list of all traffic lights in the network
    std::vector<std::string> TLList;

    // all incoming lanes in all traffic lights
    std::unordered_map<std::string /*lane*/, std::string /*TLid*/> incomingLanes;

    // vehicle delay for each intersection that it crosses
    std::map<std::string /*vehID*/, std::vector<delayEntry_t *>> vehDelay_perTL;

public:
    virtual ~IntersectionDelay();
    virtual void initialize(int);
    virtual void finish();
    virtual void handleMessage(omnetpp::cMessage *);
    virtual void receiveSignal(omnetpp::cComponent *, omnetpp::simsignal_t, const char *, cObject *);

    delayEntry_t* vehicleGetDelay(std::string, std::string);

protected:
    void virtual initialize_withTraCI();
    void virtual executeEachTimeStep();

private:
    void vehiclesDelay();
    void vehiclesDelayStart(std::string, delayEntry_t*);
    void vehiclesDelayDuration(std::string, delayEntry_t*);
    delayEntry_t* generateEmptyDelay(std::string, std::string);
    void vehiclesDelayToFile();
};

}

#endif
