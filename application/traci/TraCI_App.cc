/****************************************************************************/
/// @file    TraCI_App.cc
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

#include "TraCI_App.h"
#include "modules/mobility/traci/TraCIScenarioManagerInet.h"
#include "modules/mobility/traci/TraCIMobility.h"

namespace VENTOS {

Define_Module(VENTOS::TraCI_App);


TraCI_App::~TraCI_App()
{

}


void TraCI_App::initialize(int stage)
{
    TraCI_Extend::initialize(stage);

    if(stage == 0)
    {
        // get the ptr of the current module
        nodePtr = FindModule<>::findHost(this);
        if(nodePtr == NULL)
            error("can not get a pointer to the module.");

        terminate = par("terminate").doubleValue();

        subscribedPedestrians.clear();

        bikeModuleType = par("bikeModuleType").stringValue();
        bikeModuleName = par("bikeModuleName").stringValue();
        bikeModuleDisplayString = par("bikeModuleDisplayString").stringValue();

        pedModuleType = par("pedModuleType").stringValue();
        pedModuleName = par("pedModuleName").stringValue();
        pedModuleDisplayString = par("pedModuleDisplayString").stringValue();

        collectVehiclesData = par("collectVehiclesData").boolValue();
        useDetailedFilenames = par("useDetailedFilenames").boolValue();

        index = 1;

        cModule *rmodule = simulation.getSystemModule()->getSubmodule("router");
        router = static_cast< Router* >(rmodule);
    }
}


void TraCI_App::finish()
{
    TraCI_Extend::finish();
}


void TraCI_App::handleSelfMsg(cMessage *msg)
{
    TraCI_Extend::handleSelfMsg(msg);
}


// this method is called once (TraCI is up and running)
void TraCI_App::init_traci()
{
    TraCI_Extend::init_traci();

    // get the list of all TL
    TLList = TLGetIDList();

    simsignal_t Signal_executeFirstTS = registerSignal("executeFirstTS");
    nodePtr->emit(Signal_executeFirstTS, 1);
}


void TraCI_App::executeOneTimestep()
{
    EV << "### Sending Command to SUMO to perform simulation for TS = " << (getCurrentTimeMs()/1000.) << endl;

    TraCIScenarioManager::executeOneTimestep();

    addPedestriansToOMNET();

    EV << "### SUMO completed simulation for TS = " << (getCurrentTimeMs()/1000.) << endl;

    // if simulation should be ended
    int NoVehAndBike = simulationGetMinExpectedNumber();
    int NoPed = personGetIDCount();
    int totalModules = NoVehAndBike + NoPed;
    bool simulationDone = (simTime().dbl() >= terminate) or (totalModules == 0);

    if(collectVehiclesData)
    {
        vehiclesData();   // collecting data from all vehicles in each timeStep

        if(ev.isGUI()) vehiclesDataToFile();  // (if in GUI) write what we have collected so far
        else if(simulationDone) vehiclesDataToFile();  // (if in CMD) write to file at the end of simulation
    }

    // notify other modules to run one simulation TS
    simsignal_t Signal_executeEachTS = registerSignal("executeEachTS");
    nodePtr->emit(Signal_executeEachTS, (long)simulationDone);

    if(simulationDone)
    {
        simulationTerminate();  // close TraCI connection
        endSimulation();   // then terminate
    }
}


void TraCI_App::addPedestriansToOMNET()
{
    std::list<std::string> allPedestrians = personGetIDList();
    //cout << simTime().dbl() << ": " << allPedestrians.size() << endl;

    if(allPedestrians.size() == 0)
        return;

    // add new inserted pedestrians
    std::set<std::string> needSubscribe;
    std::set_difference(allPedestrians.begin(), allPedestrians.end(), subscribedPedestrians.begin(), subscribedPedestrians.end(), std::inserter(needSubscribe, needSubscribe.begin()));
    for (std::set<std::string>::const_iterator i = needSubscribe.begin(); i != needSubscribe.end(); ++i)
        subscribedPedestrians.insert(*i);

    // remove pedestrians that are not present in the network
    std::set<std::string> needUnsubscribe;
    std::set_difference(subscribedPedestrians.begin(), subscribedPedestrians.end(), allPedestrians.begin(), allPedestrians.end(), std::inserter(needUnsubscribe, needUnsubscribe.begin()));
    for (std::set<std::string>::const_iterator i = needUnsubscribe.begin(); i != needUnsubscribe.end(); ++i)
        subscribedPedestrians.erase(*i);



    //    bool isSubscribed = (subscribedPedestrians.find(objectId) != subscribedPedestrians.end());
    //    double px;
    //    double py;
    //    std::string edge;
    //    double speed;
    //    double angle_traci;
    //    // int signals;
    //        int numRead = 0;

    //        if (variable1_resp == VAR_POSITION) {
    //            uint8_t varType; buf >> varType;
    //            ASSERT(varType == POSITION_2D);
    //            buf >> px;
    //            buf >> py;
    //            numRead++;
    //        } else if (variable1_resp == VAR_ROAD_ID) {
    //            uint8_t varType; buf >> varType;
    //            ASSERT(varType == TYPE_STRING);
    //            buf >> edge;
    //            numRead++;
    //        } else if (variable1_resp == VAR_SPEED) {
    //            uint8_t varType; buf >> varType;
    //            ASSERT(varType == TYPE_DOUBLE);
    //            buf >> speed;
    //            numRead++;
    //        } else if (variable1_resp == VAR_ANGLE) {
    //            uint8_t varType; buf >> varType;
    //            ASSERT(varType == TYPE_DOUBLE);
    //            buf >> angle_traci;
    //            numRead++;
    ////        }   else if (variable1_resp == VAR_SIGNALS) {
    ////            uint8_t varType; buf >> varType;
    ////            ASSERT(varType == TYPE_INTEGER);
    ////            buf >> signals;
    ////            numRead++;
    //        } else {
    //            error("Received unhandled pedestrian subscription result");
    //        }
    //    }
    //
    //    // bail out if we didn't want to receive these subscription results
    //    if (!isSubscribed) return;
    //
    //    // make sure we got updates for all attributes
    //    if (numRead != 4) return;
    //
    //    Coord p = connection->traci2omnet(TraCICoord(px, py));
    //    if ((p.x < 0) || (p.y < 0)) error("received bad node position (%.2f, %.2f), translated to (%.2f, %.2f)", px, py, p.x, p.y);
    //
    //    double angle = connection->traci2omnetAngle(angle_traci);
    //
    //    cModule* mod = getManagedModule(objectId);
    //
    //    // is it in the ROI?
    //    bool inRoi = isInRegionOfInterest(TraCICoord(px, py), edge, speed, angle);
    //    if (!inRoi)
    //    {
    //        if (mod)
    //        {
    //            deleteManagedModule(objectId);
    //            cout << "Pedestrian #" << objectId << " left region of interest" << endl;
    //        }
    //        else if(unEquippedHosts.find(objectId) != unEquippedHosts.end())
    //        {
    //            unEquippedHosts.erase(objectId);
    //            cout << "Pedestrian (unequipped) # " << objectId<< " left region of interest" << endl;
    //        }
    //        return;
    //    }
    //
    //    if (isModuleUnequipped(objectId))
    //    {
    //        return;
    //    }
    //
    //    if (!mod)
    //    {
    //        // no such module - need to create
    //        TraCIScenarioManager::addModule(objectId, pedModuleType, pedModuleName, pedModuleDisplayString, p, edge, speed, angle);
    //        cout << "Added pedestrian #" << objectId << endl;
    //    }
    //    else
    //    {
    //        // module existed - update position
    //        for (cModule::SubmoduleIterator iter(mod); !iter.end(); iter++)
    //        {
    //            cModule* submod = iter();
    //            ifInetTraCIMobilityCallNextPosition(submod, p, edge, speed, angle);
    //            TraCIMobility* mm = dynamic_cast<TraCIMobility*>(submod);
    //            if (!mm) continue;
    //            cout << "module " << objectId << " moving to " << p.x << "," << p.y << endl;
    //            mm->nextPosition(p, edge, speed, angle);
    //        }
    //    }
    //

}


// for vehicles and bikes only (not pedestrians!)
void TraCI_App::addModule(std::string nodeId, std::string type, std::string name, std::string displayString, const Coord& position, std::string road_id, double speed, double angle)
{
    std::string vehType = vehicleGetTypeID(nodeId);  // get vehicle type
    int SUMOControllerType = vehicleTypeGetControllerType(vehType);   // get controller type

    if(SUMOControllerType == SUMO_TAG_CF_KRAUSS || SUMOControllerType == SUMO_TAG_CF_ACC || SUMOControllerType == SUMO_TAG_CF_CACC)
    {
        // this is default (do nothing)
        // type is "c3po.ned.vehicle"
        // name is "V"
    }
    else if (vehType == "TypeBicycle")
    {
        type = bikeModuleType;
        name =  bikeModuleName;
        displayString = bikeModuleDisplayString;
    }
    else
        error("Unknown module type in TraCI_App::addModule");

    TraCIScenarioManager::addModule(nodeId, type, name, displayString, position, road_id, speed, angle);
}


void TraCI_App::vehiclesData()
{
    // get all lanes in the network
    std::list<std::string> myList = laneGetIDList();

    for(std::list<std::string>::iterator i = myList.begin(); i != myList.end(); ++i)
    {
        // get all vehicles on lane i
        std::list<std::string> myList2 = laneGetLastStepVehicleIDs( i->c_str() );

        for(std::list<std::string>::reverse_iterator k = myList2.rbegin(); k != myList2.rend(); ++k)
            saveVehicleData(k->c_str());
    }

    // increase index after writing data for all vehicles
    if (vehicleGetIDCount() > 0)
        index++;
}


void TraCI_App::saveVehicleData(std::string vID)
{
    double timeStep = (simTime()-updateInterval).dbl();
    std::string vType = vehicleGetTypeID(vID);
    std::string lane = vehicleGetLaneID(vID);
    double pos = vehicleGetLanePosition(vID);
    double speed = vehicleGetSpeed(vID);
    double accel = vehicleGetCurrentAccel(vID);
    int CFMode_Enum = vehicleGetCarFollowingMode(vID);
    std::string CFMode;

    enum CFMODES {
        Mode_Undefined,
        Mode_NoData,
        Mode_DataLoss,
        Mode_SpeedControl,
        Mode_GapControl,
        Mode_EmergencyBrake,
        Mode_Stopped
    };

    switch(CFMode_Enum)
    {
    case Mode_Undefined:
        CFMode = "Undefined";
        break;
    case Mode_NoData:
        CFMode = "NoData";
        break;
    case Mode_DataLoss:
        CFMode = "DataLoss";
        break;
    case Mode_SpeedControl:
        CFMode = "SpeedControl";
        break;
    case Mode_GapControl:
        CFMode = "GapControl";
        break;
    case Mode_EmergencyBrake:
        CFMode = "EmergencyBrake";
        break;
    case Mode_Stopped:
        CFMode = "Stopped";
        break;
    default:
        error("Not a valid CFModel!");
        break;
    }

    // get the timeGap setting
    double timeGapSetting = vehicleGetTimeGap(vID);

    // get the gap
    std::vector<std::string> vleaderIDnew = vehicleGetLeader(vID, 900);
    std::string vleaderID = vleaderIDnew[0];
    double spaceGap = -1;

    if(vleaderID != "")
        spaceGap = atof( vleaderIDnew[1].c_str() );

    // calculate timeGap (if leading is present)
    double timeGap = -1;

    if(vleaderID != "" && speed != 0)
        timeGap = spaceGap / speed;

    // get the TLid that controls this vehicle
    // empty string means the vehicle is not controlled by any TLid
    std::string TLid = "";
    for (std::list<std::string>::iterator it = TLList.begin() ; it != TLList.end(); ++it)
    {
        std::list<std::string> lan = TLGetControlledLanes(*it);
        for(std::list<std::string>::iterator it2 = lan.begin(); it2 != lan.end(); ++it2)
        {
            if(*it2 == lane)
            {
                TLid = *it;
                break;
            }
        }
    }

    // if the signal is yellow or red
    int YorR = vehicleGetTrafficLightAhead(vID);

    VehicleData *tmp = new VehicleData(index, timeStep,
            vID.c_str(), vType.c_str(),
            lane.c_str(), pos,
            speed, accel, CFMode.c_str(),
            timeGapSetting, spaceGap, timeGap,
            TLid.c_str(), YorR);
    Vec_vehiclesData.push_back(*tmp);
}


void TraCI_App::vehiclesDataToFile()
{
    boost::filesystem::path filePath;

    if( ev.isGUI() )
    {
        filePath = "results/gui/vehicleData.txt";
    }
    else
    {
        // get the current run number
        int currentRun = ev.getConfigEx()->getActiveRunNumber();
        std::ostringstream fileName;

        if(useDetailedFilenames)
        {
            int TLMode = (*router->net->TLs.begin()).second->TLLogicMode;
            std::ostringstream filePrefix;
            filePrefix << router->totalVehicleCount << "_" << router->nonReroutingVehiclePercent << "_" << TLMode;
            fileName << filePrefix.str() << "_vehicleData.txt";
        }
        else
        {
            fileName << currentRun << "_vehicleData.txt";
        }

        filePath = "results/cmd/" + fileName.str();
    }

    FILE *filePtr = fopen (filePath.string().c_str(), "w");

    // write header
    fprintf (filePtr, "%-10s","index");
    fprintf (filePtr, "%-12s","timeStep");
    fprintf (filePtr, "%-15s","vehicleName");
    fprintf (filePtr, "%-17s","vehicleType");
    fprintf (filePtr, "%-12s","lane");
    fprintf (filePtr, "%-11s","pos");
    fprintf (filePtr, "%-12s","speed");
    fprintf (filePtr, "%-12s","accel");
    fprintf (filePtr, "%-20s","CFMode");
    fprintf (filePtr, "%-20s","timeGapSetting");
    fprintf (filePtr, "%-10s","SpaceGap");
    fprintf (filePtr, "%-16s","timeGap");
    fprintf (filePtr, "%-17s","TLid");
    fprintf (filePtr, "%-17s\n\n","yellowOrRed");

    int oldIndex = -1;

    // write body
    for(std::vector<VehicleData>::iterator y = Vec_vehiclesData.begin(); y != Vec_vehiclesData.end(); y++)
    {
        if(oldIndex != y->index)
        {
            fprintf(filePtr, "\n");
            oldIndex = y->index;
        }

        fprintf (filePtr, "%-10d ", y->index);
        fprintf (filePtr, "%-10.2f ", y->time );
        fprintf (filePtr, "%-15s ", y->vehicleName.c_str());
        fprintf (filePtr, "%-15s ", y->vehicleType.c_str());
        fprintf (filePtr, "%-12s ", y->lane.c_str());
        fprintf (filePtr, "%-10.2f ", y->pos);
        fprintf (filePtr, "%-10.2f ", y->speed);
        fprintf (filePtr, "%-10.2f ", y->accel);
        fprintf (filePtr, "%-20s", y->CFMode.c_str());
        fprintf (filePtr, "%-20.2f ", y->timeGapSetting);
        fprintf (filePtr, "%-10.2f ", y->spaceGap);
        fprintf (filePtr, "%-16.2f ", y->timeGap);
        fprintf (filePtr, "%-17s ", y->TLid.c_str());
        fprintf (filePtr, "%-17d \n", y->YorR);
    }

    fclose(filePtr);
}

}
