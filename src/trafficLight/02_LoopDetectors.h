/****************************************************************************/
/// @file    LoopDetectors.h
/// @author  Mani Amoozadeh <maniam@ucdavis.edu>
/// @author
/// @date    April 2015
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

#ifndef LOOPDETECTORS_H
#define LOOPDETECTORS_H

#include "trafficLight/01_Base.h"

namespace VENTOS {

class LoopDetectorData
{
public:
    std::string detectorName;
    std::string lane;
    std::string vehicleName;
    double entryTime;
    double leaveTime;
    double entrySpeed;
    double leaveSpeed;

    LoopDetectorData( std::string str1, std::string str2, std::string str3, double entryT=-1, double leaveT=-1, double entryS=-1, double leaveS=-1 )
    {
        this->detectorName = str1;
        this->lane = str2;
        this->vehicleName = str3;
        this->entryTime = entryT;
        this->leaveTime = leaveT;
        this->entrySpeed = entryS;
        this->leaveSpeed = leaveS;
    }

    friend bool operator== (const LoopDetectorData &v1, const LoopDetectorData &v2) {
        return ( v1.detectorName == v2.detectorName && v1.vehicleName == v2.vehicleName );
    }
};


class LoopDetectors : public TrafficLightBase
{
private:
    typedef TrafficLightBase super;

    bool collectInductionLoopData;
    std::vector<std::string> AllLDs;
    std::vector<LoopDetectorData> Vec_loopDetectors;

public:
    virtual ~LoopDetectors();
    virtual void initialize(int);
    virtual void finish();
    virtual void handleMessage(omnetpp::cMessage *);

protected:
    void virtual initialize_withTraCI();
    void virtual executeEachTimeStep();

private:
    void collectLDsData();
    void saveLDsData();
};

}

#endif
