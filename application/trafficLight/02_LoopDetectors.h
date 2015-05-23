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

#include <01_TL_Base.h>
#include <Appl.h>
#include "TraCI_Extend.h"

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

    friend bool operator== (const LoopDetectorData &v1, const LoopDetectorData &v2)
    {
        return ( v1.detectorName == v2.detectorName && v1.vehicleName == v2.vehicleName );
    }
};


class IntersectionQueueData
{
  public:
    double time;
    std::string TLid;
    std::string lane;
    int qSize;

    IntersectionQueueData( double t, std::string str1, std::string str2, int q )
    {
        this->time = t;
        this->TLid = str1;
        this->lane = str2;
        this->qSize = q;
    }
};


class LoopDetectors : public TrafficLightBase
{
  public:
    virtual ~LoopDetectors();
    virtual void finish();
    virtual void initialize(int);
    virtual void handleMessage(cMessage *);

  protected:
    void virtual executeFirstTimeStep();
    void virtual executeEachTimeStep(bool);

  private:
    void getAllDetectors();
    void collectLDsData();
    void saveLDsData();
    void measureTD();
    void measureQ();
    void saveQueueData();

  protected:
    bool collectInductionLoopData;
    bool measureIntersectionQueue;
    bool measureTrafficDemand;

    std::map<std::string, std::string> LD_demand;       // ids of loop detectors for measuring incoming traffic demand
    std::map<std::string, std::string> LD_actuated;     // ids of loop detectors for actuated-time signal control
    std::map<std::string, std::string> AD_queue;        // ids of area detectors for measuring queue length

    std::map<std::string /*lane*/, std::pair<std::string /*TLid*/,int /*queue size*/>> laneQueueSize;
    std::map<std::pair<std::string /*TLid*/,int /*link*/>, int /*queue size*/> linkQueueSize;

    std::vector<LoopDetectorData> Vec_loopDetectors;
    std::vector<IntersectionQueueData> Vec_queueSize;

  private:
    bool freeze = false;
    double lastDetectionT_old = 0;
    double total = 0;
    int passedVeh = 0;
};

}

#endif
