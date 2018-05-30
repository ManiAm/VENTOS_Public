/****************************************************************************/
/// @file    Net.cc
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

#include "router/Net.h"

namespace VENTOS {

Net::~Net()
{
    for(auto &items : TLs)
        delete items.second;

    for(auto &items : edges)
        delete items.second;

    for(auto &items : nodes)
        delete items.second;

    for(auto &items : vehicles)
        delete items.second;

    for(auto &items : connections)
        for(auto &items2: items.second)
            delete items2;

    for(auto &item : transitions)
        delete item.second;

}

Net::Net(std::string netBase, omnetpp::cModule* router, int ltc, int rtc, int stc, int utc):leftTurnCost(ltc), rightTurnCost(rtc), straightCost(stc), uTurnCost(utc)
{
    debugLevel = omnetpp::getSimulation()->getSystemModule()->par("debugLevel").intValue();

    routerModule = router;
    LoadHelloNet(netBase);
}

//Returns the expected time waiting to make the turn between two given edges
//For a TL, this is the time until the next phase allowing that motion
//Otherwise, this is a constant fixed cost
double Net::junctionCost(double time, Edge* start, Edge* end)
{
    if(start->to->type == "traffic_light")
        return timeToPhase(start->to->tl, time, nextAcceptingPhase(time, start, end));
    else
        return turnTypeCost(start, end);
}

double Net::turnTypeCost(Edge* start, Edge* end)
{
    std::string key = start->id + end->id;
    char type = turnTypes[key];

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

    if(omnetpp::cSimulation::getActiveEnvir()->isGUI() && debugLevel > 1)
    {
        std::cout << "Turn did not have an associated type!  This should never happen." << std::endl;
        std::cout.flush();
    }

    return 100000;
}

std::vector<int>* Net::TLTransitionPhases(Edge* start, Edge* end)
{
    return transitions[start->id + end->id];
}

double Net::timeToPhase(TrafficLightRouter* tl, double time, int targetPhase)
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

int Net::nextAcceptingPhase(double time, Edge* start, Edge* end)
{
    TrafficLightRouter* tl = start->to->tl;           // The traffic-light in question
    const std::vector<Phase*> phases = tl->phases;   // And its set of phases

    int curPhase = tl->currentPhaseAtTime(time);
    int phase = curPhase;
    std::vector<int>* acceptingPhases = TLTransitionPhases(start, end);  // Grab the vector of accepting phases for the given turn

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


void Net::LoadHelloNet(std::string netBase)
{
    omnetpp::cModuleType* moduleType = omnetpp::cModuleType::get("VENTOS.src.trafficLight.TL_Router");    //Get the TL module

    std::string netFile = netBase + "/hello.net.xml";

    rapidxml::file <> xmlFile(netFile.c_str());   //Make a new rapidXML document to parse
    rapidxml::xml_document<> doc;
    rapidxml::xml_node<> *node;
    doc.parse<0>(xmlFile.data());

    //For every node
    for(node = doc.first_node()->first_node("junction"); node; node = node->next_sibling("junction"))
    {
        //Read all the attributes in
        rapidxml::xml_attribute<> *attr = node->first_attribute();
        std::string id = attr->value();

        attr = attr->next_attribute();
        std::string type = attr->value();

        attr = attr->next_attribute();
        double x = atof(attr->value());

        attr = attr->next_attribute();
        double y = atof(attr->value());

        //Break the incoming lanes list into a vector
        attr = attr->next_attribute();
        std::string incLanesString = attr->value();
        std::vector<std::string>* incLanes = new std::vector<std::string>;
        std::stringstream ssin(incLanesString);
        while(ssin.good())
        {
            std::string temp;
            ssin >> temp;
            if(temp.length() > 0)
                incLanes->push_back(temp);
        }

        //If we're looking at a traffic light
        TrafficLightRouter *tl = NULL;
        if(type == "traffic_light")
        {
            //Find the referenced traffic light (based on name, the first attribute)
            for(rapidxml::xml_node<> *tlNode = doc.first_node()->first_node("tlLogic"); tlNode; tlNode = tlNode->next_sibling("tlLogic"))
            {
                if(tlNode->first_attribute()->value() == id)
                {
                    //Read its attributes in
                    rapidxml::xml_attribute<> *tlAttr = tlNode->first_attribute();
                    std::string tlid = tlAttr->value();
                    tlAttr = tlAttr->next_attribute();
                    std::string tltype = tlAttr->value();
                    tlAttr = tlAttr->next_attribute();
                    std::string programID = tlAttr->value();
                    tlAttr = tlAttr->next_attribute();
                    double tloffset = atof(tlAttr->value());

                    //Read in its set of phases
                    std::vector<Phase*> phasesVec;
                    for(rapidxml::xml_node<> *phaseNode = tlNode->first_node("phase"); phaseNode; phaseNode = phaseNode->next_sibling("phase"))
                    {
                        double duration = atof(phaseNode->first_attribute()->value());
                        std::string state = phaseNode->first_attribute()->next_attribute()->value();
                        Phase *ph = new Phase(duration, state);
                        phasesVec.push_back(ph);   //Build and link a new phase for each attribute
                    }

                    omnetpp::cModule *mod = moduleType->create("TrafficLight", routerModule); //Create a TL module with router as its parent
                    tl = omnetpp::check_and_cast<TrafficLightRouter*>(mod);                   //Cast the new module to a TL
                    tl->build(tlid, tltype, programID, tloffset, phasesVec, this);   //And build the traffic light with all this info
                    TLs[tlid] = tl; //Add the TL to the TL set
                    break;
                }//if matching traffic light
            }//for each traffic light
        }//if traffic light

        Node *n = new Node(id, x, y, type, incLanes, tl); //Finally build the node
        nodes[id] = n;
    }

    //For every edge
    for(node = doc.first_node()->first_node("edge"); node; node = node->next_sibling("edge"))
    {
        rapidxml::xml_attribute<> *attr = node->first_attribute();    //Read the basic attributes in
        std::string id = attr->value();
        attr = attr->next_attribute();
        std::string fromVal = attr->value();
        attr = attr->next_attribute();
        std::string toVal = attr->value();
        attr = attr->next_attribute();
        int priority = atoi(attr->value());

        std::vector<Lane*>* lanesVec = new std::vector<Lane*>;    //For every lane on that edge
        for(rapidxml::xml_node<> *lane = node->first_node(); lane; lane = lane->next_sibling())
        {
            attr = lane->first_attribute(); //Read the lane's attributes in
            std::string laneid = attr->value();
            attr = attr->next_attribute()->next_attribute();
            double speed = atof(attr->value());
            attr = attr->next_attribute();
            double length = atof(attr->value());
            Lane* l = new Lane(laneid, speed, length);  //Build the lane
            lanesVec->push_back(l); //Add it to a vector of this edge's lanes
        }
        Node* from = nodes[fromVal];  //Get a pointer to the start node
        Node* to = nodes[toVal];      //Get a pointer to the end node
        edges[id] = new Edge(id, from, to, priority, *lanesVec);

        from->outEdges.push_back(edges.at(id));   //Add the edge to the start node's list
    }   //For every edge

    for(std::map<std::string, Edge*>::iterator it = edges.begin(); it != edges.end(); ++it)   //For each edge
        (*it).second->to->inEdges.push_back((*it).second);  //Go to the destination fo that edge, and add that edge to its in-edges

    //Make a new mapping from std::string to int vector.  strings will be the start and end lanes, and lanes will be the lane numbers from start than connect them.
    for(node = doc.first_node()->first_node("connection"); node; node = node->next_sibling("connection"))
    {   //For every connection
        rapidxml::xml_attribute<> *attr = node->first_attribute();    //Read in the start and end edge ids, and the lane number of this instance.
        std::string e1 = attr->value();

        attr = attr->next_attribute();
        std::string e2 = attr->value();
        std::string key = e1 + e2;       //Key is the concatenation of both IDs.

        attr = attr->next_attribute();
        int fromLaneNum = atoi(attr->value());

        attr = attr->next_attribute();
        int toLaneNum = atoi(attr->value());

        attr = attr->next_attribute();
        if((std::string)attr->name() == "tl")    //Read the tl attributes if necessary
        {
            if(transitions.find(key) == transitions.end()) //If this vector doesn't yet exist
                transitions[key] = new std::vector<int>;  //Create an empty vector in place
            TrafficLightRouter* tl = TLs[attr->value()];    //Find the associated traffic light
            std::string TLid = attr->value();

            attr = attr->next_attribute();
            int linkIndex = atoi(attr->value());

            for(unsigned int i = 0; i < tl->phases.size(); i++)
            {
                Phase* phase = tl->phases[i];
                if(phase->state[linkIndex] != 'r')  //if it's not a red light at this time
                    transitions[key]->push_back(i);  //add that we can travel these edges during this phase
            }

            Edge* fromEdge = edges.at(e1);
            Lane* fromLane = (fromEdge->lanes)[fromLaneNum];

            for(unsigned int i = 0; i < tl->phases.size(); i++)      //These 3 lines took me way too long to develop
                //if (tl->phases[i]->state[linkIndex] == 'g' || tl->phases[i]->state[linkIndex] == 'G') //Check each of the TL's phases -- if the state's value at the given link index allows movement,
                if(tl->phases[i]->state[linkIndex] != 'r')
                    fromLane->greenPhases.push_back(i);     //Push that phase to a list of green phases for that lane

            attr = attr->next_attribute();
            char dir = attr->value()[0];
            turnTypes[key] = dir;

            attr = attr->next_attribute();
            char state = attr->value()[0];

            connections[key].push_back(new Connection(e1, e2, fromLaneNum, toLaneNum, TLid, linkIndex, dir, state));
        }//if it's a tl
    }//for each connection
}

} // end of namespace
