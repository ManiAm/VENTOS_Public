//
// Copyright (C) 2006-2012 Christoph Sommer <christoph.sommer@uibk.ac.at>
//
// Second author 2: Mani Amoozadeh <maniam@ucdavis.edu>
//
// Documentation for these modules is at http://veins.car2x.org/
//
// This program is free software; you can redistribute it and/or modify
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

#include "TraCIStart.h"
#include "TraCIConstants.h"
#include "TraCIScenarioManagerInet.h"
#include "TraCIMobility.h"
#include "ObstacleControl.h"
#include "Router.h"

#include <rapidxml.hpp>
#include <rapidxml_utils.hpp>
#include <rapidxml_print.hpp>

#include <cmath>
#include <algorithm>
#include <iomanip>

// un-defining ev!
// why? http://stackoverflow.com/questions/24103469/cant-include-the-boost-filesystem-header
#undef ev
#include "boost/filesystem.hpp"
#define ev  (*cSimulation::getActiveEnvir())

#define MYDEBUG EV

namespace VENTOS {

Define_Module(VENTOS::TraCI_Start);

TraCI_Start::TraCI_Start() : world(0),
        cc(0),
        mobRng(0),
        myAddVehicleTimer(0),
        connectAndStartTrigger(0),
        executeOneTimestepTrigger(0) { }


TraCI_Start::~TraCI_Start()
{
    cancelAndDelete(connectAndStartTrigger);
    cancelAndDelete(executeOneTimestepTrigger);
    cancelAndDelete(myAddVehicleTimer);

    delete connection;
}


void TraCI_Start::initialize(int stage)
{
    TraCI_Commands::initialize(stage);

    if (stage == 1)
    {
        debug = par("debug");
        connectAt = par("connectAt");
        terminate = par("terminate").doubleValue();
        updateInterval = par("updateInterval");

        // no need to bring up SUMO
        if(connectAt == -1)
        {
            executeOneTimestepTrigger = new cMessage("step");
            scheduleAt(updateInterval, executeOneTimestepTrigger);
        }
        else
        {
            connectAndStartTrigger = new cMessage("connect");
            scheduleAt(connectAt, connectAndStartTrigger);

            firstStepAt = par("firstStepAt");
            if (firstStepAt == -1)
                firstStepAt = connectAt + updateInterval;
            ASSERT(firstStepAt > connectAt);

            executeOneTimestepTrigger = new cMessage("step");
            scheduleAt(firstStepAt, executeOneTimestepTrigger);

            host = par("host").stdstringValue();

            autoShutdown = par("autoShutdown");

            collectVehiclesData = par("collectVehiclesData").boolValue();
            vehicleDataLevel = par("vehicleDataLevel").longValue();

            vehicleModuleType = par("vehicleModuleType").stdstringValue();
            vehicleModuleName = par("vehicleModuleName").stdstringValue();
            vehicleModuleDisplayString = par("vehicleModuleDisplayString").stdstringValue();

            bikeModuleType = par("bikeModuleType").stringValue();
            bikeModuleName = par("bikeModuleName").stringValue();
            bikeModuleDisplayString = par("bikeModuleDisplayString").stringValue();

            pedModuleType = par("pedModuleType").stringValue();
            pedModuleName = par("pedModuleName").stringValue();
            pedModuleDisplayString = par("pedModuleDisplayString").stringValue();

            numVehicles = par("numVehicles").longValue();
            penetrationRate = par("penetrationRate").doubleValue();
            vehicleRngIndex = par("vehicleRngIndex");
            mobRng = getRNG(vehicleRngIndex);

            vehicleNameCounter = 0;
            activeVehicleCount = 0;
            parkingVehicleCount = 0;
            drivingVehicleCount = 0;

            nextNodeVectorIndex = 0;
            hosts.clear();
            subscribedVehicles.clear();
            subscribedPedestrians.clear();

            myAddVehicleTimer = new cMessage("myAddVehicleTimer");

            cModule *module = simulation.getSystemModule()->getSubmodule("world");
            world = static_cast<BaseWorldUtility*>(module);
            ASSERT(world);

            module = simulation.getSystemModule()->getSubmodule("connMan");
            cc = static_cast<ConnectionManager*>(module);
            ASSERT(cc);

            equilibrium_vehicle = par("equilibrium_vehicle").boolValue();
            addedNodes.clear();
        }
    }
}


void TraCI_Start::finish()
{
    TraCI_Commands::finish();

    if(collectVehiclesData)
        vehiclesDataToFile();

    if (connection && !par("TraCIclosed").boolValue())
        TraCIBuffer buf = connection->query(CMD_CLOSE, TraCIBuffer());

    while (hosts.begin() != hosts.end())
        deleteManagedModule(hosts.begin()->first);
}


void TraCI_Start::handleMessage(cMessage *msg)
{
    TraCI_Commands::handleMessage(msg);

    if (! msg->isSelfMessage())
        error("TraCI_Start doesn't handle messages from other modules");

    if (msg == connectAndStartTrigger)
    {
        // get SUMO executable
        std::string SUMOexe = par("SUMOexe").stringValue();
        // check if this file exists?
        if( !boost::filesystem::exists(SUMOexe) )
            error("SUMO executable not found at %s. Check SUMOexe variable!", SUMOexe.c_str());

        int seed = par("seed").longValue();

        // start SUMO TraCI server
        int port = TraCIConnection::startServer(SUMOexe, getSUMOConfigFullPath(), seed);

        // connect to SUMO TraCI server
        connection = TraCIConnection::connect(host.c_str(), port);

        // initialize TraCI
        init_traci();

        // call AddVehicle to insert flows if needed
        simsignal_t Signal_addFlow = registerSignal("addFlow");
        this->emit(Signal_addFlow, 0);

        std::cout << "Initializing modules with TraCI support ..." << std::endl << std::endl;
        simsignal_t Signal_executeFirstTS = registerSignal("executeFirstTS");
        this->emit(Signal_executeFirstTS, 1);

        initRoi();
        if(par("roiSquareSizeRSU").doubleValue() > 0)
            roiRSUs(); // this method should be called after sending executeFirstTS so that all RSUs are built
        drawRoi();
    }
    else if (msg == executeOneTimestepTrigger)
    {
        if (connectAt != -1)
            executeOneTimestep();

        // notify other modules to run one simulation TS
        simsignal_t Signal_executeEachTS = registerSignal("executeEachTS");
        this->emit(Signal_executeEachTS, 0);

        // we reached to max simtime and should terminate OMNET++ simulation
        // upon calling endSimulation(), TraCI_Start::finish() will close TraCI connection
        if(terminate != -1 && simTime().dbl() >= terminate)
            endSimulation();

        scheduleAt(simTime() + updateInterval, executeOneTimestepTrigger);
    }
    else
    {
        error("TraCI_Start received unknown self-message");
    }
}


void TraCI_Start::init_traci()
{
    // get the version of SUMO TraCI API
    std::pair<uint32_t, std::string> versionS = getVersion();
    uint32_t apiVersionS = versionS.first;
    std::string serverVersionS = versionS.second;

    if (apiVersionS == 10)
        std::cout << "SUMO TraCI server \"" << serverVersionS << "\" reports API version " << apiVersionS << endl;
    else
        error("TraCI server \"%s\" reports API version %d, which is unsupported.", serverVersionS.c_str(), apiVersionS);

    // query road network boundaries from SUMO
    double *boundaries = simulationGetNetBoundary();

    double x1 = boundaries[0];  // x1
    double y1 = boundaries[1];  // y1
    double x2 = boundaries[2];  // x2
    double y2 = boundaries[3];  // y2

    std::cout << "TraCI reports network boundaries (" << x1 << "," << y1 << ")-(" << x2 << "," << y2 << ")" << endl;

    netbounds1 = TraCICoord(x1, y1);
    netbounds2 = TraCICoord(x2, y2);

    if ((traci2omnet(netbounds2).x > world->getPgs()->x) || (traci2omnet(netbounds1).y > world->getPgs()->y))
        std::cout << "WARNING: Playground size (" << world->getPgs()->x << ", " << world->getPgs()->y << ") might be too small for vehicle at network bounds (" << traci2omnet(netbounds2).x << ", " << traci2omnet(netbounds1).y << ")" << endl;

    {
        // subscribe to list of departed and arrived vehicles, as well as simulation time
        std::vector<uint8_t> variables {VAR_DEPARTED_VEHICLES_IDS,
            VAR_ARRIVED_VEHICLES_IDS,
            VAR_TIME_STEP,
            VAR_TELEPORT_STARTING_VEHICLES_IDS,
            VAR_TELEPORT_ENDING_VEHICLES_IDS,
            VAR_PARKING_STARTING_VEHICLES_IDS,
            VAR_PARKING_ENDING_VEHICLES_IDS};
        TraCIBuffer buf = simulationSubscribe(0, 0x7FFFFFFF, "", variables);
        processSubcriptionResult(buf);
        ASSERT(buf.eof());
    }

    {
        // subscribe to list of vehicle ids
        std::vector<uint8_t> variables {ID_LIST};
        TraCIBuffer buf = vehicleSubscribe(0, 0x7FFFFFFF, "", variables);
        processSubcriptionResult(buf);
        ASSERT(buf.eof());
    }

    Veins::ObstacleControl* obstacles = Veins::ObstacleControlAccess().getIfExists();
    if (obstacles)
    {
        // get list of polygons
        std::list<std::string> ids = polygonGetIDList();

        for (std::list<std::string>::iterator i = ids.begin(); i != ids.end(); ++i)
        {
            std::string id = *i;
            std::string typeId = polygonGetTypeID(id);

            if (!obstacles->isTypeSupported(typeId))
                continue;

            std::list<Coord> coords = polygonGetShape(id);
            std::vector<Coord> shape;
            std::copy(coords.begin(), coords.end(), std::back_inserter(shape));
            obstacles->addFromTypeAndShape(id, typeId, shape);
        }
    }
}


void TraCI_Start::initRoi()
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

