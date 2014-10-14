#include "Router.h"
//#include <algorithm>

namespace VENTOS {

Define_Module(VENTOS::Router);

Router::~Router()
{

}


void Router::initialize(int stage)
{
    if(stage == 0)
    {
        enableRouting = par("enableRouting").boolValue();
        if(!enableRouting)
            return;

        //Build nodePtr and traci manager
        nodePtr = FindModule<>::findHost(this);
        TraCI = FindModule<TraCI_Extend*>::findGlobalModule();
        if(nodePtr == NULL || TraCI == NULL)
            error("can not get a pointer to the module.");

        //register signals
        Signal_system = registerSignal("system");
        simulation.getSystemModule()->subscribe("system", this);

        leftTurnCost = par("leftTurnCost").doubleValue();
        rightTurnCost = par("rightTurnCost").doubleValue();
        straightCost = par("straightCost").doubleValue();
        uTurnCost = par("uTurnCost").doubleValue();
        TLLookahead = par("TLLookahead").doubleValue();
        timePeriodMax = par("timePeriodMax").doubleValue();

        // get the file paths
        boost::filesystem::path SUMODirectory = simulation.getSystemModule()->par("SUMODirectory").stringValue();
        boost::filesystem::path VENTOSfullDirectory = cSimulation::getActiveSimulation()->getEnvir()->getConfig()->getConfigEntry("network").getBaseDirectory();
        string netFile = (VENTOSfullDirectory / SUMODirectory / "/hello.net.xml").string();

        net = new Net(netFile, this->getParentModule());
    } // if stage == 0
}


void Router::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj)
{
    if(signalID == Signal_system) //Check if it's the right kind of symbol q
    {
        systemData *s = static_cast<systemData *>(obj); //Cast to a systemData class
        if(string(s->getRecipient()) == "system")  //If this is the intended target
        {
            if(s->getRequestType() == 0)    //Request with dijkstra's routing
            {
                list<string> info = getRoute(net->edges[s->getEdge()], net->nodes[s->getNode()], s->getSender());
                simsignal_t Signal_router = registerSignal("router");//Prepare to send a router message
                //Systemdata wants string edge, string node, string sender, int requestType, string recipient, list<string> edgeList
                nodePtr->emit(Signal_router, new systemData("", "", "router", 0, s->getSender(), info));
            }//If request type is 0
            else if(s->getRequestType() == 1)   //Request with new routing
            {
                simsignal_t Signal_router = registerSignal("router");//Prepare to send a router message
                list<string> info;
                Edge* curEdge = net->edges[s->getEdge()];
                info.push_back(curEdge->id);

                Node* targetNode = net->nodes[s->getNode()];

                Hypertree* ht;
                //Return memoization only if the vehicle has traveled less than X intersections, otherwise recalculate a new one
                //(this counter is kept vehicle side)
                if(hypertreeMemo.find(s->getNode()) == hypertreeMemo.end() /*&&  if old hyperpath is less than 60 second old*/)
                    hypertreeMemo[s->getNode()] = buildHypertree(simTime().dbl(), net->nodes[targetNode->id]);
                ht = hypertreeMemo[s->getNode()];
                string nextEdge = ht->transition[key(curEdge->from, curEdge->to, simTime().dbl())];
                //cout << "Estimated time is " << ht->label[key(curEdge->from, curEdge->to, simTime().dbl())] << endl;
                if(nextEdge != "end")
                    info.push_back(nextEdge);
                nodePtr->emit(Signal_router, new systemData("", "", "router", 1, s->getSender(), info));
            }
        }//if recipient is right
    }//if Signal_system
}

ostream& operator<<(ostream& os, Router &rhs) // Print a router's network
{
    os << "Nodes:" << endl;
    for(map<string, Node*>::iterator it = rhs.net->nodes.begin(); it != rhs.net->nodes.end(); it++)
        os << (*it).second << endl;
    os << endl;
    os << "Edges:" << endl;
    for(map<string, Edge*>::iterator it = rhs.net->edges.begin(); it != rhs.net->edges.end(); it++)
        os << (*it).second << endl;
    return os;
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
    cout << "Turn did not have an associated type!  This should never happen." << endl;
    return 100000;
}

