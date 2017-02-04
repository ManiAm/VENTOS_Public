/****************************************************************************/
/// @file    IntersectionDemand.h
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

#ifndef INTERSECTIONDEMAND_H
#define INTERSECTIONDEMAND_H

#include "trafficLight/02_IntersectionQueue.h"
#include <boost/circular_buffer.hpp>

namespace VENTOS {

class IntersectionDemand : public IntersectionQueue
{
protected:
    // real-time traffic demand for each incoming lane for each intersection
    std::unordered_map<std::string /*lane*/, std::pair<std::string /*TLid*/, boost::circular_buffer<std::vector<double>> /*TD*/>> TD_perLane;

    // real-time traffic demand for each link in each intersection
    std::map<std::pair<std::string /*TLid*/, int /*link*/>, boost::circular_buffer<std::vector<double>> /*TD*/> TD_perLink;

    double saturationTD;

private:
    typedef IntersectionQueue super;

    // NED variables
    bool record_trafficDemand_stat;
    int trafficDemandMode;
    int trafficDemandBuffSize;

    // list of all traffic lights in the network
    std::vector<std::string> TLList;

    // list of all 'side walks' in each TL
    std::unordered_map< std::string /*TLid*/, std::vector<std::string> > sideWalks_perTL;

    // all incoming lanes in all traffic lights
    std::unordered_map<std::string /*lane*/, std::string /*TLid*/> incomingLanes;

    // all outgoing link # for each incoming lane
    std::multimap<std::string /*lane*/, std::pair<std::string /*TLid*/, int /*link number*/>> outgoingLinks_perLane;

    // loop detector ids used for measuring incoming traffic demand
    std::unordered_map<std::string /*lane*/, std::pair<std::string /*LD id*/, double /*last actuation*/>> LD_demand;

    typedef struct laneVehInfo
    {
        double firstArrivalTime;
        double lastArrivalTime;
        int totalVehCount;
    } laneVehInfo_t;

    std::unordered_map<std::string /*lane*/, std::pair<std::string /*TLid*/, laneVehInfo_t> > laneTotalVehCount;

public:
    virtual ~IntersectionDemand();
    virtual void initialize(int);
    virtual void finish();
    virtual void handleMessage(omnetpp::cMessage *);

protected:
    void virtual initialize_withTraCI();
    void virtual executeEachTimeStep();
    void updateTrafficDemand();

private:
    void initVariables();
    void checkLoopDetectors();
    void measureTrafficDemand();
    void saveTLQueueingData();
};

}

#endif