        roiRects.push_back(std::pair<TraCICoord, TraCICoord>(TraCICoord(x1, y1), TraCICoord(x2, y2)));
    }
}


void TraCI_Start::roiRSUs()
{
    // get a pointer to the first RSU
    cModule *module = simulation.getSystemModule()->getSubmodule("RSU", 0);
    // no RSUs in the network
    if(module == NULL)
        return;

    // how many RSUs are in the network?
    int RSUcount = module->getVectorSize();

    // iterate over RSUs
    for(int i = 0; i < RSUcount; ++i)
    {
        module = simulation.getSystemModule()->getSubmodule("RSU", i);
        cModule *appl =  module->getSubmodule("appl");
        double centerX = appl->par("myCoordX").doubleValue();
        double centerY = appl->par("myCoordY").doubleValue();

        double squSize = par("roiSquareSizeRSU").doubleValue();

        // calculate corners of the roi square
        TraCICoord bottomLeft = TraCICoord(centerX - squSize, centerY - squSize);
        TraCICoord topRight = TraCICoord(centerX + squSize, centerY + squSize);

        // add them into roiRects
        roiRects.push_back(std::pair<TraCICoord, TraCICoord>(bottomLeft, topRight));
    }
}


void TraCI_Start::drawRoi()
{
    int roiCount = 0;

    // draws a polygon in SUMO to show the roi
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

        polygonAddTraCI(polID, "region", Color::colorNameToRGB("green"), 0, 1, detectionRegion);
    }
}


