/****************************************************************************/
/// @file    Router.cc
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

#include "Router.h"
#include "RouterGlobals.h"
#include <stdlib.h>

using namespace std;

namespace VENTOS {

Define_Module(VENTOS::Router);

bool fexists(const char *filename)
{
    ifstream ifile(filename);
    return ifile;
}

set<string>* randomUniqueVehiclesInRange(int numInts, int rangeMin, int rangeMax)
{             //Generates n random unique ints in range [rangeMin, rangeMax)
              //Not a very efficient implementation, but it shouldn't matter much

    vector<int>* initialInts = new vector<int>;
    for(int i = rangeMin; i < rangeMax; i++)
        initialInts->push_back(i);

    if(rangeMin < rangeMax)
        random_shuffle(initialInts->begin(), initialInts->end());

    set<string>* randInts = new set<string>;
    for(int i = 0; i < numInts; i++)
        randInts->insert(SSTR(initialInts->at(i) + 1));

    return randInts;
}

string key(Node* n1, Node* n2, int time)
{
    return n1->id + "#" + n2->id + "#" + SSTR(time);
}

struct EdgeRemoval
{
    string edge;
    int start;
    int end;
    EdgeRemoval(string edge, int start, int end):edge(edge), start(start), end(end){}
};

class routerCompare // Comparator object for getRoute weighting
{
public:
    bool operator()(const Edge* n1, const Edge* n2)
    {
        return n1->curCost > n2->curCost;
    }
};

Router::~Router()
{

}

void Router::initialize(int stage)
{
    int test = MODE_EWMA;
    enableRouting = par("enableRouting").boolValue();
    if(!enableRouting)
        return;

    if(stage == 0)
    {
        // Build nodePtr and traci manager
        nodePtr = FindModule<>::findHost(this);
        TraCI = FindModule<TraCI_Extend*>::findGlobalModule();
        if(nodePtr == NULL || TraCI == NULL)
            error("can not get a pointer to the module.");

        leftTurnCost = par("leftTurnCost").doubleValue();
        rightTurnCost = par("rightTurnCost").doubleValue();
        straightCost = par("straightCost").doubleValue();
        uTurnCost = par("uTurnCost").doubleValue();

        TLLookahead = par("TLLookahead").doubleValue();
        timePeriodMax = par("timePeriodMax").doubleValue();
        UseHysteresis = par("UseHysteresis").boolValue();

        laneCostsMode = static_cast<LaneCostsMode>(par("LaneCostsMode").longValue());
        HysteresisCount = par("HysteresisCount").longValue();

        createTime = par("createTime").longValue();
        totalVehicleCount = par("vehicleCount").longValue();
        currentVehicleCount = totalVehicleCount;
        nonReroutingVehiclePercent = par("nonReroutingVehiclePercent").doubleValue();

        UseAccidents = par("UseAccidents").boolValue();
        AccidentCheckInterval = par("AccidentCheckInterval").longValue();

        // get the file paths
        VENTOS_FullPath = cSimulation::getActiveSimulation()->getEnvir()->getConfig()->getConfigEntry("network").getBaseDirectory();
        SUMO_Path = simulation.getSystemModule()->par("SUMODirectory").stringValue();
        SUMO_FullPath = VENTOS_FullPath / SUMO_Path;
        // check if this directory is valid?
        if( !exists( SUMO_FullPath ) )
        {
            error("SUMO directory is not valid! Check it again.");
        }

        // register signals
        Signal_system = registerSignal("system");
        simulation.getSystemModule()->subscribe("system", this);
        Signal_executeFirstTS = registerSignal("executeFirstTS");
        simulation.getSystemModule()->subscribe("executeFirstTS", this);

        if(UseAccidents)
        {
            ifstream edgeRemovals(SUMO_FullPath.string() + "/EdgeRemovals.txt");
            string line;
            while(getline(edgeRemovals, line))
            {
                stringstream ls(line);
                string edgeID;
                int start;
                int end;
                ls >> edgeID >> start >> end;
                EdgeRemovals.push_back(EdgeRemoval(edgeID, start, end));
            }
            if(EdgeRemovals.size() > 0)
            {
                routerMsg = new cMessage("routerMsg");   //Create a new internal message
                scheduleAt(AccidentCheckInterval, routerMsg); //Schedule them to start sending
            }
        }

        net = new Net(SUMO_FullPath.string(), this->getParentModule());

        parseLaneCostsFile();

    }
    else if (stage == 1)
    {
        int numNonRerouting = (double)totalVehicleCount * nonReroutingVehiclePercent;

        int TLMode = (*net->TLs.begin()).second->TLLogicMode;
        ostringstream filePrefix;
        ostringstream filePrefixNoTL;
        filePrefix << totalVehicleCount << "_" << nonReroutingVehiclePercent << "_" << TLMode;
        filePrefixNoTL << totalVehicleCount << "_" << nonReroutingVehiclePercent;
        string NonReroutingFileName = VENTOS_FullPath.string() + "results/router/" + filePrefixNoTL.str() + "_nonRerouting" + ".txt";
        if(fexists(NonReroutingFileName.c_str()))
        {
            nonReroutingVehicles = new set<string>();
            ifstream NonReroutingFile(NonReroutingFileName);
            string vehNum;
            while(NonReroutingFile >> vehNum)
                nonReroutingVehicles->insert(vehNum);
            NonReroutingFile.close();
        }
        else
        {
            nonReroutingVehicles = randomUniqueVehiclesInRange(numNonRerouting, 0, totalVehicleCount);
            ofstream NonReroutingFile;
            NonReroutingFile.open(NonReroutingFileName.c_str());
            for(string veh : *nonReroutingVehicles)
                NonReroutingFile << veh << endl;
            NonReroutingFile.close();
        }

        string endTimeFile = VENTOS_FullPath.string() + "results/router/" + filePrefix.str() + "_endTimes.txt";
        vehicleEndTimesFile.open(endTimeFile.c_str());

        if(collectVehicleTimeData)
        {
            string TravelTimesFileName = VENTOS_FullPath.string() + "results/router/" + filePrefix.str() + ".txt";
            vehicleTravelTimesFile.open(TravelTimesFileName.c_str());  //Open the edgeWeights file
        }
    }
}

void Router::handleMessage(cMessage* msg)
{
    checkEdgeRemovals();

    routerMsg = new cMessage("routerMsg");   //Create a new internal message
    scheduleAt(simTime().dbl() + AccidentCheckInterval, routerMsg); //Schedule them to start sending
}

void Router::receiveSignal(cComponent *source, simsignal_t signalID, long i)
{
    Enter_Method_Silent();

    if(signalID == Signal_executeFirstTS)
    {
        if(laneCostsMode == MODE_EWMA || laneCostsMode == MODE_AVERAGE)
            laneCostsData();

        // if simulation is about to end
        if(i && laneCostsMode == MODE_AVERAGE)
            LaneCostsToFile();
    }
}

void Router::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj)
{
    if(signalID == Signal_system) // Check if it's the right kind of symbol q
    {
        systemData *s = static_cast<systemData *>(obj); // Cast to a systemData class
        if(string(s->getRecipient()) == "system")  // If this is the intended target
        {
            //If request is for a dijkstra's route
            if(s->getRequestType() == DIJKSTRA)
            {
                list<string> info = getRoute(net->edges[s->getEdge()], net->nodes[s->getNode()], s->getSender());
                simsignal_t Signal_router = registerSignal("router");// Prepare to send a router message
                // Systemdata wants string edge, string node, string sender, int requestType, string recipient, list<string> edgeList
                nodePtr->emit(Signal_router, new systemData("", "", "router", DIJKSTRA, s->getSender(), info));
            }

            else if(s->getRequestType() == HYPERTREE)   // Request with new routing
            {
                simsignal_t Signal_router = registerSignal("router");// Prepare to send a router message
                list<string> info;
                Edge* curEdge = net->edges[s->getEdge()];
                info.push_back(curEdge->id);

                Node* targetNode = net->nodes[s->getNode()];

                Hypertree* ht;
                // Return memoization only if the vehicle has traveled less than X intersections, otherwise recalculate a new one
                if(hypertreeMemo.find(s->getNode()) == hypertreeMemo.end() /*&&  if old hyperpath is less than 60 second old*/)
                    hypertreeMemo[s->getNode()] = buildHypertree(simTime().dbl(), net->nodes[targetNode->id]);
                ht = hypertreeMemo[s->getNode()];
                string nextEdge = ht->transition[key(curEdge->from, curEdge->to, simTime().dbl())];
                if(nextEdge != "end")
                    info.push_back(nextEdge);
                nodePtr->emit(Signal_router, new systemData("", "", "router", HYPERTREE, s->getSender(), info));
            }
            else if(s->getRequestType() == DONE)   //Vehicle is done message
            {
                currentVehicleCount--;
                string SUMOvID = s->getSender();
                if(ev.isGUI()) cout << "(" << currentVehicleCount << " left)" << endl;
                int currentRun = ev.getConfigEx()->getActiveRunNumber();
                if(collectVehicleTimeData)
                {
                    vehicleTravelTimes[SUMOvID] = simTime().dbl() - vehicleTravelTimes[SUMOvID];
                    vehicleTravelTimesFile << SUMOvID << " " << vehicleTravelTimes[SUMOvID] << endl;
                }
                if(currentVehicleCount == 0)
                {
                    if(collectVehicleTimeData)
                    {
                        vehicleTravelTimesFile.close();

                        double avg = 0;
                        int count = 0;
                        for(map<string, int>::iterator it = vehicleTravelTimes.begin(); it != vehicleTravelTimes.end(); it++)
                        {
                            count++;
                            avg += it->second;
                        }
                        avg /= count;
                        if(ev.isGUI()) cout << "Average vehicle travel time was " << avg << " seconds." << endl;

                        vehicleTravelTimesFile.close();

                        int TLMode = (*net->TLs.begin()).second->TLLogicMode;
                        ostringstream filePrefix;
                        filePrefix << totalVehicleCount << "_" << nonReroutingVehiclePercent << "_" << TLMode;

                        ofstream outfile;
                        string fileName = VENTOS_FullPath.string() + "results/router/AverageTravelTimes.txt";
                        outfile.open(fileName.c_str(), ofstream::app);  //Open the edgeWeights file
                        outfile << filePrefix.str() <<": " << avg << " " << simTime().dbl() << endl;
                        outfile.close();
                    }

                    for(map<string, TrafficLightRouter*>::iterator tl = net->TLs.begin(); tl != net->TLs.end(); tl++)
                        (*tl).second->finish();
                }
            }
            else if(s->getRequestType() == 3 and collectVehicleTimeData)   //Vehicle is created message
            {
                string SUMOvID = s->getSender();
                vehicleTravelTimes[SUMOvID] = simTime().dbl();
            }
        }// if recipient is right
    }// if Signal_system
    delete obj;
}