double Router::junctionCost(double time, Edge* start, Edge* end)
{
    if(start->to->type == "traffic_light")
        return timeToPhase(start->to->tl, time, nextAcceptingPhase(time, start, end));
    else
        return turnTypeCost(start, end);
}

double Router::getEdgeMeanSpeed(Edge *edge)
{
    double sum = 0;
    for(vector<Lane*>::iterator it = edge->lanes->begin(); it != edge->lanes->end(); it++)  //For every lane on the edge
        sum += ((*it)->length / TraCI->getCommandInterface()->getLaneMeanSpeed((*it)->id));   //Get the mean speed of that lane
    return sum / edge->lanes->size();   //Average them all together
}

class routerCompare //Comparator object for getRoute weighting
{
public:
    bool operator()(const Edge* n1, const Edge* n2)
    {
        return n1->curCost > n2->curCost;
    }
};

/* TODO:
 *
 * Create code to run data-gathering cases.  This should write to a file the estimated
 * travel time (tau) vectors and associated probabilities (rho) for every edge.  This should be
 * improved every run (when a bool is set to true), and used when that variable is set
 * to false. --DONE
 *
 * Generate the transition (pi) vector for each node.  Given a time and a predecessor, this should
 * point to the next node that should be visited. -- DONE
 *
 * Calculate the traffic-light movement costs (psi) -- done, but needs to be optimized. --DONE
 *
 * Return the hypertree to the vehicle, and have it work through that tree independently.
 *
 * Remember: the label (lambda) is never calculated directly, serves only as a label.
 *
 *
 *
 *
 *  Step 0: Initialization
 *      For each non-destination node i, for every time in our time-range, for every predecessor h of node i:
 *          Set lambda (label) from h to i to infinity
 *          Set pi (transition) from h to i to null (or infinity)
 *      For every time in our time-range and for every predecessor h of the destination node D:
 *          Set lambda (label) from h to D to 0
 *          Set pi (transition) from h to D to 0
 *      Insert D into the SE list
 *
 *  Step 1: Scanning the list
 *      If the list is empty, finished
 *      Otherwise, pop the first node j from the list (FIFO)
 *
 *  Step 2: Update labels
 *      For every predecessor i of j:
 *          For every for every predecessor h of i:
 *              l = TL cost from (h,i) to (i,j)
 *              For every time in our time-range:
 *                  n from h to i = BIG FORMULA
 *                  if n from h to i < lambda from h to i:
 *                      lambda from h to i = n from h to i
 *                      pi from h to i = j
 *                      list += i
 *
 */

#define SSTR( x ) dynamic_cast< std::ostringstream & >( (std::ostringstream() << std::dec << x ) ).str()

string key(Node* n1, Node* n2, int time)
{
    string s = n1->id + "#" + n2->id + "#" + SSTR(time);
    //cout << "Using key " << s << endl;
    return s;
}

vector<int>* Router::TLTransitionPhases(Edge* start, Edge* end)
{
    vector<int>* ret = (*net->transitions)[start->id + end->id];
    return ret;
}

double Router::timeToPhase(TrafficLight* tl, double time, int targetPhase)
{
    const vector<Phase*> phases = (tl->phases);   //The traffic light's phases

    double timeRemaining = 0;
    int curPhase = tl->currentPhaseAtTime(time, &timeRemaining);
    if(curPhase == targetPhase)
        return 0;
    else
    {
        double waitTime = timeRemaining;

        do
        {
            if(++curPhase >= phases.size())
                curPhase = 0;
            waitTime += phases[curPhase]->duration;
        } while(targetPhase != curPhase);
        waitTime -= phases[curPhase]->duration;

        return waitTime;
    }

}

