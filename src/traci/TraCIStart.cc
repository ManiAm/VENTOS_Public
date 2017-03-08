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

#undef ev
#include "boost/filesystem.hpp"
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

    if (stage == 1)
    {
        active = par("active").boolValue();
        debug = par("debug");
        terminate = par("terminate").doubleValue();

        // no need to bring up SUMO
        if(!active)
        {
            updateInterval = 1;
            executeOneTimestepTrigger = new omnetpp::cMessage("step");
            scheduleAt(updateInterval, executeOneTimestepTrigger);
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
            addNode_module = omnetpp::getSimulation()->getSystemModule()->getSubmodule("addNode");
            ASSERT(addNode_module);

            autoShutdown = par("autoShutdown");
            penetrationRate = par("penetrationRate").doubleValue();
            equilibrium_vehicle = par("equilibrium_vehicle").boolValue();

            if(simStartDateTime == "")
            {
                // current date/time based on current system
                // is show the number of sec since January 1,1970
                time_t now = time(0);

                tm *ltm = localtime(&now);

                std::ostringstream dateTime;
                dateTime << boost::format("%4d%02d%02d-%02d:%02d:%02d") %
                        (1900 + ltm->tm_year) %
                        (1 + ltm->tm_mon) %
                        (ltm->tm_mday) %
                        (ltm->tm_hour) %
                        (ltm->tm_min) %
                        (ltm->tm_sec);

                simStartDateTime = dateTime.str();
                simStartTime = std::chrono::high_resolution_clock::now();
            }

            record_sim_stat = par("record_sim_stat").boolValue();

            init_traci();

            // should be called after init_traci()
            if(record_sim_stat)
            {
                std::string record_sim_list = par("record_sim_list").stringValue();
                // make sure record_sim_list is not empty
                if(record_sim_list == "")
                    throw omnetpp::cRuntimeError("record_sim_list is empty");

                // applications are separated by |
                std::vector<std::string> records;
                boost::split(records, record_sim_list, boost::is_any_of("|"));

                // iterate over each application
                for(std::string appl : records)
                {
                    if(appl == "")
                        throw omnetpp::cRuntimeError("Invalid record_sim_list format");

                    // remove leading and trailing spaces from the string
                    boost::trim(appl);

                    // convert to lower case
                    boost::algorithm::to_lower(appl);

                    record_sim_tokenize.push_back(appl);
                }

                // record simulation data right after TraCI establishment
                record_Sim_data();
            }

            executeOneTimestepTrigger = new omnetpp::cMessage("step");
            scheduleAt(updateInterval, executeOneTimestepTrigger);
        }
    }
}


void TraCI_Start::finish()
{
    super::finish();

    // record simulation data one last time before closing TraCI
    if(!TraCIclosed && record_sim_stat)
        record_Sim_data();

    save_Sim_data_toFile();
    save_Veh_data_toFile();

    // if the TraCI link was not closed due to error
    if(!TraCIclosed)
    {
        // close TraCI interface with SUMO
        if (connection)
            close_TraCI_connection();
    }

    while (hosts.begin() != hosts.end())
        deleteManagedModule(hosts.begin()->first);

    cancelAndDelete(executeOneTimestepTrigger);

    delete connection;
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
            allPedestrians.clear();
            allPedestrians = personGetIDList();
            if(!allPedestrians.empty())
                addPedestrian();

            // record simulation data after proceeding one time step
            if(record_sim_stat)
                record_Sim_data();
        }

        // notify other modules to run one simulation TS
        omnetpp::simsignal_t Signal_executeEachTS = registerSignal("executeEachTS");
        this->emit(Signal_executeEachTS, 0);

        // we reached max simtime and should terminate OMNET++ simulation
        if(terminate != -1 && omnetpp::simTime().dbl() >= terminate)
            terminate_simulation();

        scheduleAt(omnetpp::simTime() + updateInterval, executeOneTimestepTrigger);
    }
    else
        super::handleMessage(msg);
}