uint32_t TraCI_Start::getCurrentTimeMs()
{
    return static_cast<uint32_t>(round(simTime().dbl() * 1000));
}


void TraCI_Start::executeOneTimestep()
{
    if (simTime() > 1)
    {
        if (vehicleTypeIds.size() == 0)
        {
            std::list<std::string> vehTypes = vehicleGetIDList();
            for (std::list<std::string>::const_iterator i = vehTypes.begin(); i != vehTypes.end(); ++i)
            {
                if (i->compare("DEFAULT_VEHTYPE") != 0)
                {
                    //MYDEBUG << *i << std::endl;
                    vehicleTypeIds.push_back(*i);
                }
            }
        }

        if (routeIds.size() == 0)
        {
            std::list<std::string> routes = routeGetIDList();
            for (std::list<std::string>::const_iterator i = routes.begin(); i != routes.end(); ++i)
            {
                std::string routeId = *i;
                if (par("useRouteDistributions").boolValue() && std::count(routeId.begin(), routeId.end(), '#') >= 1)
                {
                    //MYDEBUG << "Omitting route " << routeId << " as it seems to be a member of a route distribution (found '#' in name)" << std::endl;
                    continue;
                }

                //MYDEBUG << "Adding " << routeId << " to list of possible routes" << std::endl;
                routeIds.push_back(routeId);
            }
        }

        for (int i = activeVehicleCount + queuedVehicles.size(); i < numVehicles; i++)
            insertNewVehicle();
    }

    //MYDEBUG << "Triggering TraCI server simulation advance to t=" << simTime() <<endl;

    uint32_t targetTime = getCurrentTimeMs();

    if (targetTime > round(connectAt.dbl() * 1000))
    {
        insertVehicles();

        // proceed SUMO simulation to targetTime
        std::pair<TraCIBuffer, uint32_t> output = simulationTimeStep(targetTime);

        for (uint32_t i = 0; i < output.second /*# of subscription results*/; ++i)
            processSubcriptionResult(output.first);

        // todo: should get pedestrians using subscription
        allPedestrians.clear();
        allPedestrians = personGetIDList();
        if(!allPedestrians.empty())
            addPedestriansToOMNET();
    }
}


void TraCI_Start::insertNewVehicle()
{
    std::string type;
    if (vehicleTypeIds.size())
    {
        int vehTypeId = mobRng->intRand(vehicleTypeIds.size());
        type = vehicleTypeIds[vehTypeId];
    }
    else
    {
        type = "DEFAULT_VEHTYPE";
    }

    int routeId = mobRng->intRand(routeIds.size());
    vehicleInsertQueue[routeId].push(type);
}