int Router::nextAcceptingPhase(double time, Edge* start, Edge* end)
{
    TrafficLight* tl = start->to->tl;               //The traffic-light in question
    const vector<Phase*> phases = tl->phases;   //And its set of phases

    int curPhase = tl->currentPhaseAtTime(time);
    int phase = curPhase;
    vector<int>* acceptingPhases = TLTransitionPhases(start, end);  //Grab the vector of accepting phases for the given turn

    do
    {
        if(find(acceptingPhases->begin(), acceptingPhases->end(), phase) != acceptingPhases->end()) //If the phase is in the accepting phases, return it
            return phase;
        else    //Otherwise, check the next phase
        {
            phase++;
            if(phase >= phases.size())
                phase = 0;
        }
    }while(phase != curPhase);  //Break when we're back to the first phase (should never reach here)
    return -1;
}

Hypertree* Router::buildHypertree(int startTime, Node* destination)
{
    Hypertree* ht = new Hypertree();
    map<string, bool> visited;
    list<Node*> SE;

    for(map<string, Node*>::iterator node = net->nodes.begin(); node != net->nodes.end(); node++)    //Reset the temporary pathing data
    {
        Node* i = (*node).second;                            //i is the destination node
        visited[i->id] = 0;                   //Set each node as not visited
        for(int t = startTime; t <= timePeriodMax; t++)   //For every second in the time interval
        {
            for(vector<Edge*>::iterator inEdge = i->inEdges.begin(); inEdge != i->inEdges.end(); inEdge++)  //For every predecessor to i
            {
                Node* h = (*inEdge)->from;              //Call each predecessor h
                ht->label[key(h, i, t)] = 1000000;      //Set the cost from h to i at time t to infinity
                ht->transition[key(h, i, t)] = "none";  //And the transition target to none
            }
        }
    }
    cout << "Searching nodes for " << destination->id << endl;
    Node* D = destination;    //Find the destination, call it D
    for(int t = startTime; t <= timePeriodMax; t++)           //For every second in the time interval
    {
        for(vector<Edge*>::iterator inEdge = D->inEdges.begin(); inEdge != D->inEdges.end(); inEdge++)  //For every predecessor to D
        {
            Node* h = (*inEdge)->from;  //Call each predecessor h
            ht->label[key(h, D, t)] = 0;    //Set the cost from h to D to 0
            ht->transition[key(h, D, t)] = "end";  //And the transition to none
        }
    }

    SE.push_back(D);    //Add the destination node to the scan-eligible list
    while(!SE.empty())  //While the list is not empty
    {
        Node* j = SE.front();   //Set j to the first node
        SE.pop_front();         //And remove the first node from the list
        for(vector<Edge*>::iterator ijEdge = j->inEdges.begin(); ijEdge != j->inEdges.end(); ijEdge++)  //For each predecessor to j, ijEdge
        {
            Node* i = (*ijEdge)->from;                                  //Set i to be the predecessor node
            for(vector<Edge*>::iterator hiEdge = i->inEdges.begin(); hiEdge != i->inEdges.end(); hiEdge++)  //For each predecessor to i, hiEdge
            {
                Histogram* hist = (*ijEdge)->travelTimes;
                Node* h = (*hiEdge)->from;  //Set h to be the predecessor node
                for(int t = startTime; t <= timePeriodMax; t++)   //For every time step of interest
                {
                    double TLDelay = junctionCost(t, *hiEdge, *ijEdge);//The tldelay is the time to the next accepting phase between (h, i) and (i, j)
                    double n = 0;
                    if(hist->count > 0) //If we have histogram data
                    {
                        for(map<int, int>::iterator val = hist->data.begin(); val != hist->data.end(); val++)   //For each unique entry in the history of edge travel times
                        {
                            int travelTime = val->first;                //Set travel time
                            double prob = hist->percentAt(travelTime);  //And calculate its probability
                            double endLabel = ht->label[key(i, j, t + TLDelay + travelTime)];   //The endlabel is the label after (i, j) after we've gone through the TL and traveled (i,j)
                            n += (TLDelay + travelTime + endLabel) * prob;  //Add this weight multiplied by its probability
                        }
                    }
                    else    //Otherwise, use the default getCost() function
                    {
                        n = TLDelay + (*ijEdge)->getCost() + ht->label[key(i, j, t + TLDelay + (*ijEdge)->getCost())];
                    }

                    if (n < ht->label[key(h, i, t)])            //If the newly calculated label is better
                    {
                        ht->label[key(h, i, t)] = n;            //Record the cost for making this transition at this time
                        ht->transition[key(h, i, t)] = (*ijEdge)->id;
                    }

                    if(visited[i->id] == 0) //If i is not in the SE-set
                    {
                        SE.push_back(i);    //Add it
                        visited[i->id] = 1; //And mark it as in the set
                    }
                }   //For each time in the interval
            }   //For each predecessor to i, h
        }   //For each predecessor to j, i
    }   //While SE list has elements

    //Debug stuff!

    /*
    int i = 0;
    for(map<string,double>::iterator l = ht->label.begin(); l != ht->label.end(); l++)
    {
        cout << l->first << "->" << l->second << "  " << ht->transition[l->first] << endl;
        i++;
    }
    cout << "Total: " << i << " entries" << endl;*/
    return ht;
}


