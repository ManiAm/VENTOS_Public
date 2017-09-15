/****************************************************************************/
/// @file    Router.h
/// @author  Dylan Smith <dilsmith@ucdavis.edu>
/// @author  second author here
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

//Handles communication between all router-related elements.

#ifndef ROUTER_H
#define ROUTER_H

#include <vector>
#include <list>
#include <fstream>
#include <set>
#include <map>

#include "baseAppl/03_BaseApplLayer.h"
#include "router/Node.h"
#include "router/Edge.h"
#include "router/Net.h"
#include "router/Hypertree.h"
#include "trafficLight/TSC/09_Router.h"

namespace VENTOS {

class Node;
class Edge;
class TrafficLightRouter;
class Net;
class Hypertree;
struct EdgeRemoval;

std::string key(Node* n1, Node* n2, int time);

class Router : public BaseApplLayer    //Responsible for routing cars in our system.  Should only be one of these.
{
public:
    ~Router();
    virtual void initialize(int stage);
    virtual void finish();
    virtual void handleMessage(omnetpp::cMessage* msg);
    virtual void receiveSignal(omnetpp::cComponent *, omnetpp::simsignal_t, cObject *, cObject* details);
    virtual void receiveSignal(omnetpp::cComponent *source, omnetpp::simsignal_t signalID, long i, cObject* details);

    int createTime; //The duration over which vehicles are created
    int currentVehicleCount; //The current number of vehicles in the sim
    int totalVehicleCount;   //The total number of vehicles added to the sim
    double nonReroutingVehiclePercent; //% of vehicles which will not re-route
    std::set<std::string>* nonReroutingVehicles = NULL; //Set of vehicles which will not re-route
    Net* net; //Network description (TLs, edges, nodes, etc)
    void sendRerouteSignal(std::string vid);     //Forces a vehicle to reroute
    bool UseHysteresis; //If true, re-routes based on # of turns rather than time

    std::ofstream vehicleEndTimesFile; //File of when each vehicle completes its trip

protected:
    boost::filesystem::path SUMOConfigDirectory;

    void addVehs();

    //Edge-removal Algo
    omnetpp::cMessage* routerMsg = NULL;
    void issueStop(std::string vehID, std::string edgeID, double position, int laneIndex); //Tells a vehicle to stop on the given edge
    void issueStart(std::string vehID); //Tells a vehicle to resume
    void checkEdgeRemovals(); //Check for vehicles on disabled edges
    std::vector<EdgeRemoval> EdgeRemovals; //vector of currently disabled edges
    std::set<std::string> RemovedVehicles; //vector of vehicles on disabled edges
    int AccidentCheckInterval; //Time between running checkEdgeRemovals()
    bool UseAccidents; //If true, uses the above variables to simulate an accident

    std::map<std::string, Hypertree*> hypertreeMemo; //Map from a destination to its respective hypertree

    //Vehicle stuff
    bool collectVehicleTimeData; //If true, records travel time data to a file
    std::map<std::string, int> vehicleTravelTimes; //Map from vehicle ID to travel time
    std::ofstream vehicleTravelTimesFile; //File to write travel times to

    bool enableRouter; //If false, runs no code

    int TLLookahead;

    int timePeriodMax;     //Max time for hypertrees
    Hypertree* buildHypertree(int startTime, Node* destination);    //Builds a hypertree to the destination, bounded between the start time and timePeriodMax;
    std::list<std::string> getRoute(Edge* origin, Node* destination, std::string vName);       //Returns a list of edges between origin and destination, using dijskstra's
    //or an empty list if they're not connected
    void receiveDijkstraRequest(Edge* origin, Node* destination, std::string sender);
    void receiveHypertreeRequest(Edge* origin, Node* destination, std::string sender);
    void receiveDoneRequest(std::string sender);
    void receiveStartedRequest(std::string sender);

    int dijkstraOutdateTime;
    std::map<std::string, std::list<std::string> > dijkstraRoutes;
    std::map<std::string, int> dijkstraTimes;

    // Message passing
    TraCI_Commands* TraCI = NULL;
    omnetpp::simsignal_t Signal_system;
    omnetpp::simsignal_t Signal_executeEachTS;
    omnetpp::simsignal_t Signal_initialize_withTraCI;

    int debugLevel;

    // Edge weight-gathering
    std::map<std::string, std::string> vehicleEdges;
    std::map<std::string, double> vehicleTimes;

    //Hysteresis implementation
    std::map<std::string, int> vehicleLaneChangeCount; //Map from vehicle ID to how many times it's changed lanes
    int HysteresisCount; //Number of lane changes before a reroute is done
    LaneCostsMode laneCostsMode = MODE_NOTHING;

    double EWMARate;

    void LaneCostsToFile();
    void parseLaneCostsFile();
    void laneCostsData();
};

}

#endif
