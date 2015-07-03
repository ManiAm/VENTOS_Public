/****************************************************************************/
/// @file    LoopDetectors.cc
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

#include <02_LoopDetectors.h>
#define MAX_BUFF 1

namespace VENTOS {

Define_Module(VENTOS::LoopDetectors);


LoopDetectors::~LoopDetectors()
{

}


void LoopDetectors::initialize(int stage)
{
    TrafficLightBase::initialize(stage);

    if(stage == 0)
    {
        measureTrafficDemand = par("measureTrafficDemand").boolValue();
        measureIntersectionQueue = par("measureIntersectionQueue").boolValue();

        collectInductionLoopData = par("collectInductionLoopData").boolValue();
        collectTLQueuingData = par("collectTLQueuingData").boolValue();
        collectTLPhasingData = par("collectTLPhasingData").boolValue();

        // we need to turn on measureIntersectionQueue
        if(collectTLQueuingData || collectTLPhasingData)
            measureIntersectionQueue = true;

        LD_demand.clear();
        LD_actuated.clear();
        AD_queue.clear();

        lanesTL.clear();
        linksTL.clear();

        laneQueueSize.clear();
        linkQueueSize.clear();

        laneTD.clear();
        linkTD.clear();

        phaseTL.clear();
        statusTL.clear();

        Vec_loopDetectors.clear();
        Vec_totalQueueSize.clear();
    }
}


void LoopDetectors::finish()
{
    TrafficLightBase::finish();
}


void LoopDetectors::handleMessage(cMessage *msg)
{
    TrafficLightBase::handleMessage(msg);
}


void LoopDetectors::executeFirstTimeStep()
{
    TrafficLightBase::executeFirstTimeStep();

    TLList = TraCI->TLGetIDList();

    getAllDetectors();

    // for each traffic light
    for (std::list<std::string>::iterator it = TLList.begin(); it != TLList.end(); ++it)
    {
        std::string TLid = *it;

        // get all incoming lanes
        std::list<std::string> lan = TraCI->TLGetControlledLanes(*it);

        // remove duplicate entries
        lan.unique();

        // for each incoming lane
        for(std::list<std::string>::iterator it2 = lan.begin(); it2 != lan.end(); ++it2)
        {
            std::string lane = *it2;

            lanesTL[lane] = TLid;

            boost::circular_buffer<double> CB;        // create a circular buffer
            CB.set_capacity(MAX_BUFF);                // set max capacity
            CB.clear();
            laneTD[lane] = std::make_pair(TLid, CB);  // initialize laneTD

            // initialize queue value in laneQueueSize to zero
            laneQueueSize[lane] = std::make_pair(TLid, 0);
        }

        totalQueueSize[TLid] = std::make_pair(0, lan.size());

        // get all links controlled by this TL
        std::map<int,std::vector<std::string>> result = TraCI->TLGetControlledLinks(TLid);

        // for each link in this TLid
        for(std::map<int,std::vector<std::string>>::iterator it2 = result.begin(); it2 != result.end(); ++it2)
        {
            int linkNumber = (*it2).first;
            std::vector<std::string> link = (*it2).second;
            std::string incommingLane = link[0];

            linksTL.insert( std::make_pair(incommingLane, std::make_pair(TLid,linkNumber)) );

            boost::circular_buffer<double> CB;   // create a circular buffer
            CB.set_capacity(MAX_BUFF);           // set max capacity
            CB.clear();
            linkTD[std::make_pair(TLid,linkNumber)] = CB;   // initialize linkTD

            // initialize queue value in linkQueueSize to zero
            linkQueueSize.insert( std::make_pair(std::make_pair(TLid,linkNumber), 0) );
        }
    }
}


void LoopDetectors::executeEachTimeStep(bool simulationDone)
{
    TrafficLightBase::executeEachTimeStep(simulationDone);

    if(collectInductionLoopData)
    {
        collectLDsData();    // collecting induction loop data in each timeStep

        if(ev.isGUI())
            saveLDsData();  // (if in GUI) write to file what we have collected so far
        else if(simulationDone)
            saveLDsData();  // (if in CMD) write to file at the end of simulation
    }

    if(measureIntersectionQueue || measureTrafficDemand)
        measureTrafficParameters();

    if(collectTLQueuingData)
    {
        if(ev.isGUI())
            saveTLQueueingData();  // (if in GUI) write to file what we have collected so far
        else if(simulationDone)
            saveTLQueueingData();  // (if in CMD) write to file at the end of simulation
    }

    if(collectTLPhasingData)
    {
        if(ev.isGUI())
            saveTLPhasingData();  // (if in GUI) write to file what we have collected so far
        else if(simulationDone)
            saveTLPhasingData();  // (if in CMD) write to file at the end of simulation
    }
}


void LoopDetectors::getAllDetectors()
{
    // get all loop detectors
    std::list<std::string> str = TraCI->LDGetIDList();

    // for each loop detector
    for (std::list<std::string>::iterator it=str.begin(); it != str.end(); ++it)
    {
        std::string lane = TraCI->LDGetLaneID(*it);

        if( std::string(*it).find("demand_") != std::string::npos )
            LD_demand[lane] = std::make_pair(*it,-1);
        else if( std::string(*it).find("actuated_") != std::string::npos )
            LD_actuated[lane] = *it;
    }

    // get all area detectors
    std::list<std::string> str2 = TraCI->LADGetIDList();

    // for each area detector detector
    for (std::list<std::string>::iterator it=str2.begin(); it != str2.end(); ++it)
    {
        std::string lane = TraCI->LADGetLaneID(*it);

        if( std::string(*it).find("queue_") != std::string::npos )
            AD_queue[lane] = *it;
    }

    /*
    std::cout << endl << LD_demand.size() << " demand loop detectors found!" << endl;
    std::cout << LD_actuated.size() << " actuated loop detectors found!" << endl;
    std::cout << AD_queue.size() << " area detectors found!" << endl << endl;
     */

    // make sure we have all detectors we need
    for (std::list<std::string>::iterator it = TLList.begin(); it != TLList.end(); ++it)
    {
        std::list<std::string> lan = TraCI->TLGetControlledLanes(*it);

        // remove duplicate entries
        lan.unique();

        // for each incoming lane
        for(std::list<std::string>::iterator it2 = lan.begin(); it2 != lan.end(); ++it2)
        {
            // some traffic signal controls need one actuated LD on each incoming lane
            if( TLControlMode == TL_Adaptive_Time && LD_actuated.find(*it2) == LD_actuated.end() )
                std::cout << "WARNING: no loop detector found on lane (" << *it2 << "). No actuation is available for this lane." << endl;

            // if we are measuring queue length then make sure we have an area detector in each lane
            if( measureIntersectionQueue && AD_queue.find(*it2) == AD_queue.end() )
                std::cout << "WARNING: no area detector found on lane (" << *it2 << "). No queue measurement is available for this lane." << endl;
        }
    }
}


void LoopDetectors::collectLDsData()
{
    // get all loop detectors
    std::list<std::string> str = TraCI->LDGetIDList();

    // for each loop detector
    for (std::list<std::string>::iterator it=str.begin(); it != str.end(); ++it)
    {
        std::vector<std::string>  st = TraCI->LDGetLastStepVehicleData(*it);

        // only if this loop detector detected a vehicle
        if( st.size() > 0 )
        {
            // laneID of loop detector
            std::string lane = TraCI->LDGetLaneID(*it);

            // get vehicle information
            std::string vehicleName = st.at(0);
            double entryT = atof( st.at(2).c_str() );
            double leaveT = atof( st.at(3).c_str() );
            double speed = TraCI->LDGetLastStepMeanVehicleSpeed(*it);  // vehicle speed at current moment

            // save it only when collectInductionLoopData=true
            if(collectInductionLoopData)
            {
                const LoopDetectorData *searchFor = new LoopDetectorData( (*it).c_str(), "", vehicleName.c_str() );
                std::vector<LoopDetectorData>::iterator counter = std::find(Vec_loopDetectors.begin(), Vec_loopDetectors.end(), *searchFor);

                // its a new entry, so we add it
                if(counter == Vec_loopDetectors.end())
                {
                    LoopDetectorData *tmp = new LoopDetectorData( (*it).c_str(), lane.c_str(), vehicleName.c_str(), entryT, leaveT, speed, speed );
                    Vec_loopDetectors.push_back(*tmp);
                }
                // if found, just update leaveTime and leaveSpeed
                else
                {
                    counter->leaveTime = leaveT;
                    counter->leaveSpeed = speed;
                }
            }
        }
    }
}


void LoopDetectors::saveLDsData()
{
    boost::filesystem::path filePath;

    if( ev.isGUI() )
    {
        filePath = "results/gui/loopDetector.txt";
    }
    else
    {
        // get the current run number
        int currentRun = ev.getConfigEx()->getActiveRunNumber();
        std::ostringstream fileName;
        fileName << currentRun << "_loopDetector.txt";
        filePath = "results/cmd/" + fileName.str();
    }

    FILE *filePtr = fopen (filePath.string().c_str(), "w");

    // write header
    fprintf (filePtr, "%-30s","loopDetector");
    fprintf (filePtr, "%-20s","lane");
    fprintf (filePtr, "%-20s","vehicleName");
    fprintf (filePtr, "%-20s","vehicleEntryTime");
    fprintf (filePtr, "%-20s","vehicleLeaveTime");
    fprintf (filePtr, "%-22s","vehicleEntrySpeed");
    fprintf (filePtr, "%-22s\n\n","vehicleLeaveSpeed");

    // write body
    for(std::vector<LoopDetectorData>::iterator y = Vec_loopDetectors.begin(); y != Vec_loopDetectors.end(); ++y)
    {
        fprintf (filePtr, "%-30s ", y->detectorName.c_str());
        fprintf (filePtr, "%-20s ", y->lane.c_str());
        fprintf (filePtr, "%-20s ", y->vehicleName.c_str());
        fprintf (filePtr, "%-20.2f ", y->entryTime);
        fprintf (filePtr, "%-20.2f ", y->leaveTime);
        fprintf (filePtr, "%-20.2f ", y->entrySpeed);
        fprintf (filePtr, "%-20.2f\n", y->leaveSpeed);
    }

    fclose(filePtr);
}


void LoopDetectors::measureTrafficParameters()
{
    // for each 'lane i' that is controlled by traffic light j
    for(std::map<std::string, std::string>::iterator y = lanesTL.begin(); y != lanesTL.end(); ++y)
    {
        std::string lane = (*y).first;
        std::string TLid = (*y).second;

        if(measureIntersectionQueue)
        {
            // make sure we have an area detector in this lane
            if( AD_queue.find(lane) == AD_queue.end() )
                continue;

            std::string ADid = AD_queue[lane];
            int q = TraCI->LADGetLastStepVehicleHaltingNumber(ADid);

            // update laneQueueSize
            std::map<std::string,std::pair<std::string,int>>::iterator location = laneQueueSize.find(lane);
            std::pair<std::string,int> store = location->second;
            location->second = std::make_pair( store.first, q );

            // save total queue size for each TLid
            totalQueueSize[TLid].first = totalQueueSize[TLid].first + q;

            // get # of outgoing links from this lane
            int NoLinks = linksTL.count(lane);

            // iterate over outgoing links
            std::pair<std::multimap<std::string, std::pair<std::string, int>>::iterator, std::multimap<std::string, std::pair<std::string, int>>::iterator > ppp;
            ppp = linksTL.equal_range(lane);
            for(std::multimap<std::string, std::pair<std::string, int>>::iterator z = ppp.first; z != ppp.second; ++z)
            {
                int linkNumber = (*z).second.second;

                // each link gets a portion of the queue size
                q = ceil( (double)q / (double)NoLinks );

                // update linkQueueSize
                std::map<std::pair<std::string,int>, int>::iterator location = linkQueueSize.find( make_pair(TLid,linkNumber) );
                location->second = q;
            }
        }

        if(measureTrafficDemand)
        {
            // make sure we have a demand loop detector in this lane
            std::map<std::string, std::pair<std::string, double>>::iterator loc = LD_demand.find(lane);
            if( loc == LD_demand.end() )
                continue;

            std::string LDid = loc->second.first;
            double lastDetection_old = loc->second.second;   // lastDetection_old is one step behind lastDetection
            double lastDetection = TraCI->LDGetElapsedTimeLastDetection(LDid);

            double diff = simTime().dbl() - lastDetection_old - updateInterval;

            // lastDetection == 0        if a vehicle is above the LD
            // lastDetection_old != 0    if this is the first detection for this vehicle (we ignore any subsequent detections for the same vehicle)
            // diff > 0.0001             ignore the very first detection on this LDid
            if(lastDetection == 0 && lastDetection_old != 0 && diff > 0.0001)
            {
                // calculate the traffic demand
                double TD = 3600. / lastDetection_old;

                // push it into the circular buffer
                std::map<std::string, std::pair<std::string, boost::circular_buffer<double>>>::iterator loc = laneTD.find(lane);
                (loc->second).second.push_back(TD);

                // iterate over outgoing links
                std::pair<std::multimap<std::string, std::pair<std::string, int>>::iterator, std::multimap<std::string, std::pair<std::string, int>>::iterator > ppp;
                ppp = linksTL.equal_range(lane);
                for(std::multimap<std::string, std::pair<std::string, int>>::iterator z = ppp.first; z != ppp.second; ++z)
                {
                    int linkNumber = (*z).second.second;

                    // push the new TD into the circular buffer
                    std::map<std::pair<std::string,int>, boost::circular_buffer<double>>::iterator location = linkTD.find( make_pair(TLid,linkNumber) );
                    (location->second).push_back(TD);
                }
            }

            // update lastDetection in this LD
            loc->second.second = lastDetection;
        }
    }

    if(collectTLQueuingData)
    {
        for(std::map<std::string, std::pair<int, int>>::iterator y = totalQueueSize.begin(); y != totalQueueSize.end(); ++y)
        {
            std::string TLid = (*y).first;
            int totalQueue = (*y).second.first;
            int laneCount = (*y).second.second;

            currentQueueSize *entry = new currentQueueSize(simTime().dbl(), TLid, totalQueue, laneCount);
            Vec_totalQueueSize.push_back(*entry);

            // reset total queue
            (*y).second.first = 0;
        }
    }
}


void LoopDetectors::saveTLQueueingData()
{
    boost::filesystem::path filePath;

    if( ev.isGUI() )
    {
        filePath = "results/gui/TLqueuingData.txt";
    }
    else
    {
        // get the current run number
        int currentRun = ev.getConfigEx()->getActiveRunNumber();
        std::ostringstream fileName;
        fileName << currentRun << "_TLqueuingData.txt";
        filePath = "results/cmd/" + fileName.str();
    }

    FILE *filePtr = fopen (filePath.string().c_str(), "w");

    // write header
    fprintf (filePtr, "%-10s", "index");
    fprintf (filePtr, "%-10s", "timeStep");
    fprintf (filePtr, "%-10s", "TLid");
    fprintf (filePtr, "%-15s", "totalQueue");
    fprintf (filePtr, "%-10s\n\n", "laneCount");

    double oldTime = -1;
    int index = 0;

    for(std::vector<currentQueueSize>::iterator y = Vec_totalQueueSize.begin(); y != Vec_totalQueueSize.end(); ++y )
    {
        if(oldTime != (*y).time)
        {
            fprintf(filePtr, "\n");
            oldTime = (*y).time;
            index++;
        }

        fprintf (filePtr, "%-10d", index);
        fprintf (filePtr, "%-10.2f", (*y).time);
        fprintf (filePtr, "%-10s", (*y).TLid.c_str());
        fprintf (filePtr, "%-15d", (*y).totoalQueueSize);
        fprintf (filePtr, "%-10d\n", (*y).laneCount);
    }

    fclose(filePtr);
}


void LoopDetectors::saveTLPhasingData()
{
    boost::filesystem::path filePath;

    if( ev.isGUI() )
    {
        filePath = "results/gui/TLphasingData.txt";
    }
    else
    {
        // get the current run number
        int currentRun = ev.getConfigEx()->getActiveRunNumber();
        std::ostringstream fileName;
        fileName << currentRun << "_TLphasingData.txt";
        filePath = "results/cmd/" + fileName.str();
    }

    FILE *filePtr = fopen (filePath.string().c_str(), "w");

    // write header
    fprintf (filePtr, "%-12s", "TLid");
    fprintf (filePtr, "%-12s", "phase");
    fprintf (filePtr, "%-35s", "allowedMovements");
    fprintf (filePtr, "%-15s", "greenStart");
    fprintf (filePtr, "%-15s", "yellowStart");
    fprintf (filePtr, "%-15s", "redStart");
    fprintf (filePtr, "%-15s", "phaseEnd");
    fprintf (filePtr, "%-15s", "#lanes");
    fprintf (filePtr, "%-15s\n\n", "totalqueueSize");

    // write body
    for( std::map<std::pair<std::string, int>, currentStatusTL>::iterator y = statusTL.begin(); y != statusTL.end(); ++y)
    {
        std::string TLid = ((*y).first).first;
        int phaseNumber = ((*y).first).second;
        currentStatusTL status = (*y).second;

        std::string allowedMovements = status.allowedMovements;
        double greenStart = status.greenStart;
        double yellowStart = status.yellowStart;
        double redStart = status.redStart;
        double phaseEnd = status.phaseEnd;
        int incommingLanes = status.incommingLanes;
        int totalQueueSize = status.totalQueueSize;

        fprintf (filePtr, "%-12s", TLid.c_str());
        fprintf (filePtr, "%-12d", phaseNumber);
        fprintf (filePtr, "%-35s", allowedMovements.c_str());
        fprintf (filePtr, "%-15.2f", greenStart);
        fprintf (filePtr, "%-15.2f", yellowStart);
        fprintf (filePtr, "%-15.2f", redStart);
        fprintf (filePtr, "%-15.2f", phaseEnd);
        fprintf (filePtr, "%-15d", incommingLanes);
        fprintf (filePtr, "%-15d\n", totalQueueSize);
    }

    fclose(filePtr);
}

}
