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
#include <stdlib.h>
#include "SignalObj.h"
#include <queue>

namespace VENTOS {

Define_Module(VENTOS::Router);

//Generates n random unique ints in range [rangeMin, rangeMax)
//Not a very efficient implementation, but it shouldn't matter much
std::set<std::string>* randomUniqueVehiclesInRange(int numInts, int rangeMin, int rangeMax)
        {
    std::vector<int>* initialInts = new std::vector<int>;
    for(int i = rangeMin; i < rangeMax; i++)
        initialInts->push_back(i);

    if(rangeMin < rangeMax)
        random_shuffle(initialInts->begin(), initialInts->end());

    std::set<std::string>* randInts = new std::set<std::string>;
    for(int i = 0; i < numInts; i++)
        randInts->insert(std::to_string(initialInts->at(i) + 1));

    return randInts;
        }

std::string key(Node* n1, Node* n2, int time)
{
    return n1->id + "#" + n2->id + "#" + std::to_string(time);
}

struct EdgeRemoval
{
    std::string edge;
    int start;
    int end;
    double pos;
    int laneIndex;
    bool blocked = false;
    EdgeRemoval(std::string edge, int start, int end, double pos, int laneIndex, bool blocked):edge(edge), start(start), end(end), pos(pos), laneIndex(laneIndex){}
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
    enableRouter = par("enableRouter").boolValue();
    if(!enableRouter)
        return;

