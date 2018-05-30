/****************************************************************************/
/// @file    IntersectionDemand.cc
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

#include <iomanip>
#include "trafficLight/03_IntersectionDemand.h"

namespace VENTOS {

Define_Module(VENTOS::IntersectionDemand);


IntersectionDemand::~IntersectionDemand()
{

}


void IntersectionDemand::initialize(int stage)
{
    super::initialize(stage);

    if(stage == 0)
    {
        record_trafficDemand_stat = par("record_trafficDemand_stat").boolValue();
        trafficDemandMode = par("trafficDemandMode").intValue();
        trafficDemandBuffSize = par("trafficDemandBuffSize").intValue();
    }
}


void IntersectionDemand::finish()
{
    super::finish();
}


void IntersectionDemand::handleMessage(omnetpp::cMessage *msg)
{
    super::handleMessage(msg);
}


void IntersectionDemand::initialize_withTraCI()
{
    super::initialize_withTraCI();

    if(record_trafficDemand_stat)
    {
        TLList = TraCI->TLGetIDList();

        initVariables();
        checkLoopDetectors();

        // todo: change this later
        // saturation = (3*TD) / ( 1-(35/cycle) )
        // max TD = 1900, max cycle = 120
        saturationTD = 2000.;
    }
}


void IntersectionDemand::executeEachTimeStep()
{
    super::executeEachTimeStep();

    if(record_trafficDemand_stat)
        measureTrafficDemand();
}


void IntersectionDemand::initVariables()
{
    // for each traffic light
    for (auto &TLid : TLList)
    {
        // get all incoming lanes
        auto lan = TraCI->TLGetControlledLanes(TLid);

        // remove duplicate entries
        sort( lan.begin(), lan.end() );
        lan.erase( unique( lan.begin(), lan.end() ), lan.end() );

        std::vector<std::string> sideWalkList;

        // for each incoming lane
        for(auto &lane : lan)
        {
            incomingLanes[lane] = TLid;

            // store all bike lanes and side walks
            auto allowedClasses = TraCI->laneGetAllowedClasses(lane);
            if(allowedClasses.size() == 1 && allowedClasses.front() == "pedestrian")
                sideWalkList.push_back(lane);

            boost::circular_buffer<std::vector<double>> CB; // create a circular buffer
            CB.set_capacity(trafficDemandBuffSize);         // set max capacity
            CB.clear();
            TD_perLane[lane] = std::make_pair(TLid, CB);    // initialize laneTD

            // initialize laneTotalVehCount
            laneVehInfo_t entry = {};

            entry.firstArrivalTime = -1;
            entry.lastArrivalTime = -1;
            entry.totalVehCount = 0;

            laneTotalVehCount.insert( std::make_pair(lane, std::make_pair(TLid, entry)) );
        }

        sideWalks_perTL[TLid] = sideWalkList;

        // get all links controlled by this TL
        auto result = TraCI->TLGetControlledLinks(TLid);

        // for each link in this TLid
        for(auto &it : result)
        {
            int linkNumber = it.first;
            std::vector<std::string> link = it.second;
            std::string incommingLane = link[0];

            boost::circular_buffer<std::vector<double>> CB;   // create a circular buffer
            CB.set_capacity(trafficDemandBuffSize);           // set max capacity
            CB.clear();
            TD_perLink[std::make_pair(TLid,linkNumber)] = CB; // initialize linkTD

            outgoingLinks_perLane.insert( std::make_pair(incommingLane, std::make_pair(TLid,linkNumber)) );
        }
    }
}


void IntersectionDemand::checkLoopDetectors()
{
    // get all loop detectors
    auto str = TraCI->LDGetIDList();

    // for each loop detector
    for (auto &it : str)
    {
        std::string lane = TraCI->LDGetLaneID(it);

        if( std::string(it).find("demand_") != std::string::npos )
            LD_demand[lane] = std::make_pair(it,-1);
    }

    if(str.size() > 0)
        LOG_INFO << boost::format(">>> %1%/%2% loop detectors are used for measuring traffic demand. \n") % LD_demand.size() % str.size() << std::flush;

    // make sure we have all detectors we need
    for (auto &TLid : TLList)
    {
        auto lan = TraCI->TLGetControlledLanes(TLid);

        // remove duplicate entries
        sort( lan.begin(), lan.end() );
        lan.erase( unique( lan.begin(), lan.end() ), lan.end() );

        // for each incoming lane
        for(auto &lane : lan)
        {
            // ignore side walk
            if( std::find(sideWalks_perTL[TLid].begin(), sideWalks_perTL[TLid].end(), lane) != sideWalks_perTL[TLid].end() )
                continue;

            // if we are measuring traffic demand using loop detectors then make sure we have an LD on each lane
            if(LD_demand.find(lane) == LD_demand.end() )
                LOG_WARNING << boost::format("WARNING: no loop detector found on lane (%1%). No traffic demand measurement is available for this lane. \n") % lane;
        }
    }
}


void IntersectionDemand::measureTrafficDemand()
{
    // for each 'lane i' that is controlled by traffic light j
    for(auto &y : incomingLanes)
    {
        std::string lane = y.first;
        std::string TLid = y.second;

        // ignore side walk
        if( std::find(sideWalks_perTL[TLid].begin(), sideWalks_perTL[TLid].end(), lane) != sideWalks_perTL[TLid].end() )
            continue;

        // make sure we have a demand loop detector in this lane
        auto loc = LD_demand.find(lane);
        if( loc == LD_demand.end() )
            continue;

        std::string LDid = loc->second.first;
        double lastDetection_old = loc->second.second;   // lastDetection_old is one step behind lastDetection
        double lastDetection = TraCI->LDGetElapsedTimeLastDetection(LDid);

        // lastDetection == 0        if a vehicle is above the LD
        // lastDetection_old != 0    if this is the first detection for this vehicle (we ignore any subsequent detections for the same vehicle)
        if(lastDetection == 0 && lastDetection_old != 0)
        {
            double TD = 0;

            // traffic measurement is done by measuring headway time between each two consecutive vehicles
            if(trafficDemandMode == 1)
            {
                double diff = omnetpp::simTime().dbl() - lastDetection_old - updateInterval;

                // ignore the very first detection on this LD
                if(diff > 0.0001)
                {
                    // calculate the instantaneous traffic demand
                    TD = 3600. / lastDetection_old;

                    // bound TD
                    TD = std::min(TD, saturationTD);

                    // calculate lagT: when the measured TD will be effective?
                    // measured TD does not represent the condition in the intersection, and is effective after lagT
                    double LDPos = TraCI->laneGetLength(lane) - TraCI->LDGetPosition(LDid);  // get position of the LD from end of lane
                    double approachSpeed = TraCI->LDGetLastStepMeanVehicleSpeed(LDid);
                    double lagT = std::fabs(LDPos) / approachSpeed;

                    // push it into the circular buffer
                    auto loc = TD_perLane.find(lane);
                    std::vector<double> entry {TD /*traffic demand*/, omnetpp::simTime().dbl() /*time of measure*/, lagT /*time it takes to arrive at intersection*/};
                    (loc->second).second.push_back(entry);

                    // iterate over outgoing links
                    auto ppp = outgoingLinks_perLane.equal_range(lane);
                    for(std::multimap<std::string, std::pair<std::string, int>>::iterator z = ppp.first; z != ppp.second; ++z)
                    {
                        int linkNumber = (*z).second.second;

                        // push the new TD into the circular buffer
                        auto location = TD_perLink.find( make_pair(TLid,linkNumber) );
                        std::vector<double> entry {TD /*traffic demand*/, omnetpp::simTime().dbl() /*time of measure*/, lagT /*time it takes to arrive at intersection*/};
                        (location->second).push_back(entry);
                    }
                }
            }
            // traffic demand measurement is done by counting total # of passed vehicles in interval
            // Note that updating laneTD and laneLinks is done at the beginning of each cycle at updateTLstate method
            else if(trafficDemandMode == 2)
            {
                auto u = laneTotalVehCount.find(lane);

                if(u == laneTotalVehCount.end())
                    throw omnetpp::cRuntimeError("lane %s does not exist in laneTotalVehCount", lane.c_str());

                (*u).second.second.totalVehCount = (*u).second.second.totalVehCount + 1;

                // if this is the first vehicle on this lane
                if((*u).second.second.totalVehCount == 1)
                    (*u).second.second.firstArrivalTime = omnetpp::simTime().dbl();

                // last detection time
                (*u).second.second.lastArrivalTime = omnetpp::simTime().dbl();
            }
        }

        // update lastDetection in this LD
        loc->second.second = lastDetection;
    }
}


// update traffic demand for each lane at the beginning of each cycle
// this method is called only when measureTrafficDemandMode == 2
void IntersectionDemand::updateTrafficDemand()
{
    if(!record_trafficDemand_stat || trafficDemandMode != 2)
        return;

    for(auto & u : laneTotalVehCount)
    {
        std::string lane = u.first;
        std::string TLid = u.second.first;

        // ignore side walk
        if( std::find(sideWalks_perTL[TLid].begin(), sideWalks_perTL[TLid].end(), lane) != sideWalks_perTL[TLid].end() )
            continue;

        double interval = u.second.second.lastArrivalTime - u.second.second.firstArrivalTime;

        double TD = (interval == 0) ? 0 : 3600. * ( (double)u.second.second.totalVehCount / interval );

        // bound TD
        TD = std::min(TD, saturationTD);

        // if interval is too big then clear the buffer and restart!
        if(interval >= 200)
        {
            u.second.second.totalVehCount = 0;
            u.second.second.firstArrivalTime = omnetpp::simTime().dbl();

            // clear buffer for this lane in laneTD
            const auto &loc = TD_perLane.find(lane);
            (loc->second).second.clear();

            // clear buffer for this lane in linkTD
            const auto &ppp = outgoingLinks_perLane.equal_range(lane);
            for(std::multimap<std::string, std::pair<std::string, int>>::iterator z = ppp.first; z != ppp.second; ++z)
            {
                int linkNumber = (*z).second.second;

                // push the new TD into the circular buffer
                const auto &location = TD_perLink.find( make_pair(TLid,linkNumber) );
                (location->second).clear();
            }

            LOG_DEBUG << boost::format("\n>>> Traffic demand measurement restarted for lane %1% \n") % lane << std::flush;
        }

        if(TD != 0)
        {
            // push it into the circular buffer
            auto loc = TD_perLane.find(lane);
            std::vector<double> entry {TD /*traffic demand*/, omnetpp::simTime().dbl() /*time of measure*/, -1};
            (loc->second).second.push_back(entry);

            // iterate over outgoing links
            auto ppp = outgoingLinks_perLane.equal_range(lane);
            for(std::multimap<std::string, std::pair<std::string, int>>::iterator z = ppp.first; z != ppp.second; ++z)
            {
                int linkNumber = (*z).second.second;

                // push the new TD into the circular buffer
                auto location = TD_perLink.find( make_pair(TLid,linkNumber) );
                std::vector<double> entry {TD /*traffic demand*/, omnetpp::simTime().dbl() /*time of measure*/, -1};
                (location->second).push_back(entry);
            }
        }
    }
}

}