void TraCI_Start::init_traci()
{
    std::string switches = "";

    if(par("quitOnEnd").boolValue())
        switches = switches + " --quit-on-end";

    if(par("startAfterLoading").boolValue())
        switches = switches + " --start";

    if(par("CMDstepLog").boolValue())
        switches = switches + " --no-step-log";

    int seed = par("seed").longValue();

    bool runSUMO = par("runSUMO").boolValue();

    std::string appl = par("SUMOapplication").stringValue();
    if(appl != "sumo" && appl != "sumoD" && appl != "sumo-gui" && appl != "sumo-guiD")
        throw omnetpp::cRuntimeError("SUMOapplication parameter is not set correctly!: %s", appl.c_str());

    // always use sumo in the command-line mode
    if(!omnetpp::cSimulation::getActiveEnvir()->isGUI())
        appl = "sumo";

    // start 'SUMO TraCI server' first
    int port = TraCIConnection::startSUMO(getFullPath_SUMOExe(appl), getFullPath_SUMOConfig(), switches, seed, runSUMO);

    // then connect to the 'SUMO TraCI server'
    connection = TraCIConnection::connect("localhost", port, runSUMO);

    // get the version of SUMO TraCI API
    std::pair<uint32_t, std::string> versionS = getVersion();
    uint32_t apiVersionS = versionS.first;
    std::string serverVersionS = versionS.second;

    LOG_INFO << boost::format("    TraCI server \"%1%\" reports API version %2% \n") % serverVersionS % apiVersionS << std::flush;

    if (apiVersionS != 14)
        throw omnetpp::cRuntimeError("Unsupported TraCI server API version!");

    updateInterval = (double)simulationGetTimeStep() / 1000.;
    if(updateInterval <= 0)
        throw omnetpp::cRuntimeError("step-length value should be >0");

    // query road network boundaries from SUMO
    simBoundary_t boundaries = simulationGetNetBoundary();

    LOG_INFO << boost::format("    TraCI reports network boundaries (%1%,%2%)-(%3%,%4%) \n") % boundaries.x1 % boundaries.y1 % boundaries.x2 % boundaries.y2 << std::flush;

    netbounds1 = TraCICoord(boundaries.x1, boundaries.y1);
    netbounds2 = TraCICoord(boundaries.x2, boundaries.y2);

    if ((traci2omnetCoord(netbounds2).x > world->getPgs()->x) || (traci2omnetCoord(netbounds1).y > world->getPgs()->y))
        LOG_WARNING << boost::format("  WARNING: Playground size (%1%,%2%) might be too small for vehicle at network bounds (%3%,%4%) \n") % world->getPgs()->x % world->getPgs()->y % traci2omnetCoord(netbounds2).x % traci2omnetCoord(netbounds1).y;

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
                shape.push_back(traci2omnetCoord(point));

            obstacles->addFromTypeAndShape(id, typeId, shape);
        }
    }

    LOG_INFO << "    Initializing modules with TraCI support ... \n\n" << std::flush;

    omnetpp::simsignal_t Signal_initialize_withTraCI = registerSignal("initialize_withTraCI");
    this->emit(Signal_initialize_withTraCI, 1);

    // should be called after sending the 'initialize_withTraCI' signal
    init_roi();
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
            TraCICoord center_SUMO = omnet2traciCoord(center);

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

                auto it = departureArrival.find(idstring);
                if(it != departureArrival.end())
                    throw omnetpp::cRuntimeError("%s was added before!", idstring.c_str());
                departureArrivalEntry_t entry = {omnetpp::simTime().dbl(), -1};
                departureArrival[idstring] = entry;

                if(equilibrium_vehicle)
                {
                    // saving information for later use
                    departedNodes node = {};

                    node.vehicleId = idstring;
                    node.vehicleTypeId = vehicleGetTypeID(idstring);
                    node.routeId = vehicleGetRouteID(idstring);
                    node.pos = 0; /*vehicleGetLanePosition(idstring)*/  // todo
                    node.speed = 0; /*vehicleGetSpeed(idstring)*/       // todo
                    node.lane = vehicleGetLaneIndex(idstring);

                    auto it = departedVehicles.find(idstring);
                    if(it != departedVehicles.end())
                        throw omnetpp::cRuntimeError("%s was added before!", idstring.c_str());
                    else
                        departedVehicles.insert(std::make_pair(idstring, node));
                }
            }

            departedVehicleCount += count;
            activeVehicleCount += count;
            drivingVehicleCount += count;
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

                if(unEquippedHosts.find(idstring) != unEquippedHosts.end())
                    unEquippedHosts.erase(idstring);

                auto it = departureArrival.find(idstring);
                if(it == departureArrival.end())
                    throw omnetpp::cRuntimeError("cannot find %s in the departureArrival map!", idstring.c_str());
                it->second.arrival = omnetpp::simTime().dbl();

                // update vehicle status one last time
                record_Veh_data(idstring, true);

                if(equilibrium_vehicle)
                {
                    auto it = departedVehicles.find(idstring);
                    if(it == departedVehicles.end())
                        throw omnetpp::cRuntimeError("cannot find %s in the departedVehicles map!", idstring.c_str());

                    departedNodes node = it->second;

                    LOG_INFO << boost::format("t=%1%: %2% of type %3% arrived. Inserting it again on edge %4% in pos %5% with entrySpeed of %6% from lane %7% \n")
                    % omnetpp::simTime().dbl() % node.vehicleId % node.vehicleTypeId % node.routeId % node.pos % node.speed % node.lane << std::flush;

                    departedVehicles.erase(it);  // remove this entry before adding
                    vehicleAdd(node.vehicleId, node.vehicleTypeId, node.routeId, (omnetpp::simTime().dbl() * 1000)+1, node.pos, node.speed, node.lane);
                }

                // check if this object has been deleted already (e.g. because it was outside the ROI)
                cModule* mod = getManagedModule(idstring);
                if (mod) deleteManagedModule(idstring);
            }

            arrivedVehicleCount += count;
            activeVehicleCount -= count;
            drivingVehicleCount -= count;

            // should we stop simulation?
            if(autoShutdown)
            {
                // terminate only if equilibrium_vehicle is off
                if(!equilibrium_vehicle)
                {
                    // get number of vehicles and bikes
                    int count1 = simulationGetMinExpectedNumber();
                    // get number of pedestrians
                    int count2 = personGetIDCount();

                    // terminate if all departed vehicles have arrived
                    if (count > 0 && count1 + count2 == 0)
                        terminate_simulation();
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
                if (mod) deleteManagedModule(idstring);

                if(unEquippedHosts.find(idstring) != unEquippedHosts.end())
                    unEquippedHosts.erase(idstring);
            }

            activeVehicleCount -= count;
            drivingVehicleCount -= count;
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
            }

            activeVehicleCount += count;
            drivingVehicleCount += count;
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
            }

            parkingVehicleCount += count;
            drivingVehicleCount -= count;
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
            }

            parkingVehicleCount -= count;
            drivingVehicleCount += count;
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

            if(count > activeVehicleCount)
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
                if(count < activeVehicleCount)
                {
                    // check if this object has been deleted already
                    cModule* mod = getManagedModule(*i);
                    if (mod) deleteManagedModule(*i);

                    if(unEquippedHosts.find(*i) != unEquippedHosts.end())
                        unEquippedHosts.erase(*i);

                    activeVehicleCount--;
                    drivingVehicleCount--;
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

    Coord p = traci2omnetCoord(TraCICoord(px, py));
    if ((p.x < 0) || (p.y < 0))
        throw omnetpp::cRuntimeError("received bad node position (%.2f, %.2f), translated to (%.2f, %.2f)", px, py, p.x, p.y);

    double angle = traci2omnetAngle(angle_traci);

    cModule* mod = getManagedModule(objectId);

    // is it in the ROI?
    bool inRoi = isInRegionOfInterest(TraCICoord(px, py), edge, speed, angle);
    if (!inRoi)
    {
        // vehicle leaving the region of interest
        if (mod)
            deleteManagedModule(objectId);
        // vehicle (un-equipped) leaving the region of interest
        else if(unEquippedHosts.find(objectId) != unEquippedHosts.end())
            unEquippedHosts.erase(objectId);

        return;
    }

    if (isModuleUnequipped(objectId))
        return;

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

    // collecting data for this vehicle in this timeStep
    record_Veh_data(objectId);
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

    cModule* nic = mod->getSubmodule("nic");
    if (nic)
        cc->unregisterNic(nic);

    // remove mapping of SUMO id and OMNET++ id
    auto i1 = SUMOid_OMNETid_mapping.find(nodeId);
    if(i1 == SUMOid_OMNETid_mapping.end())
        throw omnetpp::cRuntimeError("SUMO id %s does not exist in the network!", nodeId.c_str());
    SUMOid_OMNETid_mapping.erase(i1);

    // remove mapping of OMNET++ id and SUMO id
    auto i2 = OMNETid_SUMOid_mapping.find(mod->getFullName());
    if(i2 == OMNETid_SUMOid_mapping.end())
        throw omnetpp::cRuntimeError("OMNET++ id %s does not exist in the network!", mod->getFullName());
    OMNETid_SUMOid_mapping.erase(i2);

    // remove mapping of SUMO id and IPv4 -- for emulated vehicle
    auto i3 = SUMOid_ipv4_mapping.find(nodeId);
    // if this vehicle is emulated
    if(i3 != SUMOid_ipv4_mapping.end())
    {
        std::string ipv4 = i3->second;
        SUMOid_ipv4_mapping.erase(i3);

        // then remove mapping of IPv4 and OMNET id
        auto i4 = ipv4_OMNETid_mapping.find(ipv4);
        if(i4 == ipv4_OMNETid_mapping.end())
            throw omnetpp::cRuntimeError("IP address '%s' does not exist in the map!", ipv4.c_str());
        ipv4_OMNETid_mapping.erase(i4);
    }

    hosts.erase(nodeId);
    mod->callFinish();
    mod->deleteModule();
}