list<string> Router::getRoute(Edge* origin, Node* destination, string vName)
{
    for(map<string, Edge*>::iterator it = net->edges.begin(); it != net->edges.end(); it++)  //Reset pathing data
    {
        (*it).second->curCost = 1000000;
        (*it).second->best = NULL;
        (*it).second->visited = 0;
    }

    priority_queue<Edge*, vector<Edge*>, routerCompare> heap;   //Build a priority queue of edge pointers, based on a vector of edge pointers, sorted via routerCompare funciton

    origin->curCost = 0;    //Set the origin's start cost to 0
    heap.push(origin);      //Add the origin to the heap

    vector<string> destinationEdges;
    for(vector<Edge*>::iterator it = destination->inEdges.begin(); it != destination->inEdges.end(); it++)
        destinationEdges.push_back((*it)->id);

    while(!heap.empty())    //While there are unexplored edges (always, if graph is fully connected)
    {
        Edge* parent = heap.top();          //Set parent to the closest unexplored edge
        heap.pop();                         //Remove parent from the heap
        if(parent->visited)                 //If it was visited already, ignore it
            continue;
        parent->visited = 1;                //If not, we're about to

        double distanceAlongLane = 1;
        if(parent->id == origin->id)    //Vehicles may not necessarily start at the beginning of a lane. Check for that
        {
            double lanePos = TraCI->getCommandInterface()->getLanePosition(vName);
            string lane = TraCI->getCommandInterface()->getLaneId(vName);
            double laneLength = TraCI->getCommandInterface()->getLaneLength(lane);
            distanceAlongLane = 1 - (lanePos / laneLength); //And modify distanceAlongLane
        }
        double curLaneCost = distanceAlongLane * parent->getCost();
        if(std::find(destinationEdges.begin(), destinationEdges.end(), parent->id) == destinationEdges.end())   //If we're not at a destination edge
        {
            for(vector<Edge*>::iterator child = parent->to->outEdges.begin(); child != parent->to->outEdges.end(); child++)   //Go through every edge was can get to from the parent
            {
                double newCost = parent->curCost + curLaneCost;       //Time to get to the junction is the time we get to the edge plus the edge cost
                newCost += junctionCost(newCost + simTime().dbl(), parent, *child); //The cost at the junction is calculated from when we'd arrive there
                if(!(*child)->visited && newCost < (*child)->curCost)       //If we haven't finished the edge and out new cost is lower
                {
                    (*child)->curCost = newCost;    //Cost to the child is newCost
                    (*child)->best = parent;        //Best path to the child is through the parent
                    heap.push(*child);              //Add the child to the heap
                }
            }
        }
        else                                //If we found the destination!
        {
            //cout << "Estimated time is " << parent->curCost + simTime().dbl() << endl;
            list<string> routeIDs;          //Start backtracking to generate the route
            routeIDs.push_back(parent->id);        //Add the end edge
            while(parent->best != NULL)     //While there are more edges
            {
                routeIDs.insert(routeIDs.begin(),parent->best->id); //Add the edge
                parent = parent->best;                              //And go to its parent
            }
            return routeIDs;    //Return the final list
        }
    }
    cout << "Pathing failed!  Either destination does not exist or cannot be reached.  Stopping at the end of this edge" << endl;
    list<string> ret;
    ret.push_back(origin->id);
    return ret;
}

}
