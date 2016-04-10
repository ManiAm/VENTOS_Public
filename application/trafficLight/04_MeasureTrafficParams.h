/****************************************************************************/
/// @file    MeasureTrafficParams.h
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

#ifndef MEASURETRAFFICPARAMS_H
#define MEASURETRAFFICPARAMS_H

#include <03_TrafficLights.h>
#include <boost/circular_buffer.hpp>

namespace VENTOS {

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


class MeasureTrafficParams : public TrafficLights
{
public:
    virtual ~MeasureTrafficParams();
    virtual void initialize(int);
    virtual void finish();
    virtual void handleMessage(cMessage *);

protected:
    void virtual executeFirstTimeStep();
    void virtual executeEachTimeStep();
    void updateTrafficDemand();

private:
    void CheckDetectors();
    void measureTrafficParameters();
    void saveTLQueueingData();

protected:
    // loop detector ids used for measuring incoming traffic demand
    std::unordered_map<std::string /*lane*/, std::pair<std::string /*LD id*/, double /*last actuation*/>> LD_demand;
    // loop detector ids used for actuated-time signal control
    std::unordered_map<std::string /*lane*/, std::string /*LD id*/> LD_actuated;
    // area detector ids used for measuring queue length
    std::unordered_map<std::string /*lane*/, std::string /*AD id*/> AD_queue;

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

protected:
    bool measureIntersectionQueue;
    bool measureTrafficDemand;
    int measureTrafficDemandMode;
    int trafficDemandBuffSize;

    double saturationTD;

private:
    // NED variables
    bool collectTLPhasingData;
    bool collectTLQueuingData;

    std::vector<queueDataEntryDetailed> Vec_queueSize;
};

}

#endif