void Router::issueStop(string vehID, string edgeID)
{
    Edge& edge = *net->edges[edgeID];
    int randLane = rand() % edge.lanes.size();
    Lane* lane = edge.lanes[randLane];
    TraCI->vehicleSetStop(vehID, lane->id, lane->length - 1, randLane, 100000, 2);
}

void Router::issueStart(string vehID)
{

    TraCI->vehicleResume(vehID);
}

void Router::checkEdgeRemovals()
{
    int curTime = simTime().dbl();

    for(EdgeRemoval& er : EdgeRemovals)
    {
        if(er.start <= curTime && er.end > curTime) //If edge is currently removed
        {
            Edge& edge = *net->edges[er.edge];
            edge.disabled = true;   //mark it as disabled

            for(Lane* lane : edge.lanes) //For each lane
            {
                list<string> vehicleIDs = TraCI->laneGetLastStepVehicleIDs(lane->id); //Get all vehicles on that lane
                for(string veh : vehicleIDs)    //For each vehicle
                {
                    if(find(RemovedVehicles.begin(), RemovedVehicles.end(), veh) == RemovedVehicles.end()) //if vehicle isn't yet paused
                    {
                        issueStop(veh, edge.id); //pause vehicle
                        RemovedVehicles.insert(veh); //Add vehicle to paused vehicles
                    }
                }
            }

            Node& sourceNode = *edge.from;
            for(Edge* incEdge : sourceNode.inEdges) //For each edge leading into the source node
            {
                for(Lane* lane : incEdge->lanes) //For each lane
                {
                    list<string> vehicleIDs = TraCI->laneGetLastStepVehicleIDs(lane->id); //Get all vehicles on that lane
                    for(string veh : vehicleIDs) //For each vehicle on lane
                    {
                        if(find(nonReroutingVehicles->begin(), nonReroutingVehicles->end(), veh) == nonReroutingVehicles->end())
                        {
                            //if vehicle is routing
                            sendRerouteSignal(veh); //force vehicle to reroute
                        }
                    }
                }
            }
        }
        else //if edge not currently removed
        {
            Edge& edge = *net->edges[er.edge];
            if(edge.disabled == true)
            {
                edge.disabled = false;
                for(Lane* lane : edge.lanes) //For each lane
                {
                    list<string> vehicleIDs = TraCI->laneGetLastStepVehicleIDs(lane->id); //Get all vehicles on that lane
                    for(string veh : vehicleIDs)    //For each vehicle
                    {
                        issueStart(veh); //Resume vehicle
                        RemovedVehicles.erase(veh); //And remove it from the set of paused vehicles
                    }
                }
            }
        }
    }
}



