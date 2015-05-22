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

        // get the file paths
        VENTOS_FullPath = cSimulation::getActiveSimulation()->getEnvir()->getConfig()->getConfigEntry("network").getBaseDirectory();
        SUMO_Path = simulation.getSystemModule()->par("SUMODirectory").stringValue();
        SUMO_FullPath = VENTOS_FullPath / SUMO_Path;
        // check if this directory is valid?
        if( !exists( SUMO_FullPath ) )
        {
            error("SUMO directory is not valid! Check it again.");
        }

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
        TLStatePerVehicle();   // collecting data from all vehicles in each timeStep
        if(ev.isGUI()) TLDataToFile();  // (if in GUI) write what we have collected so far
    }
}


void TrafficLightBase::TLStatePerVehicle()
{
    // get all lanes in the network
    std::list<std::string> myList = TraCI->laneGetIDList();

    for(std::list<std::string>::iterator i = myList.begin(); i != myList.end(); ++i)
    {
        // get all vehicles on lane i
        std::list<std::string> myList2 = TraCI->laneGetLastStepVehicleIDs( i->c_str() );

        for(std::list<std::string>::reverse_iterator k = myList2.rbegin(); k != myList2.rend(); ++k)
            saveTLStatePerVehicle(k->c_str());
    }

    // increase index after writing data for all vehicles
    if (TraCI->vehicleGetIDCount() > 0)
        index++;
}


void TrafficLightBase::saveTLStatePerVehicle(std::string vID)
{
    std::string lane = TraCI->vehicleGetLaneID(vID);

    // get the TLid that controls this vehicle
    // empty string means the vehicle is not controlled by any TLid
    std::string TLid = "";
    for (std::list<std::string>::iterator it = TLList.begin() ; it != TLList.end(); ++it)
    {
        std::list<std::string> lan = TraCI->TLGetControlledLanes(*it);
        for(std::list<std::string>::iterator it2 = lan.begin(); it2 != lan.end(); ++it2)
        {
            if(*it2 == lane)
            {
                TLid = *it;
                break;
            }
        }
    }

    std::string TLprogram = "";
    if(TLid != "")
        TLprogram = TraCI->TLGetProgram(TLid);

    // if the signal is yellow or red
    int YorR = TraCI->vehicleGetTrafficLightAhead(vID);

    double timeStep = (simTime()-updateInterval).dbl();
    double pos = TraCI->vehicleGetLanePosition(vID);
    double speed = TraCI->vehicleGetSpeed(vID);

    TLVehicleData *tmp = new TLVehicleData(index, timeStep,
                                           vID.c_str(), lane.c_str(), pos,
                                           speed, TLid.c_str(), TLprogram.c_str(), YorR);
    Vec_vehiclesData.push_back(*tmp);
}


void TrafficLightBase::TLDataToFile()
{
    boost::filesystem::path filePath;

    if( ev.isGUI() )
    {
        filePath = "results/gui/TLStatePerVehicle.txt";
    }
    else
    {
        // get the current run number
        int currentRun = ev.getConfigEx()->getActiveRunNumber();
        std::ostringstream fileName;

        fileName << currentRun << "_TLStatePerVehicle.txt";

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
    fprintf (filePtr, "%-12s","TLprogram");
    fprintf (filePtr, "%-17s\n\n","yellowOrRed");

    int oldIndex = -1;

    // write body
    for(std::vector<TLVehicleData>::iterator y = Vec_vehiclesData.begin(); y != Vec_vehiclesData.end(); y++)
    {
        if(oldIndex != y->index)
        {
            fprintf(filePtr, "\n");
            oldIndex = y->index;
        }

        fprintf (filePtr, "%-10d ", y->index);
        fprintf (filePtr, "%-10.2f ", y->time );
        fprintf (filePtr, "%-15s ", y->vehicleName);
        fprintf (filePtr, "%-12s ", y->lane);
        fprintf (filePtr, "%-10.2f ", y->pos);
        fprintf (filePtr, "%-10.2f ", y->speed);
        fprintf (filePtr, "%-15s ", y->TLid);
        fprintf (filePtr, "%-15s ", y->TLprogram);
        fprintf (filePtr, "%-15d \n", y->yellowOrRedSignal);
    }

    fclose(filePtr);
}

}
