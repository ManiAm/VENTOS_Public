/****************************************************************************/
/// @file    TrafficLightBase.cc
/// @author  Mani Amoozadeh <maniam@ucdavis.edu>
/// @author  second author name
/// @date    August 2013
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

#include <01_TL_Base.h>

namespace VENTOS {

Define_Module(VENTOS::TrafficLightBase);


TrafficLightBase::~TrafficLightBase()
{

}


void TrafficLightBase::initialize(int stage)
{
    if(stage == 0)
	{
        // get a pointer to the TraCI module
        cModule *module = simulation.getSystemModule()->getSubmodule("TraCI");
        TraCI = static_cast<TraCI_Extend *>(module);

        Signal_executeFirstTS = registerSignal("executeFirstTS");
        simulation.getSystemModule()->subscribe("executeFirstTS", this);

        Signal_executeEachTS = registerSignal("executeEachTS");
        simulation.getSystemModule()->subscribe("executeEachTS", this);

        updateInterval = TraCI->par("updateInterval").doubleValue();
        collectTLData = par("collectTLData").boolValue();
        TLControlMode = par("TLControlMode").longValue();

        index = 1;
    }
}


void TrafficLightBase::finish()
{

}


void TrafficLightBase::handleMessage(cMessage *msg)
{

}


void TrafficLightBase::receiveSignal(cComponent *source, simsignal_t signalID, long i)
{
    Enter_Method_Silent();

    if(signalID == Signal_executeFirstTS)
    {
        executeFirstTimeStep();
    }
    else if(signalID == Signal_executeEachTS)
    {
        executeEachTimeStep((bool)i);
    }
}


void TrafficLightBase::executeFirstTimeStep()
{
    TLList = TraCI->TLGetIDList();
}


void TrafficLightBase::executeEachTimeStep(bool simulationDone)
{
    if(collectTLData)
    {
        TLData();   // collecting data from all vehicles in each timeStep
        if(ev.isGUI()) TLDataToFile();  // (if in GUI) write what we have collected so far
    }
}


void TrafficLightBase::TLData()
{
    // get all lanes in the network
    list<string> myList = TraCI->laneGetIDList();

    for(list<string>::iterator i = myList.begin(); i != myList.end(); ++i)
    {
        // get all vehicles on lane i
        list<string> myList2 = TraCI->laneGetLastStepVehicleIDs( i->c_str() );

        for(list<string>::reverse_iterator k = myList2.rbegin(); k != myList2.rend(); ++k)
            saveTLData(k->c_str());
    }

    // increase index after writing data for all vehicles
    if (TraCI->vehicleGetIDCount() > 0)
        index++;
}


void TrafficLightBase::saveTLData(string vID)
{
    string lane = TraCI->vehicleGetLaneID(vID);

    // get the TLid that controls this vehicle
    // empty string means the vehicle is not controlled by any TLid
    string TLid = getTrafficLightController(lane);

    // if the signal is yellow or red
    int YorR = TraCI->vehicleGetTrafficLightAhead(vID);

    double timeStep = (simTime()-updateInterval).dbl();
    double pos = TraCI->vehicleGetLanePosition(vID);
    double speed = TraCI->vehicleGetSpeed(vID);

    TLVehicleData *tmp = new TLVehicleData(index, timeStep,
                                           vID.c_str(), lane.c_str(), pos,
                                           speed, TLid.c_str(), YorR);
    Vec_vehiclesData.push_back(tmp);
}


// if the lane is controlled by a TL controller, then
// this method will return the TLid
string TrafficLightBase::getTrafficLightController(string lane)
{
    for (list<string>::iterator it = TLList.begin() ; it != TLList.end(); ++it)
    {
        list<string> lan = TraCI->TLGetControlledLanes(*it);

        bool found = false;
        for(list<string>::iterator it2 = lan.begin(); it2 != lan.end(); ++it2)
        {
            if(lane == *it2)
            {
                found = true;
                break;
            }
        }

        if(found)
            return *it;
    }

    return "";
}


void TrafficLightBase::TLDataToFile()
{
    boost::filesystem::path filePath;

    if( ev.isGUI() )
    {
        filePath = "results/gui/TLData.txt";
    }
    else
    {
        // get the current run number
        int currentRun = ev.getConfigEx()->getActiveRunNumber();
        ostringstream fileName;

        fileName << currentRun << "_TLData.txt";

        filePath = "results/cmd/" + fileName.str();
    }

    FILE *filePtr = fopen (filePath.string().c_str(), "w");

    // write header
    fprintf (filePtr, "%-10s","index");
    fprintf (filePtr, "%-12s","timeStep");
    fprintf (filePtr, "%-15s","vehicleName");
    fprintf (filePtr, "%-12s","lane");
    fprintf (filePtr, "%-11s","pos");
    fprintf (filePtr, "%-12s","speed");
    fprintf (filePtr, "%-12s","TLid");
    fprintf (filePtr, "%-17s\n\n","yellowOrRed");

    int oldIndex = -1;

    // write body
    for(unsigned int k=0; k<Vec_vehiclesData.size(); k++)
    {
        if(oldIndex != Vec_vehiclesData[k]->index)
        {
            fprintf(filePtr, "\n");
            oldIndex = Vec_vehiclesData[k]->index;
        }

        fprintf (filePtr, "%-10d ", Vec_vehiclesData[k]->index);
        fprintf (filePtr, "%-10.2f ", Vec_vehiclesData[k]->time );
        fprintf (filePtr, "%-15s ", Vec_vehiclesData[k]->vehicleName);
        fprintf (filePtr, "%-12s ", Vec_vehiclesData[k]->lane);
        fprintf (filePtr, "%-10.2f ", Vec_vehiclesData[k]->pos);
        fprintf (filePtr, "%-10.2f ", Vec_vehiclesData[k]->speed);
        fprintf (filePtr, "%-15s ", Vec_vehiclesData[k]->TLid);
        fprintf (filePtr, "%-15d \n", Vec_vehiclesData[k]->yellowOrRedSignal);
    }

    fclose(filePtr);
}

}
