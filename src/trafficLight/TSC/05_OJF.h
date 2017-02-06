/****************************************************************************/
/// @file    OJF.h
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

#ifndef TRAFFICLIGHTOJF_H
#define TRAFFICLIGHTOJF_H

#include "trafficLight/TSC/04_LQF_NoStarv.h"

namespace VENTOS {

class TrafficLightOJF : public TrafficLightLQF_NoStarv
{
private:
    typedef TrafficLightLQF_NoStarv super;

    std::string currentInterval;
    double intervalDuration;
    std::string nextGreenInterval;

    double nextGreenTime;
    omnetpp::cMessage* intervalChangeEVT = NULL;

    std::map<std::string /*TLid*/, std::string /*first green interval*/> firstGreen;

    std::map<std::pair<std::string /*TLid*/, int /*link*/>, std::string /*lane*/> link2Lane;

    // list of all 'incoming lanes' in each TL
    std::unordered_map< std::string /*TLid*/, std::vector<std::string> > incomingLanes_perTL;

    std::vector< std::vector<int> > allMovements;

    typedef struct sortedEntry
    {
        int oneCount;
        int maxVehCount;
        double totalDelay;
        std::vector<int> batchMovements;
    } sortedEntry_t;

    typedef struct sortFunc
    {
        bool operator()(sortedEntry_t p1, sortedEntry_t p2)
        {
            if( p1.totalDelay < p2.totalDelay )
                return true;
            else if( p1.totalDelay == p2.totalDelay && p1.oneCount < p2.oneCount)
                return true;
            else
                return false;
        }
    } sortFunc_t;

    // batch of all non-conflicting movements, sorted by total vehicle delay per batch
    typedef std::priority_queue< sortedEntry_t /*type of each element*/, std::vector<sortedEntry_t> /*container*/, sortFunc_t > priorityQ;

public:
    virtual ~TrafficLightOJF();
    virtual void initialize(int);
    virtual void finish();
    virtual void handleMessage(omnetpp::cMessage *);

protected:
    void virtual initialize_withTraCI();
    void virtual executeEachTimeStep();

private:
    void chooseNextInterval(std::string);
    void chooseNextGreenInterval(std::string);
};

}

#endif
