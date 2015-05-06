/****************************************************************************/
/// @file    ApplV_03_System.h
/// @author  Dylan Smith <dilsmith@ucdavis.edu>
/// @author  Mani Amoozadeh <maniam@ucdavis.edu>
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

#include "ApplV_03_System.h"

namespace VENTOS {

Define_Module(VENTOS::ApplVSystem);

ApplVSystem::~ApplVSystem()
{

}


void ApplVSystem::initialize(int stage)
{
    ApplVBeacon::initialize(stage); //Initialize lower-levels

    if (stage == 0)
    {
        // NED variables (beaconing parameters)
        requestRoutes = par("requestRoutes").boolValue();
        if(!requestRoutes)
            return;

        routeUpdateInterval = par("routeUpdateInterval").doubleValue();

        requestInterval = par("requestInterval").doubleValue();
        maxOffset = par("maxSystemOffset").doubleValue();
        systemMsgLengthBits = par("systemMsgLengthBits").longValue();
        systemMsgPriority = par("systemMsgPriority").longValue();
        useDijkstrasRouting = par("useDijkstrasRouting").boolValue();

        // get the rootFilePath
        cModule *module = simulation.getSystemModule()->getSubmodule("router");
        router = static_cast< Router* >(module);
        string rootFilePath = SUMO_FullPath.string();
        rootFilePath += "/Vehicles" + SSTR(router->totalVehicleCount) + ".xml";

        // Routing
        //Temporary fix to get a vehicle's target: get it from the xml
        file<> xmlFile( (rootFilePath).c_str() );          // Convert our file to a rapid-xml readable object
        xml_document<> doc;                                // Build a rapidxml doc
        doc.parse<0>(xmlFile.data());                      // Fill it with data from our file
        xml_node<> *node = doc.first_node("vehicles");     // Parse up to the "nodes" declaration
        for(node = node->first_node("vehicle"); node->first_attribute()->value() != SUMOvID; node = node->next_sibling()); //Find our vehicle in the .xml

        xml_attribute<> *attr = node->first_attribute()->next_attribute()->next_attribute()->next_attribute();  //Navigate to the destination attribute
        if((string)attr->name() == "destination")  //Double-check
           targetNode = attr->value(); //And save it
        else
            error("XML formatted wrong! Some vehicle was missing its destination!");

        if(find(router->nonReroutingVehicles->begin(), router->nonReroutingVehicles->end(), SUMOvID.substr(1, SUMOvID.length() - 1)) != router->nonReroutingVehicles->end())
        {
            requestReroutes = false;
            //cout << SUMOvID << " is not routing" << endl;
        }
        else
        {
            requestReroutes = true;
        }
        numReroutes = 0;

        //Register to receive signals from the router
        Signal_router = registerSignal("router");
        simulation.getSystemModule()->subscribe("router", this);

        //Slightly offset all vehicles (0-4 seconds)
        double systemOffset = dblrand() * 2 + 2;

        simsignal_t Signal_system = registerSignal("system"); //Prepare to send a system message
        nodePtr->emit(Signal_system, new systemData("", "", SUMOvID, 3, string("system")));

        sendSystemMsgEvt = new cMessage("systemmsg evt");   //Create a new internal message
        if (requestRoutes) //&& VANETenabled ) //If this vehicle is supposed to send system messages
            scheduleAt(simTime() + systemOffset, sendSystemMsgEvt); //Schedule them to start sending
    }
}


void ApplVSystem::finish()
{
    ApplVBeacon::finish();

    if(!requestRoutes)
        return;

    if(ev.isGUI() && requestRoutes) cout << SUMOvID << " took " << simTime().dbl() - entryTime << " seconds to complete its route." << endl;
    router->vehicleEndTimesFile << SUMOvID << " " << simTime().dbl() << endl;

    simsignal_t Signal_system = registerSignal("system"); //Prepare to send a system message
    nodePtr->emit(Signal_system, new systemData("", "", SUMOvID, 2, string("system")));

    if(requestRoutes)
    {
        if (sendSystemMsgEvt->isScheduled())
        {
            cancelAndDelete(sendSystemMsgEvt);
        }
        else
        {
            delete sendSystemMsgEvt;
        }
    }

    simulation.getSystemModule()->unsubscribe("router",this);
}

void ApplVSystem::handleSelfMsg(cMessage* msg)  //Internal messages to self
{
    ApplVBeacon::handleSelfMsg(msg);    //Pass it down

    if (msg == sendSystemMsgEvt and requestRoutes)  //If it's a system message
    {
        delete msg;
        reroute();
    }
}

void ApplVSystem::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj) //Ran upon receiving signals
{
    if(signalID == Signal_router)   //If the signal is of type router
    {
        systemData *s = static_cast<systemData *>(obj); //Cast to usable data
        if(string(s->getSender()) == "router" and string(s->getRecipient()) == SUMOvID) //If sent from the router and to this vehicle
        {
            if((s->getRequestType() == 0 || s->getRequestType() == 1) && (requestReroutes or numReroutes == 0)) //If sent from the router and to this vehicle
            {
                numReroutes++;
                //cout << "Setting new route for " << SUMOvID << " at t=" << simTime().dbl() << endl;
                list<string> sRoute = s->getInfo(); //Copy the info from the signal (breaks if we don't do this, for some reason)
                if (*sRoute.begin() != "failed")
                    TraCI->vehicleSetRoute(s->getRecipient(), sRoute);  //Update this vehicle's path with the proper info
            }
            else if(s->getRequestType() == 2)//later use
            {

            }
        }
    }
}


