#include "ApplV_03_System.h"

#include "rapidxml.hpp"
#include "rapidxml_utils.hpp"
using namespace rapidxml;

#include <iostream>

Define_Module(ApplVSystem);


void ApplVSystem::initialize(int stage)
{
    ApplVBeacon::initialize(stage); //Initialize lower-levels

    if (stage == 0)
    {
        // NED variables (beaconing parameters)
        requestRoutes = par("requestRoutes").boolValue();
        if(!requestRoutes)
            return;

        requestInterval = par("requestInterval").doubleValue();
        maxOffset = par("maxSystemOffset").doubleValue();
        systemMsgLengthBits = par("systemMsgLengthBits").longValue();
        systemMsgPriority = par("systemMsgPriority").longValue();

        // get the rootFilePath
        boost::filesystem::path SUMODirectory = simulation.getSystemModule()->par("SUMODirectory").stringValue();
        boost::filesystem::path VENTOSfullDirectory = cSimulation::getActiveSimulation()->getEnvir()->getConfig()->getConfigEntry("network").getBaseDirectory();
        string rootFilePath = (VENTOSfullDirectory / SUMODirectory / "/Vehicles.xml").string();

        // Routing
        //Temporary fix to get a vehicle's target: get it from the xml
        file<> xmlFile( (rootFilePath).c_str() );          // Convert our file to a rapid-xml readable object
        xml_document<> doc;                                // Build a rapidxml doc
        doc.parse<0>(xmlFile.data());                      // Fill it with data from our file
        xml_node<> *node = doc.first_node("vehicles");     // Parse up to the "nodes" declaration

        for(node = node->first_node("vehicle"); node->first_attribute()->value() != SUMOvID; node = node->next_sibling()){} //Find our vehicle in the .xml

        xml_attribute<> *attr = node->first_attribute()->next_attribute()->next_attribute()->next_attribute();  //Navigate to the destination attribute
        if((string)attr->name() == "destination")  //Double-check
           targetNode = attr->value(); //And save it
        else
            error("XML formatted wrong! Some vehicle was missing its destination!");

        //Register to receive signals from the router
        Signal_router = registerSignal("router");
        simulation.getSystemModule()->subscribe("router", this);

        //Slightly offset all vehicles (0-4 seconds)
        double systemOffset = dblrand() * 2 + 2;

        sendSystemMsgEvt = new cMessage("systemmsg evt", SEND_SYSTEMMSG_EVT);   //Create a new internal message
        if (requestRoutes) //&& VANETenabled ) //If this vehicle is supposed to send system messages
            scheduleAt(simTime() + systemOffset, sendSystemMsgEvt); //Schedule them to start sending
    }
}


void ApplVSystem::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj) //Ran upon receiving signals
{
    if(signalID == Signal_router)   //If the signal is of type router
    {
        systemData *s = static_cast<systemData *>(obj); //Cast to usable data
        if(string(s->getSender()) == "router" && string(s->getRecipient()) == SUMOvID) //If sent from the router and to this vehicle
        {
            list<string> temp = s->getInfo(); //Copy the info from the signal (breaks if we don't do this, for some reason)
            TraCI->commandSetRouteFromList(s->getRecipient(), temp);  //Update this vehicle's path with the proper info
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

void ApplVSystem::handleSelfMsg(cMessage* msg)  //Internal messages to self
{
    ApplVBeacon::handleSelfMsg(msg);    //Pass it down

    if (msg->getKind() == SEND_SYSTEMMSG_EVT && requestRoutes)  //If it's a system message
    {
        simsignal_t Signal_system = registerSignal("system"); //Prepare to send a system message
        //Systemdata wants string edge, string node, string sender, int requestType, string recipient, list<string> edgeList
        nodePtr->emit(Signal_system, new systemData(TraCI->commandGetEdgeId(SUMOvID), targetNode, SUMOvID, 0, string("system")));
        scheduleAt(simTime() + requestInterval, sendSystemMsgEvt);// schedule for next beacon broadcast
    }
}


SystemMsg*  ApplVSystem::prepareSystemMsg()
{
    if (!VANETenabled)
    {
        error("Only VANETenabled vehicles can send systemmsg!");
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
    wsm->setEdge( TraCI->commandGetEdgeId(SUMOvID).c_str() );

    // set target node - read this from the vehicle's data
    wsm->setTarget(1);

    // set associated info - none in this case
    //wsm->setInfo();

    return wsm;
}


// print systemmsg fields (for debugging purposes)
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


void ApplVSystem::onSystemMsg(SystemMsg* wsm)
{
    error("ApplVSystem should not receive any systemmsg!");
}


// is called, every time the position of vehicle changes
void ApplVSystem::handlePositionUpdate(cObject* obj)
{
    ApplVBeacon::handlePositionUpdate(obj);
}


void ApplVSystem::finish()
{
    ApplVBeacon::finish();

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
}


ApplVSystem::~ApplVSystem()
{
    simulation.getSystemModule()->unsubscribe("router",this);
}