void TraCI_Start::insertVehicles()
{
    for (std::map<int, std::queue<std::string> >::iterator i = vehicleInsertQueue.begin(); i != vehicleInsertQueue.end(); )
    {
        std::string route = routeIds[i->first];
        //MYDEBUG << "process " << route << std::endl;
        std::queue<std::string> vehicles = i->second;
        while (!i->second.empty())
        {
            std::string type = i->second.front();
            std::stringstream veh;
            veh << type << "_" << vehicleNameCounter;
            //MYDEBUG << "trying to add " << veh.str() << " with " << route << " vehicle type " << type << std::endl;

            vehicleAdd(veh.str(), type, route, std::floor(simTime().dbl() * 1000), -4/*DEPART_POS_BASE*/, -3/*DEPART_SPEED_MAX*/, -5 /*DEPART_LANE_BEST_FREE*/);
            //MYDEBUG << "successful inserted " << veh.str() << std::endl;
            queuedVehicles.insert(veh.str());
            i->second.pop();
            vehicleNameCounter++;
        }

        std::map<int, std::queue<std::string> >::iterator tmp = i;
        ++tmp;
        vehicleInsertQueue.erase(i);
        i = tmp;
    }
}


void TraCI_Start::processSubcriptionResult(TraCIBuffer& buf)
{
    uint8_t cmdLength_resp; buf >> cmdLength_resp;
    uint32_t cmdLengthExt_resp; buf >> cmdLengthExt_resp;
    uint8_t commandId_resp; buf >> commandId_resp;
    std::string objectId_resp; buf >> objectId_resp;

    if (commandId_resp == RESPONSE_SUBSCRIBE_VEHICLE_VARIABLE)
        processVehicleSubscription(objectId_resp, buf);
    else if (commandId_resp == RESPONSE_SUBSCRIBE_SIM_VARIABLE)
        processSimSubscription(objectId_resp, buf);
    else
        error("Received unhandled subscription result");
}


