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
        collectInductionLoopData = par("collectInductionLoopData").boolValue();
        measureTrafficDemand = par("measureTrafficDemand").boolValue();
        measureIntersectionQueue = par("measureIntersectionQueue").boolValue();
        collectTLData = par("collectTLData").boolValue();

        minGreenTime = par("minGreenTime").doubleValue();
        maxGreenTime = par("maxGreenTime").doubleValue();
        yellowTime = par("yellowTime").doubleValue();
        redTime = par("redTime").doubleValue();
        passageTime = par("passageTime").doubleValue();
        greenExtension = par("greenExtension").boolValue();

        if(minGreenTime <= 0)
            error("minGreenTime value is wrong!");

        if(maxGreenTime <= 0 || maxGreenTime < minGreenTime)
            error("maxGreenTime value is wrong!");

        if(yellowTime <= 0)
            error("yellowTime value is wrong!");

        if(redTime <= 0)
            error("redTime value is wrong!");

        phaseNumber = 0;

        LD_demand.clear();
        LD_actuated.clear();
        AD_queue.clear();

        laneQueueSize.clear();
        linkQueueSize.clear();

        Vec_loopDetectors.clear();
        Vec_IntersectionData.clear();
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
        // get all incoming lanes
        std::list<std::string> lan = TraCI->TLGetControlledLanes(*it);

        // remove duplicate entries
        lan.unique();

        // for each incoming lane
        for(std::list<std::string>::iterator it2 = lan.begin(); it2 != lan.end(); ++it2)
        {
            std::string TLid = *it;
            std::string lane = *it2;

            lanesTL[lane] = TLid;

            // initialize all queue value in laneQueueSize to zero
            laneQueueSize[lane] = std::make_pair(TLid, 0);
        }

        // get all links controlled by this TL
        std::map<int,std::string> result = TraCI->TLGetControlledLinks(*it);

        // for each link in this TLid
        for(std::map<int,std::string>::iterator it2 = result.begin(); it2 != result.end(); ++it2)
        {
            std::string TLid = *it;
            int linkNumber = (*it2).first;
            std::string link = (*it2).second;

            linksTL[make_pair(TLid,linkNumber)] = link;

            // initialize all queue value in linkQueueSize to zero
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

    if(measureTrafficDemand)
        measureTD();

    if(measureIntersectionQueue)
        measureQ();

    // should be after measureQ
    if(collectTLData)
    {
        // collect TL data at the end of each phase
        if( currentInterval == "red" && std::fabs(intervalElapseTime - (redTime-updateInterval)) < 0.001 )
        {
            collectTrafficLightData();

            if(ev.isGUI())
                saveTLData();  // (if in GUI) write to file what we have collected so far
        }

        if(!ev.isGUI() && simulationDone)
            saveTLData();  // (if in CMD) write to file at the end of simulation
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
            LD_demand[lane] = *it;
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

    std::cout << endl;
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


// todo: dynamic demand calculation does not make sense!
void LoopDetectors::measureTD()
{
    // for each loop detector that measures the traffic demand
    for (std::map<std::string,std::string>::iterator it=LD_demand.begin(); it != LD_demand.end(); ++it)
    {
        if( std::string( (*it).second ) == "demand_WC_3" )
        {
            double lastDetectionT = TraCI->LDGetElapsedTimeLastDetection( (*it).second );

            if(lastDetectionT == 0 && !freeze)
            {
                passedVeh++;
                freeze = true;

                if(passedVeh == 1)
                    total = lastDetectionT_old = 0;
            }

            if(freeze && lastDetectionT != 0)
                freeze = false;

            total = total + lastDetectionT_old;

            //std::cout << endl << passedVeh << ", " << total << ", " << (3600 * passedVeh)/total << endl;

            lastDetectionT_old = lastDetectionT;
        }
    }
}


// measure queue size per incoming lane per simTime in each intersection
void LoopDetectors::measureQ()
{
    // for each lane i that is controlled by traffic light j
    for(std::map<std::string, std::string>::iterator y = lanesTL.begin(); y != lanesTL.end(); ++y)
    {
        std::string lane = (*y).first;
        std::string TLid = (*y).second;

        // make sure we have an area detector in this lane
        if( AD_queue.find(lane) == AD_queue.end() )
            continue;

        std::string ADid = AD_queue[lane];
        int q = TraCI->LADGetLastStepVehicleNumber(ADid);

        // update laneQueueSize
        std::map<std::string,std::pair<std::string,int>>::iterator location = laneQueueSize.find(lane);
        std::pair<std::string,int> store = location->second;
        location->second = std::make_pair( store.first, q );
    }

    for(std::map<std::pair<std::string,int>,std::string>::iterator y = linksTL.begin(); y != linksTL.end(); ++y)
    {
        std::string TLid = (*y).first.first;
        int linkNumber = (*y).first.second;
        std::string link = (*y).second;

        std::string incommingLane;
        boost::char_separator<char> sep("|");
        boost::tokenizer< boost::char_separator<char> > tokens(link, sep);
        boost::tokenizer< boost::char_separator<char> >::iterator beg = tokens.begin();
        incommingLane = (*beg).c_str(); // get the first token

        // make sure we have an area detector in this lane
        if( AD_queue.find(incommingLane) == AD_queue.end() )
            continue;

        std::string ADid = AD_queue[incommingLane];
        int q = TraCI->LADGetLastStepVehicleNumber(ADid);

        // update linkQueueSize
        std::map<std::pair<std::string,int>, int>::iterator location = linkQueueSize.find( make_pair(TLid,linkNumber) );
        location->second = q;
    }
}


// collect TL data
void LoopDetectors::collectTrafficLightData()
{
    phaseNumber++;

    // for each lane i that is controlled by traffic light j
    for(std::map<std::string, std::string>::iterator y = lanesTL.begin(); y != lanesTL.end(); ++y)
    {
        std::string lane = (*y).first;
        std::string TLid = (*y).second;
        int qSize = laneQueueSize[lane].second;

        IntersectionTLData *tmp = new IntersectionTLData( phaseNumber, simTime().dbl(), TLid, lane, qSize);
        Vec_IntersectionData.push_back(*tmp);
    }
}


void LoopDetectors::saveTLData()
{
    boost::filesystem::path filePath;

    if( ev.isGUI() )
    {
        filePath = "results/gui/intersectionData.txt";
    }
    else
    {
        // get the current run number
        int currentRun = ev.getConfigEx()->getActiveRunNumber();
        std::ostringstream fileName;
        fileName << currentRun << "_intersectionData.txt";
        filePath = "results/cmd/" + fileName.str();
    }

    FILE *filePtr = fopen (filePath.string().c_str(), "w");

    // write header
    fprintf (filePtr, "%-15s","phase");
    fprintf (filePtr, "%-15s","endTime");
    fprintf (filePtr, "%-15s","TLid");
    fprintf (filePtr, "%-15s","lane");
    fprintf (filePtr, "%-20s\n","queueSize");

    double oldTime = -1;

    // write body
    for(std::vector<IntersectionTLData>::iterator y = Vec_IntersectionData.begin(); y != Vec_IntersectionData.end(); ++y)
    {
        if(oldTime != y->time)
        {
            fprintf(filePtr, "\n");
            oldTime = y->time;
        }

        fprintf (filePtr, "%-15d ", y->phaseNumber);
        fprintf (filePtr, "%-15.2f ", y->time);
        fprintf (filePtr, "%-15s ", y->TLid.c_str());
        fprintf (filePtr, "%-15s ", y->lane.c_str());
        fprintf (filePtr, "%-15d\n", y->qSize);
    }

    fclose(filePtr);
}

}
