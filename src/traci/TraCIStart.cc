/****************************************************************************/
/// @file    TraCIStart.h
/// @author  Mani Amoozadeh   <maniam@ucdavis.edu>
/// @author  Christoph Sommer <mail@christoph-sommer.de>
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

#include <cmath>
#include <algorithm>
#include <iomanip>
#include <boost/algorithm/string.hpp>

#include "traci/TraCIStart.h"
#include "traci/TraCIConstants.h"
#include "traci/TraCIScenarioManagerInet.h"
#include "mobility/TraCIMobility.h"

#include "MIXIM_veins/obstacle/ObstacleControl.h"
#include "router/Router.h"
#include "logging/VENTOS_logging.h"

namespace VENTOS {

Define_Module(VENTOS::TraCI_Start);

TraCI_Start::TraCI_Start()
{

}


void TraCI_Start::initialize(int stage)
{
    super::initialize(stage);

    // this code should be initialized at the last last stage
    if (stage == 2)
    {
        active = par("active").boolValue();
        debug = par("debug");
        terminateTime = par("terminateTime").doubleValue();

        // no need to bring up SUMO
        if(!active)
        {
            updateInterval = 1;
        }
        else
        {
            cModule *module = omnetpp::getSimulation()->getSystemModule()->getSubmodule("world");
            world = static_cast<BaseWorldUtility*>(module);
            ASSERT(world);

            module = omnetpp::getSimulation()->getSystemModule()->getSubmodule("connMan");
            cc = static_cast<ConnectionManager*>(module);
            ASSERT(cc);

            // get a pointer to the AddNode module
            module = omnetpp::getSimulation()->getSystemModule()->getSubmodule("addNode");
            ADDNODE = static_cast<AddNode*>(module);
            ASSERT(ADDNODE);

            // get a pointer to the Statistics module
            module = omnetpp::getSimulation()->getSystemModule()->getSubmodule("statistics");
            STAT = static_cast<Statistics*>(module);
            ASSERT(STAT);

            Signal_departed_vehs = registerSignal("vehicleDepartedSignal");
            this->subscribe("vehicleDepartedSignal", this);

            Signal_arrived_vehs = registerSignal("vehicleArrivedSignal");
            this->subscribe("vehicleArrivedSignal", this);

            Signal_module_added = registerSignal("vehicleModuleAddedSignal");
            omnetpp::getSimulation()->getSystemModule()->subscribe("vehicleModuleAddedSignal", this);

            Signal_module_deleted = registerSignal("vehicleModuleDeletedSignal");
            omnetpp::getSimulation()->getSystemModule()->subscribe("vehicleModuleDeletedSignal", this);

            autoTerminate = par("autoTerminate");
            equilibrium_vehicle = par("equilibrium_vehicle").boolValue();

            init_traci();

            init_obstacles();

            // should be called after 'init_traci'
            init_roi();
        }

        executeOneTimestepTrigger = new omnetpp::cMessage("step");
        scheduleAt(updateInterval, executeOneTimestepTrigger);
    }
}


void TraCI_Start::finish()
{
    super::finish();

    // if the TraCI link was not closed due to error
    if(!TraCIclosed)
    {
        // close TraCI interface with SUMO
        if (connection)
            close_TraCI_connection();
    }

    while (hosts.begin() != hosts.end())
        deleteManagedModule(hosts.begin()->first);

    // todo: for some reasons canceling this message when
    // TraCI is not active throws exception!
    if(active)
        cancelAndDelete(executeOneTimestepTrigger);

    delete connection;
    connection = NULL;

    // flush all output buffer
    LOG_FLUSH;
    GLOG_FLUSH_ALL;
}


void TraCI_Start::handleMessage(omnetpp::cMessage *msg)
{
    if (msg == executeOneTimestepTrigger)
    {
        if (active)
        {
            // get current simulation time (in ms)
            uint32_t targetTime = static_cast<uint32_t>(round(omnetpp::simTime().dbl() * 1000));
            ASSERT(targetTime > 0);

            // proceed SUMO simulation to advance to targetTime
            auto output = simulationTimeStep(targetTime);

            for (uint32_t i = 0; i < output.second /*number of subscription results*/; ++i)
                processSubcriptionResult(output.first);

            // todo: should get pedestrians using subscription
            //            allPedestrians.clear();
            //            allPedestrians = personGetIDList();
            //            if(!allPedestrians.empty())
            //                addPedestrian();
        }

        // notify other modules to run one simulation TS
        omnetpp::simsignal_t Signal_executeEachTS = registerSignal("executeEachTimeStepSignal");
        this->emit(Signal_executeEachTS, 0);

        // we reached max simtime and should terminate OMNET++ simulation
        if(terminateTime != -1 && omnetpp::simTime().dbl() >= terminateTime)
            simulationTerminate();

        scheduleAt(omnetpp::simTime() + updateInterval, executeOneTimestepTrigger);
    }
    else
        super::handleMessage(msg);
}


void TraCI_Start::receiveSignal(omnetpp::cComponent *source, omnetpp::simsignal_t signalID, const char *SUMOID, cObject* details)
{
    Enter_Method_Silent();

    if(signalID == Signal_departed_vehs)
    {
        recordDeparture(SUMOID);

        if(equilibrium_vehicle)
        {
            // saving information for later use
            departedNodes node = {};

            node.vehicleId = SUMOID;
            node.vehicleTypeId = vehicleGetTypeID(SUMOID);
            node.routeId = vehicleGetRouteID(SUMOID);
            node.pos = 0; /*vehicleGetLanePosition(SUMOID)*/  // todo
            node.speed = 0; /*vehicleGetSpeed(SUMOID)*/       // todo
            node.lane = vehicleGetLaneIndex(SUMOID);

            auto it = departedVehicles.find(SUMOID);
            if(it != departedVehicles.end())
                throw omnetpp::cRuntimeError("%s was added before!", SUMOID);
            else
                departedVehicles.insert(std::make_pair(SUMOID, node));
        }
    }
    else if(signalID == Signal_arrived_vehs)
    {
        recordArrival(SUMOID);

        if(equilibrium_vehicle)
        {
            auto it = departedVehicles.find(SUMOID);
            if(it == departedVehicles.end())
                throw omnetpp::cRuntimeError("cannot find %s in the departedVehicles map!", SUMOID);

            departedNodes node = it->second;

            LOG_INFO << boost::format("t=%1%: %2% of type %3% arrived. Inserting it again on edge %4% in pos %5% with entrySpeed of %6% from lane %7% \n")
            % omnetpp::simTime().dbl() % node.vehicleId % node.vehicleTypeId % node.routeId % node.pos % node.speed % node.lane << std::flush;

            departedVehicles.erase(it);  // remove this entry before adding
            vehicleAdd(node.vehicleId, node.vehicleTypeId, node.routeId, (omnetpp::simTime().dbl() * 1000)+1, node.pos, node.speed, node.lane);
        }
    }
}


void TraCI_Start::receiveSignal(omnetpp::cComponent *source, omnetpp::simsignal_t signalID, cObject *obj, cObject *details)
{
    Enter_Method_Silent();

    // note that this signal can be emitted more than once for a vehicle
    // when that vehicle enters/exits ROI (region of interest)
    if(signalID == Signal_module_added)
    {
        omnetpp::cModule *vehModule = static_cast<omnetpp::cModule *>(obj);
        ASSERT(vehModule);

        std::string SUMOID = vehModule->par("SUMOID");
        ASSERT(SUMOID != "");

        addMapping(SUMOID, vehModule->getFullName());
        addMapping_emulated(SUMOID);
    }
    else if(signalID == Signal_module_deleted)
    {
        omnetpp::cModule *vehModule = static_cast<omnetpp::cModule *>(obj);
        ASSERT(vehModule);

        std::string SUMOID = vehModule->par("SUMOID");
        ASSERT(SUMOID != "");

        removeMapping(SUMOID);
        removeMapping_emulated(SUMOID);
    }
}


void TraCI_Start::init_traci()
{
    // always use sumo in the command-line mode
    if(!omnetpp::cSimulation::getActiveEnvir()->isGUI())
        par("SUMOapplication").setStringValue("sumo");

    // make sure the sumo application is correct
    std::string appl = par("SUMOapplication").stringValue();
    if(appl != "sumo" && appl != "sumoD" && appl != "sumo-gui" && appl != "sumo-guiD")
        throw omnetpp::cRuntimeError("SUMOapplication parameter is not set correctly!: %s", appl.c_str());

    int remotePort = par("remotePort").longValue();

    int port = 0;
    if(remotePort == -1)
        port = TraCIConnection::getFreeEphemeralPort();
    else if(remotePort > 0)
        port = remotePort;
    else
        throw omnetpp::cRuntimeError("Remote port %d is invalid!", remotePort);

    std::string SUMOcommandLine = par("SUMOcommandLine").stringValue();

    // start 'SUMO TraCI server' first
    if(par("forkSUMO").boolValue())
        TraCIConnection::startSUMO(getFullPath_SUMOApplication().string(), getFullPath_SUMOConfig().string(), SUMOcommandLine, port);

    // then connect to the 'SUMO TraCI server'
    connection = TraCIConnection::connect("localhost", port);

    // get the version of SUMO TraCI API
    std::pair<uint32_t, std::string> versionS = getVersion();
    uint32_t apiVersionS = versionS.first;
    std::string serverVersionS = versionS.second;

    LOG_INFO << boost::format("    TraCI server \"%1%\" reports API version %2% \n") % serverVersionS % apiVersionS << std::flush;

    if (apiVersionS != 14)
        throw omnetpp::cRuntimeError("Unsupported TraCI server API version!");

    TraCIclosed = false;

    updateInterval = (double)simulationGetTimeStep() / 1000.;
    if(updateInterval <= 0)
        throw omnetpp::cRuntimeError("step-length value should be >0");

    LOG_INFO << boost::format("    Simulation time step is %f seconds \n") %  updateInterval << std::flush;

    // query road network boundaries from SUMO
    simBoundary_t boundaries = simulationGetNetBoundary();

    LOG_INFO << boost::format("    TraCI reports network boundaries (%1%,%2%)-(%3%,%4%) \n") % boundaries.x1 % boundaries.y1 % boundaries.x2 % boundaries.y2 << std::flush;

    netbounds1 = TraCICoord(boundaries.x1, boundaries.y1);
    netbounds2 = TraCICoord(boundaries.x2, boundaries.y2);

    if ((convertCoord_traci2omnet(netbounds2).x > world->getPgs()->x) || (convertCoord_traci2omnet(netbounds1).y > world->getPgs()->y))
        LOG_WARNING << boost::format("  WARNING: Playground size (%1%,%2%) might be too small for vehicle at network bounds (%3%,%4%) \n") %
        world->getPgs()->x %
        world->getPgs()->y %
        convertCoord_traci2omnet(netbounds2).x %
        convertCoord_traci2omnet(netbounds1).y;

    {
        // subscribe to a bunch of stuff in simulation
        std::vector<uint8_t> variables {VAR_TIME_STEP,
            VAR_DEPARTED_VEHICLES_IDS,
            VAR_ARRIVED_VEHICLES_IDS,
            VAR_TELEPORT_STARTING_VEHICLES_IDS,
            VAR_TELEPORT_ENDING_VEHICLES_IDS,
            VAR_PARKING_STARTING_VEHICLES_IDS,
            VAR_PARKING_ENDING_VEHICLES_IDS};
        TraCIBuffer buf = subscribeSimulation(0, 0x7FFFFFFF, "", variables);
        processSubcriptionResult(buf);
        ASSERT(buf.eof());
    }

    {
        // subscribe to a list of vehicle ids
        std::vector<uint8_t> variables {ID_LIST};
        TraCIBuffer buf = subscribeVehicle(0, 0x7FFFFFFF, "", variables);
        processSubcriptionResult(buf);
        ASSERT(buf.eof());
    }

    LOG_INFO << "    Initializing modules with TraCI support ... \n\n" << std::flush;

    omnetpp::simsignal_t Signal_initialize_withTraCI = registerSignal("initializeWithTraCISignal");
    this->emit(Signal_initialize_withTraCI, 1);
}


void TraCI_Start::init_obstacles()
{
    Veins::ObstacleControl* obstacles = Veins::ObstacleControlAccess().getIfExists();
    if (obstacles)
    {
        // get list of polygons
        auto ids = polygonGetIDList();

        for (auto &id : ids)
        {
            std::string typeId = polygonGetTypeID(id);

            if (!obstacles->isTypeSupported(typeId))
                continue;

            auto coords = polygonGetShape(id);

            // convert to OMNET++ coordinates
            std::vector<Coord> shape;
            for(auto &point : coords)
                shape.push_back(convertCoord_traci2omnet(point));

            obstacles->addFromTypeAndShape(id, typeId, shape);
        }
    }
}


void TraCI_Start::init_roi()
{
    std::string roiRoads_s = par("roiRoads");

    // parse roiRoads
    roiRoads.clear();
    std::istringstream roiRoads_i(roiRoads_s);
    std::string road;
    while (std::getline(roiRoads_i, road, ' '))
        roiRoads.push_back(road);

    std::string roiRects_s = par("roiRects");

    // parse roiRects
    roiRects.clear();
    std::istringstream roiRects_i(roiRects_s);
    std::string rect;
    while (std::getline(roiRects_i, rect, ' '))
    {
        std::istringstream rect_i(rect);

        double x1; rect_i >> x1; ASSERT(rect_i);
        char c1; rect_i >> c1; ASSERT(rect_i);
        double y1; rect_i >> y1; ASSERT(rect_i);
        char c2; rect_i >> c2; ASSERT(rect_i);
        double x2; rect_i >> x2; ASSERT(rect_i);
        char c3; rect_i >> c3; ASSERT(rect_i);
        double y2; rect_i >> y2; ASSERT(rect_i);

        // make sure coordinates are correct
        ASSERT(x2 > x1);
        ASSERT(y2 > y1);

        roiRects.push_back(std::make_pair(TraCICoord(x1, y1), TraCICoord(x2, y2)));
    }

    // this should be called after sending initialize_withTraCI so that all RSUs are built
    if(par("roiSquareSizeRSU").doubleValue() > 0)
    {
        // get a pointer to the first RSU
        omnetpp::cModule *module = omnetpp::getSimulation()->getSystemModule()->getSubmodule("RSU", 0);
        // no RSUs in the network
        if(module == NULL)
            return;

        // how many RSUs are in the network?
        int RSUcount = module->getVectorSize();

        // iterate over RSUs
        for(int i = 0; i < RSUcount; ++i)
        {
            module = omnetpp::getSimulation()->getSystemModule()->getSubmodule("RSU", i)->getSubmodule("mobility");

            Coord center (module->par("x").doubleValue(), module->par("y").doubleValue());
            TraCICoord center_SUMO = convertCoord_omnet2traci(center);

            double squSize = par("roiSquareSizeRSU").doubleValue();

            // calculate corners of the roi square
            TraCICoord bottomLeft = TraCICoord(center_SUMO.x - squSize, center_SUMO.y - squSize);
            TraCICoord topRight = TraCICoord(center_SUMO.x + squSize, center_SUMO.y + squSize);

            // add them into roiRects
            roiRects.push_back(std::pair<TraCICoord, TraCICoord>(bottomLeft, topRight));
        }
    }

    // draws a polygon in SUMO to show the roi
    int roiCount = 0;
    for(auto i : roiRects)
    {
        TraCICoord bottomLeft = i.first;
        TraCICoord topRight = i.second;

        // calculate the other two corners
        TraCICoord topLeft = TraCICoord(bottomLeft.x, topRight.y);
        TraCICoord bottomRight = TraCICoord(topRight.x, bottomLeft.y);

        // draw roi in SUMO
        std::list<TraCICoord> detectionRegion;

        detectionRegion.push_back(topLeft);
        detectionRegion.push_back(topRight);
        detectionRegion.push_back(bottomRight);
        detectionRegion.push_back(bottomLeft);
        detectionRegion.push_back(topLeft);

        roiCount++;
        std::string polID = "roi_" + std::to_string(roiCount);

        polygonAdd(polID, "region", Color::colorNameToRGB("green"), 0, 1, detectionRegion);
    }
}


void TraCI_Start::processSubcriptionResult(TraCIBuffer& buf)
{
    uint8_t cmdLength_resp; buf >> cmdLength_resp;
    uint32_t cmdLengthExt_resp; buf >> cmdLengthExt_resp;
    uint8_t commandId_resp; buf >> commandId_resp;
    std::string objectId_resp; buf >> objectId_resp;

    if (commandId_resp == RESPONSE_SUBSCRIBE_SIM_VARIABLE)
        processSimSubscription(objectId_resp, buf);
    else if (commandId_resp == RESPONSE_SUBSCRIBE_VEHICLE_VARIABLE)
        processVehicleSubscription(objectId_resp, buf);
    else
        throw omnetpp::cRuntimeError("Received unhandled subscription result");
}


void TraCI_Start::processSimSubscription(std::string objectId, TraCIBuffer& buf)
{
    uint8_t variableNumber_resp; buf >> variableNumber_resp;
    for (uint8_t j = 0; j < variableNumber_resp; ++j)
    {
        uint8_t variable1_resp; buf >> variable1_resp;
        uint8_t isokay; buf >> isokay;
        if (isokay != RTYPE_OK)
        {
            uint8_t varType; buf >> varType;
            ASSERT(varType == TYPE_STRING);
            std::string description; buf >> description;
            if (isokay == RTYPE_NOTIMPLEMENTED)
                throw omnetpp::cRuntimeError("TraCI server reported subscribing to variable 0x%2x not implemented (\"%s\"). Might need newer version.", variable1_resp, description.c_str());

            throw omnetpp::cRuntimeError("TraCI server reported error subscribing to variable 0x%2x (\"%s\").", variable1_resp, description.c_str());
        }

        if (variable1_resp == VAR_TIME_STEP)
        {
            uint8_t varType; buf >> varType;
            ASSERT(varType == TYPE_INTEGER);
            uint32_t serverTimestep; buf >> serverTimestep; // serverTimestep: current timestep reported by server in ms
            uint32_t omnetTimestep = static_cast<uint32_t>(round(omnetpp::simTime().dbl() * 1000));
            ASSERT(omnetTimestep == serverTimestep);
        }
        else if (variable1_resp == VAR_DEPARTED_VEHICLES_IDS)
        {
            uint8_t varType; buf >> varType;
            ASSERT(varType == TYPE_STRINGLIST);
            uint32_t count; buf >> count;  // count: number of departed vehicles in this time step
            for (uint32_t i = 0; i < count; ++i)
            {
                std::string idstring; buf >> idstring;
                // adding modules is handled on the fly when entering/leaving the ROI

                // signal to those interested that this vehicle has departed
                omnetpp::simsignal_t Signal_departed = registerSignal("vehicleDepartedSignal");
                this->emit(Signal_departed, idstring.c_str());

                STAT->departedVehicleCount++;
                STAT->activeVehicleCount++;
                STAT->drivingVehicleCount++;
            }
        }
        else if (variable1_resp == VAR_ARRIVED_VEHICLES_IDS)
        {
            uint8_t varType; buf >> varType;
            ASSERT(varType == TYPE_STRINGLIST);
            uint32_t count; buf >> count;  // count: number of arrived vehicles
            for (uint32_t i = 0; i < count; ++i)
            {
                std::string idstring; buf >> idstring;

                if (subscribedVehicles.find(idstring) != subscribedVehicles.end())
                {
                    subscribedVehicles.erase(idstring);

                    // unsubscribe
                    std::vector<uint8_t> variables;
                    TraCIBuffer buf = subscribeVehicle(0, 0x7FFFFFFF, idstring, variables);
                    ASSERT(buf.eof());
                }

                // signal to those interested that this vehicle has arrived
                omnetpp::simsignal_t Signal_arrived = registerSignal("vehicleArrivedSignal");
                this->emit(Signal_arrived, idstring.c_str());

                // check if this object has been deleted already (e.g. because it was outside the ROI)
                cModule* mod = getManagedModule(idstring);
                if (mod) deleteManagedModule(idstring);

                STAT->arrivedVehicleCount++;
                STAT->activeVehicleCount--;
                STAT->drivingVehicleCount--;
            }

            // should we stop simulation?
            if(autoTerminate)
            {
                // terminate only if equilibrium_vehicle is off
                if(!equilibrium_vehicle)
                {
                    if(count > 0)
                    {
                        // get number of vehicles and bikes
                        int count1 = simulationGetMinExpectedNumber();
                        // get number of pedestrians
                        int count2 = personGetIDCount();

                        // terminate if all departed vehicles have arrived
                        if (count1 + count2 == 0)
                            simulationTerminate();
                    }
                }
            }
        }
        else if (variable1_resp == VAR_TELEPORT_STARTING_VEHICLES_IDS)
        {
            uint8_t varType; buf >> varType;
            ASSERT(varType == TYPE_STRINGLIST);
            uint32_t count; buf >> count;   // count: number of vehicles starting to teleport
            for (uint32_t i = 0; i < count; ++i)
            {
                std::string idstring; buf >> idstring;

                // check if this object has been deleted already (e.g. because it was outside the ROI)
                cModule* mod = getManagedModule(idstring);
                if (mod)
                {
                    deleteManagedModule(idstring);

                    STAT->activeVehicleCount--;
                    STAT->drivingVehicleCount--;
                }

                LOG_WARNING << boost::format(">>> WARNING: '%s' started teleporting at '%.3f'. \n") %
                        idstring %
                        (omnetpp::simTime().dbl() - updateInterval) << std::flush;
            }
        }
        else if (variable1_resp == VAR_TELEPORT_ENDING_VEHICLES_IDS)
        {
            uint8_t varType; buf >> varType;
            ASSERT(varType == TYPE_STRINGLIST);
            uint32_t count; buf >> count;  // count: number of vehicles ending teleport
            for (uint32_t i = 0; i < count; ++i)
            {
                std::string idstring; buf >> idstring;
                // adding modules is handled on the fly when entering/leaving the ROI

                STAT->activeVehicleCount++;
                STAT->drivingVehicleCount++;

                // we do not create omnet++ module here once teleporting is finished.
                // we let processVehicleSubscription to take care of this!
            }
        }
        else if (variable1_resp == VAR_PARKING_STARTING_VEHICLES_IDS)
        {
            uint8_t varType; buf >> varType;
            ASSERT(varType == TYPE_STRINGLIST);
            uint32_t count; buf >> count;  // count: number of vehicles starting to park
            for (uint32_t i = 0; i < count; ++i)
            {
                std::string idstring; buf >> idstring;

                cModule* mod = getManagedModule(idstring);
                for (cModule::SubmoduleIterator iter(mod); !iter.end(); iter++)
                {
                    cModule* submod = *iter;
                    TraCIMobilityMod* mm = dynamic_cast<TraCIMobilityMod*>(submod);
                    if (!mm) continue;
                    mm->changeParkingState(true);
                }

                STAT->parkingVehicleCount++;
                STAT->drivingVehicleCount--;
            }
        }
        else if (variable1_resp == VAR_PARKING_ENDING_VEHICLES_IDS)
        {
            uint8_t varType; buf >> varType;
            ASSERT(varType == TYPE_STRINGLIST);
            uint32_t count; buf >> count;  // count: number of vehicles ending to park
            for (uint32_t i = 0; i < count; ++i)
            {
                std::string idstring; buf >> idstring;

                cModule* mod = getManagedModule(idstring);
                for (cModule::SubmoduleIterator iter(mod); !iter.end(); iter++)
                {
                    cModule* submod = *iter;
                    TraCIMobilityMod* mm = dynamic_cast<TraCIMobilityMod*>(submod);
                    if (!mm) continue;
                    mm->changeParkingState(false);
                }

                STAT->parkingVehicleCount--;
                STAT->drivingVehicleCount++;
            }
        }
        else
            throw omnetpp::cRuntimeError("Received unhandled sim subscription result");
    }
}


void TraCI_Start::processVehicleSubscription(std::string objectId, TraCIBuffer& buf)
{
    bool isSubscribed = (subscribedVehicles.find(objectId) != subscribedVehicles.end());
    double px;
    double py;
    std::string edge;
    double speed;
    double angle_traci;
    int vehSignals;
    int numRead = 0;

    uint8_t variableNumber_resp; buf >> variableNumber_resp;
    for (uint8_t j = 0; j < variableNumber_resp; ++j)
    {
        uint8_t variable1_resp; buf >> variable1_resp;
        uint8_t isokay; buf >> isokay;

        if (isokay != RTYPE_OK)
        {
            uint8_t varType; buf >> varType;
            ASSERT(varType == TYPE_STRING);
            std::string errormsg; buf >> errormsg;
            if (isSubscribed)
            {
                if (isokay == RTYPE_NOTIMPLEMENTED)
                    throw omnetpp::cRuntimeError("TraCI server reported subscribing to vehicle variable 0x%2x not implemented (\"%s\"). Might need newer version.", variable1_resp, errormsg.c_str());

                throw omnetpp::cRuntimeError("TraCI server reported error subscribing to vehicle variable 0x%2x (\"%s\").", variable1_resp, errormsg.c_str());
            }
        }
        else if (variable1_resp == ID_LIST)
        {
            uint8_t varType; buf >> varType;
            ASSERT(varType == TYPE_STRINGLIST);
            uint32_t count; buf >> count;  // count: number of active vehicles

            if(count > STAT->activeVehicleCount)
                throw omnetpp::cRuntimeError("SUMO is reporting a higher vehicle count.");

            std::set<std::string> drivingVehicles;
            for (uint32_t i = 0; i < count; ++i)
            {
                std::string idstring; buf >> idstring;
                drivingVehicles.insert(idstring);
            }

            // check for vehicles that need subscribing to
            std::set<std::string> needSubscribe;
            std::set_difference(drivingVehicles.begin(), drivingVehicles.end(), subscribedVehicles.begin(), subscribedVehicles.end(), std::inserter(needSubscribe, needSubscribe.begin()));
            for (std::set<std::string>::const_iterator i = needSubscribe.begin(); i != needSubscribe.end(); ++i)
            {
                subscribedVehicles.insert(*i);

                // subscribe to some attributes of the vehicle
                std::vector<uint8_t> variables {VAR_POSITION, VAR_ROAD_ID, VAR_SPEED, VAR_ANGLE, VAR_SIGNALS};
                TraCIBuffer buf = subscribeVehicle(0, 0x7FFFFFFF, *i, variables);
                processSubcriptionResult(buf);
                ASSERT(buf.eof());
            }

            // check for vehicles that need unsubscribing from
            std::set<std::string> needUnsubscribe;
            std::set_difference(subscribedVehicles.begin(), subscribedVehicles.end(), drivingVehicles.begin(), drivingVehicles.end(), std::inserter(needUnsubscribe, needUnsubscribe.begin()));
            for (std::set<std::string>::const_iterator i = needUnsubscribe.begin(); i != needUnsubscribe.end(); ++i)
            {
                subscribedVehicles.erase(*i);

                // unsubscribe
                std::vector<uint8_t> variables;
                TraCIBuffer buf = subscribeVehicle(0, 0x7FFFFFFF, *i, variables);
                ASSERT(buf.eof());

                // vehicle removal (manual or due to an accident)
                if(count < STAT->activeVehicleCount)
                {
                    // check if this object has been deleted already
                    cModule* mod = getManagedModule(*i);
                    if (mod) deleteManagedModule(*i);

                    STAT->activeVehicleCount--;
                    STAT->drivingVehicleCount--;
                }
            }
        }
        else if (variable1_resp == VAR_POSITION)
        {
            uint8_t varType; buf >> varType;
            ASSERT(varType == POSITION_2D);
            buf >> px;
            buf >> py;
            numRead++;
        }
        else if (variable1_resp == VAR_ROAD_ID)
        {
            uint8_t varType; buf >> varType;
            ASSERT(varType == TYPE_STRING);
            buf >> edge;
            numRead++;
        }
        else if (variable1_resp == VAR_SPEED)
        {
            uint8_t varType; buf >> varType;
            ASSERT(varType == TYPE_DOUBLE);
            buf >> speed;
            numRead++;
        }
        else if (variable1_resp == VAR_ANGLE)
        {
            uint8_t varType; buf >> varType;
            ASSERT(varType == TYPE_DOUBLE);
            buf >> angle_traci;
            numRead++;
        }
        else if (variable1_resp == VAR_SIGNALS)
        {
            uint8_t varType; buf >> varType;
            ASSERT(varType == TYPE_INTEGER);
            buf >> vehSignals;
            numRead++;
        }
        else
            throw omnetpp::cRuntimeError("Received unhandled vehicle subscription result");
    }

    // bail out if we didn't want to receive these subscription results
    if (!isSubscribed) return;

    // make sure we got updates for all attributes
    if (numRead != 5) return;

    Coord p = convertCoord_traci2omnet(TraCICoord(px, py));
    if ((p.x < 0) || (p.y < 0))
        throw omnetpp::cRuntimeError("received bad node position (%.2f, %.2f), translated to (%.2f, %.2f)", px, py, p.x, p.y);

    double angle = convertAngle_traci2omnet(angle_traci);

    cModule* mod = getManagedModule(objectId);

    // is it in the ROI?
    bool inRoi = isInRegionOfInterest(TraCICoord(px, py), edge, speed, angle);
    if (!inRoi)
    {
        // vehicle leaving the region of interest
        if (mod)
            deleteManagedModule(objectId);

        return;
    }

    // no such module - need to create
    if (!mod)
    {
        addModule(objectId, p, edge, speed, angle);
    }
    else
    {
        // module existed - update position
        for (cModule::SubmoduleIterator iter(mod); !iter.end(); iter++)
        {
            cModule* submod = *iter;
            ifInetTraCIMobilityCallNextPosition(submod, p, edge, speed, angle);
            TraCIMobilityMod* mm = dynamic_cast<TraCIMobilityMod*>(submod);
            if (mm)
                mm->nextPosition(p, edge, speed, angle);
        }
    }
}


omnetpp::cModule* TraCI_Start::getManagedModule(std::string nodeId)
{
    if (hosts.find(nodeId) == hosts.end())
        return 0;

    return hosts[nodeId];
}


bool TraCI_Start::isInRegionOfInterest(const TraCICoord& position, std::string road_id, double speed, double angle)
{
    if ((roiRoads.size() == 0) && (roiRects.size() == 0))
        return true;

    if (roiRoads.size() > 0)
    {
        for (std::list<std::string>::const_iterator i = roiRoads.begin(); i != roiRoads.end(); ++i)
        {
            if (road_id == *i)
                return true;
        }
    }

    if (roiRects.size() > 0)
    {
        for (std::list<std::pair<TraCICoord, TraCICoord> >::const_iterator i = roiRects.begin(); i != roiRects.end(); ++i)
        {
            if ((position.x >= i->first.x) && (position.y >= i->first.y) && (position.x <= i->second.x) && (position.y <= i->second.y))
                return true;
        }
    }

    return false;
}


void TraCI_Start::deleteManagedModule(std::string nodeId /*sumo id*/)
{
    cModule* mod = getManagedModule(nodeId);
    if (!mod)
        throw omnetpp::cRuntimeError("no vehicle with Id \"%s\" found", nodeId.c_str());

    if(mod->par("DSRCenabled"))
    {
        cModule* nic = mod->getSubmodule("nic");
        if (nic)
            cc->unregisterNic(nic);
    }

    // signal to those interested that a module is deleted
    omnetpp::simsignal_t Signal_module_deleted = registerSignal("vehicleModuleDeletedSignal");
    this->emit(Signal_module_deleted, mod);

    hosts.erase(nodeId);
    mod->callFinish();
    mod->deleteModule();
}


void TraCI_Start::addModule(std::string nodeId /*sumo id*/, const Coord& position /*omnet coordinates*/, std::string road_id, double speed, double angle)
{
    if (hosts.find(nodeId) != hosts.end())
        throw omnetpp::cRuntimeError("tried adding duplicate module");

    std::string vClass = vehicleGetClass(nodeId);

    omnetpp::cModule *addedModule = NULL;

    // reserved for obstacles
    if(vClass == "custom1")
    {
        addedModule = addVehicle(nodeId,
                ADDNODE->par("obstacle_ModuleType").stringValue(),
                ADDNODE->par("obstacle_ModuleName").stdstringValue(),
                ADDNODE->par("obstacle_ModuleDisplayString").stdstringValue(),
                nextObstacleVectorIndex++,
                vClass,
                position,
                road_id,
                speed,
                angle);
    }
    // all motor vehicles
    else if(vClass == "passenger" || vClass == "private" || vClass == "emergency" || vClass == "bus" || vClass == "truck")
    {
        addedModule = addVehicle(nodeId,
                ADDNODE->par("vehicle_ModuleType").stdstringValue(),
                ADDNODE->par("vehicle_ModuleName").stdstringValue(),
                ADDNODE->par("vehicle_ModuleDisplayString").stdstringValue(),
                nextMotorVectorIndex++,
                vClass,
                position,
                road_id,
                speed,
                angle);
    }
    else if(vClass == "bicycle")
    {
        addedModule = addVehicle(nodeId,
                ADDNODE->par("bike_ModuleType").stringValue(),
                ADDNODE->par("bike_ModuleName").stringValue(),
                ADDNODE->par("bike_ModuleDisplayString").stringValue(),
                nextBikeVectorIndex++,
                vClass,
                position,
                road_id,
                speed,
                angle);
    }
    // todo
    else if(vClass == "pedestrian")
    {
        // calling addPedestrian() method?

        //ADDNODE->par("ped_ModuleType").stringValue();
        //ADDNODE->par("ped_ModuleName").stringValue();
        //ADDNODE->par("ped_ModuleDisplayString").stringValue();
    }
    else
        throw omnetpp::cRuntimeError("Unknown vClass '%s' for vehicle '%s'", vClass.c_str(), nodeId.c_str());

    // signal to those interested that a new module is interested
    omnetpp::simsignal_t Signal_module_added = registerSignal("vehicleModuleAddedSignal");
    this->emit(Signal_module_added, addedModule);
}


omnetpp::cModule* TraCI_Start::addVehicle(std::string SUMOID, std::string type, std::string name, std::string displayString, int32_t nodeVectorIndex, std::string vClass, const Coord& position, std::string road_id, double speed, double angle)
{
    cModule* parentmod = getParentModule();
    if (!parentmod)
        throw omnetpp::cRuntimeError("Parent Module not found");

    omnetpp::cModuleType* nodeType = omnetpp::cModuleType::get(type.c_str());
    if (!nodeType)
        throw omnetpp::cRuntimeError("Module Type \"%s\" not found", type.c_str());

    //TODO: this trashes the vectsize member of the cModule, although nobody seems to use it
    cModule* mod = nodeType->create(name.c_str(), parentmod, nodeVectorIndex+1 /*vector size*/, nodeVectorIndex);
    mod->finalizeParameters();
    mod->getDisplayString().parse(displayString.c_str());
    mod->buildInside();

    mod->par("SUMOID") = SUMOID;
    mod->par("SUMOType") = vehicleGetTypeID(SUMOID);
    mod->par("vehicleClass") = vClass;

    std::string IPaddress_val = vehicleId2ip(SUMOID);
    mod->par("hasOBU") = (IPaddress_val != "") ? true : false;
    mod->par("IPaddress") = IPaddress_val;

    // update initial position in obstacles
    if(vClass == "custom1")
    {
        mod->getSubmodule("mobility")->par("x") = position.x;
        mod->getSubmodule("mobility")->par("y") = position.y;
    }

    // get the DSRC status of all vehicles inserted by the AddNode module
    auto VehsDSRCStatus = ADDNODE->getVehsDSRCAttributeStatus();
    // look for this vehicle
    auto it = std::find_if(VehsDSRCStatus.begin(), VehsDSRCStatus.end(), [SUMOID](DSRC_val_t const& n)
            {return (n.SUMOID == SUMOID);});
    // if the vehicle exists
    if(it != VehsDSRCStatus.end())
    {
        if(it->DSRC_attribute_status == 0)
            mod->par("DSRCenabled") = false;
        else if(it->DSRC_attribute_status == 1)
            mod->par("DSRCenabled") = true;
        if(it->DSRC_attribute_status == -1)
        {
            // 'DSRCprob' attribute is not set for this vehicle.
            // So we do not touch the 'DSRCenabled' parameter!
            // The configuration file determines the value of 'DSRCenabled'
        }
    }

    mod->scheduleStart(omnetpp::simTime() + updateInterval);

    // pre-initialize TraCIMobilityMod
    for (cModule::SubmoduleIterator iter(mod); !iter.end(); iter++)
    {
        cModule* submod = *iter;
        ifInetTraCIMobilityCallPreInitialize(submod, SUMOID, position, road_id, speed, angle);
        TraCIMobilityMod* mm = dynamic_cast<TraCIMobilityMod*>(submod);
        if (!mm) continue;
        mm->preInitialize(SUMOID, position, road_id, speed, angle);
    }

    mod->callInitialize();
    hosts[SUMOID] = mod;

    // post-initialize TraCIMobilityMod
    for (cModule::SubmoduleIterator iter(mod); !iter.end(); iter++)
    {
        cModule* submod = *iter;
        TraCIMobilityMod* mm = dynamic_cast<TraCIMobilityMod*>(submod);
        if (!mm) continue;
        mm->changePosition();
    }

    return mod;
}


omnetpp::cModule* TraCI_Start::addPedestrian()
{
    //cout << simTime().dbl() << ": " << allPedestrians.size() << endl;

    //    // add new inserted pedestrians
    //    std::set<std::string> needSubscribe;
    //    std::set_difference(allPedestrians.begin(), allPedestrians.end(), subscribedPedestrians.begin(), subscribedPedestrians.end(), std::inserter(needSubscribe, needSubscribe.begin()));
    //    for (std::set<std::string>::const_iterator i = needSubscribe.begin(); i != needSubscribe.end(); ++i)
    //        subscribedPedestrians.insert(*i);
    //
    //    // remove pedestrians that are not present in the network
    //    std::set<std::string> needUnsubscribe;
    //    std::set_difference(subscribedPedestrians.begin(), subscribedPedestrians.end(), allPedestrians.begin(), allPedestrians.end(), std::inserter(needUnsubscribe, needUnsubscribe.begin()));
    //    for (std::set<std::string>::const_iterator i = needUnsubscribe.begin(); i != needUnsubscribe.end(); ++i)
    //        subscribedPedestrians.erase(*i);

    return NULL;
}

}
