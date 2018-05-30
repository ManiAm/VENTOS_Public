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
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

#include "traci/TraCIStart.h"
#include "traci/TraCIConstants.h"
#include "traci/TraCIScenarioManagerInet.h"
#include "mobility/TraCIMobility.h"

#include "MIXIM_veins/obstacle/ObstacleControl.h"
#include "router/Router.h"
#include "logging/VENTOS_logging.h"

namespace VENTOS {

Define_Module(VENTOS::TraCI_Start);

#define STEP_MSG_KIND   30461  // reserved msg kind for step

TraCI_Start::TraCI_Start()
{

}


TraCI_Start::~TraCI_Start()
{
    cancelAndDelete(sumo_step);

    // delete all modules
    while (hosts.begin() != hosts.end())
        deleteManagedModule(hosts.begin()->first);

    // if the TraCI link was not closed due to error
    if(!TraCIclosedOnError)
    {
        // close TraCI interface with SUMO
        if (connection)
            close_TraCI_connection();
    }

    delete connection;
    connection = NULL;
}


void TraCI_Start::initialize(int stage)
{
    super::initialize(stage);

    if (stage == 0)
    {
        active = par("active").boolValue();
        debug = par("debug");
        terminateTime = par("terminateTime").doubleValue();

        sumo_step = new omnetpp::cMessage("step", STEP_MSG_KIND);
        // sumo_step has the highest priority among all other msgs scheduled in the same time
        sumo_step->setSchedulingPriority(std::numeric_limits<short>::max());
        // the first simulation time step is always at 0
        scheduleAt(0, sumo_step);

        if(active)
        {
            cModule *module = omnetpp::getSimulation()->getSystemModule()->getSubmodule("world");
            world = static_cast<BaseWorldUtility*>(module);
            ASSERT(world);

            module = omnetpp::getSimulation()->getSystemModule()->getSubmodule("connMan");
            cc = static_cast<ConnectionManager*>(module);
            ASSERT(cc);

            // get a pointer to the AddNode module
            ADDNODE = AddNode::getAddNodeInterface();

            // get a pointer to the Statistics module
            module = omnetpp::getSimulation()->getSystemModule()->getSubmodule("statistics");
            STAT = static_cast<Statistics*>(module);
            ASSERT(STAT);

            Signal_departed_vehs = registerSignal("vehicleDepartedSignal");
            this->subscribe("vehicleDepartedSignal", this);

            Signal_arrived_vehs = registerSignal("vehicleArrivedSignal");
            this->subscribe("vehicleArrivedSignal", this);

            autoTerminate = par("autoTerminate");
            equilibrium_vehicle = par("equilibrium_vehicle").boolValue();
        }
        // no need to bring up SUMO
        else
        {
            updateInterval = 1;
        }
    }
    // TraCI connection is established in the last last stage
    else if(stage == 2)
    {
        if(active)
        {
            init_traci();

            init_obstacles();

            // should be called after 'init_traci'
            init_roi();
        }
    }
}


void TraCI_Start::finish()
{
    super::finish();

    // flush all output buffer
    LOG_FLUSH;
    GLOG_FLUSH_ALL;
}


void TraCI_Start::handleMessage(omnetpp::cMessage *msg)
{
    if (msg == sumo_step)
    {
        // get current simulation time (in ms)
        uint32_t targetTime = static_cast<uint32_t>(round(omnetpp::simTime().dbl() * 1000));

        if (active)
        {
            // proceed SUMO simulation to advance to targetTime
            auto output = simulationTimeStep(targetTime);

            for (uint32_t i = 0; i < output.second /*number of subscription results*/; ++i)
                processSubcriptionResult(output.first);
        }

        // notify other modules to run one simulation TS
        // note that this signal notifies the beginning of the time step (targetTime)
        omnetpp::simsignal_t Signal_executeEachTS = registerSignal("executeEachTimeStepSignal");
        this->emit(Signal_executeEachTS, 0);

        // we reached max simtime and should terminate OMNET++ simulation
        if(terminateTime != -1 && omnetpp::simTime().dbl() >= terminateTime)
            simulationTerminate();

        scheduleAt(omnetpp::simTime() + updateInterval, sumo_step);
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
            departedNodes_t node = {};

            node.vehicleId = SUMOID;
            node.vehicleTypeId = vehicleGetTypeID(SUMOID);
            node.pos = vehicleGetLanePosition(SUMOID);
            node.speed = vehicleGetSpeed(SUMOID);
            node.lane = vehicleGetLaneIndex(SUMOID);
            node.routeId = vehicleGetRouteID(SUMOID);
            node.color = vehicleGetColor(SUMOID);  // color should be updated every time step

            auto it = equilibrium_departedVehs.find(SUMOID);
            if(it != equilibrium_departedVehs.end())
                throw omnetpp::cRuntimeError("%s was added before!", SUMOID);
            else
                equilibrium_departedVehs.insert(std::make_pair(SUMOID, node));
        }
    }
    else if(signalID == Signal_arrived_vehs)
    {
        recordArrival(SUMOID);

        if(equilibrium_vehicle)
        {
            auto it = equilibrium_departedVehs.find(SUMOID);
            if(it == equilibrium_departedVehs.end())
                throw omnetpp::cRuntimeError("cannot find %s in the equilibrium_departedVehs map!", SUMOID);

            uint32_t SUMOtime_ms = simulationGetCurrentTime();

            LOG_INFO << boost::format("Vehicle '%1%' arrived at %2%. Inserting it again at %3% ... \n") %
                    SUMOID %
                    omnetpp::simTime().dbl() %
                    (SUMOtime_ms / 1000.) << std::flush;

            // add the vehicle into SUMO
            vehicleAdd(SUMOID, it->second.vehicleTypeId, it->second.routeId, SUMOtime_ms, it->second.pos, it->second.speed, it->second.lane);

            // set the same vehicle color
            vehicleSetColor(SUMOID, it->second.color);

            std::string ipv4 = vehicleId2ip(SUMOID);
            if(ipv4 != "")
                ADDNODE->updateDeferredAttribute_ip(SUMOID, ipv4);

            equilibrium_departedVehs.erase(it);
        }
    }
}


