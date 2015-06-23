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


class currentStatusTL
{
public:
    std::string allowedMovements;
    double greenStart;
    double yellowStart;
    double redStart;
    double phaseEnd;
    int incommingLanes;
    int totalQueueSize;

    currentStatusTL(std::string str1, double d1, double d2, double d3, double d4, int i1, int i2)
    {
        this->allowedMovements = str1;
        this->greenStart = d1;
        this->yellowStart = d2;
        this->redStart = d3;
        this->phaseEnd = d4;
        this->incommingLanes = i1;
        this->totalQueueSize = i2;
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

    void saveTLData();

  protected:
    // NED variables
    bool collectInductionLoopData;
    bool measureTrafficDemand;
    bool measureIntersectionQueue;
    bool collectTLData;

    std::list<std::string> TLList;   // list of traffic-lights in the network

    std::map<std::string /*lane*/, std::string /*id*/> LD_demand;      // ids of loop detectors for measuring incoming traffic demand
    std::map<std::string /*lane*/, std::string /*id*/> LD_actuated;    // ids of loop detectors for actuated-time signal control
    std::map<std::string /*lane*/, std::string /*id*/> AD_queue;       // ids of area detectors for measuring queue length

    std::map<std::string /*lane*/, std::string /*TLid*/> lanesTL;                                  // all incoming lanes belong to each intersection
    std::map<std::string /*lane*/, int /*# of links*/> linksCount;                                 // number of outgoing links for each lane
    std::map<std::pair<std::string /*TLid*/,int /*link number*/>, std::string /*link*/> linksTL;   // all links belong to each intersection

    std::map<std::string /*lane*/, std::pair<std::string /*TLid*/,int /*queue size*/>> laneQueueSize;   // real-time queue size for each incoming lane of each intersection
    std::map<std::pair<std::string /*TLid*/,int /*link*/>, int /*queue size*/> linkQueueSize;           // real-time queue size for each link of each intersection

    std::map<std::string /*TLid*/, int /*phase number*/> phaseTL;                                  // current phase in each TL
    std::map<std::pair<std::string /*TLid*/, int /*phase number*/>, currentStatusTL> statusTL;     // current status of each TL in each phase

    std::vector<LoopDetectorData> Vec_loopDetectors;

  private:
    bool freeze = false;
    double lastDetectionT_old = 0;
    double total = 0;
    int passedVeh = 0;
};

}

#endif