void ApplVSystem::onBeaconVehicle(BeaconVehicle* wsm)
{

}


void ApplVSystem::onBeaconRSU(BeaconRSU* wsm)
{

}


void ApplVSystem::onData(PlatoonMsg* wsm)
{

}

void ApplVSystem::onSystemMsg(SystemMsg* wsm)
{
    error("ApplVSystem should not receive any system msg!");
}

void ApplVSystem::reroute()
{
    sendSystemMsgEvt = new cMessage("systemmsg evt");   //Create a new internal message
    simsignal_t Signal_system = registerSignal("system"); //Prepare to send a system message
    //Systemdata wants string edge, string node, string sender, int requestType, string recipient, list<string> edgeList
    if(useDijkstrasRouting)
    {
        nodePtr->emit(Signal_system, new systemData(TraCI->vehicleGetEdgeID(SUMOvID), targetNode, SUMOvID, 0, string("system")));
        if(!router->UseHysteresis)
            scheduleAt(simTime() + requestInterval, sendSystemMsgEvt);// schedule for next beacon broadcast
    }
    else
    {
        nodePtr->emit(Signal_system, new systemData(TraCI->vehicleGetEdgeID(SUMOvID), targetNode, SUMOvID, 1, string("system")));
        scheduleAt(simTime() + routeUpdateInterval, sendSystemMsgEvt);// schedule for next beacon broadcast
    }
}

SystemMsg*  ApplVSystem::prepareSystemMsg()
{
    if (!VANETenabled)
    {
        error("Only VANETenabled vehicles can send system msg!");
    }

    SystemMsg* wsm = new SystemMsg("systemmsg");

    // add header length
    wsm->addBitLength(headerLength);

    // add payload length
    wsm->addBitLength(systemMsgLengthBits);

    wsm->setWsmVersion(1);
    wsm->setSecurityType(1);

    wsm->setChannelNumber(Channels::CCH);

    wsm->setDataRate(1);
    wsm->setPriority(systemMsgPriority);
    wsm->setPsid(0);

    // wsm->setSerial(serial);
    // wsm->setTimestamp(simTime());

    // fill in the sender field
    wsm->setSender(SUMOvID.c_str());

    wsm->setRecipient("system");

    // set request type
    wsm->setRequestType(0);

    // set current lane
    wsm->setEdge( TraCI->vehicleGetEdgeID(SUMOvID).c_str() );

    // set target node - read this from the vehicle's data
    wsm->setTarget(1);

    // set associated info - none in this case
    //wsm->setInfo();

    return wsm;
}

// print system msg fields (for debugging purposes)
void ApplVSystem::printSystemMsgContent(SystemMsg* wsm)
{
    EV << wsm->getWsmVersion() << " | ";
    EV << wsm->getSecurityType() << " | ";
    EV << wsm->getChannelNumber() << " | ";
    EV << wsm->getDataRate() << " | ";
    EV << wsm->getPriority() << " | ";
    EV << wsm->getPsid() << " | ";
    EV << wsm->getPsc() << " | ";
    EV << wsm->getWsmLength() << " | ";
    EV << wsm->getWsmData() << " ||| ";

    EV << wsm->getSender() << " | ";

    EV << wsm->getRequestType() << " | ";
    EV << wsm->getEdge() << " | ";
    EV << wsm->getTarget() << " | ";
}

// is called, every time the position of vehicle changes
void ApplVSystem::handlePositionUpdate(cObject* obj)
{
    ApplVBeacon::handlePositionUpdate(obj);
}

}
