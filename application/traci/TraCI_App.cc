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
        terminate = par("terminate").doubleValue();

        subscribedPedestrians.clear();

        bikeModuleType = par("bikeModuleType").stringValue();
        bikeModuleName = par("bikeModuleName").stringValue();
        bikeModuleDisplayString = par("bikeModuleDisplayString").stringValue();

        pedModuleType = par("pedModuleType").stringValue();
        pedModuleName = par("pedModuleName").stringValue();
        pedModuleDisplayString = par("pedModuleDisplayString").stringValue();
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

    simsignal_t Signal_executeFirstTS = registerSignal("executeFirstTS");
    nodePtr->emit(Signal_executeFirstTS, 1);
}


void TraCI_App::executeOneTimestep()
{
    // Sending Command to SUMO to perform simulation for (getCurrentTimeMs()/1000.)
    TraCIScenarioManager::executeOneTimestep();

    allPedestrians.clear();
    allPedestrians = personGetIDList();
    if(!allPedestrians.empty())
        addPedestriansToOMNET();

    // check if simulationDone flag should be set
    int NoVehAndBike = simulationGetMinExpectedNumber();
    int NoPed = personGetIDCount();
    int totalModules = NoVehAndBike + NoPed;
    bool simulationDone = (simTime().dbl() >= terminate) or (totalModules == 0);

    // notify other modules to run one simulation TS
    simsignal_t Signal_executeEachTS = registerSignal("executeEachTS");
    nodePtr->emit(Signal_executeEachTS, (long)simulationDone);

    if(simulationDone)
        endSimulation();   // terminate the simulation (TraCI connection will be closed by veins)
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


void TraCI_App::addPedestriansToOMNET()
{
    //cout << simTime().dbl() << ": " << allPedestrians.size() << endl;

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

}