void Router::parseLaneCostsFile()
{
    ifstream inFile;
    string fileName = SUMO_FullPath.string() + "/edgeWeights.txt";
    inFile.open(fileName.c_str());  //Open the edgeWeights file

    string edgeName;
    while(inFile >> edgeName)   //While there are more edges to read
    {
        EdgeCosts& ec = edgeCosts[edgeName];//Get the histogram for the edge
        inFile >> ec.count;                      //And read in the number of data points
        int readValuesCount = 0;
        while(readValuesCount < ec.count)  //While we haven't read in all the data points
        {
            int time, timeCount;
            inFile >> time;      //Read in a value and how many times it occurs
            inFile >> timeCount;
            ec.data[time] = timeCount; //And write this to the histogram
            ec.average += time * timeCount;
            readValuesCount += timeCount;  //And mark we read this many more data points
        }
        ec.average /= readValuesCount;
    }
    inFile.close();
}

// is called at the end of simulation
void Router::LaneCostsToFile()
{
    ofstream outFile;
    string fileName = SUMO_FullPath.string() + "/edgeWeights.txt";
    outFile.open(fileName.c_str()); //Open the edgeWeights file

    for(auto& pair : edgeCosts)
    {
        string name = pair.first;
        if(name != "") //If it has a name (empty-ID histograms occur when vehicles update in an intersection)
        {
            EdgeCosts& ec = pair.second;
            outFile << name << " " << ec.count << endl; //Write the edge ID and its number of data points
            for(auto& pair2 : ec.data)
            //for(map<int, int>::iterator it2 = hist->data.begin(); it2 != hist->data.end(); it2++)
            {
                int time = pair2.first;
                int count = pair2.second;
                outFile << time << " " << count << "  ";    //And then write each data point followed by the number of occurrences
            }
            outFile << endl;
        }
    }
}