void TraCI_Start::processVehicleSubscription(std::string objectId, TraCIBuffer& buf)
{
    bool isSubscribed = (subscribedVehicles.find(objectId) != subscribedVehicles.end());
    double px;
    double py;
    std::string edge;
    double speed;
    double angle_traci;
    int signals;
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
                    error("TraCI server reported subscribing to vehicle variable 0x%2x not implemented (\"%s\"). Might need newer version.", variable1_resp, errormsg.c_str());

                error("TraCI server reported error subscribing to vehicle variable 0x%2x (\"%s\").", variable1_resp, errormsg.c_str());
            }
        }
        else if (variable1_resp == ID_LIST)
        {
            uint8_t varType; buf >> varType;
            ASSERT(varType == TYPE_STRINGLIST);
            uint32_t count; buf >> count;
            //MYDEBUG << "TraCI reports " << count << " active vehicles." << endl;
            ASSERT(count == drivingVehicleCount);
            std::set<std::string> drivingVehicles;
            for (uint32_t i = 0; i < count; ++i)
            {
                std::string idstring; buf >> idstring;
                drivingVehicles.insert(idstring);

                // collecting data from this vehicle in this timeStep
                if(collectVehiclesData)
                    saveVehicleData(idstring);
            }

            // check for vehicles that need subscribing to
            std::set<std::string> needSubscribe;
            std::set_difference(drivingVehicles.begin(), drivingVehicles.end(), subscribedVehicles.begin(), subscribedVehicles.end(), std::inserter(needSubscribe, needSubscribe.begin()));
            for (std::set<std::string>::const_iterator i = needSubscribe.begin(); i != needSubscribe.end(); ++i)
            {
                subscribedVehicles.insert(*i);

                // subscribe to some attributes of the vehicle
                std::vector<uint8_t> variables {VAR_POSITION, VAR_ROAD_ID, VAR_SPEED, VAR_ANGLE, VAR_SIGNALS};
                TraCIBuffer buf = vehicleSubscribe(0, 0x7FFFFFFF, *i, variables);
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
                TraCIBuffer buf = vehicleSubscribe(0, 0x7FFFFFFF, *i, variables);
                ASSERT(buf.eof());
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
            buf >> signals;
            numRead++;
        }
        else
        {
            error("Received unhandled vehicle subscription result");
        }
    }

    // bail out if we didn't want to receive these subscription results
    if (!isSubscribed) return;

    // make sure we got updates for all attributes
    if (numRead != 5) return;

    Coord p = traci2omnet(TraCICoord(px, py));
    if ((p.x < 0) || (p.y < 0))
        error("received bad node position (%.2f, %.2f), translated to (%.2f, %.2f)", px, py, p.x, p.y);

    double angle = traci2omnetAngle(angle_traci);

    cModule* mod = getManagedModule(objectId);

    // is it in the ROI?
    bool inRoi = isInRegionOfInterest(TraCICoord(px, py), edge, speed, angle);
    if (!inRoi)
    {
        if (mod)
        {
            deleteManagedModule(objectId);
            //MYDEBUG << "Vehicle #" << objectId << " left region of interest" << endl;
        }
        else if(unEquippedHosts.find(objectId) != unEquippedHosts.end())
        {
            unEquippedHosts.erase(objectId);
            //MYDEBUG << "Vehicle (unequipped) # " << objectId<< " left region of interest" << endl;
        }

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
            cModule* submod = iter();
            ifInetTraCIMobilityCallNextPosition(submod, p, edge, speed, angle);
            TraCIMobilityMod* mm = dynamic_cast<TraCIMobilityMod*>(submod);
            if (!mm) continue;
            //MYDEBUG << "module " << objectId << " moving to " << p.x << "," << p.y << endl;
            mm->nextPosition(p, edge, speed, angle);
        }
    }
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
                error("TraCI server reported subscribing to variable 0x%2x not implemented (\"%s\"). Might need newer version.", variable1_resp, description.c_str());

            error("TraCI server reported error subscribing to variable 0x%2x (\"%s\").", variable1_resp, description.c_str());
        }

        if (variable1_resp == VAR_DEPARTED_VEHICLES_IDS)
        {
            uint8_t varType; buf >> varType;
            ASSERT(varType == TYPE_STRINGLIST);
            uint32_t count; buf >> count;  // count: number of departed vehicles
            for (uint32_t i = 0; i < count; ++i)
            {
                std::string idstring; buf >> idstring;
                // adding modules is handled on the fly when entering/leaving the ROI

                if(equilibrium_vehicle)
                {
                    // saving information for later use
                    // todo: always set entryPos and entrySpeed to 0
                    departedNodes *node = new departedNodes(idstring,
                            vehicleGetTypeID(idstring),
                            vehicleGetRouteID(idstring),
                            0 /*vehicleGetLanePosition(idstring)*/,
                            0 /*vehicleGetSpeed(idstring)*/,
                            vehicleGetLaneIndex(idstring));

                    auto it = addedNodes.find(idstring);
                    if(it != addedNodes.end())
                        error("%s was added before!", idstring.c_str());
                    else
                        addedNodes.insert(std::make_pair(idstring, *node));
                }
            }

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
                    TraCIBuffer buf = vehicleSubscribe(0, 0x7FFFFFFF, idstring, variables);
                    ASSERT(buf.eof());
                }

                // check if this object has been deleted already (e.g. because it was outside the ROI)
                cModule* mod = getManagedModule(idstring);
                if (mod) deleteManagedModule(idstring);

                if(unEquippedHosts.find(idstring) != unEquippedHosts.end())
                    unEquippedHosts.erase(idstring);

                if(equilibrium_vehicle)
                {
                    auto it = addedNodes.find(idstring);
                    if(it == addedNodes.end())
                        error("cannot find %s in the addedNodes map!", idstring.c_str());

                    departedNodes node = it->second;

                    std::cout << "t=" << simTime().dbl() << ": " << " vehicle arrived. Inserting again ("
                            << "id:" << node.vehicleId << ", "
                            << "type:" << node.vehicleTypeId << ", "
                            << "routeId:" << node.routeId << ", "
                            << "pos:" << node.pos << ", "
                            << "entrySpeed:" << node.speed << ", "
                            << "entryLaneIndex:" << node.lane << ")"
                            << std::endl;

                    addedNodes.erase(it);  // remove this entry before adding
                    vehicleAdd(node.vehicleId, node.vehicleTypeId, node.routeId, (simTime().dbl() * 1000)+1, node.pos, node.speed, node.lane);
                }
            }

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
                        endSimulation();
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
                    cModule* submod = iter();
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
                    cModule* submod = iter();
                    TraCIMobilityMod* mm = dynamic_cast<TraCIMobilityMod*>(submod);
                    if (!mm) continue;
                    mm->changeParkingState(false);
                }
            }

            parkingVehicleCount -= count;
            drivingVehicleCount += count;
        }
        else if (variable1_resp == VAR_TIME_STEP)
        {
            uint8_t varType; buf >> varType;
            ASSERT(varType == TYPE_INTEGER);
            uint32_t serverTimestep; buf >> serverTimestep; // serverTimestep: current timestep reported by server in ms
            uint32_t omnetTimestep = getCurrentTimeMs();
            ASSERT(omnetTimestep == serverTimestep);
        }
        else
        {
            error("Received unhandled sim subscription result");
        }
    }
}


