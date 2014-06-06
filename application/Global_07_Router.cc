#include "Global_07_Router.h"
#include "rapidxml.hpp"
#include "rapidxml_utils.hpp"
#include "ApplV_02_Beacon.h"

#include <msg/Messages_m.h>
#include <iostream>  // For testing - remove
#include <iomanip>   // For operator<< code
#include <queue>     // For pathing
#include <climits>   // For INT_MAX
#include <algorithm> // For sort
#include <sstream>   // For getline() in stringToList
#include <Global_01_TraCI_Extend.h>

Define_Module(Router);

void Router::initialize(int stage)
{
    if(stage == 0)
    {
        enableRouting = par("enableRouting").boolValue();
        if(!enableRouting)
            return;

        //Build nodePtr and traci manager
        nodePtr = FindModule<>::findHost(this);
        manager = FindModule<TraCI_Extend*>::findGlobalModule();
        if(nodePtr == NULL || manager == NULL)
            error("can not get a pointer to the module.");


        //register signals
        Signal_system = registerSignal("system");
        simulation.getSystemModule()->subscribe("system", this);

        string rootFile = par("rootFilePath").stringValue();
        string nodeFile = rootFile + ".nod.xml";
        string edgeFile = rootFile + ".edg.xml";


        using namespace rapidxml;           // Using the rapidxml library to parse our files
        file<> xmlFile(nodeFile.c_str());   // Convert our file to a rapid-xml readable object
        xml_document<> doc;                 // Build a rapidxml doc
        doc.parse<0>(xmlFile.data());       // Fill it with data from our file
        xml_node<> *node = doc.first_node("nodes"); // Parse up to the "nodes" declaration
        string idValInt, type;
        double xVal, yVal;
        for(node = node->first_node("node"); node; node = node->next_sibling()) // For each node in nodes
        {

            type = "";  //Reset type, in case this node doesn't have one
            int readCount = 0;
            for(xml_attribute<> *attr = node->first_attribute(); attr; attr = attr->next_attribute())//For each attribute, read in to the right variable
            {
                switch (readCount)
                {
                case 0:
                    idValInt = attr->value();
                    break;
                case 1:
                    xVal = atof(attr->value());
                    break;
                case 2:
                    yVal = atof(attr->value());
                    break;
                case 3:
                    type = attr->value();
                    break;
                }
                readCount++;
            }
            if(readCount < 3)
            {
                error("XML formatted wrong! Not enough elements given for some node.");
            }

            Node *n = new Node(idValInt, xVal, yVal, type);    // Build a node with these attributes
            nodes.push_back(*n);    // Add it to the router's node vector
        }
        sort(nodes.begin(), nodes.end(), NodeIDSort);   //Sort them, for binary searches later on


        xmlFile = file<>(edgeFile.c_str()); // Identical to above, but for edges
        doc.parse<0>(xmlFile.data());
        node = doc.first_node("edges");
        string idVal, fromVal, toVal;
        int priorityVal, numLanesVal;
        double speedVal;
        for(node = node->first_node("edge"); node; node = node->next_sibling())
        {
            int readCount = 0;
            for(xml_attribute<> *attr = node->first_attribute(); attr; attr = attr->next_attribute())
            {
                switch (readCount)
                {
                case 0:
                    idVal = attr->value();
                    break;
                case 1:
                    fromVal = attr->value();
                    break;
                case 2:
                    toVal = attr->value();
                    break;
                case 3:
                    priorityVal = atoi(attr->value());
                    break;
                case 4:
                    numLanesVal = atoi(attr->value());
                    break;
                case 5:
                    speedVal = atof(attr->value());
                    break;
                }
                readCount++;
            }
            if(readCount < 6)
            {
                error("XML formatted wrong! Not enough elements given for some edge.");
            }
            Node* from = binarySearch(&nodes, fromVal); //Get pointers to its origin and destination nodes
            Node* to   = binarySearch(&nodes, toVal);
            Edge *e = new Edge(idVal, from, to, priorityVal, numLanesVal, speedVal);  //Build a new edge
            from->edges.push_back(e);     //Link the three
            edges.push_back(*e);          //And push back the finished edge
        }
        sort(edges.begin(), edges.end(), EdgeIDSort);//, EdgeIDSort);   //Sort the edges for our binary searches later
    } // if stage == 0
}

void Router::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj)
{

    if(signalID == Signal_system) //Check if it's the right kind of symbol
    {
        systemData *s = static_cast<systemData *>(obj); //Cast to a systemData class
        if(string(s->getRecipient()) == "system")  //If this is the intended target
        {
            if(s->getRequestType() == 0)    //If this is a path request
            {
                if(recalculateCount == 0 || simTime().dbl() - lastUpdateTime >= 10)   //If we haven't gotten edge weights yet or it's been longer than 10 seconds since an update
                    updateWeights();

                list<string> info = getRoute(s->getEdge(), s->getNode());
                /*
                cout << "Got route:" << endl;
                for(list<string>::iterator it = info.begin(); it != info.end(); it++)
                    cout << *it << endl;
                */
                simsignal_t Signal_router = registerSignal("router");//Prepare to send a router message

                //Systemdata wants string edge, string node, string sender, int requestType, string recipient, list<string> edgeList
                nodePtr->emit(Signal_router, new systemData("", "", "router", 0, s->getSender(), info));
            }
        }
    }
}