void Router::laneCostsData()
{
    list<string> vList = TraCI->vehicleGetIDList();

    for(list<string>::iterator it = vList.begin(); it != vList.end(); it++) //Look at each vehicle
    {
        string curEdge = TraCI->vehicleGetEdgeID(*it);  //The edge it's currently on
        if(TraCI->vehicleGetLanePosition(*it) * 1.05 > TraCI->laneGetLength(TraCI->vehicleGetLaneID(*it)))   //If the vehicle is on (or extremely close to) the end of the lane
            curEdge = "";
        string prevEdge = vehicleEdges[*it];    //The last edge we saw it on
        if(vehicleEdges.find(*it) == vehicleEdges.end())    //If we haven't yet seen this vehicle
        {
            vehicleEdges[*it] = curEdge;           //Initialize its current edge
            vehicleTimes[*it] = simTime().dbl();   //And current time
            vehicleLaneChangeCount[*it] = -1;
        }

        if(prevEdge != curEdge) //If we've moved edges
        {
            if(UseHysteresis and ++vehicleLaneChangeCount[*it] > HysteresisCount * 2)
            {
                vehicleLaneChangeCount[*it] = 0;
                sendRerouteSignal(*it);
            }
            edgeCosts[prevEdge].insert(simTime().dbl() - vehicleTimes[*it]);   //Add the time the vehicle traveled to the data set for that edge
            vehicleEdges[*it] = curEdge;                                            //And set its edge to the new one
            vehicleTimes[*it] = simTime().dbl();                                    //And that edges start time to now
            //if(ev.isGUI()) cout << *it << " moves to edge " << curEdge << " at time " << simTime().dbl() << endl;  //Print a change
        }
    }
}