cModule* TraCI_Start::getManagedModule(std::string nodeId)
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


void TraCI_Start::deleteManagedModule(std::string nodeId)
{
    cModule* mod = getManagedModule(nodeId);
    if (!mod) error("no vehicle with Id \"%s\" found", nodeId.c_str());

    cModule* nic = mod->getSubmodule("nic");
    if (nic)
        cc->unregisterNic(nic);

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


// name: host; Car; i=vehicle.gif
void TraCI_Start::addModule(std::string nodeId, const Coord& position, std::string road_id, double speed, double angle)
{
    std::string type = "";
    std::string name = "";
    std::string displayString = "";

    std::string vClass = vehicleGetClass(nodeId);
    int vClassEnum = -1;

    if(vClass == "passenger")
    {
        type = vehicleModuleType;
        name =  vehicleModuleName;
        displayString = vehicleModuleDisplayString;
        vClassEnum = SVC_PASSENGER;
    }
    else if(vClass == "private")
    {
        type = vehicleModuleType;
        name =  vehicleModuleName;
        displayString = vehicleModuleDisplayString;
        vClassEnum = SVC_PRIVATE;
    }
    else if(vClass == "emergency")
    {
        type = vehicleModuleType;
        name =  vehicleModuleName;
        displayString = vehicleModuleDisplayString;
        vClassEnum = SVC_EMERGENCY;
    }
    else if(vClass == "bus")
    {
        type = vehicleModuleType;
        name =  vehicleModuleName;
        displayString = vehicleModuleDisplayString;
        vClassEnum = SVC_BUS;
    }
    else if(vClass == "truck")
    {
        type = vehicleModuleType;
        name =  vehicleModuleName;
        displayString = vehicleModuleDisplayString;
        vClassEnum = SVC_TRUCK;
    }
    else if(vClass == "bicycle")
    {
        type = bikeModuleType;
        name =  bikeModuleName;
        displayString = bikeModuleDisplayString;
        vClassEnum = SVC_BICYCLE;
    }
    else if(vClass == "pedestrian")  // todo: should we handle pedestrian separately?
    {
        type = pedModuleType;
        name = pedModuleName;
        displayString = pedModuleDisplayString;
        vClassEnum = SVC_PEDESTRIAN;
    }
    else
        error("Unknown vClass '%s' for vehicle '%s'. Change vClass in traffic demand file based on global/Appl.h",
                vClass.c_str(),
                nodeId.c_str());

    if (hosts.find(nodeId) != hosts.end())
        error("tried adding duplicate module");

    if (queuedVehicles.find(nodeId) != queuedVehicles.end())
        queuedVehicles.erase(nodeId);

    double option1 = hosts.size() / (hosts.size() + unEquippedHosts.size() + 1.0);
    double option2 = (hosts.size() + 1) / (hosts.size() + unEquippedHosts.size() + 1.0);

    if (fabs(option1 - penetrationRate) < fabs(option2 - penetrationRate))
    {
        unEquippedHosts.insert(nodeId);
        return;
    }

    cModule* parentmod = getParentModule();
    if (!parentmod)
        error("Parent Module not found");

    cModuleType* nodeType = cModuleType::get(type.c_str());
    if (!nodeType)
        error("Module Type \"%s\" not found", type.c_str());

    //TODO: this trashes the vectsize member of the cModule, although nobody seems to use it
    int32_t nodeVectorIndex = nextNodeVectorIndex++;
    cModule* mod = nodeType->create(name.c_str(), parentmod, nodeVectorIndex, nodeVectorIndex);
    mod->finalizeParameters();
    mod->getDisplayString().parse(displayString.c_str());
    mod->buildInside();

    std::string vehType = vehicleGetTypeID(nodeId);  // get vehicle type
    int SUMOControllerType = vehicleTypeGetControllerType(vehType); // get controller type
    int SUMOControllerNumber = vehicleTypeGetControllerNumber(vehType);  // get controller number from SUMO
    mod->getSubmodule("appl")->par("SUMOID") = nodeId;
    mod->getSubmodule("appl")->par("SUMOType") = vehType;
    mod->getSubmodule("appl")->par("vehicleClass") = vClass;
    mod->getSubmodule("appl")->par("vehicleClassEnum") = vClassEnum;
    mod->getSubmodule("appl")->par("SUMOControllerType") = SUMOControllerType;
    mod->getSubmodule("appl")->par("SUMOControllerNumber") = SUMOControllerNumber;

    mod->scheduleStart(simTime() + updateInterval);

    // pre-initialize TraCIMobilityMod
    for (cModule::SubmoduleIterator iter(mod); !iter.end(); iter++)
    {
        cModule* submod = iter();
        ifInetTraCIMobilityCallPreInitialize(submod, nodeId, position, road_id, speed, angle);
        TraCIMobilityMod* mm = dynamic_cast<TraCIMobilityMod*>(submod);
        if (!mm) continue;
        mm->preInitialize(nodeId, position, road_id, speed, angle);
    }

    mod->callInitialize();
    hosts[nodeId] = mod;

    // post-initialize TraCIMobilityMod
    for (cModule::SubmoduleIterator iter(mod); !iter.end(); iter++)
    {
        cModule* submod = iter();
        TraCIMobilityMod* mm = dynamic_cast<TraCIMobilityMod*>(submod);
        if (!mm) continue;
        mm->changePosition();
    }
}


// todo: work on pedestrian
void TraCI_Start::addPedestriansToOMNET()
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
    //            TraCIMobilityMod* mm = dynamic_cast<TraCIMobilityMod*>(submod);
    //            if (!mm) continue;
    //            cout << "module " << objectId << " moving to " << p.x << "," << p.y << endl;
    //            mm->nextPosition(p, edge, speed, angle);
    //        }
    //    }
    //
}


