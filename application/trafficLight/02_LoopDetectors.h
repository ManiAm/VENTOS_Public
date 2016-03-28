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
#include <boost/circular_buffer.hpp>
#include <unordered_map>

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


class queueDataEntry
{
public:
    int totalQueueSize;
    int maxQueueSize;
    int totalLanes;

    queueDataEntry(int i1, int i2, int i3, int i4)
    {
        this->totalQueueSize = i1;
        this->maxQueueSize = i2;
        this->totalLanes = i4;
    }
};


// identical to queueDataEntry with time and TLid
class queueDataEntryDetailed
{
public:
    double time;
    std::string TLid;
    int totalQueueSize;
    int maxQueueSize;
    int totalLanes;

    queueDataEntryDetailed(double d1, std::string str1, int i1, int i2, int i3)
    {
        this->time = d1;
        this->TLid = str1;
        this->totalQueueSize = i1;
        this->maxQueueSize = i2;
        this->totalLanes = i3;
    }
};


class currentStatusTL
{
public:
    int cycle;
    std::string allowedMovements;
    double greenLength;
    double greenStart;
    double yellowStart;
    double redStart;
    double phaseEnd;
    int incommingLanes;
    int totalQueueSize;

    currentStatusTL(int i0, std::string str1, double d0, double d1, double d2, double d3, double d4, int i1, int i2)
    {
        this->cycle = i0;
        this->allowedMovements = str1;
        this->greenLength = d0;
        this->greenStart = d1;
        this->yellowStart = d2;
        this->redStart = d3;
        this->phaseEnd = d4;
        this->incommingLanes = i1;
        this->totalQueueSize = i2;
    }
};


class laneVehInfo
{
public:
    double firstArrivalTime;
    double lastArrivalTime;
    int totalVehCount;

    laneVehInfo(double d1, double d2, double d3)
    {
        this->firstArrivalTime = d1;
        this->lastArrivalTime = d2;
        this->totalVehCount = d3;
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
    void virtual executeEachTimeStep();

    void updateTLstate(std::string, std::string, std::string = "", bool = false);

  private:
    void getAllDetectors();

    void collectLDsData();
    void saveLDsData();

    void measureTrafficParameters();
    void updateTrafficDemand();
    void saveTLQueueingData();
    void saveTLPhasingData();

  protected:
    // NED variables
    double minGreenTime;
    double maxGreenTime;
    double yellowTime;
    double redTime;
    double maxCycleLength;

    // NED variables
    bool measureIntersectionQueue;
    bool measureTrafficDemand;
    int measureTrafficDemandMode;
    int trafficDemandBuffSize;

    bool collectInductionLoopData;
    bool collectTLQueuingData;
    bool collectTLPhasingData;

    double saturationTD;

    // list of all traffic lights in the network
    std::list<std::string> TLList;


    // loop detector ids used for measuring incoming traffic demand
    std::unordered_map<std::string /*lane*/, std::pair<std::string /*LD id*/, double /*last actuation*/>> LD_demand;
    // loop detector ids used for actuated-time signal control
    std::unordered_map<std::string /*lane*/, std::string /*LD id*/> LD_actuated;
    // area detector ids used for measuring queue length
    std::unordered_map<std::string /*lane*/, std::string /*AD id*/> AD_queue;


    // list of all 'incoming lanes' in each TL
    std::unordered_map< std::string /*TLid*/, std::pair<int /*lane count*/, std::list<std::string>> > laneListTL;
    // list of all 'bike lanes' in each TL
    std::unordered_map< std::string /*TLid*/, std::list<std::string> > bikeLaneListTL;
    // list of all 'side walks' in each TL
    std::unordered_map< std::string /*TLid*/, std::list<std::string> > sideWalkListTL;


    // all incoming lanes in all traffic lights
    std::unordered_map<std::string /*lane*/, std::string /*TLid*/> allIncomingLanes;
    // all outgoing link # for each incoming lane
    std::multimap<std::string /*lane*/, std::pair<std::string /*TLid*/, int /*link number*/>> outgoingLinks;
    // the corresponding lane for each outgoing link #
    std::map<std::pair<std::string /*TLid*/, int /*link*/>, std::string /*lane*/> linkToLane;


    // real-time queue size for each incoming lane for each intersection
    std::unordered_map<std::string /*lane*/, std::pair<std::string /*TLid*/, int /*queue size*/>> laneQueueSize;
    // real-time queue size for each link in each intersection
    std::map<std::pair<std::string /*TLid*/, int /*link*/>, int /*queue size*/> linkQueueSize;
    // real-time queue size data for each TLid
    std::unordered_map<std::string /*TLid*/, queueDataEntry> queueSizeTL;


    // real-time traffic demand for each incoming lane for each intersection
    std::unordered_map<std::string /*lane*/, std::pair<std::string /*TLid*/, boost::circular_buffer<std::vector<double>> /*TD*/>> laneTD;
    // real-time traffic demand for each link in each intersection
    std::map<std::pair<std::string /*TLid*/, int /*link*/>, boost::circular_buffer<std::vector<double>> /*TD*/> linkTD;
    std::unordered_map<std::string /*lane*/, std::pair<std::string /*TLid*/, laneVehInfo> > laneTotalVehCount;


    // current phase in each TL
    std::unordered_map<std::string /*TLid*/, int /*phase number*/> phaseTL;
    // current status of each TL in each phase
    std::map<std::pair<std::string /*TLid*/, int /*phase number*/>, currentStatusTL> statusTL;

  private:
    std::vector<LoopDetectorData> Vec_loopDetectors;
    std::vector<queueDataEntryDetailed> Vec_queueSize;
};

}

#endif