void Router::sendRerouteSignal(string vehID)
{
    simsignal_t Signal_router = registerSignal("router");// Prepare to send a router message
    string curEdge = TraCI->vehicleGetEdgeID(vehID);
    string dest = net->vehicles[vehID]->destination;
    list<string> info = getRoute(net->edges[curEdge], net->nodes[dest], vehID);
    nodePtr->emit(Signal_router, new systemData("", "", "router", 0, vehID, info));
}

double Router::turnTypeCost(Edge* start, Edge* end)
{
    string key = start->id + end->id;
    char type = (*net->turnTypes)[key];
    switch (type)
    {
        case 's':
            return straightCost;
        case 'r':
            return rightTurnCost;
        case 'l':
            return leftTurnCost;
        case 't':
            return uTurnCost;
    }
    if(ev.isGUI()) cout << "Turn did not have an associated type!  This should never happen." << endl;
    return 100000;
}

double Router::junctionCost(double time, Edge* start, Edge* end)
{
    if(start->to->type == "traffic_light")
        return timeToPhase(start->to->tl, time, nextAcceptingPhase(time, start, end));
    else
        return turnTypeCost(start, end);
}

vector<int>* Router::TLTransitionPhases(Edge* start, Edge* end)
{
    return (*net->transitions)[start->id + end->id];
}

double Router::timeToPhase(TrafficLightRouter* tl, double time, int targetPhase)
{
    double *waitTime = new double;
    int curPhase = tl->currentPhaseAtTime(time, waitTime);  //Get the current phase, and how long until it ends

    if(curPhase == targetPhase) //If that phase is active now, we're done
    {
        return 0;
    }
    else
    {
        curPhase = (curPhase + 1) % tl->phases.size();
        if(curPhase == targetPhase)
        {
            return *waitTime;
        }
        else
        {
            while(curPhase != targetPhase)
            {
                *waitTime += (double)tl->phases[curPhase]->duration;
                curPhase = (curPhase + 1) % tl->phases.size();
            }
            return *waitTime;
        }
    }

}

int Router::nextAcceptingPhase(double time, Edge* start, Edge* end)
{
    TrafficLightRouter* tl = start->to->tl;           // The traffic-light in question
    const vector<Phase*> phases = tl->phases;   // And its set of phases

    int curPhase = tl->currentPhaseAtTime(time);
    int phase = curPhase;
    vector<int>* acceptingPhases = TLTransitionPhases(start, end);  // Grab the vector of accepting phases for the given turn

    do
    {
        if(find(acceptingPhases->begin(), acceptingPhases->end(), phase) != acceptingPhases->end()) // If the phase is in the accepting phases, return it
            return phase;
        else    // Otherwise, check the next phase
        {
            phase++;
            if((unsigned)phase >= phases.size())
                phase = 0;
        }
    }while(phase != curPhase);  // Break when we're back to the first phase (should never reach here)

    return -1;
}