    if(stage == 0)
    {
        // get a pointer to the TraCI module
        cModule *module = simulation.getSystemModule()->getSubmodule("TraCI");
        TraCI = static_cast<TraCI_Commands *>(module);
        ASSERT(TraCI);

        debugLevel = simulation.getSystemModule()->par("debugLevel").longValue();

        EWMARate = par("EWMARate").doubleValue();
        TLLookahead = par("TLLookahead").doubleValue();
        timePeriodMax = par("timePeriodMax").doubleValue();
        UseHysteresis = par("UseHysteresis").boolValue();

        laneCostsMode = static_cast<LaneCostsMode>(par("LaneCostsMode").longValue());
        HysteresisCount = par("HysteresisCount").longValue();

        createTime = par("createTime").longValue();
        totalVehicleCount = par("vehicleCount").longValue();
        currentVehicleCount = totalVehicleCount;
        nonReroutingVehiclePercent = 1 - par("ReroutingVehiclePercent").doubleValue();

        UseAccidents = par("UseAccidents").boolValue();
        AccidentCheckInterval = par("AccidentCheckInterval").longValue();
        collectVehicleTimeData = par("collectVehicleTimeData").boolValue();
        dijkstraOutdateTime = par("dijkstraOutdateTime").longValue();

        // register and subscribe to signals
        Signal_system = registerSignal("system");
        simulation.getSystemModule()->subscribe("system", this);
        Signal_executeEachTS = registerSignal("executeEachTS");
        simulation.getSystemModule()->subscribe("executeEachTS", this);

        // get the file paths
        SUMO_FullPath = TraCI->getSUMOFullDir();
        if( !boost::filesystem::exists( SUMO_FullPath ) )
            error("SUMO directory is not valid! Check it again.");

        if(UseAccidents)
        {
            std::string AccidentFile = SUMO_FullPath.string() + "/EdgeRemovals.txt";
            std::ifstream edgeRemovals(AccidentFile.c_str());
            std::string line;
            while(getline(edgeRemovals, line))
            {
                std::stringstream ls(line);
                std::string edgeID;
                int start;
                int end;
                double pos; //the location of accident
                int laneIndex;
                ls >> edgeID >> start >> end >> pos >> laneIndex;
                EdgeRemovals.push_back(EdgeRemoval(edgeID, start, end, pos, laneIndex, false));
            }

            if(ev.isGUI() && debugLevel > 1)
            {
                std::cout << "Loaded " << EdgeRemovals.size() << " accidents from " << AccidentFile << endl;
                std::cout.flush();
            }

            if(EdgeRemovals.size() > 0)
            {
                routerMsg = new cMessage("routerMsg");   //Create a new internal message
                scheduleAt(AccidentCheckInterval, routerMsg); //Schedule them to start sending
            }
            else
            {
                std::cout << "Accidents are enabled but no accidents were read in!" << endl;
            }
        }

        int ltc = par("leftTurnCost").doubleValue();
        int rtc = par("rightTurnCost").doubleValue();
        int stc = par("straightCost").doubleValue();
        int utc = par("uTurnCost").doubleValue();
        net = new Net(SUMO_FullPath.string(), this->getParentModule(), ltc, rtc, stc, utc);
    }
    else if (stage == 1)
    {
        std::ostringstream filePrefix;
        int TLMode = (*net->TLs.begin()).second->TLLogicMode;
        filePrefix << totalVehicleCount << "_" << nonReroutingVehiclePercent << "_" << TLMode;
        if(nonReroutingVehiclePercent > 0)
        {
            int numNonRerouting = (double)totalVehicleCount * nonReroutingVehiclePercent;

            std::ostringstream filePrefixNoTL;
            filePrefixNoTL << totalVehicleCount << "_" << nonReroutingVehiclePercent;
            std::string NonReroutingFileName = VENTOS_FullPath.string() + "results/router/" + filePrefixNoTL.str() + "_nonRerouting" + ".txt";
            if( boost::filesystem::exists( NonReroutingFileName ) )
            {
                nonReroutingVehicles = new std::set<std::string>();
                std::ifstream NonReroutingFile(NonReroutingFileName);
                std::string vehNum;
                while(NonReroutingFile >> vehNum)
                    nonReroutingVehicles->insert(vehNum);
                NonReroutingFile.close();

                if(ev.isGUI() && debugLevel > 1)
                {
                    std::cout << "Loaded " << numNonRerouting << " nonRerouting vehicles from file " << NonReroutingFileName << endl;
                    std::cout.flush();
                }
            }
            else
            {
                nonReroutingVehicles = randomUniqueVehiclesInRange(numNonRerouting, 0, totalVehicleCount);
                std::ofstream NonReroutingFile;
                NonReroutingFile.open(NonReroutingFileName.c_str());
                for(std::string veh : *nonReroutingVehicles)
                    NonReroutingFile << veh << endl;
                NonReroutingFile.close();

                if(ev.isGUI() && debugLevel > 1)
                {
                    std::cout << "Created " << numNonRerouting << "-vehicle nonRerouting file " << NonReroutingFileName << endl;
                    std::cout.flush();
                }
            }
        }
        else
        {
            nonReroutingVehicles = new std::set<std::string>();
        }

        std::string endTimeFile = VENTOS_FullPath.string() + "results/router/" + filePrefix.str() + "_endTimes.txt";
        vehicleEndTimesFile.open(endTimeFile.c_str());

        if(collectVehicleTimeData)
        {
            std::string TravelTimesFileName = VENTOS_FullPath.string() + "results/router/" + filePrefix.str() + ".txt";

            if(ev.isGUI() && debugLevel > 1)
            {
                std::cout << "Opened edge-weights file at " << TravelTimesFileName << endl;
                std::cout.flush();
            }

            vehicleTravelTimesFile.open(TravelTimesFileName.c_str());  //Open the edgeWeights file
        }

        parseLaneCostsFile();
    }
}


void Router::finish()
{
    if(laneCostsMode == MODE_RECORD)
        LaneCostsToFile();
}


//Receives a signal every time-step
void Router::receiveSignal(cComponent *source, simsignal_t signalID, long i)
{
    Enter_Method_Silent();

    //Runs once per timestep
    if(signalID == Signal_executeEachTS)
    {
        if(laneCostsMode == MODE_EWMA || laneCostsMode == MODE_RECORD || UseHysteresis)
            laneCostsData();
    }
}


void Router::handleMessage(cMessage* msg)
{
    checkEdgeRemovals();

    routerMsg = new cMessage("routerMsg");   //Create a new internal message
    scheduleAt(simTime().dbl() + AccidentCheckInterval, routerMsg); //Schedule them to start sending
}


void Router::receiveDijkstraRequest(Edge* origin, Node* destination, std::string sender)
{
    std::string key = origin->id + "#" + destination->id;

    if(dijkstraTimes.find(key) == dijkstraTimes.end() || (simTime().dbl() - dijkstraTimes[key]) > dijkstraOutdateTime)
    {
        dijkstraTimes[key] = simTime().dbl();
        dijkstraRoutes[key] = getRoute(origin, destination, sender);

        if(ev.isGUI() && debugLevel > 2)
        {
            std::cout << "Created dijkstra's route from " << origin->id << " to " << destination->id << " at t=" << simTime().dbl() << endl;
            std::cout.flush();
        }
    }
    else
    {
        if(ev.isGUI() && debugLevel > 2)
        {
            std::cout << "Using old dijkstras route at t=" << simTime().dbl() << endl;
            std::cout.flush();
        }
    }

    // Systemdata wants string edge, string node, string sender, int requestType, string recipient, list<string> edgeList
    simsignal_t Signal_router = registerSignal("router");
    this->emit(Signal_router, new systemData("", "", "router", DIJKSTRA, sender, dijkstraRoutes[key]));
}

void Router::receiveHypertreeRequest(Edge* origin, Node* destination, std::string sender)
{
    std::list<std::string> info;
    info.push_back(origin->id);

    // Return memoization only if the vehicle has traveled less than X intersections, otherwise recalculate a new one
    if(hypertreeMemo.find(destination->id) == hypertreeMemo.end() /*&&  if old hyperpath is less than 60 second old*/)
        hypertreeMemo[destination->id] = buildHypertree(simTime().dbl(), destination);

    std::string nextEdge = hypertreeMemo[destination->id]->transition[key(origin->from, origin->to, simTime().dbl())];
    if(nextEdge != "end")
        info.push_back(nextEdge);

    simsignal_t Signal_router = registerSignal("router");
    this->emit(Signal_router, new systemData("", "", "router", HYPERTREE, sender, info));
}

void Router::receiveDoneRequest(std::string sender)
{
    //Decrement vehicle count, print
    currentVehicleCount--;

    if(ev.isGUI() && debugLevel > 0)
    {
        std::cout << currentVehicleCount << " vehicles left." << endl;
        std::cout.flush();
    }

    //DTODO: Write currentRun to a file
    //int currentRun = ev.getConfigEx()->getActiveRunNumber();
    if(collectVehicleTimeData)
    {
        vehicleTravelTimes[sender] = simTime().dbl() - vehicleTravelTimes[sender];
        vehicleTravelTimesFile << sender << " " << vehicleTravelTimes[sender] << endl;
    }

    if(currentVehicleCount == 0)
    {
        //If no vehicles are left, print vehicle info and terminate all traffic lights
        if(collectVehicleTimeData)
        {
            vehicleTravelTimesFile.close();

            double avg = 0;
            int count = 0;
            for(std::map<std::string, int>::iterator it = vehicleTravelTimes.begin(); it != vehicleTravelTimes.end(); it++)
            {
                count++;
                avg += it->second;
            }

            avg /= count;
            std::cout << "Average vehicle travel time was " << avg << " seconds." << endl;
            vehicleTravelTimesFile.close();

            int TLMode = (*net->TLs.begin()).second->TLLogicMode;
            std::ostringstream filePrefix;
            filePrefix << totalVehicleCount << "_" << nonReroutingVehiclePercent << "_" << TLMode;
            std::ofstream outfile;
            std::string fileName = VENTOS_FullPath.string() + "results/router/AverageTravelTimes.txt";
            outfile.open(fileName.c_str(), std::ofstream::app);  //Open the edgeWeights file
            outfile << filePrefix.str() <<": " << avg << " " << simTime().dbl() << endl;
            outfile.close();
        }

        for(std::map<std::string, TrafficLightRouter*>::iterator tl = net->TLs.begin(); tl != net->TLs.end(); tl++)
            (*tl).second->finish();
    }
}

void Router::receiveStartedRequest(std::string sender)
{
    vehicleTravelTimes[sender] = simTime().dbl();
}

void Router::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj)
{
    if(signalID != Signal_system)
    {
        delete obj;
        return;
    }

    systemData *s = static_cast<systemData*>(obj);
    if(std::string(s->getRecipient()) != "system") // Check if it's the right kind of symbol
    {
        delete obj;
        return;
    }

    std::string tedge = s->getEdge();
    std::string tnode = s->getNode();
    std::string tsender = s->getSender();

    switch(s->getRequestType())
    {
    case DIJKSTRA:
        receiveDijkstraRequest(net->edges.at(tedge), net->nodes.at(tnode), tsender);
        break;
    case HYPERTREE:
        receiveHypertreeRequest(net->edges.at(tedge), net->nodes.at(tnode), tsender);
        break;
    case DONE:
        receiveDoneRequest(s->getSender());
        break;
    case STARTED:
        receiveStartedRequest(s->getSender());
        break;
    }

    delete obj;
}

