/****************************************************************************/
/// @file    VehDelay.h
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

#ifndef VEHDELAY_H
#define VEHDELAY_H

#include "02_LoopDetectors.h"


namespace VENTOS {

class delayEntry
{
public:
    std::string TLid;
    std::string lastLane;
    double startDeccel;
    double startStopping;
    double startAccel;
    double endDelay;

    delayEntry(std::string str1, std::string str2, double d1, double d2, double d3, double d4)
    {
        this->TLid = str1;
        this->lastLane = str2;
        this->startDeccel = d1;
        this->startStopping = d2;
        this->startAccel = d3;
        this->endDelay = d4;
    }
};


class VehDelay : public LoopDetectors
{
public:
    virtual ~VehDelay();
    virtual void finish();
    virtual void initialize(int);
    virtual void handleMessage(cMessage *);

protected:
    void virtual executeFirstTimeStep();
    void virtual executeEachTimeStep(bool);

private:
    void vehiclesDelay();
    void vehiclesDelayEach(std::string);
    void vehiclesDelayToFile();

protected:
    bool measureVehDelay;
    std::list<std::string> TLList;      // list of traffic-lights in the network
    std::list<std::string> lanesList;   // list of all lanes in the network
    std::map<std::string, delayEntry> delay;
};

}

#endif