void TraCI_Start::saveVehicleData(std::string vID)
{
    double timeStep = -1;
    std::string vType = "n/a";
    std::string lane = "n/a";
    double pos = -1;
    double speed = -1;
    double accel = std::numeric_limits<double>::infinity();
    std::string CFMode = "n/a";
    double timeGapSetting = -1;
    double spaceGap = -2;
    double timeGap = -2;
    std::string TLid = "n/a";   // TLid that controls this vehicle. Empty string means the vehicle is not controlled by any TLid
    char linkStatus = '\0';     // status of the TL ahead (character 'n' means no TL ahead)

    // full collection
    if(vehicleDataLevel == 1)
    {
        timeStep = (simTime()-updateInterval).dbl();
        vType = vehicleGetTypeID(vID);
        lane = vehicleGetLaneID(vID);
        pos = vehicleGetLanePosition(vID);
        speed = vehicleGetSpeed(vID);
        accel = vehicleGetCurrentAccel(vID);

        int CFMode_Enum = vehicleGetCarFollowingMode(vID);
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
        timeGapSetting = vehicleGetTimeGap(vID);

        // get the space gap
        std::vector<std::string> vleaderIDnew = vehicleGetLeader(vID, 900);
        std::string vleaderID = vleaderIDnew[0];
        spaceGap = (vleaderID != "") ? atof(vleaderIDnew[1].c_str()) : -1;

        // calculate timeGap (if leading is present)
        if(vleaderID != "" && speed != 0)
            timeGap = spaceGap / speed;
        else
            timeGap = -1;

        // get the TLid that controls this vehicle
        // empty string means the vehicle is not controlled by any TLid
        TLid = vehicleGetTLID(vID);

        // get the signal status ahead
        // character 'n' means no link status
        linkStatus = vehicleGetTLLinkStatus(vID);
    }
    // router scenario
    else if(vehicleDataLevel == 2)
    {
        timeStep = (simTime()-updateInterval).dbl();
        vType = vehicleGetTypeID(vID);
        lane = vehicleGetLaneID(vID);
        pos = vehicleGetLanePosition(vID);
        speed = vehicleGetSpeed(vID);
        accel = vehicleGetCurrentAccel(vID);
    }
    // traffic light scenario
    else if(vehicleDataLevel == 3)
    {
        timeStep = (simTime()-updateInterval).dbl();
        lane = vehicleGetLaneID(vID);
        speed = vehicleGetSpeed(vID);
        linkStatus = vehicleGetTLLinkStatus(vID);
    }
    // CACC vehicle stream
    else if(vehicleDataLevel == 4)
    {
        timeStep = (simTime()-updateInterval).dbl();
        pos = vehicleGetLanePosition(vID);
        speed = vehicleGetSpeed(vID);
        accel = vehicleGetCurrentAccel(vID);

        // get the space gap
        std::vector<std::string> vleaderIDnew = vehicleGetLeader(vID, 900);
        std::string vleaderID = vleaderIDnew[0];
        spaceGap = (vleaderID != "") ? atof(vleaderIDnew[1].c_str()) : -1;
    }

    VehicleData *tmp = new VehicleData(timeStep, vID.c_str(), vType.c_str(),
            lane.c_str(), pos, speed, accel,
            CFMode.c_str(), timeGapSetting, spaceGap, timeGap,
            TLid.c_str(), linkStatus);

    Vec_vehiclesData.push_back(*tmp);
}