void TraCI_Start::init_traci()
{
    // make sure the sumo application is correct
    std::string appl = par("SUMOapplication").stringValue();
    if(appl != "sumo" && appl != "sumoD" && appl != "sumo-gui" && appl != "sumo-guiD")
        throw omnetpp::cRuntimeError("SUMO application '%s' is not recognized. Make sure the Network.TraCI.SUMOapplication parameter in set correctly.", appl.c_str());

    int remotePort = par("remotePort").intValue();

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

    LOG_DEBUG << boost::format("    TraCI server \"%1%\" reports API version %2% \n") % serverVersionS % apiVersionS << std::flush;

    if (apiVersionS != 14)
        throw omnetpp::cRuntimeError("Unsupported TraCI server API version!");

    TraCIclosedOnError = false;

    updateInterval = (double)simulationGetDelta() / 1000.;
    if(updateInterval <= 0)
        throw omnetpp::cRuntimeError("step-length value should be >0");

    LOG_DEBUG << boost::format("    Simulation time step is %f seconds \n") %  updateInterval << std::flush;

    // query road network boundaries from SUMO
    simBoundary_t boundaries = simulationGetNetBoundary();

    LOG_DEBUG << boost::format("    TraCI reports network boundaries (%1%,%2%)-(%3%,%4%) \n") % boundaries.x1 % boundaries.y1 % boundaries.x2 % boundaries.y2 << std::flush;

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
        std::vector<uint8_t> variables {VAR_DEPARTED_VEHICLES_IDS, VAR_ARRIVED_VEHICLES_IDS,
            VAR_TELEPORT_STARTING_VEHICLES_IDS, VAR_TELEPORT_ENDING_VEHICLES_IDS,
            VAR_PARKING_STARTING_VEHICLES_IDS, VAR_PARKING_ENDING_VEHICLES_IDS};

        // todo: subscribe to VAR_DEPARTED_PERSON_IDS and VAR_ARRIVED_PERSON_IDS

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

    //    {
    //        // subscribe to a list of person ids
    //        std::vector<uint8_t> variables {ID_LIST};
    //        TraCIBuffer buf = subscribePerson(0, 0x7FFFFFFF, "", variables);
    //        processSubcriptionResult(buf);
    //        ASSERT(buf.eof());
    //    }

    LOG_DEBUG << "    Initializing modules with TraCI support ... \n\n" << std::flush;

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

    auto allEdges = edgeGetIDList();

    // parse roiRoads
    roiRoads.clear();
    std::istringstream roiRoads_i(roiRoads_s);
    std::string road;
    while (std::getline(roiRoads_i, road, ' '))
    {
        auto ii = std::find(allEdges.begin(), allEdges.end(), road);
        if(ii == allEdges.end())
            throw omnetpp::cRuntimeError("Edge '%s' listed in the roiRoads parameter does not exist in the network", road.c_str());

        roiRoads.push_back(road);
    }

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

    // parsing roiRectsRSU.
    // this should be done after sending initialize_withTraCI so that all RSUs are built

    std::string roiRectsRSU = par("roiRectsRSU").stringValue();

    if(roiRectsRSU != "")
    {
        // tokenize 'length' and 'width'
        std::vector<std::string> roiRectsRSU_tok;
        boost::split(roiRectsRSU_tok, roiRectsRSU, boost::is_any_of(","));

        // convert to double
        double length = 0;
        double width = 0;
        try
        {
            length = boost::lexical_cast<double>(roiRectsRSU_tok[0]);
            width = boost::lexical_cast<double>(roiRectsRSU_tok[1]);
        }
        catch (boost::bad_lexical_cast const&)
        {
            throw omnetpp::cRuntimeError("parameter 'roiRectsRSU' is badly formatted: %s", roiRectsRSU.c_str());
        }

        if(length > 0 && width > 0)
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

                // calculate corners of the roi square
                TraCICoord bottomLeft = TraCICoord(center_SUMO.x - length/2, center_SUMO.y - width/2);
                TraCICoord topRight = TraCICoord(center_SUMO.x + length/2, center_SUMO.y + width/2);

                // add them into roiRects
                roiRects.push_back(std::pair<TraCICoord, TraCICoord>(bottomLeft, topRight));
            }
        }
    }

    // draws a polygon in SUMO to show the roiRects
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

    if(commandId_resp == RESPONSE_SUBSCRIBE_SIM_VARIABLE)
        processSimSubscription(objectId_resp, buf);
    else if(commandId_resp == RESPONSE_SUBSCRIBE_VEHICLE_VARIABLE)
        processVehicleSubscription(objectId_resp, buf);
    else if(commandId_resp == RESPONSE_SUBSCRIBE_PERSON_VARIABLE)
        processPersonSubscription(objectId_resp, buf);
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

        if (variable1_resp == VAR_DEPARTED_VEHICLES_IDS)
        {
            uint8_t varType; buf >> varType;
            ASSERT(varType == TYPE_STRINGLIST);
            uint32_t numDepartedVehs; buf >> numDepartedVehs;
            for (uint32_t i = 0; i < numDepartedVehs; ++i)
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
            uint32_t numArrivedVehs; buf >> numArrivedVehs;
            for (uint32_t i = 0; i < numArrivedVehs; ++i)
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

            if(checkEndSimulation(numArrivedVehs))
                simulationTerminate();
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
        //        else if (variable1_resp == VAR_DEPARTED_PERSON_IDS)
        //        {
        //            uint8_t varType; buf >> varType;
        //            ASSERT(varType == TYPE_STRINGLIST);
        //            uint32_t numDepartedPerson; buf >> numDepartedPerson;
        //            for (uint32_t i = 0; i < numDepartedPerson; ++i)
        //            {
        //                std::string idstring; buf >> idstring;
        //                // adding modules is handled on the fly when entering/leaving the ROI
        //
        //                // signal to those interested that this vehicle has departed
        //                omnetpp::simsignal_t Signal_departed = registerSignal("personDepartedSignal");
        //                this->emit(Signal_departed, idstring.c_str());
        //
        //                STAT->departedPersonCount++;
        //                STAT->activePersonCount++;
        //            }
        //        }
        //        else if (variable1_resp == VAR_ARRIVED_PERSON_IDS)
        //        {
        //            uint8_t varType; buf >> varType;
        //            ASSERT(varType == TYPE_STRINGLIST);
        //            uint32_t numArrivedPerson; buf >> numArrivedPerson;
        //            for (uint32_t i = 0; i < numArrivedPerson; ++i)
        //            {
        //                std::string idstring; buf >> idstring;
        //
        //                if (subscribedPerson.find(idstring) != subscribedPerson.end())
        //                {
        //                    subscribedPerson.erase(idstring);
        //
        //                    // unsubscribe
        //                    std::vector<uint8_t> variables;
        //                    TraCIBuffer buf = subscribePerson(0, 0x7FFFFFFF, idstring, variables);
        //                    ASSERT(buf.eof());
        //                }
        //
        //                // signal to those interested that this vehicle has arrived
        //                omnetpp::simsignal_t Signal_arrived = registerSignal("personArrivedSignal");
        //                this->emit(Signal_arrived, idstring.c_str());
        //
        //                // check if this object has been deleted already (e.g. because it was outside the ROI)
        //                cModule* mod = getManagedModule(idstring);
        //                if (mod) deleteManagedModule(idstring);
        //
        //                STAT->arrivedPersonCount++;
        //                STAT->activePersonCount--;
        //            }
        //
        //            if(checkEndSimulation(numArrivedPerson))
        //                simulationTerminate();
        //        }
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

                // vehicle removal (with TraCI->vehicleRemove command or due to teleport)
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

    if(equilibrium_vehicle)
    {
        auto it = equilibrium_departedVehs.find(objectId);
        if(it == equilibrium_departedVehs.end())
            throw omnetpp::cRuntimeError("cannot find %s in the equilibrium_departedVehs map!", objectId.c_str());

        it->second.color = vehicleGetColor(objectId);
    }

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
        addVehicleModule(objectId, p, edge, speed, angle);
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


void TraCI_Start::processPersonSubscription(std::string objectId, TraCIBuffer& buf)
{
    //    bool isSubscribed = (subscribedPerson.find(objectId) != subscribedPerson.end());
    //    double px;
    //    double py;
    //    std::string edge;
    //    double speed;
    //    double angle_traci;
    //    int numRead = 0;
    //
    //    uint8_t variableNumber_resp; buf >> variableNumber_resp;
    //    for (uint8_t j = 0; j < variableNumber_resp; ++j)
    //    {
    //        uint8_t variable1_resp; buf >> variable1_resp;
    //        uint8_t isokay; buf >> isokay;
    //
    //        if (isokay != RTYPE_OK)
    //        {
    //            uint8_t varType; buf >> varType;
    //            ASSERT(varType == TYPE_STRING);
    //            std::string errormsg; buf >> errormsg;
    //            if (isSubscribed)
    //            {
    //                if (isokay == RTYPE_NOTIMPLEMENTED)
    //                    throw omnetpp::cRuntimeError("TraCI server reported subscribing to person variable 0x%2x not implemented (\"%s\"). Might need newer version.", variable1_resp, errormsg.c_str());
    //
    //                throw omnetpp::cRuntimeError("TraCI server reported error subscribing to person variable 0x%2x (\"%s\").", variable1_resp, errormsg.c_str());
    //            }
    //        }
    //        else if (variable1_resp == ID_LIST)
    //        {
    //            uint8_t varType; buf >> varType;
    //            ASSERT(varType == TYPE_STRINGLIST);
    //            uint32_t count; buf >> count;  // count: number of active persons
    //
    //            if(count > STAT->activePersonCount)
    //                throw omnetpp::cRuntimeError("SUMO is reporting a higher person count.");
    //
    //            std::set<std::string> activePersonSet;
    //            for (uint32_t i = 0; i < count; ++i)
    //            {
    //                std::string idstring; buf >> idstring;
    //                activePersonSet.insert(idstring);
    //            }
    //
    //            // check for person that need subscribing to
    //            std::set<std::string> needSubscribe;
    //            std::set_difference(activePersonSet.begin(), activePersonSet.end(), subscribedPerson.begin(), subscribedPerson.end(), std::inserter(needSubscribe, needSubscribe.begin()));
    //            for (std::set<std::string>::const_iterator i = needSubscribe.begin(); i != needSubscribe.end(); ++i)
    //            {
    //                subscribedPerson.insert(*i);
    //
    //                // subscribe to some attributes of the person
    //                std::vector<uint8_t> variables {VAR_POSITION, VAR_ROAD_ID, VAR_SPEED, VAR_ANGLE};
    //                TraCIBuffer buf = subscribePerson(0, 0x7FFFFFFF, *i, variables);
    //                processSubcriptionResult(buf);
    //                ASSERT(buf.eof());
    //            }
    //
    //            // check for vehicles that need unsubscribing from
    //            std::set<std::string> needUnsubscribe;
    //            std::set_difference(subscribedPerson.begin(), subscribedPerson.end(), activePersonSet.begin(), activePersonSet.end(), std::inserter(needUnsubscribe, needUnsubscribe.begin()));
    //            for (std::set<std::string>::const_iterator i = needUnsubscribe.begin(); i != needUnsubscribe.end(); ++i)
    //            {
    //                subscribedPerson.erase(*i);
    //
    //                // unsubscribe
    //                std::vector<uint8_t> variables;
    //                TraCIBuffer buf = subscribePerson(0, 0x7FFFFFFF, *i, variables);
    //                ASSERT(buf.eof());
    //
    //                // vehicle removal (with TraCI->vehicleRemove command or due to teleport)
    //                if(count < STAT->activePersonCount)
    //                {
    //                    // check if this object has been deleted already
    //                    cModule* mod = getManagedModule(*i);
    //                    if (mod) deleteManagedModule(*i);
    //
    //                    STAT->activePersonCount--;
    //                }
    //            }
    //        }
    //        else if (variable1_resp == VAR_POSITION)
    //        {
    //            uint8_t varType; buf >> varType;
    //            ASSERT(varType == POSITION_2D);
    //            buf >> px;
    //            buf >> py;
    //            numRead++;
    //        }
    //        else if (variable1_resp == VAR_ROAD_ID)
    //        {
    //            uint8_t varType; buf >> varType;
    //            ASSERT(varType == TYPE_STRING);
    //            buf >> edge;
    //            numRead++;
    //        }
    //        else if (variable1_resp == VAR_SPEED)
    //        {
    //            uint8_t varType; buf >> varType;
    //            ASSERT(varType == TYPE_DOUBLE);
    //            buf >> speed;
    //            numRead++;
    //        }
    //        else if (variable1_resp == VAR_ANGLE)
    //        {
    //            uint8_t varType; buf >> varType;
    //            ASSERT(varType == TYPE_DOUBLE);
    //            buf >> angle_traci;
    //            numRead++;
    //        }
    //        else
    //            throw omnetpp::cRuntimeError("Received unhandled person subscription result");
    //    }
    //
    //    // bail out if we didn't want to receive these subscription results
    //    if (!isSubscribed) return;
    //
    //    // make sure we got updates for all attributes
    //    if (numRead != 5) return;
    //
    //    Coord p = convertCoord_traci2omnet(TraCICoord(px, py));
    //    if ((p.x < 0) || (p.y < 0))
    //        throw omnetpp::cRuntimeError("received bad node position (%.2f, %.2f), translated to (%.2f, %.2f)", px, py, p.x, p.y);
    //
    //    double angle = convertAngle_traci2omnet(angle_traci);
    //
    //    cModule* mod = getManagedModule(objectId);
    //
    //    // is it in the ROI?
    //    bool inRoi = isInRegionOfInterest(TraCICoord(px, py), edge, speed, angle);
    //    if (!inRoi)
    //    {
    //        // person leaving the region of interest
    //        if (mod)
    //            deleteManagedModule(objectId);
    //
    //        return;
    //    }
    //
    //    // no such module - need to create
    //    if (!mod)
    //    {
    //        addPerson(objectId, p, edge, speed, angle);
    //    }
    //    else
    //    {
    //        // module existed - update position
    //        for (cModule::SubmoduleIterator iter(mod); !iter.end(); iter++)
    //        {
    //            cModule* submod = *iter;
    //            ifInetTraCIMobilityCallNextPosition(submod, p, edge, speed, angle);
    //            TraCIMobilityMod* mm = dynamic_cast<TraCIMobilityMod*>(submod);
    //            if (mm)
    //                mm->nextPosition(p, edge, speed, angle);
    //        }
    //    }
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
        throw omnetpp::cRuntimeError("no module with Id '%s' found", nodeId.c_str());

    if(mod->par("DSRCenabled"))
    {
        cModule* nic = mod->getSubmodule("nic");
        if (nic)
            cc->unregisterNic(nic);
    }

    // if this module belongs to a person
    if(std::string(mod->getName()) == std::string(ADDNODE->par("ped_ModuleName").stringValue()))
    {
        // signal to those interested that a module is deleted
        omnetpp::simsignal_t Signal_module_deleted = registerSignal("personModuleDeletedSignal");
        this->emit(Signal_module_deleted, mod);
    }
    // any other modules
    else
    {
        // signal to those interested that a module is deleted
        omnetpp::simsignal_t Signal_module_deleted = registerSignal("vehicleModuleDeletedSignal");
        this->emit(Signal_module_deleted, mod);
    }

    std::string SUMOID = mod->par("SUMOID");
    ASSERT(SUMOID != "");

    std::string ipv4 = vehicleId2ip(SUMOID);
    if(ipv4 != "")
    {
        // double check!
        std::string v_ipv4 = mod->par("IPaddress").stdstringValue();
        ASSERT(v_ipv4 == ipv4);

        // signal to those interested that an emulated module is deleted
        omnetpp::simsignal_t Signal_emulated_module_deleted = registerSignal("emulatedVehicleModuleDeletedSignal");
        this->emit(Signal_emulated_module_deleted, mod);
    }

    // remove mapping after sending the DeletedSignal
    removeMapping(SUMOID);
    removeMappingEmulated(SUMOID);

    // remove from managed hosts
    hosts.erase(nodeId);

    mod->callFinish();
    mod->deleteModule();
}


// responsible for adding obstacles, motor-vehicles (car, bus, truck, etc.) and bicycle
void TraCI_Start::addVehicleModule(std::string nodeId /*SUMOID*/, const Coord& position /*omnet coordinates*/, std::string road_id, double speed, double angle)
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
    else
        throw omnetpp::cRuntimeError("Unknown vClass '%s' for vehicle '%s'", vClass.c_str(), nodeId.c_str());

    // signal to those interested that a new module is inserted
    omnetpp::simsignal_t Signal_module_added = registerSignal("vehicleModuleAddedSignal");
    this->emit(Signal_module_added, addedModule);

    std::string ipv4 = vehicleId2ip(nodeId);
    if(ipv4 != "")
    {
        // double check!
        std::string v_ipv4 = addedModule->par("IPaddress").stdstringValue();
        ASSERT(v_ipv4 == ipv4);

        // signal to those interested that an emulated module is inserted
        omnetpp::simsignal_t Signal_emulated_module_added = registerSignal("emulatedVehicleModuleAddedSignal");
        this->emit(Signal_emulated_module_added, addedModule);
    }
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

    // update hosts map
    hosts[SUMOID] = mod;

    mod->par("SUMOID") = SUMOID;
    mod->par("SUMOType") = vehicleGetTypeID(SUMOID);
    mod->par("vehicleClass") = vClass;

    // update initial position in obstacle
    if(vClass == "custom1")
    {
        mod->getSubmodule("mobility")->par("x") = position.x;
        mod->getSubmodule("mobility")->par("y") = position.y;
    }

    if(ADDNODE->hasDeferredAttribute(SUMOID))
    {
        // get all deferred attributes for this vehicle
        veh_deferred_attributes_t def = ADDNODE->getDeferredAttribute(SUMOID);

        // if an attribute is -1, then the attribute is not defined for this vehicle.
        // So we do not touch the corresponding parameter!
        // The configuration file determines the value of parameter

        if(def.DSRC_status != -1)
            mod->par("DSRCenabled") = (bool)def.DSRC_status;

        omnetpp::cModule *appl = mod->getSubmodule("appl");

        if(def.plnMode != -1)
            appl->par("plnMode") = def.plnMode;

        if(def.plnId != "")
            appl->par("myPlnID") = def.plnId;

        if(def.plnDepth != -1)
            appl->par("myPlnDepth") = def.plnDepth;

        if(def.plnSize != -1)
            appl->par("plnSize") = def.plnSize;

        if(def.maxSize != -1)
            appl->par("maxPlatoonSize") = def.maxSize;

        if(def.optSize != -1)
            appl->par("optPlatoonSize") = def.optSize;

        // between platoons
        if(def.interGap != -1)
            appl->par("TP") = def.interGap;

        if(def.ipv4 != "")
        {
            mod->par("hasOBU") = true;
            mod->par("IPaddress") = def.ipv4;
        }

        if(def.color != "")
        {
            VENTOS::RGB newColor = VENTOS::Color::colorNameToRGB(def.color);
            vehicleSetColor(SUMOID, newColor);
        }

        ADDNODE->removeDeferredAttribute(SUMOID);
    }

    // update the mapping before calling scheduleStart.
    // this allows the initialize code of vehicles to have
    // access to the latest mapping

    addMapping(SUMOID, mod->getFullName());

    std::string ipv4 = mod->par("IPaddress");
    if(ipv4 != "")
        addMappingEmulated(ipv4, SUMOID);

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


void TraCI_Start::addPerson(std::string nodeId /*sumo id*/, const Coord& position /*omnet coordinates*/, std::string road_id, double speed, double angle)
{
    if (hosts.find(nodeId) != hosts.end())
        throw omnetpp::cRuntimeError("tried adding duplicate module");





    //    addedModule = addVehicle(nodeId,
    //            ADDNODE->par("bike_ModuleType").stringValue(),
    //            ADDNODE->par("bike_ModuleName").stringValue(),
    //            ADDNODE->par("bike_ModuleDisplayString").stringValue(),
    //            nextBikeVectorIndex++,
    //            vClass,
    //            position,
    //            road_id,
    //            speed,
    //            angle);



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

}


bool TraCI_Start::checkEndSimulation(uint32_t numArrivednodes)
{
    // at least one node should have arrived
    if(numArrivednodes == 0)
        return false;

    // is 'autoTerminate' parameter enabled?
    if(autoTerminate)
    {
        // is 'equilibrium_vehicle' parameter disabled?
        if(!equilibrium_vehicle)
        {
            // get number of motor-vehicles and bikes
            int count1 = simulationGetMinExpectedNumber();
            // get number of person
            int count2 = personGetIDCount();

            // terminate if all departed vehicles have arrived
            if (count1 + count2 == 0)
                return true;
        }
    }

    return false;
}

}