Hypertree* Router::buildHypertree(int startTime, Node* destination)
{
    Hypertree* ht = new Hypertree();
    map<string, bool> visited;
    list<Node*> SE;

    for(map<string, Node*>::iterator node = net->nodes.begin(); node != net->nodes.end(); node++)    // Reset the temporary pathing data
    {
        Node* i = (*node).second;                            // i is the destination node
        visited[i->id] = 0;                   // Set each node as not visited
        for(int t = startTime; t <= timePeriodMax; t++)   // For every second in the time interval
        {
            for(vector<Edge*>::iterator inEdge = i->inEdges.begin(); inEdge != i->inEdges.end(); inEdge++)  // For every predecessor to i
            {
                Node* h = (*inEdge)->from;              // Call each predecessor h
                ht->label[key(h, i, t)] = 1000000;      // Set the cost from h to i at time t to infinity
                ht->transition[key(h, i, t)] = "none";  // And the transition target to none
            }
        }
    }
    if(ev.isGUI()) cout << "Searching nodes for " << destination->id << endl;
    Node* D = destination;    // Find the destination, call it D
    for(int t = startTime; t <= timePeriodMax; t++)           // For every second in the time interval
    {
        for(vector<Edge*>::iterator inEdge = D->inEdges.begin(); inEdge != D->inEdges.end(); inEdge++)  // For every predecessor to D
        {
            Node* h = (*inEdge)->from;  // Call each predecessor h
            ht->label[key(h, D, t)] = 0;    // Set the cost from h to D to 0
            ht->transition[key(h, D, t)] = "end";  // And the transition to none
        }
    }

    SE.push_back(D);    // Add the destination node to the scan-eligible list
    while(!SE.empty())  // While the list is not empty
    {
        Node* j = SE.front();   // Set j to the first node
        SE.pop_front();         // And remove the first node from the list
        for(vector<Edge*>::iterator ijEdge = j->inEdges.begin(); ijEdge != j->inEdges.end(); ijEdge++)  // For each predecessor to j, ijEdge
        {
            Node* i = (*ijEdge)->from;                                  // Set i to be the predecessor node
            for(vector<Edge*>::iterator hiEdge = i->inEdges.begin(); hiEdge != i->inEdges.end(); hiEdge++)  // For each predecessor to i, hiEdge
            {
                EdgeCosts* hist = (*ijEdge)->travelTimes;
                Node* h = (*hiEdge)->from;  // Set h to be the predecessor node
                for(int t = startTime; t <= timePeriodMax; t++)   // For every time step of interest
                {
                    double TLDelay = junctionCost(t, *hiEdge, *ijEdge);// The tldelay is the time to the next accepting phase between (h, i) and (i, j)
                    double n = 0;
                    if(hist->count > 0) // If we have histogram data
                    {
                        for(map<int, int>::iterator val = hist->data.begin(); val != hist->data.end(); val++)   // For each unique entry in the history of edge travel times
                        {
                            int travelTime = val->first;                // Set travel time
                            double prob = hist->percentAt(travelTime);  // And calculate its probability
                            double endLabel = ht->label[key(i, j, t + TLDelay + travelTime)];   // The endlabel is the label after (i, j) after we've gone through the TL and traveled (i,j)
                            n += (TLDelay + travelTime + endLabel) * prob;  // Add this weight multiplied by its probability
                        }
                    }
                    else    // Otherwise, use the default getCost() function
                    {
                        n = TLDelay + (*ijEdge)->getCost() + ht->label[key(i, j, t + TLDelay + (*ijEdge)->getCost())];
                    }

                    if (n < ht->label[key(h, i, t)])            // If the newly calculated label is better
                    {
                        ht->label[key(h, i, t)] = n;            // Record the cost for making this transition at this time
                        ht->transition[key(h, i, t)] = (*ijEdge)->id;
                    }

                    if(visited[i->id] == 0) // If i is not in the SE-set
                    {
                        SE.push_back(i);    // Add it
                        visited[i->id] = 1; // And mark it as in the set
                    }
                }   // For each time in the interval
            }   // For each predecessor to i, h
        }   // For each predecessor to j, i
    }   // While SE list has elements

    return ht;
}


