//
// Copyright (C) 2011 David Eckhoff <eckhoff@cs.fau.de>
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

/*
 * Based on PhyLayer.cc from Karl Wessel
 * and modifications by Christopher Saloman
 *
 * Second author: Mani Amoozadeh <maniam@ucdavis.edu>
 */

#include "PhyLayer80211p.h"
#include "MIXIM_veins/nic/phy/decider/Decider80211p.h"
#include "MIXIM_veins/connectionManager/BaseConnectionManager.h"
#include "MIXIM_veins/nic/Consts80211p.h"
#include "msg/AirFrame11p_serial.h"
#include "msg/Mac80211Pkt_m.h"
#include "MIXIM_veins/nic/mac/MacToPhyControlInfo.h"
#include "PhyToMacControlInfo.h"
#include "MIXIM_veins/nic/phy/decider/DeciderResult80211.h"

#include "MIXIM_veins/nic/phy/analogueModel/SimplePathlossModel.h"
#include "MIXIM_veins/nic/phy/analogueModel/BreakpointPathlossModel.h"
#include "MIXIM_veins/nic/phy/analogueModel/LogNormalShadowing.h"
#include "MIXIM_veins/nic/phy/analogueModel/JakesFading.h"
#include "MIXIM_veins/nic/phy/analogueModel/PERModel.h"
#include "MIXIM_veins/nic/phy/analogueModel/SimpleObstacleShadowing.h"
#include "MIXIM_veins/nic/phy/analogueModel/TwoRayInterferenceModel.h"
#include "MIXIM_veins/nic/phy/analogueModel/NakagamiFading.h"