void Router::issueStop(std::string vehID, std::string edgeID, double position, int laneIndex)
{
    TraCI->vehicleSetStop(vehID, edgeID, position, laneIndex, 100000, 2);
}

void Router::issueStart(std::string vehID)
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
            if(!er.blocked)  //if the lane is not blocked
            {
                // New implementation
                Edge& edge = *net->edges[er.edge];
                edge.disabled = true;   //mark it as disabled

                // Find the closest vehicle behind the accident location
                std::string laneID = er.edge + "_" + std::to_string(er.laneIndex); // Construct the accident lane ID
                std::list<std::string> vehicleIDs = TraCI->laneGetLastStepVehicleIDs(laneID); // Get the vehicles on that lane

                if(!vehicleIDs.empty()) // If there are vehicles on that link
                {
                    vehicleIDs.reverse(); // Reverse the list so it can start from the further end
                    for(std::string veh : vehicleIDs)
                    {
                        if(TraCI->vehicleGetLanePosition(veh) + 100 < er.pos) // Here, we ask the vehicle to stop at the location 100 meters ahead of its current position to avoid the error of " too close to stop". This is not ideal, but it works for the purpose
                        {
                            //if(veh == "v22")
                            //std::cout << "Found it!" << std::endl;
                            issueStop(veh, er.edge, TraCI->vehicleGetLanePosition(veh) + 100, er.laneIndex);
                            // Change its color to red
                            RGB newColor = Color::colorNameToRGB("red");
                            TraCI->vehicleSetColor(veh, newColor);
                            er.blocked = true;
                            break;
                        }
                    }
                }
            }
        }
        else //if edge not currently removed
        {
            if(er.blocked == true)
            {
                std::string laneID = er.edge + "_" + std::to_string(er.laneIndex);
                std::list<std::string> vehicleIDs = TraCI->laneGetLastStepVehicleIDs(laneID);

                for(std::string veh : vehicleIDs)
                {
                    uint8_t state= TraCI->vehicleGetStopState(veh);
                    //std::cout << "Vehicle: " << veh << ", State:" << state << std::endl;

                    if(state == 5) // Find a stopped and triggered vehicle
                    {
                        issueStart(veh);
                        // Change its color to white denotes the vehicle starts to moving again
                        RGB newColor = Color::colorNameToRGB("white");
                        TraCI->vehicleSetColor(veh, newColor);
                        er.blocked = false;
                        break;
                    }
                }
            }
        }
    }
}