list<string> Router::getRoute(Edge* origin, Node* destination, string vName)
{
    for(map<string, Edge*>::iterator it = net->edges.begin(); it != net->edges.end(); it++)  // Reset pathing data
    {
        (*it).second->curCost = 1000000;
        (*it).second->best = NULL;
        (*it).second->visited = 0;
    }

    priority_queue<Edge*, vector<Edge*>, routerCompare> heap;   // Build a priority queue of edge pointers, based on a vector of edge pointers, sorted via routerCompare funciton

    origin->curCost = 0;    // Set the origin's start cost to 0
    heap.push(origin);      // Add the origin to the heap

    vector<string> destinationEdges;
    for(vector<Edge*>::iterator it = destination->inEdges.begin(); it != destination->inEdges.end(); it++)
        destinationEdges.push_back((*it)->id);

    while(!heap.empty())    // While there are unexplored edges (always, if graph is fully connected)
    {
        Edge* parent = heap.top();          // Set parent to the closest unexplored edge
        heap.pop();                         // Remove parent from the heap
        if(parent->visited || parent->disabled)                 // If it was visited already, ignore it
            continue;
        parent->visited = 1;                // If not, we're about to

        double distanceAlongLane = 1;
        if(parent->id == origin->id)    // Vehicles may not necessarily start at the beginning of a lane. Check for that
        {
            double lanePos = TraCI->vehicleGetLanePosition(vName);
            string lane = TraCI->vehicleGetLaneID(vName);
            double laneLength = TraCI->laneGetLength(lane);
            distanceAlongLane = 1 - (lanePos / laneLength); //And modify distanceAlongLane
        }
        double curLaneCost = distanceAlongLane * parent->getCost();
        if(find(destinationEdges.begin(), destinationEdges.end(), parent->id) == destinationEdges.end())   // If we're not at a destination edge
        {
            for(vector<Edge*>::iterator child = parent->to->outEdges.begin(); child != parent->to->outEdges.end(); child++)   // Go through every edge was can get to from the parent
            {
                double newCost = parent->curCost + curLaneCost;                     // Time to get to the junction is the time we get to the edge plus the edge cost
                if(newCost < TLLookahead)
                    newCost += junctionCost(newCost + simTime().dbl(), parent, *child); // The cost at the junction is calculated from when we'd arrive there
                if(!(*child)->visited && newCost < (*child)->curCost)               // If we haven't finished the edge and out new cost is lower
                {
                    (*child)->curCost = newCost;    // Cost to the child is newCost
                    (*child)->best = parent;        // Best path to the child is through the parent
                    heap.push(*child);              // Add the child to the heap
                }
            }
        }
        else // If we found the destination!
        {
            list<string> routeIDs;          // Start backtracking to generate the route
            routeIDs.push_back(parent->id); // Add the end edge
            while(parent->best != NULL)     // While there are more edges
            {
                routeIDs.insert(routeIDs.begin(),parent->best->id); // Add the edge
                parent = parent->best;                              // And go to its parent
            }
            return routeIDs;    // Return the final list
        }
    }// While heap isn't empty
    if(ev.isGUI()) cout << "Pathing failed from " << origin->id << " to " << destination->id << " at t=" << simTime().dbl() << "!  Either destination cannot be reached or vehicle is on an edge with an accident.  Route will not be changed" << endl;
    list<string> ret;
    ret.push_back("failed");
    return ret;
}

}