namespace Veins {

Define_Module(Veins::PhyLayer80211p);


PhyLayer80211p::PhyLayer80211p()
{

}


PhyLayer80211p::~PhyLayer80211p()
{
    //get AirFrames from ChannelInfo and delete
    // (although ChannelInfo normally owns the AirFrames it
    // is not able to cancel and delete them itself)
    AirFrameVector channel;
    channelInfo.getAirFrames(0, omnetpp::simTime(), channel);

    for(auto it = channel.begin(); it != channel.end(); ++it)
        cancelAndDelete(*it);

    // free timer messages
    if(txOverTimer)
        cancelAndDelete(txOverTimer);

    if(radioSwitchingOverTimer)
        cancelAndDelete(radioSwitchingOverTimer);

    if(radioDelayTimer)
        cancelAndDelete(radioDelayTimer);

    // free thermal noise mapping
    if(thermalNoise)
        delete thermalNoise;

    // free Decider
    if(decider != 0)
        delete decider;

    /*
     * get a pointer to the radios RSAM again to avoid deleting it,
     * it is not created by calling new (BasePhyLayer is not the owner)!
     */
    AnalogueModel* rsamPointer = radio ? radio->getAnalogueModel() : NULL;

    // free AnalogueModels
    for(auto it = analogueModels.begin(); it != analogueModels.end(); it++)
    {
        AnalogueModel* tmp = *it;

        // do not delete the RSAM, it's not allocated by new!
        if (tmp == rsamPointer)
        {
            rsamPointer = 0;
            continue;
        }

        if(tmp != 0)
            delete tmp;
    }

    // free radio
    if(radio != 0)
        delete radio;
}


void PhyLayer80211p::initialize(int stage)
{
    ChannelAccess::initialize(stage);

    if (stage == 0)
    {
        // get pointer to the world module
        world = FindModule<BaseWorldUtility*>::findGlobalModule();
        if (world == NULL)
            throw omnetpp::cRuntimeError("Could not find BaseWorldUtility module");

        // get a pointer to the Statistics module
        omnetpp::cModule *module = omnetpp::getSimulation()->getSystemModule()->getSubmodule("statistics");
        STAT = static_cast<VENTOS::Statistics*>(module);
        ASSERT(STAT);

        // get a reference to this node
        ptrNode = this->getParentModule()->getParentModule();

        // if using sendDirect, make sure that messages arrive without delay
        gate("radioIn")->setDeliverOnReceptionStart(true);

        // get gate ids
        upperLayerIn = findGate("upperLayerIn");
        upperLayerOut = findGate("upperLayerOut");
        upperControlOut = findGate("upperControlOut");
        upperControlIn = findGate("upperControlIn");

        headerLength = par("headerLength").intValue();
        if (headerLength != PHY_HDR_TOTAL_LENGTH)
            throw omnetpp::cRuntimeError("The header length of the 802.11p standard is 46bit, please change your omnetpp.ini accordingly by either setting it to 46bit or removing the entry");

        recordStats = par("recordStats").boolValue();
        record_stat = par("record_stat").boolValue();
        record_frameTxRx = par("record_frameTxRx").boolValue();
        emulationActive = par("emulationActive").boolValue();

        // initialize radio
        radio = initializeRadio();

        // initialize analog models
        initializeAnalogueModels();

        // initialize decider
        initializeDecider();

        // initialize timer messages
        radioSwitchingOverTimer = new omnetpp::cMessage("radio switching over", RADIO_SWITCHING_OVER);
        txOverTimer = new omnetpp::cMessage("transmission over", TX_OVER);
        radioDelayTimer = new omnetpp::cMessage("radio delay", RADIO_DELAY);

        myId = getParentModule()->getParentModule()->getFullName();
    }
}


void PhyLayer80211p::finish()
{
    // give decider the chance to do something
    decider->finish();
}


void PhyLayer80211p::handleMessage(omnetpp::cMessage* msg)
{
    // self messages
    if(msg->isSelfMessage())
        handleSelfMessage(msg);

    // data message coming from MAC
    else if(msg->getArrivalGateId() == upperLayerIn)
        handleUpperMessage(msg);

    // control message coming from MAC
    else if(msg->getArrivalGateId() == upperControlIn)
        handleUpperControlMessage(msg);

    // received an AirFrame
    else if(msg->getKind() == AIR_FRAME)
    {
        // if DSRCenabled is false, then ChannelAccess class will not send me any frames.
        // But we double-check here to make sure that DSRCenabled is 'true'
        if(!ptrNode->par("DSRCenabled"))
            throw omnetpp::cRuntimeError("Cannot send msg %s: DSRCenabled parameter is false in %s", msg->getName(), ptrNode->getFullName());

        handleAirFrame(static_cast<AirFrame*>(msg));
    }
    // unknown message
    else
    {
        EV << "Unknown message received. \n";
        delete msg;
    }
}


void PhyLayer80211p::handleSelfMessage(omnetpp::cMessage* msg)
{
    switch(msg->getKind())
    {
    // transmission over
    case TX_OVER:
    {
        assert(msg == txOverTimer);

        sendControlMsgToMac(new VENTOS::PhyToMacReport("Transmission over", TX_OVER));

        Decider80211p* dec = dynamic_cast<Decider80211p*>(decider);
        assert(dec);

        // check if there is another packet on the channel
        if (dec->cca(omnetpp::simTime(),NULL))
        {
            // change the channel state to idle
            DBG << "Channel idle after transmit!\n";
            dec->setChannelIdleStatus(true);
        }
        else
            DBG << "Channel not yet idle after transmit! \n";

        break;
    }
    // waiting for the radio is over
    case RADIO_DELAY:
    {
        // transmit the frame
        sendToChannel(readyToSendFrame);

        NumSentFrames++;

        if(record_stat)
            record_PHY_stat_func();

        break;
    }
    // radio switch over
    case RADIO_SWITCHING_OVER:
    {
        assert(msg == radioSwitchingOverTimer);
        finishRadioSwitching();
        break;
    }
    // AirFrame
    case AIR_FRAME:
    {
        handleAirFrame(static_cast<AirFrame*>(msg));
        break;
    }
    // ChannelSenseRequest
    case CHANNEL_SENSE_REQUEST:
    {
        handleChannelSenseRequest(msg);
        break;
    }
    default:
        break;
    }
}


void PhyLayer80211p::handleUpperMessage(omnetpp::cMessage* msg)
{
    // check if Radio is in TX state
    if (radio->getCurrentState() != Radio::TX)
    {
        delete msg;
        msg = 0;

        throw omnetpp::cRuntimeError("Error: message for sending received, but radio not in state TX");
    }

    // check if not already sending
    if(txOverTimer->isScheduled())
    {
        delete msg;
        msg = 0;

        throw omnetpp::cRuntimeError("Error: message for sending received, but radio already sending");
    }

    // make sure msg is of type packet
    assert(dynamic_cast<omnetpp::cPacket*>(msg) != 0);

    // build the AirFrame to send
    readyToSendFrame = encapsMsg(static_cast<omnetpp::cPacket*>(msg));

    // make sure there is no self message of kind TX_OVER scheduled
    assert (!txOverTimer->isScheduled());
    // txOverTimer notifies the transmission over time
    scheduleAt(omnetpp::simTime() + RADIODELAY_11P + readyToSendFrame->getDuration(), txOverTimer);

    // wait for the radio delay and then send the frame to the channel
    assert (!radioDelayTimer->isScheduled());
    scheduleAt(omnetpp::simTime() + RADIODELAY_11P, radioDelayTimer);
}


void PhyLayer80211p::handleUpperControlMessage(omnetpp::cMessage* msg)
{
    switch(msg->getKind())
    {
    case CHANNEL_SENSE_REQUEST:
        handleChannelSenseRequest(msg);
        break;
    default:
        throw omnetpp::cRuntimeError("Received unknown control message from upper layer!");
        break;
    }
}


Radio* PhyLayer80211p::initializeRadio()
{
    int initialRadioState = par("initialRadioState").intValue();
    double radioMinAtt = par("radioMinAtt").doubleValue();
    double radioMaxAtt = par("radioMaxAtt").doubleValue();
    int initialRadioChannel = hasPar("initialRadioChannel") ? par("initialRadioChannel") : 0;
    int nbRadioChannels = hasPar("nbRadioChannels") ? par("nbRadioChannels") : 1;

    Radio* radio = Radio::createNewRadio(recordStats, initialRadioState,
            radioMinAtt, radioMaxAtt, initialRadioChannel, nbRadioChannels);

    //  - switch times to TX
    //if no RX to TX defined asume same time as sleep to TX
    radio->setSwitchTime(Radio::RX, Radio::TX, (hasPar("timeRXToTX") ? par("timeRXToTX") : par("timeSleepToTX")).doubleValue());
    //if no sleep to TX defined asume same time as RX to TX
    radio->setSwitchTime(Radio::SLEEP, Radio::TX, (hasPar("timeSleepToTX") ? par("timeSleepToTX") : par("timeRXToTX")).doubleValue());

    //  - switch times to RX
    //if no TX to RX defined asume same time as sleep to RX
    radio->setSwitchTime(Radio::TX, Radio::RX, (hasPar("timeTXToRX") ? par("timeTXToRX") : par("timeSleepToRX")).doubleValue());
    //if no sleep to RX defined asume same time as TX to RX
    radio->setSwitchTime(Radio::SLEEP, Radio::RX, (hasPar("timeSleepToRX") ? par("timeSleepToRX") : par("timeTXToRX")).doubleValue());

    //  - switch times to sleep
    //if no TX to sleep defined asume same time as RX to sleep
    radio->setSwitchTime(Radio::TX, Radio::SLEEP, (hasPar("timeTXToSleep") ? par("timeTXToSleep") : par("timeRXToSleep")).doubleValue());
    //if no RX to sleep defined asume same time as TX to sleep
    radio->setSwitchTime(Radio::RX, Radio::SLEEP, (hasPar("timeRXToSleep") ? par("timeRXToSleep") : par("timeTXToSleep")).doubleValue());

    return radio;
}


// load all the analog models listed in the xml file
void PhyLayer80211p::initializeAnalogueModels()
{
    boost::filesystem::path VENTOS_FullPath = omnetpp::getEnvir()->getConfig()->getConfigEntry("network").getBaseDirectory();
    boost::filesystem::path configFullPath = VENTOS_FullPath / "config.xml";
    omnetpp::cXMLElement* xmlConfig = omnetpp::getEnvir()->getXMLDocument(configFullPath.c_str());
    if(xmlConfig == 0)
        throw omnetpp::cRuntimeError("No analogue models configuration file specified.");

    omnetpp::cXMLElementList analogueModelList = xmlConfig->getElementsByTagName("AnalogueModel");
    if(analogueModelList.empty())
        throw omnetpp::cRuntimeError("No analogue models configuration found in configuration file.");

    for(auto &analogueModelData : analogueModelList)
    {
        const char* name = analogueModelData->getAttribute("type");
        if(name == 0)
            throw omnetpp::cRuntimeError("Could not read name of analogue model.");

        ParameterMap params;
        getParametersFromXML(analogueModelData, params);

        AnalogueModel* newAnalogueModel = getAnalogueModelFromName(name, params);
        if(newAnalogueModel == 0)
            throw omnetpp::cRuntimeError("Could not find an analogue model with the name \"%s\".", name);

        // attach the new AnalogueModel to the AnalogueModelList
        analogueModels.push_back(newAnalogueModel);

        coreEV << "AnalogueModel \"" << name << "\" loaded." << std::endl;
    }
}


AnalogueModel* PhyLayer80211p::getAnalogueModelFromName(std::string name, ParameterMap& params)
{
    if (name == "SimplePathlossModel")
        return initializeSimplePathlossModel(params);
    else if (name == "LogNormalShadowing")
        return initializeLogNormalShadowing(params);
    else if (name == "JakesFading")
        return initializeJakesFading(params);
    else if(name == "BreakpointPathlossModel")
        return initializeBreakpointPathlossModel(params);
    else if(name == "PERModel")
        return initializePERModel(params);
    else if (name == "SimpleObstacleShadowing")
        return initializeSimpleObstacleShadowing(params);
    else if (name == "TwoRayInterferenceModel")
    {
        if (world->use2D())
            throw omnetpp::cRuntimeError("The TwoRayInterferenceModel uses nodes' z-position as the antenna height over ground. Refusing to work in a 2D world");

        return initializeTwoRayInterferenceModel(params);
    }
    else if (name == "NakagamiFading")
        return initializeNakagamiFading(params);
    // case "RSAM", pointer is valid as long as the radio exists
    else if (name == "RadioStateAnalogueModel")
        return radio->getAnalogueModel();

    return 0;
}


AnalogueModel* PhyLayer80211p::initializeSimplePathlossModel(ParameterMap& params)
{
    // init with default value
    double alpha = 2.0;
    double carrierFrequency = 5.890e+9;
    bool useTorus = world->useTorus();
    const Coord& playgroundSize = *(world->getPgs());

    // get alpha-coefficient from config
    ParameterMap::iterator it = params.find("alpha");

    if ( it != params.end() ) // parameter alpha has been specified in config.xml
    {
        // set alpha
        alpha = it->second.doubleValue();
        coreEV << "createPathLossModel(): alpha set from config.xml to " << alpha << std::endl;

        // check whether alpha is not smaller than specified in ConnectionManager
        if(cc->hasPar("alpha") && alpha < cc->par("alpha").doubleValue())
        {
            throw omnetpp::cRuntimeError("TestPhyLayer::createPathLossModel(): alpha can't be smaller than specified in \
                   ConnectionManager. Please adjust your config.xml file accordingly");
        }
    }
    else // alpha has not been specified in config.xml
    {
        if (cc->hasPar("alpha")) // parameter alpha has been specified in ConnectionManager
        {
            // set alpha according to ConnectionManager
            alpha = cc->par("alpha").doubleValue();
            coreEV << "createPathLossModel(): alpha set from ConnectionManager to " << alpha << std::endl;
        }
        else // alpha has not been specified in ConnectionManager
        {
            // keep alpha at default value
            coreEV << "createPathLossModel(): alpha set from default value to " << alpha << std::endl;
        }
    }

    // get carrierFrequency from config
    it = params.find("carrierFrequency");
    if ( it != params.end() ) // parameter carrierFrequency has been specified in config.xml
    {
        // set carrierFrequency
        carrierFrequency = it->second.doubleValue();
        coreEV << "createPathLossModel(): carrierFrequency set from config.xml to " << carrierFrequency << std::endl;

        // check whether carrierFrequency is not smaller than specified in ConnectionManager
        if(cc->hasPar("carrierFrequency") && carrierFrequency < cc->par("carrierFrequency").doubleValue())
        {
            throw omnetpp::cRuntimeError("TestPhyLayer::createPathLossModel(): carrierFrequency can't be smaller than specified in \
                   ConnectionManager. Please adjust your config.xml file accordingly");
        }
    }
    else // carrierFrequency has not been specified in config.xml
    {
        if (cc->hasPar("carrierFrequency")) // parameter carrierFrequency has been specified in ConnectionManager
        {
            // set carrierFrequency according to ConnectionManager
            carrierFrequency = cc->par("carrierFrequency").doubleValue();
            coreEV << "createPathLossModel(): carrierFrequency set from ConnectionManager to " << carrierFrequency << std::endl;
        }
        else // carrierFrequency has not been specified in ConnectionManager
        {
            // keep carrierFrequency at default value
            coreEV << "createPathLossModel(): carrierFrequency set from default value to " << carrierFrequency << std::endl;
        }
    }

    return new SimplePathlossModel(alpha, carrierFrequency, useTorus, playgroundSize, coreDebug);
}


AnalogueModel* PhyLayer80211p::initializeLogNormalShadowing(ParameterMap& params)
{
    double mean = params["mean"].doubleValue();
    double stdDev = params["stdDev"].doubleValue();
    omnetpp::simtime_t interval = params["interval"].doubleValue();

    return new LogNormalShadowing(mean, stdDev, interval);
}


AnalogueModel* PhyLayer80211p::initializeJakesFading(ParameterMap& params)
{
    int fadingPaths = params["fadingPaths"].longValue();
    omnetpp::simtime_t delayRMS = params["delayRMS"].doubleValue();
    omnetpp::simtime_t interval = params["interval"].doubleValue();

    double carrierFrequency = 5.890e+9;

    if (params.count("carrierFrequency") > 0)
        carrierFrequency = params["carrierFrequency"];
    else if (cc->hasPar("carrierFrequency"))
        carrierFrequency = cc->par("carrierFrequency").doubleValue();

    return new JakesFading(fadingPaths, delayRMS, carrierFrequency, interval);
}


AnalogueModel* PhyLayer80211p::initializeBreakpointPathlossModel(ParameterMap& params)
{
    double alpha1 = -1, alpha2 = -1, breakpointDistance = -1;
    double L01 = -1, L02 = -1;
    double carrierFrequency = 5.890e+9;
    bool useTorus = world->useTorus();
    const Coord& playgroundSize = *(world->getPgs());
    ParameterMap::iterator it;

    it = params.find("alpha1");
    if ( it != params.end() ) // parameter alpha1 has been specified in config.xml
    {
        // set alpha1
        alpha1 = it->second.doubleValue();
        coreEV << "createPathLossModel(): alpha1 set from config.xml to " << alpha1 << std::endl;
        // check whether alpha is not smaller than specified in ConnectionManager
        if(cc->hasPar("alpha") && alpha1 < cc->par("alpha").doubleValue())
        {
            throw omnetpp::cRuntimeError("TestPhyLayer::createPathLossModel(): alpha can't be smaller than specified in \
	               ConnectionManager. Please adjust your config.xml file accordingly");
        }
    }

    it = params.find("L01");
    if(it != params.end())
        L01 = it->second.doubleValue();

    it = params.find("L02");
    if(it != params.end())
        L02 = it->second.doubleValue();

    it = params.find("alpha2");
    if ( it != params.end() ) // parameter alpha1 has been specified in config.xml
    {
        // set alpha2
        alpha2 = it->second.doubleValue();
        coreEV << "createPathLossModel(): alpha2 set from config.xml to " << alpha2 << std::endl;
        // check whether alpha is not smaller than specified in ConnectionManager
        if(cc->hasPar("alpha") && alpha2 < cc->par("alpha").doubleValue())
        {
            throw omnetpp::cRuntimeError("TestPhyLayer::createPathLossModel(): alpha can't be smaller than specified in \
	               ConnectionManager. Please adjust your config.xml file accordingly");
        }
    }

    it = params.find("breakpointDistance");
    if ( it != params.end() ) // parameter alpha1 has been specified in config.xml
    {
        breakpointDistance = it->second.doubleValue();
        coreEV << "createPathLossModel(): breakpointDistance set from config.xml to " << alpha2 << std::endl;
        // check whether alpha is not smaller than specified in ConnectionManager
    }

    // get carrierFrequency from config
    it = params.find("carrierFrequency");
    if ( it != params.end() ) // parameter carrierFrequency has been specified in config.xml
    {
        // set carrierFrequency
        carrierFrequency = it->second.doubleValue();
        coreEV << "createPathLossModel(): carrierFrequency set from config.xml to " << carrierFrequency << std::endl;

        // check whether carrierFrequency is not smaller than specified in ConnectionManager
        if(cc->hasPar("carrierFrequency") && carrierFrequency < cc->par("carrierFrequency").doubleValue())
        {
            // throw error
            throw omnetpp::cRuntimeError("TestPhyLayer::createPathLossModel(): carrierFrequency can't be smaller than specified in \
	               ConnectionManager. Please adjust your config.xml file accordingly");
        }
    }
    else // carrierFrequency has not been specified in config.xml
    {
        if (cc->hasPar("carrierFrequency")) // parameter carrierFrequency has been specified in ConnectionManager
        {
            // set carrierFrequency according to ConnectionManager
            carrierFrequency = cc->par("carrierFrequency").doubleValue();
            coreEV << "createPathLossModel(): carrierFrequency set from ConnectionManager to " << carrierFrequency << std::endl;
        }
        else // carrierFrequency has not been specified in ConnectionManager
        {
            // keep carrierFrequency at default value
            coreEV << "createPathLossModel(): carrierFrequency set from default value to " << carrierFrequency << std::endl;
        }
    }

    if(alpha1 ==-1 || alpha2==-1 || breakpointDistance==-1 || L01==-1 || L02==-1)
        throw omnetpp::cRuntimeError("Undefined parameters for breakpointPathlossModel. Please check your configuration.");

    return new BreakpointPathlossModel(L01, L02, alpha1, alpha2, breakpointDistance, carrierFrequency, useTorus, playgroundSize, coreDebug);
}


AnalogueModel* PhyLayer80211p::initializePERModel(ParameterMap& params)
{
    double per = params["packetErrorRate"].doubleValue();
    return new PERModel(per);
}


AnalogueModel* PhyLayer80211p::initializeSimpleObstacleShadowing(ParameterMap& params)
{
    // init with default value
    double carrierFrequency = 5.890e+9;
    bool useTorus = world->useTorus();
    const Coord& playgroundSize = *(world->getPgs());

    ParameterMap::iterator it;

    // get carrierFrequency from config
    it = params.find("carrierFrequency");

    if ( it != params.end() ) // parameter carrierFrequency has been specified in config.xml
    {
        // set carrierFrequency
        carrierFrequency = it->second.doubleValue();
        coreEV << "initializeSimpleObstacleShadowing(): carrierFrequency set from config.xml to " << carrierFrequency << std::endl;

        // check whether carrierFrequency is not smaller than specified in ConnectionManager
        if(cc->hasPar("carrierFrequency") && carrierFrequency < cc->par("carrierFrequency").doubleValue())
            throw omnetpp::cRuntimeError("initializeSimpleObstacleShadowing(): carrierFrequency can't be smaller than specified in ConnectionManager. Please adjust your config.xml file accordingly");
    }
    else // carrierFrequency has not been specified in config.xml
    {
        if (cc->hasPar("carrierFrequency")) // parameter carrierFrequency has been specified in ConnectionManager
        {
            // set carrierFrequency according to ConnectionManager
            carrierFrequency = cc->par("carrierFrequency").doubleValue();
            coreEV << "createPathLossModel(): carrierFrequency set from ConnectionManager to " << carrierFrequency << std::endl;
        }
        else // carrierFrequency has not been specified in ConnectionManager
        {
            // keep carrierFrequency at default value
            coreEV << "createPathLossModel(): carrierFrequency set from default value to " << carrierFrequency << std::endl;
        }
    }

    ObstacleControl* obstacleControlP = ObstacleControlAccess().getIfExists();
    if (!obstacleControlP)
        throw omnetpp::cRuntimeError("initializeSimpleObstacleShadowing(): cannot find ObstacleControl module");

    return new SimpleObstacleShadowing(*obstacleControlP, carrierFrequency, useTorus, playgroundSize, coreDebug);
}


AnalogueModel* PhyLayer80211p::initializeTwoRayInterferenceModel(ParameterMap& params)
{
    ASSERT(params.count("DielectricConstant") == 1);
    double dielectricConstant= params["DielectricConstant"].doubleValue();

    return new TwoRayInterferenceModel(dielectricConstant, coreDebug);
}


AnalogueModel* PhyLayer80211p::initializeNakagamiFading(ParameterMap& params)
{
    bool constM = params["constM"].boolValue();
    double m = 0;
    if (constM)
        m = params["m"].doubleValue();

    return new NakagamiFading(constM, m, coreDebug);
}


void PhyLayer80211p::getParametersFromXML(omnetpp::cXMLElement* xmlData, ParameterMap& outputMap)
{
    omnetpp::cXMLElementList parameters = xmlData->getElementsByTagName("Parameter");

    for(auto &it : parameters)
    {
        const char* name = it->getAttribute("name");
        const char* type = it->getAttribute("type");
        const char* value = it->getAttribute("value");

        if(name == 0 || type == 0 || value == 0)
            throw omnetpp::cRuntimeError("Invalid parameter, could not find name, type or value.");

        std::string sType = type;   //needed for easier comparison
        std::string sValue = value; //needed for easier comparison

        omnetpp::cMsgPar param(name);

        //parse type of parameter and set value
        if (sType == "bool")
            param.setBoolValue(sValue == "true" || sValue == "1");
        else if (sType == "double")
            param.setDoubleValue(strtod(value, 0));
        else if (sType == "string")
            param.setStringValue(value);
        else if (sType == "long")
            param.setLongValue(strtol(value, 0, 0));
        else
            throw omnetpp::cRuntimeError("Unknown parameter type: '%s'", sType.c_str());

        //add parameter to output map
        outputMap[name] = param;
    }
}


// load all deciders listed in the xml file
void PhyLayer80211p::initializeDecider()
{
    boost::filesystem::path VENTOS_FullPath = omnetpp::getEnvir()->getConfig()->getConfigEntry("network").getBaseDirectory();
    boost::filesystem::path configFullPath = VENTOS_FullPath / "config.xml";
    omnetpp::cXMLElement* xmlConfig = omnetpp::getEnvir()->getXMLDocument(configFullPath.c_str());
    if(xmlConfig == 0)
        throw omnetpp::cRuntimeError("No decider configuration file specified.");

    omnetpp::cXMLElementList deciderList = xmlConfig->getElementsByTagName("Decider");

    if(deciderList.empty())
        throw omnetpp::cRuntimeError("No decider configuration found in configuration file.");

    if(deciderList.size() > 1)
        throw omnetpp::cRuntimeError("More than one decider configuration found in configuration file.");

    omnetpp::cXMLElement* deciderData = deciderList.front();

    const char* name = deciderData->getAttribute("type");
    if(name == 0)
        throw omnetpp::cRuntimeError("Could not read type of decider from configuration file.");

    ParameterMap params;
    getParametersFromXML(deciderData, params);

    decider = getDeciderFromName(name, params);
    if(decider == 0)
        throw omnetpp::cRuntimeError("Could not find a decider with the name \"%s\".", name);

    coreEV << "Decider \"" << name << "\" loaded." << std::endl;
}


BaseDecider* PhyLayer80211p::getDeciderFromName(std::string name, ParameterMap& params)
{
    if(name == "Decider80211p")
    {
        protocolId = IEEE_80211;
        return initializeDecider80211p(params);
    }

    return 0;
}


BaseDecider* PhyLayer80211p::initializeDecider80211p(ParameterMap& params)
{
    sensitivity = FWMath::dBm2mW(par("sensitivity").doubleValue());

    if(cc->hasPar("sat") && (sensitivity - FWMath::dBm2mW(cc->par("sat").doubleValue())) < -0.000001)
    {
        throw omnetpp::cRuntimeError("Sensitivity can't be smaller than the "
                "signal attenuation threshold (sat) in ConnectionManager. "
                "Please adjust your omnetpp.ini file accordingly.");
    }

    // Clear Channel Assessment (CCA)
    ccaThreshold = pow(10, par("ccaThreshold").doubleValue() / 10);

    if(par("useThermalNoise").boolValue())
    {
        double thermalNoiseVal = FWMath::dBm2mW(par("thermalNoise").doubleValue());
        thermalNoise = new ConstantSimpleConstMapping(DimensionSet::timeDomain(), thermalNoiseVal);
    }

    Decider80211p* dec = new Decider80211p(this,
            sensitivity,
            ccaThreshold,
            par("allowTxDuringRx").boolValue(),
            params["centerFrequency"].doubleValue(),
            findHost()->getIndex(),
            par("collectCollisionStatistics").boolValue(),
            coreDebug);

    dec->setPath(getParentModule()->getFullPath());
    return dec;
}


void PhyLayer80211p::handleAirFrame(AirFrame* frame)
{
    //TODO: ask jerome to set air frame priority in his UWBIRPhy
    //assert(frame->getSchedulingPriority() == airFramePriority());

    switch(frame->getState())
    {
    case START_RECEIVE:
        handleAirFrameStartReceive(frame);
        break;

    case RECEIVING:
        handleAirFrameReceiving(frame);
        break;

    case END_RECEIVE:
        handleAirFrameEndReceive(frame);
        break;

    default:
        throw omnetpp::cRuntimeError("Unknown AirFrame state: %s", frame->getState());
        break;
    }
}


void PhyLayer80211p::handleAirFrameStartReceive(AirFrame* frame)
{
    coreEV << "Received new AirFrame '" << frame << "' from channel. \n";

    if(channelInfo.isChannelEmpty())
        radio->setTrackingModeTo(true);

    channelInfo.addAirFrame(frame, omnetpp::simTime());
    assert(!channelInfo.isChannelEmpty());

    if(usePropagationDelay)
    {
        // calculate the propagation delay
        omnetpp::simtime_t delay = omnetpp::simTime() - frame->getSendingTime();
        // then set it in the frame's signal
        frame->getSignal().setPropagationDelay(delay);
    }

    assert(frame->getSendingTime() + frame->getSignal().getPropagationDelay() == omnetpp::simTime());

    if(emulationActive)
    {
        // add attenuation to the signal object of the frame over time using an analog model
        filterSignal(frame);

        if(decider && frame->getProtocolId() == protocolId)
        {
            frame->setState(RECEIVING);

            // pass the AirFrame the first time to the Decider
            handleAirFrameReceiving(frame);
        }
        // if no decider is defined we will schedule the message directly to its end
        else
        {
            frame->setState(END_RECEIVE);

            omnetpp::simtime_t signalEndTime = frame->getSendingTime() + frame->getSignal().getPropagationDelay() + frame->getDuration();
            scheduleAt(signalEndTime, frame);
        }
    }
    else
    {
        // set frame state to 'END_RECEIVE'
        frame->setState(END_RECEIVE);

        // schedule the message directly to its end
        omnetpp::simtime_t signalEndTime = frame->getSendingTime() + frame->getSignal().getPropagationDelay() + frame->getDuration();
        scheduleAt(signalEndTime, frame);
    }
}


void PhyLayer80211p::handleAirFrameReceiving(AirFrame* frame)
{
    // send the frame to the decider. If the frame is healthy, then
    // it will be sent up to the MAC layer. If the frame is lost, then
    // the reason is sent up to the MAC layer
    omnetpp::simtime_t nextHandleTime = decider->processSignal(frame);

    omnetpp::simtime_t signalEndTime = frame->getSendingTime() + frame->getSignal().getPropagationDelay() + frame->getDuration();

    // check if this is the end of the receiving process
    if(omnetpp::simTime() >= signalEndTime)
    {
        frame->setState(END_RECEIVE);
        handleAirFrameEndReceive(frame);
        return;
    }

    // smaller zero means don't give it to me again
    if(nextHandleTime < 0)
    {
        nextHandleTime = signalEndTime;
        frame->setState(END_RECEIVE);
    }
    // invalid point in time
    else if(nextHandleTime < omnetpp::simTime() || nextHandleTime > signalEndTime)
        throw omnetpp::cRuntimeError("Invalid next handle time returned by Decider. Expected a value between current simulation time (%.2f) and end of signal (%.2f) but got %.2f",
                SIMTIME_DBL(omnetpp::simTime()), SIMTIME_DBL(signalEndTime), SIMTIME_DBL(nextHandleTime));

    coreEV << "Handed AirFrame with ID " << frame->getId() << " to Decider. Next handling in " << nextHandleTime - omnetpp::simTime() << "s." << std::endl;

    scheduleAt(nextHandleTime, frame);
}


void PhyLayer80211p::handleAirFrameEndReceive(AirFrame* frame)
{
    if(!emulationActive)
    {
        // emulation is not active, sending the frame up to the MAC
        DeciderResult80211 *result = new DeciderResult80211(false, 0, 0, 0);
        sendUp(frame, result);
    }

    coreEV << "End of Airframe with ID " << frame->getId() << "." << std::endl;

    omnetpp::simtime_t earliestInfoPoint = channelInfo.removeAirFrame(frame);

    /* clean information in the radio until earliest time-point
     *  of information in the ChannelInfo,
     *  since this time-point might have changed due to removal of
     *  the AirFrame
     */
    if(channelInfo.isChannelEmpty())
    {
        earliestInfoPoint = omnetpp::simTime();
        radio->setTrackingModeTo(false);
    }

    radio->cleanAnalogueModelUntil(earliestInfoPoint);
}


void PhyLayer80211p::handleChannelSenseRequest(omnetpp::cMessage* msg)
{
    MacToPhyCSR* senseReq = static_cast<MacToPhyCSR*>(msg);

    omnetpp::simtime_t nextHandleTime = decider->handleChannelSenseRequest(senseReq);

    // schedule request for next handling
    if(nextHandleTime >= omnetpp::simTime())
    {
        scheduleAt(nextHandleTime, msg);

        // don't throw away any AirFrames while ChannelSenseRequest is active
        if(!channelInfo.isRecording())
            channelInfo.startRecording(omnetpp::simTime());
    }
    else if(nextHandleTime >= 0.0)
        throw omnetpp::cRuntimeError("Next handle time of ChannelSenseRequest returned by the Decider is smaller then current simulation time: %.2f",
                SIMTIME_DBL(nextHandleTime));

    // else, i.e. nextHandleTime < 0.0, the Decider doesn't want to handle
    // the request again
}


void PhyLayer80211p::filterSignal(AirFrame *frame)
{
    if (analogueModels.empty())
        return;

    ChannelAccess *const senderModule = dynamic_cast<ChannelAccess *const>(frame->getSenderModule());
    assert(senderModule);

    // claim the Move pattern of the sender from the Signal
    ChannelMobilityPtrType sendersMobility = senderModule ? senderModule->getMobilityModule()   : NULL;
    // get the sender module position
    const Coord sendersPos  = sendersMobility ? sendersMobility->getCurrentPosition() : Coord::ZERO;

    ChannelAccess *const receiverModule = dynamic_cast<ChannelAccess *const>(frame->getArrivalModule());
    assert(receiverModule);

    ChannelMobilityPtrType receiverMobility = receiverModule ? receiverModule->getMobilityModule() : NULL;
    const Coord receiverPos = receiverMobility ? receiverMobility->getCurrentPosition() : Coord::ZERO;

    for(auto &it : analogueModels)
        it->filterSignal(frame, sendersPos, receiverPos);
}


AirFrame *PhyLayer80211p::encapsMsg(omnetpp::cPacket *macPkt)
{
    // the macPkt must always have a ControlInfo attached
    MacToPhyControlInfo *mac_control = dynamic_cast<MacToPhyControlInfo *>(macPkt->removeControlInfo());
    assert(mac_control);

    // extract parameters from the MAC control
    uint64_t bitrate = mac_control->getBitrate();
    double txPower_mW = mac_control->getPower();
    double freq = mac_control->getFreq();

    // delete the mac_control
    delete mac_control;
    mac_control = 0;

    // calculate frame duration
    omnetpp::simtime_t duration = getFrameDuration(macPkt->getBitLength(), bitrate);
    assert(duration > 0);

    // create signal
    Signal* s = createSignal(omnetpp::simTime() + RADIODELAY_11P, duration, txPower_mW, bitrate, freq);

    // create the new AirFrame11p
    AirFrame* frame = new AirFrame11p(macPkt->getName(), AIR_FRAME);

    // set frame duration
    frame->setDuration(duration);

    // and copy the signal into the AirFrame
    frame->setSignal(*s);

    // pointer and Signal not needed anymore
    delete s;
    s = 0;

    // set priority of AirFrames above the normal priority to ensure
    // channel consistency (before any thing else happens at a time
    // point t make sure that the channel has removed every AirFrame
    // ended at t and added every AirFrame started at t)
    frame->setSchedulingPriority(airFramePriority());

    frame->setProtocolId(protocolId);
    frame->setBitLength(headerLength);
    frame->setId(world->getUniqueAirFrameId());
    frame->setChannel(radio->getCurrentChannel());

    frame->encapsulate(macPkt);

    // from here on, the AirFrame is the owner of the MacPacket
    macPkt = 0;
    coreEV << "AirFrame encapsulated, length: " << frame->getBitLength() << "\n";

    return frame;
}


void PhyLayer80211p::finishRadioSwitching()
{
    radio->endSwitch(omnetpp::simTime());
    sendControlMsgToMac(new VENTOS::PhyToMacReport("Radio switching over", RADIO_SWITCHING_OVER));
}


void PhyLayer80211p::setCCAThreshold(double ccaThreshold_dBm)
{
    ccaThreshold = pow(10, ccaThreshold_dBm / 10);
    ((Decider80211p *)decider)->setCCAThreshold(ccaThreshold_dBm);
}


double PhyLayer80211p::getCCAThreshold()
{
    return 10 * log10(ccaThreshold);
}


Signal* PhyLayer80211p::createSignal(omnetpp::simtime_t start, omnetpp::simtime_t duration, double power, uint64_t bitrate, double frequency)
{
    // create an empty signal
    Signal* s = new Signal();

    omnetpp::simtime_t end = start + duration;

    // create and set tx power mapping
    ConstMapping* txPowerMapping = createSingleFrequencyMapping(start, end, frequency, 5.0e6, power);
    s->setTransmissionPower(txPowerMapping);

    Mapping* bitrateMapping = MappingUtils::createMapping(DimensionSet::timeDomain(), Mapping::STEPS);

    Argument pos(start);
    bitrateMapping->setValue(pos, bitrate);

    pos.setTime(getPhyHeaderLength() / bitrate);
    bitrateMapping->setValue(pos, bitrate);

    s->setBitrate(bitrateMapping);

    return s;
}


Signal* PhyLayer80211p::createSimpleSignal(omnetpp::simtime_t_cref start, omnetpp::simtime_t_cref length, double power, double bitrate)
{
    // create an empty signal
    Signal* s = new Signal();

    omnetpp::simtime_t end = start + length;

    //create and set tx power mapping
    Mapping* txPowerMapping = createRectangleMapping(start, end, power);
    s->setTransmissionPower(txPowerMapping);

    //create and set bitrate mapping
    Mapping* bitrateMapping = createConstantMapping(start, end, bitrate);
    s->setBitrate(bitrateMapping);

    return s;
}


Mapping* PhyLayer80211p::createRectangleMapping(omnetpp::simtime_t_cref start, omnetpp::simtime_t_cref end, Argument::mapped_type_cref value)
{
    //create mapping over time
    Mapping* m = MappingUtils::createMapping(DimensionSet::timeDomain(), Mapping::LINEAR);

    //set position Argument
    Argument startPos(start);
    //set discontinuity at position
    MappingUtils::addDiscontinuity(m, startPos, Argument::MappedZero(), MappingUtils::post(start), value);

    //set position Argument
    Argument endPos(end);
    //set discontinuity at position
    MappingUtils::addDiscontinuity(m, endPos, Argument::MappedZero(), MappingUtils::pre(end), value);

    return m;
}


Mapping* PhyLayer80211p::createConstantMapping(omnetpp::simtime_t_cref start, omnetpp::simtime_t_cref end, Argument::mapped_type_cref value)
{
    //create mapping over time
    Mapping* m = MappingUtils::createMapping(Argument::MappedZero(), DimensionSet::timeDomain(), Mapping::LINEAR);

    //set position Argument
    Argument startPos(start);

    //set mapping at position
    m->setValue(startPos, value);

    //set position Argument
    Argument endPos(end);

    //set mapping at position
    m->setValue(endPos, value);

    return m;
}


ConstMapping* PhyLayer80211p::createSingleFrequencyMapping(omnetpp::simtime_t_cref start,
        omnetpp::simtime_t_cref end,
        Argument::mapped_type_cref centerFreq,
        Argument::mapped_type_cref halfBandwidth,
        Argument::mapped_type_cref value)
{
    Mapping* res = MappingUtils::createMapping(Argument::MappedZero(), DimensionSet::timeFreqDomain(), Mapping::LINEAR);

    Argument pos(DimensionSet::timeFreqDomain());

    pos.setArgValue(Dimension::frequency(), centerFreq - halfBandwidth);
    pos.setTime(start);
    res->setValue(pos, value);

    pos.setTime(end);
    res->setValue(pos, value);

    pos.setArgValue(Dimension::frequency(), centerFreq + halfBandwidth);
    res->setValue(pos, value);

    pos.setTime(start);
    res->setValue(pos, value);

    return res;
}


void PhyLayer80211p::record_PHY_stat_func()
{
    VENTOS::PHY_stat_t entry = {};

    entry.last_stat_time = omnetpp::simTime().dbl();
    entry.NumSentFrames = NumSentFrames;
    entry.NumReceivedFrames = NumReceivedFrames;
    entry.NumLostFrames_BiteError = NumLostFrames_BiteError;
    entry.NumLostFrames_Collision = NumLostFrames_Collision;
    entry.NumLostFrames_TXRX = NumLostFrames_TXRX;

    auto it = STAT->global_PHY_stat.find(myId);
    if(it == STAT->global_PHY_stat.end())
        STAT->global_PHY_stat[myId] = entry;
    else
        it->second = entry;
}


void PhyLayer80211p::record_frameTxRx_stat_error(VENTOS::PhyToMacReport* msg, std::string report)
{
    VENTOS::PhyToMacReport *phyReport = dynamic_cast<VENTOS::PhyToMacReport *>(msg);
    ASSERT(phyReport);

    long int frameId = phyReport->getMsgId();
    long int nicId = this->getParentModule()->getId();

    std::string omnetId = this->getParentModule()->getParentModule()->getFullName();

    auto it = STAT->global_frameTxRx_stat.find(std::make_pair(frameId, nicId));
    if(it == STAT->global_frameTxRx_stat.end())
        throw omnetpp::cRuntimeError("'%s' received frame '%d' from a sender with inactive 'record_frameTxRx'", omnetId.c_str(), frameId);

    it->second.ReceivedAt = omnetpp::simTime().dbl();
    it->second.FrameRxStatus = report;
}


void PhyLayer80211p::record_frameTxRx_stat_healthy(AirFrame* frame)
{
    // we need the id of the message assigned by OMNET++
    long int frameId = (dynamic_cast<omnetpp::cPacket *>(frame))->getId();
    long int nicId = this->getParentModule()->getId();

    std::string omnetId = this->getParentModule()->getParentModule()->getFullName();

    auto it = STAT->global_frameTxRx_stat.find(std::make_pair(frameId, nicId));
    if(it == STAT->global_frameTxRx_stat.end())
        throw omnetpp::cRuntimeError("'%s' received frame '%d' from a sender with inactive 'record_frameTxRx'", omnetId.c_str(), frameId);

    it->second.ReceivedAt = omnetpp::simTime().dbl();
    it->second.FrameRxStatus = "HEALTHY";
}

// ######## implementation of MacToPhyInterface #########

int PhyLayer80211p::getRadioState()
{
    Enter_Method_Silent();

    assert(radio);
    return radio->getCurrentState();
}


omnetpp::simtime_t PhyLayer80211p::setRadioState(int rs)
{
    Enter_Method_Silent();

    assert(radio);

    if (rs == Radio::TX)
        decider->switchToTx();

    if(txOverTimer && txOverTimer->isScheduled())
        EV_WARN << "Switched radio while sending an AirFrame. The effects this would have on the transmission are not simulated by the BasePhyLayer!";

    omnetpp::simtime_t switchTime = radio->switchTo(rs, omnetpp::simTime());

    // invalid switch time, we are probably already switching
    if(switchTime < 0)
        return switchTime;

    // if switching is done in exactly zero-time no extra self-message is scheduled
    if (switchTime == 0.0)
    {
        // TODO: in case of zero-time-switch, send no control-message to MAC!
        // maybe call a method finishRadioSwitchingSilent()
        finishRadioSwitching();
    }
    else
        scheduleAt(omnetpp::simTime() + switchTime, radioSwitchingOverTimer);

    return switchTime;
}


ChannelState PhyLayer80211p::getChannelState()
{
    Enter_Method_Silent();

    assert(decider);
    return decider->getChannelState();
}


int PhyLayer80211p::getPhyHeaderLength()
{
    if (headerLength < 0)
        return par("headerLength").intValue();

    return headerLength;
}


void PhyLayer80211p::setCurrentRadioChannel(int newRadioChannel)
{
    if(txOverTimer && txOverTimer->isScheduled())
        EV_WARN << "Switched channel while sending an AirFrame. The effects this would have on the transmission are not simulated by the BasePhyLayer!";

    radio->setCurrentChannel(newRadioChannel);
    decider->channelChanged(newRadioChannel);
    coreEV << "Switched radio to channel " << newRadioChannel << std::endl;
}


int PhyLayer80211p::getCurrentRadioChannel()
{
    return radio->getCurrentChannel();
}


int PhyLayer80211p::getNbRadioChannels()
{
    return par("nbRadioChannels");
}


omnetpp::simtime_t PhyLayer80211p::getFrameDuration(int payloadLengthBits, uint64_t bitrate, enum PHY_MCS mcs) const
{
    omnetpp::simtime_t duration;

    if (mcs == MCS_DEFAULT)
    {
        // N_DBPS is derived from bitrate
        double n_dbps = -1;

        for (unsigned int i = 0; i < NUM_BITRATES_80211P; i++)
        {
            if (bitrate == BITRATES_80211P[i])
                n_dbps = N_DBPS_80211P[i];
        }

        if(n_dbps == -1)
            throw omnetpp::cRuntimeError("Chosen Bitrate is not valid for 802.11p: Valid rates are: 3Mbps, 4.5Mbps, 6Mbps, 9Mbps, 12Mbps, 18Mbps, 24Mbps and 27Mbps. Please adjust your omnetpp.ini file accordingly.");

        // calculate frame duration according to Equation (17-29) of the IEEE 802.11-2007 standard
        duration = PHY_HDR_PREAMBLE_DURATION + PHY_HDR_PLCPSIGNAL_DURATION + T_SYM_80211P * ceil( (16 + payloadLengthBits + 6)/(n_dbps) );
    }
    else
    {
        uint32_t ndbps = getNDBPS(mcs);
        duration = PHY_HDR_PREAMBLE_DURATION + PHY_HDR_PLCPSIGNAL_DURATION + T_SYM_80211P * ceil( (16 + payloadLengthBits + 6)/(ndbps) );
    }

    return duration;
}


void PhyLayer80211p::changeListeningFrequency(double freq)
{
    Decider80211p* dec = dynamic_cast<Decider80211p*>(decider);
    assert(dec);

    dec->changeFrequency(freq);
}

// ###############################################

// ######## implementation of DeciderToPhyInterface #########

void PhyLayer80211p::getChannelInfo(omnetpp::simtime_t_cref from, omnetpp::simtime_t_cref to, AirFrameVector& out)
{
    channelInfo.getAirFrames(from, to, out);
}


ConstMapping* PhyLayer80211p::getThermalNoise(omnetpp::simtime_t_cref from, omnetpp::simtime_t_cref to)
{
    if(thermalNoise)
        thermalNoise->initializeArguments(Argument(from));

    return thermalNoise;
}


void PhyLayer80211p::sendControlMsgToMac(VENTOS::PhyToMacReport* msg)
{
    Enter_Method("sendControlMsgToMac");

    // take ownership of the message
    take(msg);

    if(msg->getKind() == CHANNEL_SENSE_REQUEST)
    {
        if(channelInfo.isRecording())
            channelInfo.stopRecording();
    }
    else if (msg->getKind() == Decider80211p::BITERROR)
    {
        NumLostFrames_BiteError++;

        if(record_stat) record_PHY_stat_func();
        if(record_frameTxRx) record_frameTxRx_stat_error(msg, "BITERROR");
    }
    else if(msg->getKind() == Decider80211p::COLLISION)
    {
        NumLostFrames_Collision++;

        if(record_stat) record_PHY_stat_func();
        if(record_frameTxRx) record_frameTxRx_stat_error(msg, "COLLISION");
    }
    else if(msg->getKind() == Decider80211p::RECWHILESEND)
    {
        NumLostFrames_TXRX++;

        if(record_stat) record_PHY_stat_func();
        if(record_frameTxRx) record_frameTxRx_stat_error(msg, "RECWHILESEND");
    }

    send(msg, upperControlOut);
}


void PhyLayer80211p::sendUp(AirFrame* frame, DeciderResult80211* result)
{
    if(record_frameTxRx) record_frameTxRx_stat_healthy(frame);

    NumReceivedFrames++;

    if(record_stat)
        record_PHY_stat_func();

    coreEV << "Decapsulating MacPacket from Airframe with ID " << frame->getId() << " and sending it up to MAC." << std::endl;

    omnetpp::cMessage* packet = frame->decapsulate();
    assert(packet);

    PhyToMacControlInfo::setControlInfo(packet, result);

    send(packet, upperLayerOut);
}


void PhyLayer80211p::cancelScheduledMessage(omnetpp::cMessage* msg)
{
    if(msg->isScheduled())
        cancelEvent(msg);
    else
    {
        EV << "Warning: Decider wanted to cancel a scheduled message but message"
                << " wasn't actually scheduled. Message is: " << msg << std::endl;
    }
}


void PhyLayer80211p::rescheduleMessage(omnetpp::cMessage* msg, omnetpp::simtime_t_cref t)
{
    cancelScheduledMessage(msg);
    scheduleAt(t, msg);
}


BaseWorldUtility* PhyLayer80211p::getWorldUtility()
{
    return world;
}


void PhyLayer80211p::recordScalar(const char *name, double value, const char *unit)
{
    ChannelAccess::recordScalar(name, value, unit);
}

// ###############################################

}
