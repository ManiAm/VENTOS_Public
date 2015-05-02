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
        collectIntersectionQueue = par("collectIntersectionQueue").boolValue();

        queue_C_NC_2.clear();
        queue_C_NC_3.clear();
        queue_C_NC_4.clear();

        queue_C_SC_2.clear();
        queue_C_SC_3.clear();
        queue_C_SC_4.clear();

        queue_C_WC_2.clear();
        queue_C_WC_3.clear();
        queue_C_WC_4.clear();

        queue_C_EC_2.clear();
        queue_C_EC_3.clear();
        queue_C_EC_4.clear();
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

    LDList = TraCI->commandGetLoopDetectorList();
}


void LoopDetectors::executeEachTimeStep(bool simulationDone)
{
    TrafficLightBase::executeEachTimeStep(simulationDone);

    if(collectInductionLoopData || collectIntersectionQueue)
        inductionLoops();    // collecting induction loop data in each timeStep

    if(collectInductionLoopData && ev.isGUI())
        inductionLoopToFile();  // (if in GUI) write to file what we have collected so far

    if(collectInductionLoopData && !ev.isGUI() && simulationDone)
        inductionLoopToFile();  // (if in CMD) write to file at the end of simulation
}


void LoopDetectors::inductionLoops()
{
    // get all loop detectors
    list<string> str = TraCI->commandGetLoopDetectorList();

    // for each loop detector
    for (list<string>::iterator it=str.begin(); it != str.end(); ++it)
    {
        vector<string>  st = TraCI->commandGetLoopDetectorVehicleData(*it);

        // only if this loop detector detected a vehicle
        if( st.size() > 0 )
        {
            // laneID of loop detector
            string lane = TraCI->commandGetLoopDetectorLaneID(*it);

            // get vehicle information
            string vehicleName = st.at(0);
            double entryT = atof( st.at(2).c_str() );
            double leaveT = atof( st.at(3).c_str() );
            double speed = TraCI->commandGetLoopDetectorSpeed(*it);  // vehicle speed at current moment

            // save it only when collectInductionLoopData=true
            if(collectInductionLoopData)
            {
                int counter = findInVector(Vec_loopDetectors, (*it).c_str(), vehicleName.c_str());

                // its a new entry, so we add it
                if(counter == -1)
                {
                    LoopDetectorData *tmp = new LoopDetectorData( (*it).c_str(), lane.c_str(), vehicleName.c_str(), entryT, leaveT, speed, speed );
                    Vec_loopDetectors.push_back(tmp);
                }
                // if found, just update leaveTime and leaveSpeed
                else
                {
                    Vec_loopDetectors[counter]->leaveTime = leaveT;
                    Vec_loopDetectors[counter]->leaveSpeed = speed;
                }
            }

            // todo
            if(collectIntersectionQueue)
            {
                if( string(*it) == "C-NC_2-1" )
                {

                }
                else if( string(*it) == "C-NC_3-1" )
                {

                }
                else if( string(*it) == "C-NC_4-1" )
                {

                }
            }
        }
    }
}


void LoopDetectors::inductionLoopToFile()
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
        ostringstream fileName;
        fileName << currentRun << "_loopDetector.txt";
        filePath = "results/cmd/" + fileName.str();
    }

    FILE *filePtr = fopen (filePath.string().c_str(), "w");

    // write header
    fprintf (filePtr, "%-20s","loopDetector");
    fprintf (filePtr, "%-20s","lane");
    fprintf (filePtr, "%-20s","vehicleName");
    fprintf (filePtr, "%-20s","vehicleEntryTime");
    fprintf (filePtr, "%-20s","vehicleLeaveTime");
    fprintf (filePtr, "%-22s","vehicleEntrySpeed");
    fprintf (filePtr, "%-22s\n\n","vehicleLeaveSpeed");

    // write body
    for(unsigned int k=0; k<Vec_loopDetectors.size(); k++)
    {
        fprintf (filePtr, "%-20s ", Vec_loopDetectors[k]->detectorName);
        fprintf (filePtr, "%-20s ", Vec_loopDetectors[k]->lane);
        fprintf (filePtr, "%-20s ", Vec_loopDetectors[k]->vehicleName);
        fprintf (filePtr, "%-20.2f ", Vec_loopDetectors[k]->entryTime);
        fprintf (filePtr, "%-20.2f ", Vec_loopDetectors[k]->leaveTime);
        fprintf (filePtr, "%-20.2f ", Vec_loopDetectors[k]->entrySpeed);
        fprintf (filePtr, "%-20.2f\n", Vec_loopDetectors[k]->leaveSpeed);
    }

    fclose(filePtr);
}


int LoopDetectors::findInVector(vector<LoopDetectorData *> Vec, const char *detectorName, const char *vehicleName)
{
    unsigned int counter;
    bool found = false;

    for(counter = 0; counter < Vec.size(); counter++)
    {
        if( strcmp(Vec[counter]->detectorName, detectorName) == 0 && strcmp(Vec[counter]->vehicleName, vehicleName) == 0)
        {
            found = true;
            break;
        }
    }

    if(!found)
        return -1;
    else
        return counter;
}

}