void Router::parseLaneCostsFile()
{
    std::ifstream inFile;
    std::string fileName = SUMO_FullPath.string() + "/edgeWeights.txt";
    inFile.open(fileName.c_str());  //Open the edgeWeights file

    std::string edgeName;
    while(inFile >> edgeName)   //While there are more edges to read
    {
        int max;
        inFile >> max;
        int readCount = 0;
        std::map<int, int> m;
        int value, valueCount;
        while(readCount < max)
        {
            inFile >> value >> valueCount;
            m[value] = valueCount;
            readCount += valueCount;
        }

        net->edges.at(edgeName)->travelTimes = EdgeCosts(m);
        if(ev.isGUI() && debugLevel > 1)
        {
            std::cout << "Loaded costs for " << edgeName << ": " << net->edges.at(edgeName)->travelTimes.average << endl;
            std::cout.flush();
        }
    }

    inFile.close();
}

// is called at the end of simulation
void Router::LaneCostsToFile()
{
    std::ofstream outFile;
    std::string fileName = SUMO_FullPath.string() + "/edgeWeights.txt";
    outFile.open(fileName.c_str()); //Open the edgeWeights file

    for(auto& pair : net->edges)
    {
        std::string name = pair.first;
        if(name != "") //If it has a name (empty-ID histograms occur when vehicles update in an intersection)
        {
            EdgeCosts& ec = pair.second->travelTimes;
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
    std::list<std::string> vList = TraCI->vehicleGetIDList();

    for(std::string vehicle : vList)
    {
        std::string curEdge = TraCI->vehicleGetEdgeID(vehicle);
        if(curEdge != "")
        {
            if(vehicleEdges.find(vehicle) != vehicleEdges.end())
            {
                if(vehicleEdges[vehicle] != curEdge)
                {
                    if(ev.isGUI() && debugLevel > 2)
                    {
                        std::cout << vehicle << " changes lanes to " << curEdge << " at t=" << simTime().dbl() << "(" << vehicleLaneChangeCount[vehicle] << ")" << endl;
                        std::cout.flush();
                    }

                    double time = simTime().dbl() - vehicleTimes[vehicle];
                    net->edges.at(vehicleEdges[vehicle])->travelTimes.insert(time);
                    vehicleEdges[vehicle] = curEdge;
                    vehicleTimes[vehicle] = simTime().dbl();
                    ++vehicleLaneChangeCount[vehicle];
                    if(UseHysteresis && vehicleLaneChangeCount[vehicle] == HysteresisCount)
                    {
                        vehicleLaneChangeCount[vehicle] = 0;

                        sendRerouteSignal(vehicle);
                        if(ev.isGUI() && debugLevel > 1)
                        {
                            std::cout << "Hystereis rerouting " << vehicle << " at t=" << simTime().dbl() << endl;
                            std::cout.flush();
                        }
                    }
                }
            }
            else
            {
                vehicleEdges[vehicle] = curEdge;
                vehicleTimes[vehicle] = simTime().dbl();
                vehicleLaneChangeCount[vehicle] = 0;
            }
        }
    }
}

void Router::sendRerouteSignal(std::string vehID)
{
    std::string curEdge = TraCI->vehicleGetEdgeID(vehID);
    std::string dest = net->vehicles[vehID]->destination;
    receiveDijkstraRequest(net->edges.at(curEdge), net->nodes.at(dest), vehID);
}


Hypertree* Router::buildHypertree(int startTime, Node* destination)
{
    Hypertree* ht = new Hypertree();
    std::map<std::string, bool> visited;
    std::list<Node*> SE;

    for(std::map<std::string, Node*>::iterator node = net->nodes.begin(); node != net->nodes.end(); node++)    // Reset the temporary pathing data
    {
        Node* i = (*node).second;                            // i is the destination node
        visited[i->id] = 0;                   // Set each node as not visited
        for(int t = startTime; t <= timePeriodMax; t++)   // For every second in the time interval
        {
            for(std::vector<Edge*>::iterator inEdge = i->inEdges.begin(); inEdge != i->inEdges.end(); inEdge++)  // For every predecessor to i
            {
                Node* h = (*inEdge)->from;              // Call each predecessor h
                ht->label[key(h, i, t)] = 1000000;      // Set the cost from h to i at time t to infinity
                ht->transition[key(h, i, t)] = "none";  // And the transition target to none
            }
        }
    }

    if(ev.isGUI() && debugLevel > 2)
    {
        std::cout << "Generating a hypertree for " << destination->id << endl;
        std::cout.flush();
    }

    Node* D = destination;    // Find the destination, call it D
    for(int t = startTime; t <= timePeriodMax; t++)           // For every second in the time interval
    {
        for(std::vector<Edge*>::iterator inEdge = D->inEdges.begin(); inEdge != D->inEdges.end(); inEdge++)  // For every predecessor to D
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
        for(std::vector<Edge*>::iterator ijEdge = j->inEdges.begin(); ijEdge != j->inEdges.end(); ijEdge++)  // For each predecessor to j, ijEdge
        {
            Node* i = (*ijEdge)->from;                                  // Set i to be the predecessor node
            for(std::vector<Edge*>::iterator hiEdge = i->inEdges.begin(); hiEdge != i->inEdges.end(); hiEdge++)  // For each predecessor to i, hiEdge
            {
                EdgeCosts& travelTimes = (*ijEdge)->travelTimes;
                Node* h = (*hiEdge)->from;  // Set h to be the predecessor node
                for(int t = startTime; t <= timePeriodMax; t++)   // For every time step of interest
                {
                    double TLDelay = net->junctionCost(t, *hiEdge, *ijEdge);// The tldelay is the time to the next accepting phase between (h, i) and (i, j)
                    double n = 0;
                    if(travelTimes.count > 0) // If we have histogram data
                    {
                        for(std::map<int, int>::iterator val = travelTimes.data.begin(); val != travelTimes.data.end(); val++)   // For each unique entry in the history of edge travel times
                        {
                            int travelTime = val->first;                // Set travel time
                            double prob = travelTimes.percentAt(travelTime);  // And calculate its probability
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


std::list<std::string> Router::getRoute(Edge* origin, Node* destination, std::string vName)
{
    for(auto& pair : net->edges)
    {
        Edge* e = pair.second;
        e->curCost = 1000000;
        e->best = NULL;
        e->visited = 0;
    }

    // Build a priority queue of edge pointers, based on a vector of edge pointers, sorted via routerCompare funciton
    std::priority_queue<Edge*, std::vector<Edge*>, routerCompare> heap;

    origin->curCost = 0;    // Set the origin's start cost to 0
    heap.push(origin);      // Add the origin to the heap

    std::vector<std::string> destinationEdges;
    for(std::vector<Edge*>::iterator it = destination->inEdges.begin(); it != destination->inEdges.end(); it++)
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
            std::string lane = TraCI->vehicleGetLaneID(vName);
            double laneLength = TraCI->laneGetLength(lane);
            distanceAlongLane = 1 - (lanePos / laneLength); //And modify distanceAlongLane
        }
        double curLaneCost = distanceAlongLane * parent->getCost();
        if(find(destinationEdges.begin(), destinationEdges.end(), parent->id) == destinationEdges.end())   // If we're not at a destination edge
        {
            for(std::vector<Edge*>::iterator child = parent->to->outEdges.begin(); child != parent->to->outEdges.end(); child++)   // Go through every edge was can get to from the parent
            {
                double newCost = parent->curCost + curLaneCost;                     // Time to get to the junction is the time we get to the edge plus the edge cost
                if(newCost < TLLookahead)
                    newCost += net->junctionCost(newCost + simTime().dbl(), parent, *child); // The cost at the junction is calculated from when we'd arrive there
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
            std::list<std::string> routeIDs;          // Start backtracking to generate the route
            routeIDs.push_back(parent->id); // Add the end edge
            while(parent->best != NULL)     // While there are more edges
            {
                routeIDs.insert(routeIDs.begin(),parent->best->id); // Add the edge
                parent = parent->best;                              // And go to its parent
            }
            return routeIDs;    // Return the final list
        }
    }// While heap isn't empty

    if(ev.isGUI() && debugLevel > 0)
    {
        // Either destination cannot be reached or vehicle is on an edge with an accident. Route will not be changed.
        std::cout << "t=" << simTime().dbl() << ": " << "Pathing failed from " << origin->id << " to " << destination->id << endl;
        std::cout.flush();
    }

    std::list<std::string> ret;
    ret.push_back("failed");
    return ret;
}

}