void TraCI_Start::vehiclesDataToFile()
{
    if(Vec_vehiclesData.empty())
        return;

    boost::filesystem::path filePath;

    if(ev.isGUI())
    {
        filePath = "results/gui/vehicleData.txt";
    }
    else
    {
        // get the current run number
        int currentRun = ev.getConfigEx()->getActiveRunNumber();
        std::ostringstream fileName;

        if(vehicleDataLevel == 2)
        {
            cModule *module = simulation.getSystemModule()->getSubmodule("router");
            Router *router = static_cast< Router* >(module);

            int TLMode = (*router->net->TLs.begin()).second->TLLogicMode;
            std::ostringstream filePrefix;
            filePrefix << router->totalVehicleCount << "_" << 1- router->nonReroutingVehiclePercent << "_" << TLMode;
            fileName << filePrefix.str() << "_vehicleData.txt";
        }
        else
        {
            fileName << std::setfill('0') << std::setw(3) << currentRun << "_vehicleData.txt";
        }

        filePath = "results/cmd/" + fileName.str();
    }

    FILE *filePtr = fopen (filePath.string().c_str(), "w");

    // write simulation parameters at the beginning of the file in CMD mode
    if(!ev.isGUI())
    {
        // get the current config name
        std::string configName = ev.getConfigEx()->getVariable("configname");

        // get number of total runs in this config
        int totalRun = ev.getConfigEx()->getNumRunsInConfig(configName.c_str());

        // get the current run number
        int currentRun = ev.getConfigEx()->getActiveRunNumber();

        // get all iteration variables
        std::vector<std::string> iterVar = ev.getConfigEx()->unrollConfig(configName.c_str(), false);

        // write to file
        fprintf (filePtr, "configName      %s\n", configName.c_str());
        fprintf (filePtr, "totalRun        %d\n", totalRun);
        fprintf (filePtr, "currentRun      %d\n", currentRun);
        fprintf (filePtr, "currentConfig   %s\n\n\n", iterVar[currentRun].c_str());
    }

    double oldTime = -1;
    int index = 0;

    for(auto &y : Vec_vehiclesData)
    {
        // write header only once
        if(oldTime == -1)
        {
            fprintf (filePtr, "%-10s","index");
            fprintf (filePtr, "%-12s","timeStep");
            fprintf (filePtr, "%-20s","vehicleName");
            if(y.vehicleType != "n/a") fprintf (filePtr, "%-17s","vehicleType");
            if(y.lane != "n/a") fprintf (filePtr, "%-12s","lane");
            if(y.pos != -1) fprintf (filePtr, "%-11s","pos");
            if(y.speed != -1) fprintf (filePtr, "%-12s","speed");
            if(y.accel != std::numeric_limits<double>::infinity()) fprintf (filePtr, "%-12s","accel");
            if(y.CFMode != "n/a") fprintf (filePtr, "%-20s","CFMode");
            if(y.timeGapSetting != -1) fprintf (filePtr, "%-20s","timeGapSetting");
            if(y.spaceGap != -2) fprintf (filePtr, "%-10s","SpaceGap");
            if(y.timeGap != -2) fprintf (filePtr, "%-16s","timeGap");
            if(y.TLid != "n/a") fprintf (filePtr, "%-17s","TLid");
            if(y.linkStatus != '\0') fprintf (filePtr, "%-17s","linkStatus");

            fprintf (filePtr, "\n\n");
        }

        if(oldTime != y.time)
        {
            fprintf(filePtr, "\n");
            oldTime = y.time;
            index++;
        }

        fprintf (filePtr, "%-10d ", index);
        fprintf (filePtr, "%-10.2f ", y.time );
        fprintf (filePtr, "%-20s ", y.vehicleName.c_str());
        if(y.vehicleType != "n/a") fprintf (filePtr, "%-15s ", y.vehicleType.c_str());
        if(y.lane != "n/a") fprintf (filePtr, "%-12s ", y.lane.c_str());
        if(y.pos != -1) fprintf (filePtr, "%-10.2f ", y.pos);
        if(y.speed != -1) fprintf (filePtr, "%-10.2f ", y.speed);
        if(y.accel != std::numeric_limits<double>::infinity()) fprintf (filePtr, "%-10.2f ", y.accel);
        if(y.CFMode != "n/a") fprintf (filePtr, "%-20s", y.CFMode.c_str());
        if(y.timeGapSetting != -1) fprintf (filePtr, "%-20.2f ", y.timeGapSetting);
        if(y.spaceGap != -2) fprintf (filePtr, "%-10.2f ", y.spaceGap);
        if(y.timeGap != -2) fprintf (filePtr, "%-16.2f ", y.timeGap);
        if(y.TLid != "n/a") fprintf (filePtr, "%-17s ", y.TLid.c_str());
        if(y.linkStatus != '\0') fprintf (filePtr, "%-17c", y.linkStatus);

        fprintf (filePtr, "\n");
    }

    fclose(filePtr);
}

}
