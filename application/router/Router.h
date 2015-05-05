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

#ifndef ROUTER_H
#define ROUTER_H

#include <BaseModule.h>
#include <vector>
#include <list>
#include <fstream>
#include <set>

#include "Node.h"
#include "Edge.h"
#include "08_TL_Router.h"
#include "Net.h"
#include "Hypertree.h"

#include "ApplV_02_Beacon.h"

#define SSTR( x ) dynamic_cast< std::ostringstream & >( (std::ostringstream() << std::dec << x ) ).str()

using namespace std;

namespace VENTOS {

class Node;
class Edge;
class TrafficLightRouter;
class Net;
class Hypertree;
struct EdgeRemoval;

string key(Node* n1, Node* n2, int time);

class Router : public BaseModule    //Responsible for routing cars in our system.  Should only be one of these.
{
public:
    ~Router();
    virtual void initialize(int stage);
    virtual void handleMessage(cMessage* msg);
    virtual void receiveSignal(cComponent *, simsignal_t, cObject *);
    virtual void receiveSignal(cComponent *, simsignal_t, long);

    int createTime;
    int currentVehicleCount;
    int totalVehicleCount;
    double nonReroutingVehiclePercent;
    set<string>* nonReroutingVehicles;
    Net* net;
    void sendRerouteSignal(string vid);     //Forces a vehicle to reroute
    bool UseHysteresis;

    ofstream vehicleEndTimesFile;
    std::map<string, LaneCosts> edgeHistograms; // should be public

protected:
    boost::filesystem::path VENTOS_FullPath;
    boost::filesystem::path SUMO_Path;
    boost::filesystem::path SUMO_FullPath;

    //Edge-removal Alg
    cMessage* routerMsg;
    void issueStop(string vehID, string edgeID);
    void issueStart(string vehID, string edgeID);
    void checkEdgeRemovals();
    vector<EdgeRemoval> EdgeRemovals;
    set<string> RemovedVehicles;
    bool UseAccidents;
    int AccidentCheckInterval;

    map<string, Hypertree*> hypertreeMemo;

    //Vehicle stuff
    bool collectVehicleTimeData;
    map<string, int> vehicleTravelTimes;
    ofstream vehicleTravelTimesFile;

    bool enableRouting; //If false, runs no code
    double leftTurnCost, rightTurnCost, straightCost, uTurnCost, TLLookahead;

    double junctionCost(double time, Edge* start, Edge* end);       //If it's a TL, returns the time spent waiting.  If not, returns turnTypeCost
    double turnTypeCost(Edge* start, Edge* end);                    //Returns the turn penalty on an intersection
    double timeToPhase(TrafficLightRouter* tl, double time, int phase);   //Returns how long we must wait for a given phase at the given time
    int nextAcceptingPhase(double time, Edge* start, Edge* end);    //Returns the next phase allowing movement from start to end at the given time
    vector<int>* TLTransitionPhases(Edge* start, Edge* end);        //Returns a vector of phases allowing movement from start to end

    int timePeriodMax;     //Max time for hypertrees
    Hypertree* buildHypertree(int startTime, Node* destination);    //Builds a hypertree to the destination, bounded between the start time and timePeriodMax;
    list<string> getRoute(Edge* origin, Node* destination, string vName);       //Returns a list of edges between origin and destination,
                                                                                //or an empty list if they're not connected
    SystemMsg* prepareSystemMsg();

    //Message passing
    cModule *nodePtr;               // pointer to the Node
    mutable TraCI_Extend* TraCI;    //Link to TraCI
    simsignal_t Signal_system;      //Receives signals to here

    simsignal_t Signal_executeFirstTS;

    // Edge weight-gathering
    std::map<string, string> vehicleEdges;
    std::map<string, double> vehicleTimes;

    //Hysteresis implementation
    std::map<string, int> vehicleLaneChangeCount;
    int HysteresisCount;

    int LaneCostsMode;

    void HistogramsToFile();
    void parseHistogramFile();
    void laneCostsData();
};

}

#endif