bool TraCI_Start::isModuleUnequipped(std::string nodeId)
{
    if (unEquippedHosts.find(nodeId) == unEquippedHosts.end())
        return false;

    return true;
}


void TraCI_Start::addModule(std::string nodeId /*sumo id*/, const Coord& position /*omnet coordinates*/, std::string road_id, double speed, double angle)
{
    if (hosts.find(nodeId) != hosts.end())
        throw omnetpp::cRuntimeError("tried adding duplicate module");

    double option1 = hosts.size() / (hosts.size() + unEquippedHosts.size() + 1.0);
    double option2 = (hosts.size() + 1) / (hosts.size() + unEquippedHosts.size() + 1.0);

    if (fabs(option1 - penetrationRate) < fabs(option2 - penetrationRate))
    {
        unEquippedHosts.insert(nodeId);
        return;
    }

    std::string vClass = vehicleGetClass(nodeId);

    // reserved for obstacles
    if(vClass == "custom1")
    {
        addVehicle(nodeId,
                addNode_module->par("obstacle_ModuleType").stringValue(),
                addNode_module->par("obstacle_ModuleName").stdstringValue(),
                addNode_module->par("obstacle_ModuleDisplayString").stdstringValue(),
                vClass,
                position,
                road_id,
                speed,
                angle);
    }
    // all motor vehicles
    else if(vClass == "passenger" || vClass == "private" || vClass == "emergency" || vClass == "bus" || vClass == "truck")
    {
        addVehicle(nodeId,
                addNode_module->par("vehicle_ModuleType").stdstringValue(),
                addNode_module->par("vehicle_ModuleName").stdstringValue(),
                addNode_module->par("vehicle_ModuleDisplayString").stdstringValue(),
                vClass,
                position,
                road_id,
                speed,
                angle);
    }
    else if(vClass == "bicycle")
    {
        addVehicle(nodeId,
                addNode_module->par("bike_ModuleType").stringValue(),
                addNode_module->par("bike_ModuleName").stringValue(),
                addNode_module->par("bike_ModuleDisplayString").stringValue(),
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

        //addNode_module->par("ped_ModuleType").stringValue();
        //addNode_module->par("ped_ModuleName").stringValue();
        //addNode_module->par("ped_ModuleDisplayString").stringValue();
    }
    else
        throw omnetpp::cRuntimeError("Unknown vClass '%s' for vehicle '%s'", vClass.c_str(), nodeId.c_str());
}


omnetpp::cModule* TraCI_Start::addVehicle(std::string SUMOID, std::string type, std::string name, std::string displayString, std::string vClass, const Coord& position, std::string road_id, double speed, double angle)
{
    cModule* parentmod = getParentModule();
    if (!parentmod)
        throw omnetpp::cRuntimeError("Parent Module not found");

    omnetpp::cModuleType* nodeType = omnetpp::cModuleType::get(type.c_str());
    if (!nodeType)
        throw omnetpp::cRuntimeError("Module Type \"%s\" not found", type.c_str());

    //TODO: this trashes the vectsize member of the cModule, although nobody seems to use it
    int32_t nodeVectorIndex = nextNodeVectorIndex++;
    cModule* mod = nodeType->create(name.c_str(), parentmod, nodeVectorIndex, nodeVectorIndex);
    mod->finalizeParameters();
    mod->getDisplayString().parse(displayString.c_str());
    mod->buildInside();

    bool hasOBU_val = false;
    std::string IPaddress_val = "";
    auto ii = SUMOid_ipv4_mapping.find(SUMOID);
    if(ii != SUMOid_ipv4_mapping.end())
    {
        hasOBU_val = true;
        IPaddress_val = ii->second;

        // save ipAddress <--> omnetId mapping
        auto jj = ipv4_OMNETid_mapping.find(ii->second);
        if(jj != ipv4_OMNETid_mapping.end())
            throw omnetpp::cRuntimeError("IP address '%s' is not unique!", ii->second.c_str());
        ipv4_OMNETid_mapping[ii->second] = mod->getFullName();
    }

    mod->par("SUMOID") = SUMOID;
    mod->par("SUMOType") = vehicleGetTypeID(SUMOID);
    mod->par("vehicleClass") = vClass;

    mod->par("hasOBU") = hasOBU_val;
    mod->par("IPaddress") = IPaddress_val;

    // update initial position in obstacles
    if(vClass == "custom1")
    {
        mod->getSubmodule("mobility")->par("x") = position.x;
        mod->getSubmodule("mobility")->par("y") = position.y;
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

    // save mapping of SUMO id and OMNET++ id
    auto i1 = SUMOid_OMNETid_mapping.find(SUMOID);
    if(i1 != SUMOid_OMNETid_mapping.end())
        throw omnetpp::cRuntimeError("SUMO id %s already exists in the network!", SUMOID.c_str());
    SUMOid_OMNETid_mapping[SUMOID] = mod->getFullName();

    // save mapping of OMNET++ id and SUMO id
    auto i2 = OMNETid_SUMOid_mapping.find(mod->getFullName());
    if(i2 != OMNETid_SUMOid_mapping.end())
        throw omnetpp::cRuntimeError("OMNET++ id %s already exists in the network!", mod->getFullName());
    OMNETid_SUMOid_mapping[mod->getFullName()] = SUMOID;

    auto it = record_status.find(SUMOID);
    if(it == record_status.end())
    {
        bool active = mod->par("record_stat").boolValue();
        std::string record_list = mod->par("record_list").stringValue();

        // make sure record_list is not empty
        if(record_list == "")
            throw omnetpp::cRuntimeError("record_list is empty in vehicle '%s'", SUMOID.c_str());

        // applications are separated by |
        std::vector<std::string> records;
        boost::split(records, record_list, boost::is_any_of("|"));

        // iterate over each application
        std::vector<std::string> record_tokenize;
        for(std::string appl : records)
        {
            if(appl == "")
                throw omnetpp::cRuntimeError("Invalid record_list format in vehicle '%s'", SUMOID.c_str());

            // remove leading and trailing spaces from the string
            boost::trim(appl);

            // convert to lower case
            boost::algorithm::to_lower(appl);

            record_tokenize.push_back(appl);
        }

        veh_status_entry_t entry = {active, record_tokenize};
        record_status[SUMOID] = entry;
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


void TraCI_Start::record_Sim_data()
{
    sim_status_entry_t entry = {};

    entry.timeStep = (omnetpp::simTime()-updateInterval).dbl();
    entry.loaded = -1;
    entry.departed = -1;
    entry.arrived = -1;
    entry.running = -1;
    entry.waiting = -1;

    for(std::string record : record_sim_tokenize)
    {
        if(record == "loaded")
            entry.loaded = simulationGetLoadedVehiclesCount();
        else if(record == "departed")
            entry.departed = departedVehicleCount;
        else if(record == "arrived")
            entry.arrived = arrivedVehicleCount;
        else if(record == "running")
            entry.running = activeVehicleCount;
        else if(record == "waiting")
            entry.waiting = simulationGetMinExpectedNumber() - activeVehicleCount;
        else
            throw omnetpp::cRuntimeError("'%s' is not a valid record name", record.c_str());
    }

    sim_record_status.push_back(entry);
}


void TraCI_Start::save_Sim_data_toFile()
{
    if(sim_record_status.empty())
        return;

    int currentRun = omnetpp::getEnvir()->getConfigEx()->getActiveRunNumber();

    std::ostringstream fileName;
    fileName << boost::format("%03d_simData.txt") % currentRun;

    boost::filesystem::path filePath ("results");
    filePath /= fileName.str();

    FILE *filePtr = fopen (filePath.c_str(), "w");
    if (!filePtr)
        throw omnetpp::cRuntimeError("Cannot create file '%s'", filePath.c_str());

    // write simulation parameters at the beginning of the file
    {
        // get the current config name
        std::string configName = omnetpp::getEnvir()->getConfigEx()->getVariable("configname");

        std::string iniFile = omnetpp::getEnvir()->getConfigEx()->getVariable("inifile");

        // PID of the simulation process
        std::string processid = omnetpp::getEnvir()->getConfigEx()->getVariable("processid");

        // globally unique identifier for the run, produced by
        // concatenating the configuration name, run number, date/time, etc.
        std::string runID = omnetpp::getEnvir()->getConfigEx()->getVariable("runid");

        // get number of total runs in this config
        int totalRun = omnetpp::getEnvir()->getConfigEx()->getNumRunsInConfig(configName.c_str());

        // get the current run number
        int currentRun = omnetpp::getEnvir()->getConfigEx()->getActiveRunNumber();

        // get all iteration variables
        std::vector<std::string> iterVar = omnetpp::getEnvir()->getConfigEx()->unrollConfig(configName.c_str(), false);

        // write to file
        fprintf (filePtr, "configName      %s\n", configName.c_str());
        fprintf (filePtr, "iniFile         %s\n", iniFile.c_str());
        fprintf (filePtr, "processID       %s\n", processid.c_str());
        fprintf (filePtr, "runID           %s\n", runID.c_str());
        fprintf (filePtr, "totalRun        %d\n", totalRun);
        fprintf (filePtr, "currentRun      %d\n", currentRun);
        fprintf (filePtr, "currentConfig   %s\n", iterVar[currentRun].c_str());
        fprintf (filePtr, "sim timeStep    %u ms\n", simulationGetTimeStep());
        fprintf (filePtr, "startDateTime   %s\n", simulationGetStartTime().c_str());
        fprintf (filePtr, "endDateTime     %s\n", simulationGetEndTime().c_str());
        fprintf (filePtr, "duration        %s\n\n\n", simulationGetDuration().c_str());
    }

    // write column title
    fprintf (filePtr, "%-15s","index");
    fprintf (filePtr, "%-15s","timeStep");
    for(std::string column : record_sim_tokenize)
        fprintf (filePtr, "%-15s", column.c_str());
    fprintf (filePtr, "\n\n");

    double oldTime = -2;
    int index = 0;

    for(auto &y : sim_record_status)
    {
        if(oldTime != y.timeStep)
        {
            fprintf(filePtr, "\n");
            oldTime = y.timeStep;
            index++;
        }

        fprintf (filePtr, "%-15d", index);
        fprintf (filePtr, "%-15.2f", y.timeStep );

        for(std::string record : record_sim_tokenize)
        {
            if(record == "loaded")
                fprintf (filePtr, "%-15ld", y.loaded);
            else if(record == "departed")
                fprintf (filePtr, "%-15ld", y.departed);
            else if(record == "arrived")
                fprintf (filePtr, "%-15ld", y.arrived);
            else if(record == "running")
                fprintf (filePtr, "%-15ld", y.running);
            else if(record == "waiting")
                fprintf (filePtr, "%-15ld", y.waiting);
            else
            {
                fclose(filePtr);
                throw omnetpp::cRuntimeError("'%s' is not a valid record name", record.c_str());
            }
        }

        fprintf (filePtr, "\n");
    }

    fclose(filePtr);
}


// is called in each time step for each vehicle
void TraCI_Start::record_Veh_data(std::string vID, bool arrived)
{
    auto it = record_status.find(vID);
    if(it == record_status.end())
        return;

    if(!it->second.active)
        return;

    for(auto i = collected_veh_data.rbegin(); i != collected_veh_data.rend(); i++)
    {
        // looking for the last entry of vID
        if(i->vehId == vID)
        {
            // if vehicle has arrived
            if(arrived)
            {
                // then update the arrival/route duration
                i->arrival = (omnetpp::simTime()-updateInterval).dbl();
                i->routeDuration = i->arrival - i->departure;
                return;
            }
            else
            {
                // make sure that the arrival time is -1
                if(i->arrival == -1)
                    break;

                throw omnetpp::cRuntimeError("veh '%s' has already arrived!", vID.c_str());
            }
        }
    }

    veh_data_entry entry = {};

    entry.timeStep = (omnetpp::simTime()-updateInterval).dbl();
    entry.vehId = "n/a";
    entry.vehType = "n/a";
    entry.lane = "n/a";
    entry.lanePos = -1;
    entry.speed = -1;
    entry.accel = std::numeric_limits<double>::infinity();
    entry.departure = 0;
    entry.arrival = -1;
    entry.route = "";
    entry.routeDuration = 0;
    entry.drivingDistance = -1;
    entry.CFMode = "n/a";
    entry.timeGapSetting = -1;
    entry.timeGap = -2;
    entry.frontSpaceGap = -2;
    entry.rearSpaceGap = -2;
    entry.nextTLId = "n/a";
    entry.nextTLLinkStat = '\0';

    static int columnNumber = 0;
    for(std::string record : it->second.record_list)
    {
        if(record == "vehid")
            entry.vehId = vID;
        else if(record == "vehtype")
            entry.vehType = vehicleGetTypeID(vID);
        else if(record == "lane")
            entry.lane = vehicleGetLaneID(vID);
        else if(record == "lanepos")
            entry.lanePos = vehicleGetLanePosition(vID);
        else if(record == "speed")
            entry.speed = vehicleGetSpeed(vID);
        else if(record == "accel")
            entry.accel = vehicleGetCurrentAccel(vID);
        else if(record == "departure")
            entry.departure = vehicleGetDepartureTime(vID);
        else if(record == "arrival")
            entry.arrival = vehicleGetArrivalTime(vID);
        else if(record == "route")
        {
            // convert vector of string to std::string
            std::string route_edges = "' ";
            for (auto &s : vehicleGetRoute(vID)) { route_edges = route_edges + s + " "; }
            route_edges += "'";

            entry.route = route_edges;
        }
        else if(record == "routeduration")
        {
            double departure = vehicleGetDepartureTime(vID);
            if(departure != -1)
                entry.routeDuration = (omnetpp::simTime() - departure - updateInterval).dbl();
        }
        else if(record == "drivingdistance")
            entry.drivingDistance = vehicleGetDrivingDistance(vID);
        else if(record == "cfmode")
        {
            CFMODES_t CFMode_Enum = vehicleGetCarFollowingMode(vID);
            switch(CFMode_Enum)
            {
            case Mode_Undefined:
                entry.CFMode = "Undefined";
                break;
            case Mode_NoData:
                entry.CFMode = "NoData";
                break;
            case Mode_DataLoss:
                entry.CFMode = "DataLoss";
                break;
            case Mode_SpeedControl:
                entry.CFMode = "SpeedControl";
                break;
            case Mode_GapControl:
                entry.CFMode = "GapControl";
                break;
            case Mode_EmergencyBrake:
                entry.CFMode = "EmergencyBrake";
                break;
            case Mode_Stopped:
                entry.CFMode = "Stopped";
                break;
            default:
                throw omnetpp::cRuntimeError("Not a valid CFModel!");
                break;
            }
        }
        else if(record == "timegapsetting")
            entry.timeGapSetting = vehicleGetTimeGap(vID);
        else if(record == "timegap")
        {
            double speed = (entry.speed != -1) ? entry.speed : vehicleGetSpeed(vID);

            auto leader = vehicleGetLeader(vID, 900);
            double spaceGap = (leader.leaderID != "") ? leader.distance2Leader : -1;

            // calculate timeGap (if leading is present)
            if(leader.leaderID != "" && speed != 0)
                entry.timeGap = spaceGap / speed;
            else
                entry.timeGap = -1;
        }
        else if(record == "frontspacegap")
        {
            auto leader = vehicleGetLeader(vID, 900);
            entry.frontSpaceGap = (leader.leaderID != "") ? leader.distance2Leader : -1;
        }
        else if(record == "rearspacegap")
        {

        }
        else if(record == "nexttlid")
        {
            std::vector<TL_info_t> res = vehicleGetNextTLS(vID);

            if(!res.empty())
                entry.nextTLId = res[0].TLS_id;
            else
                entry.nextTLId = "";
        }
        else if(record == "nexttllinkstat")
        {
            std::vector<TL_info_t> res = vehicleGetNextTLS(vID);

            if(!res.empty())
                entry.nextTLLinkStat = res[0].linkState;
            else
                entry.nextTLLinkStat = 'n';
        }
        else
            throw omnetpp::cRuntimeError("'%s' is not a valid record name in veh '%s'", record.c_str(), vID.c_str());

        auto it = allColumns.find(record);
        if(it == allColumns.end())
        {
            allColumns[record] = columnNumber;
            columnNumber++;
        }
    }

    collected_veh_data.push_back(entry);
}


void TraCI_Start::save_Veh_data_toFile()
{
    if(collected_veh_data.empty())
        return;

    // file name for saving vehicles statistics
    std::string veh_stat_file = par("veh_stat_file").stringValue();

    boost::filesystem::path filePath ("results");

    // no file name specified
    if(veh_stat_file == "")
    {
        int currentRun = omnetpp::getEnvir()->getConfigEx()->getActiveRunNumber();

        std::ostringstream fileName;
        fileName << boost::format("%03d_vehicleData.txt") % currentRun;

        filePath /= fileName.str();
    }
    else
        filePath /= veh_stat_file;

    FILE *filePtr = fopen (filePath.c_str(), "w");
    if (!filePtr)
        throw omnetpp::cRuntimeError("Cannot create file '%s'", filePath.c_str());

    // write simulation parameters at the beginning of the file
    {
        // get the current config name
        std::string configName = omnetpp::getEnvir()->getConfigEx()->getVariable("configname");

        std::string iniFile = omnetpp::getEnvir()->getConfigEx()->getVariable("inifile");

        // PID of the simulation process
        std::string processid = omnetpp::getEnvir()->getConfigEx()->getVariable("processid");

        // globally unique identifier for the run, produced by
        // concatenating the configuration name, run number, date/time, etc.
        std::string runID = omnetpp::getEnvir()->getConfigEx()->getVariable("runid");

        // get number of total runs in this config
        int totalRun = omnetpp::getEnvir()->getConfigEx()->getNumRunsInConfig(configName.c_str());

        // get the current run number
        int currentRun = omnetpp::getEnvir()->getConfigEx()->getActiveRunNumber();

        // get all iteration variables
        std::vector<std::string> iterVar = omnetpp::getEnvir()->getConfigEx()->unrollConfig(configName.c_str(), false);

        // write to file
        fprintf (filePtr, "configName      %s\n", configName.c_str());
        fprintf (filePtr, "iniFile         %s\n", iniFile.c_str());
        fprintf (filePtr, "processID       %s\n", processid.c_str());
        fprintf (filePtr, "runID           %s\n", runID.c_str());
        fprintf (filePtr, "totalRun        %d\n", totalRun);
        fprintf (filePtr, "currentRun      %d\n", currentRun);
        fprintf (filePtr, "currentConfig   %s\n", iterVar[currentRun].c_str());
        fprintf (filePtr, "sim timeStep    %u ms\n", simulationGetTimeStep());
        fprintf (filePtr, "startDateTime   %s\n", simulationGetStartTime().c_str());
        fprintf (filePtr, "endDateTime     %s\n", simulationGetEndTime().c_str());
        fprintf (filePtr, "duration        %s\n\n\n", simulationGetDuration().c_str());
    }

    std::string columns_sorted[allColumns.size()];
    for(auto it : allColumns)
        columns_sorted[it.second] = it.first;

    // write column title
    fprintf (filePtr, "%-10s","index");
    fprintf (filePtr, "%-12s","timeStamp");
    for(std::string column : columns_sorted)
        fprintf (filePtr, "%-20s", column.c_str());
    fprintf (filePtr, "\n\n");

    double oldTime = -2;
    int index = 0;

    for(auto &y : collected_veh_data)
    {
        if(oldTime != y.timeStep)
        {
            fprintf(filePtr, "\n");
            oldTime = y.timeStep;
            index++;
        }

        fprintf (filePtr, "%-10d", index);
        fprintf (filePtr, "%-12.2f", y.timeStep );

        for(std::string record : columns_sorted)
        {
            if(record == "vehid")
                fprintf (filePtr, "%-20s", y.vehId.c_str());
            else if(record == "vehtype")
                fprintf (filePtr, "%-20s", y.vehType.c_str());
            else if(record == "lane")
                fprintf (filePtr, "%-20s", y.lane.c_str());
            else if(record == "lanepos")
                fprintf (filePtr, "%-20.2f", y.lanePos);
            else if(record == "speed")
                fprintf (filePtr, "%-20.2f", y.speed);
            else if(record == "accel")
                fprintf (filePtr, "%-20.2f", y.accel);
            else if(record == "departure")
                fprintf (filePtr, "%-20.2f", y.departure);
            else if(record == "arrival")
                fprintf (filePtr, "%-20.2f", y.arrival);
            else if(record == "route")
                fprintf (filePtr, "%-20s", y.route.c_str());
            else if(record == "routeduration")
                fprintf (filePtr, "%-20.2f", y.routeDuration);
            else if(record == "drivingdistance")
                fprintf (filePtr, "%-20.2f", y.drivingDistance);
            else if(record == "cfmode")
                fprintf (filePtr, "%-20s", y.CFMode.c_str());
            else if(record == "timegapsetting")
                fprintf (filePtr, "%-20.2f", y.timeGapSetting);
            else if(record == "timegap")
                fprintf (filePtr, "%-20.2f", y.timeGap);
            else if(record == "frontspacegap")
                fprintf (filePtr, "%-20.2f", y.frontSpaceGap);
            else if(record == "rearspacegap")
                fprintf (filePtr, "%-20.2f", y.rearSpaceGap);
            else if(record == "nexttlid")
                fprintf (filePtr, "%-20s", y.nextTLId.c_str());
            else if(record == "nexttllinkstat")
                fprintf (filePtr, "%-20c", y.nextTLLinkStat);
            else
            {
                fclose(filePtr);
                throw omnetpp::cRuntimeError("'%s' is not a valid record name in veh '%s'", record.c_str(), y.vehId.c_str());
            }
        }

        fprintf (filePtr, "\n");
    }

    fclose(filePtr);
}

}
