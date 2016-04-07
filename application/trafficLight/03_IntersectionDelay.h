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

#include "02_LoopDetectors.h"

namespace VENTOS {

class delayEntry
{
public:
    std::string vehType;
    std::string TLid;
    std::string lastLane;
    double intersectionEntrance;
    bool crossedIntersection;
    double crossedTime;
    double oldSpeed;
    double startDeccel;
    double startStopping;
    double startAccel;
    double endDelay;
    double accumDelay;

    // temporary buffers for storing last speed/accel/signal
    boost::circular_buffer<std::pair<double,double>> lastSpeeds;
    boost::circular_buffer<std::pair<double,double>> lastSpeeds2;
    boost::circular_buffer<std::pair<double,double>> lastAccels;
    boost::circular_buffer<char> lastSignals;

    delayEntry(std::string str0, std::string str1, std::string str2, double d0, bool b1, double d1,
            double d2, double d3, double d4, double d5, double d6, double d7,
            boost::circular_buffer<std::pair<double,double>> CB_speed,
            boost::circular_buffer<std::pair<double,double>> CB_speed2,
            boost::circular_buffer<std::pair<double,double>> CB_accel,
            boost::circular_buffer<char> CB_signal)
    {
        this->vehType = str0;
        this->TLid = str1;
        this->lastLane = str2;
        this->intersectionEntrance = d0;
        this->crossedIntersection = b1;
        this->crossedTime = d1;
        this->oldSpeed = d2;
        this->startDeccel = d3;
        this->startStopping = d4;
        this->startAccel = d5;
        this->endDelay = d6;
        this->accumDelay = d7;

        this->lastSpeeds = CB_speed;
        this->lastSpeeds2 = CB_speed2;
        this->lastAccels = CB_accel;
        this->lastSignals = CB_signal;
    }
};


class IntersectionDelay : public LoopDetectors
{
public:
    virtual ~IntersectionDelay();
    virtual void initialize(int);
    virtual void finish();
    virtual void handleMessage(cMessage *);

protected:
    void virtual executeFirstTimeStep();
    void virtual executeEachTimeStep();

private:
    void vehiclesDelay();
    void vehiclesDelayEach(std::string);
    void vehiclesAccuDelay(std::string, std::map<std::string, delayEntry>::iterator);
    void vehiclesDelayToFile();

protected:
    bool collectVehDelay;
    double deccelDelayThreshold;
    double vehStoppingDelayThreshold;
    double bikeStoppingDelayThreshold;
    int lastValueBuffSize;

    // class variables
    std::list<std::string> lanesList;  // list of all lanes in the network

    std::map<std::string /*vehID*/, delayEntry> vehDelay;   // vehicle intersection delay
    std::map<std::string /*lane*/, std::map<std::string /*vehID*/, double /*accum delay of vehID*/>> laneDelay;  // accumulated delay of each vehicle on each incoming lane
    std::map<std::pair<std::string /*TLid*/, int /*link*/>, std::map<std::string /*vehID*/, double /*accum delay of vehID*/>> linkDelay;  // accumulated delay of each vehicle for each link
};

}

#endif