ostream& operator<<(ostream& os, Router &rhs) // Print a router's network
{
    os << "Nodes:" << endl;
    for(vector<Node>::iterator it = rhs.nodes.begin(); it != rhs.nodes.end(); it++)
        os << (*it) << endl;
    os << endl;
    os << "Edges:" << endl;
    for(vector<Edge>::iterator it = rhs.edges.begin(); it != rhs.edges.end(); it++)
        os << *it << endl;
    return os;
}

void Router::reset()
{
    for(vector<Node>::iterator it = nodes.begin(); it != nodes.end(); it++)
    {
        it->curCost = DBL_MAX;
        it->best = NULL;
        it->visited = 0;
    }
}

double Router::getEdgeMeanSpeed(Edge *edge)
{
    double sum = 0;
    for(vector<Lane>::iterator it = edge->lanes.begin(); it != edge->lanes.end(); it++)
    {
        sum += (it->length / manager->commandGetLaneMeanSpeed(it->id));
    }
    return sum / edge->lanes.size();
}

void Router::updateWeights()
{
    lastUpdateTime = simTime().dbl();   //We're updating now

    if(recalculateCount == 0)   //If this is the first time, link lanes to edges and set origCost
    {
        list<string> lanes = manager->commandGetLaneIds();  //Get every lane
        for(list<string>::iterator laneID = lanes.begin(); laneID != lanes.end(); laneID++)   //For every lane in the simulation
        {
            Lane *l = new Lane(*laneID, manager->commandGetLaneLength(*laneID));    //Build a lane object, with its id and length
            Edge *e = binarySearch(&edges, manager->commandGetLaneEdgeId(*laneID)); //Link it to its edge
            e->lanes.push_back(*l);
        }
        for(vector<Edge>::iterator edge = edges.begin(); edge != edges.end(); edge++)   //For every edge
        {
            edge->updateLength();   //Update its length
            edge->origCost = edge->length / edge->speed;    //Update its starting cost -- length/speed
        }
    }

    for(vector<Edge>::iterator edge = edges.begin(); edge != edges.end(); edge++)//Always update lastCost
    {
        edge->lastCost = (getEdgeMeanSpeed(&(*edge)) + edge->origCost) / 2;
        //cout << edge->id << " has speed " << edge->lastCost << endl;
    }

    recalculateCount++; //We recalculated once more
}

list<string> Router::getRoute(string begin, string end)
{
    /*
    list<string> retu;
    retu.push_back("2to3");
    return retu;
    */

    reset();    //Set up for a new path calculation.  Hopefully I can avoid doing this in the future

    priority_queue<Node*> heap;                     // Build a heap of lowest-cost nodes
    Node* destination = binarySearch(&nodes, end);  // Get the destination from its id
    Edge* origin = binarySearch(&edges, begin);     // Get the edge the vehicle starts on from its id
    Node* start = origin->to;                       // Get the start -- the node the vehicle's current edge leads to
    start->curCost = 0;                             // Start the first edge at 0 cost
    heap.push(start);                               // Add the origin to the heap

    while(!heap.empty())                    // While we have elements left (always, if the graph is fully connected)
    {
        Node* parent = heap.top();          // Pull the smallest element off the top
        heap.pop();
        if(parent->visited)                 // If we've looked at this node already, skip it
            continue;                       // (it's possible to add a node multiple times)
        parent->visited = 1;                // Set this node as visited
        if(parent->id == destination->id)   // If we've found the best path, reconstruct
        {
            list<string> ret;               // List of edge names to return
            while(parent->best != NULL)     // While we have another edge to traverse
            {
                ret.insert(ret.begin(), parent->best->id);    // Add that edge name to the list
                parent = parent->best->from;                  // Move to the node that edge leads to
            }
            ret.insert(ret.begin(),origin->id); //Add the vehicle's current edge, because we skipped it
            return ret;
        }
        for(vector<Edge*>::iterator childEdge = parent->edges.begin(); childEdge != parent->edges.end(); childEdge++)
        {   //For each linked edge
            Node* childNode = (*childEdge)->to;                             //Set childNode to the node the edge leads to, for simplicity
            double newCost = parent->curCost + (*childEdge)->getCost();     // Calculate the cost from parent to child
            if((!childNode->visited) && (newCost < childNode->curCost))     // If it's a better cost and not visited
            {
                childNode->curCost = newCost;   // Update its cost
                childNode->best = (*childEdge); // Update where it came from
                heap.push(childNode);           // Add it to the queue
            }
        }
    }
    cout << "Pathing failed!  Either destination does not exist or cannot be reached.  Stopping at the end of this edge" << endl;
    list<string> ret;
    ret.push_back(origin->id);
    return ret;
}

Router::Router()
{
    recalculateCount = 0;
    lastUpdateTime = 0;
}

Router::~Router()
{

}

