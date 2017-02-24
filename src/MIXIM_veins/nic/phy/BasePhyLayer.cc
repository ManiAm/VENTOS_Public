
#include "BasePhyLayer.h"
#include "MacToPhyControlInfo.h"
#include "PhyToMacControlInfo.h"
#include "FindModule.h"
#include "AnalogueModel.h"
#include "Decider.h"
#include "BaseWorldUtility.h"
#include "BaseConnectionManager.h"

//introduce BasePhyLayer as module to OMNet
Define_Module(BasePhyLayer);

Coord NoMobiltyPos = Coord::ZERO;

BasePhyLayer::BasePhyLayer()
{
    this->protocolId = GENERIC;
    this->thermalNoise = 0;
    this->radio = 0;
    this->decider = 0;
    this->radioSwitchingOverTimer = 0;
    this->txOverTimer = 0;
    this->headerLength = -1;
    this->world = NULL;
}


void BasePhyLayer::initialize(int stage)
{
    ChannelAccess::initialize(stage);

    if (stage == 0)
    {
        // if using sendDirect, make sure that messages arrive without delay
        gate("radioIn")->setDeliverOnReceptionStart(true);

        //get gate ids
        upperLayerIn = findGate("upperLayerIn");
        upperLayerOut = findGate("upperLayerOut");
        upperControlOut = findGate("upperControlOut");
        upperControlIn = findGate("upperControlIn");

        if(par("useThermalNoise").boolValue())
        {
            double thermalNoiseVal = FWMath::dBm2mW(par("thermalNoise").doubleValue());
            thermalNoise = new ConstantSimpleConstMapping(DimensionSet::timeDomain(), thermalNoiseVal);
        }
        else
            thermalNoise = 0;

        sensitivity = FWMath::dBm2mW(par("sensitivity").doubleValue());

        headerLength = par("headerLength").longValue();
        recordStats = par("recordStats").boolValue();

        // initialize radio
        radio = initializeRadio();

        // get pointer to the world module
        world = FindModule<BaseWorldUtility*>::findGlobalModule();
        if (world == NULL)
            throw omnetpp::cRuntimeError("Could not find BaseWorldUtility module");

        if(cc->hasPar("sat") && (sensitivity - FWMath::dBm2mW(cc->par("sat").doubleValue())) < -0.000001)
        {
            throw omnetpp::cRuntimeError("Sensitivity can't be smaller than the "
                    "signal attenuation threshold (sat) in ConnectionManager. "
                    "Please adjust your omnetpp.ini file accordingly.");
        }

        // analogue model parameters
        initializeAnalogueModels(par("analogueModels").xmlValue());

        // decider parameters
        initializeDecider(par("decider").xmlValue());

        // initialise timer messages
        radioSwitchingOverTimer = new omnetpp::cMessage("radio switching over", RADIO_SWITCHING_OVER);
        txOverTimer = new omnetpp::cMessage("transmission over", TX_OVER);
    }
}


Radio* BasePhyLayer::initializeRadio()
{
    int initialRadioState = par("initialRadioState").longValue();
    double radioMinAtt = par("radioMinAtt").doubleValue();
    double radioMaxAtt = par("radioMaxAtt").doubleValue();
    int nbRadioChannels = hasPar("nbRadioChannels") ? par("nbRadioChannels") : 1;
    int initialRadioChannel = hasPar("initialRadioChannel") ? par("initialRadioChannel") : 0;

    Radio* radio = Radio::createNewRadio(recordStats, initialRadioState,
            radioMinAtt, radioMaxAtt,
            initialRadioChannel, nbRadioChannels);

    //	- switch times to TX
    //if no RX to TX defined asume same time as sleep to TX
    radio->setSwitchTime(Radio::RX, Radio::TX, (hasPar("timeRXToTX") ? par("timeRXToTX") : par("timeSleepToTX")).doubleValue());
    //if no sleep to TX defined asume same time as RX to TX
    radio->setSwitchTime(Radio::SLEEP, Radio::TX, (hasPar("timeSleepToTX") ? par("timeSleepToTX") : par("timeRXToTX")).doubleValue());

    //	- switch times to RX
    //if no TX to RX defined asume same time as sleep to RX
    radio->setSwitchTime(Radio::TX, Radio::RX, (hasPar("timeTXToRX") ? par("timeTXToRX") : par("timeSleepToRX")).doubleValue());
    //if no sleep to RX defined asume same time as TX to RX
    radio->setSwitchTime(Radio::SLEEP, Radio::RX, (hasPar("timeSleepToRX") ? par("timeSleepToRX") : par("timeTXToRX")).doubleValue());

    //	- switch times to sleep
    //if no TX to sleep defined asume same time as RX to sleep
    radio->setSwitchTime(Radio::TX, Radio::SLEEP, (hasPar("timeTXToSleep") ? par("timeTXToSleep") : par("timeRXToSleep")).doubleValue());
    //if no RX to sleep defined asume same time as TX to sleep
    radio->setSwitchTime(Radio::RX, Radio::SLEEP, (hasPar("timeRXToSleep") ? par("timeRXToSleep") : par("timeTXToSleep")).doubleValue());

    return radio;
}


void BasePhyLayer::getParametersFromXML(omnetpp::cXMLElement* xmlData, ParameterMap& outputMap)
{
    omnetpp::cXMLElementList parameters = xmlData->getElementsByTagName("Parameter");

    for(auto &it : parameters)
    {
        const char* name = it->getAttribute("name");
        const char* type = it->getAttribute("type");
        const char* value = it->getAttribute("value");

        if(name == 0 || type == 0 || value == 0)
            throw omnetpp::cRuntimeError("Invalid parameter, could not find name, type or value.");

        std::string sType = type; 	//needed for easier comparison
        std::string sValue = value;	//needed for easier comparison

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


void BasePhyLayer::finish()
{
    // give decider the chance to do something
    decider->finish();
}

//-----Decider initialization----------------------


void BasePhyLayer::initializeDecider(omnetpp::cXMLElement* xmlConfig)
{
    decider = 0;

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


Decider* BasePhyLayer::getDeciderFromName(std::string name, ParameterMap& params)
{
    return 0;
}


//-----AnalogueModels initialization----------------

void BasePhyLayer::initializeAnalogueModels(omnetpp::cXMLElement* xmlConfig)
{
    /*
     * first of all, attach the AnalogueModel that represents the RadioState
     * to the AnalogueModelList as first element.
     */

    std::string s("RadioStateAnalogueModel");
    ParameterMap p;

    AnalogueModel* newAnalogueModel = getAnalogueModelFromName(s, p);
    if(newAnalogueModel == 0)
        throw omnetpp::cRuntimeError("Could not find an analogue model with the name \"%s\".", s.c_str());

    analogueModels.push_back(newAnalogueModel);

    // then load all the analog models listed in the xml file

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


AnalogueModel* BasePhyLayer::getAnalogueModelFromName(std::string name, ParameterMap& params)
{
    // add default analogue models here

    // case "RSAM", pointer is valid as long as the radio exists
    if (name == "RadioStateAnalogueModel")
        return radio->getAnalogueModel();

    return 0;
}

//---------------Message handling----------------------

void BasePhyLayer::handleMessage(omnetpp::cMessage* msg)
{
    // self messages
    if(msg->isSelfMessage())
        handleSelfMessage(msg);
    // MacPkts <-- MacToPhyControlInfo
    else if(msg->getArrivalGateId() == upperLayerIn)
        handleUpperMessage(msg);
    // control messages
    else if(msg->getArrivalGateId() == upperControlIn)
        handleUpperControlMessage(msg);
    // AirFrames
    else if(msg->getKind() == AIR_FRAME)
        handleAirFrame(static_cast<AirFrame*>(msg));
    // unknown message
    else
    {
        EV << "Unknown message received. \n";
        delete msg;
    }
}


void BasePhyLayer::handleAirFrame(AirFrame* frame)
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


void BasePhyLayer::handleAirFrameStartReceive(AirFrame* frame)
{
    coreEV << "Received new AirFrame " << frame << " from channel. \n";

    if(channelInfo.isChannelEmpty())
        radio->setTrackingModeTo(true);

    channelInfo.addAirFrame(frame, omnetpp::simTime());
    assert(!channelInfo.isChannelEmpty());

    if(usePropagationDelay)
    {
        Signal& s = frame->getSignal();
        omnetpp::simtime_t delay = omnetpp::simTime() - s.getSendingStart();
        s.setPropagationDelay(delay);
    }

    assert(frame->getSignal().getReceptionStart() == omnetpp::simTime());

    frame->getSignal().setReceptionSenderInfo(frame);
    filterSignal(frame);

    if(decider && isKnownProtocolId(frame->getProtocolId()))
    {
        frame->setState(RECEIVING);

        //pass the AirFrame the first time to the Decider
        handleAirFrameReceiving(frame);
    }
    //if no decider is defined we will schedule the message directly to its end
    else
    {
        Signal& signal = frame->getSignal();

        omnetpp::simtime_t signalEndTime = signal.getReceptionStart() + frame->getDuration();
        frame->setState(END_RECEIVE);

        scheduleAt(signalEndTime, frame);
    }
}


void BasePhyLayer::handleAirFrameReceiving(AirFrame* frame)
{
    Signal& signal = frame->getSignal();
    omnetpp::simtime_t nextHandleTime = decider->processSignal(frame);

    assert(signal.getDuration() == frame->getDuration());
    omnetpp::simtime_t signalEndTime = signal.getReceptionStart() + frame->getDuration();

    //check if this is the end of the receiving process
    if(omnetpp::simTime() >= signalEndTime)
    {
        frame->setState(END_RECEIVE);
        handleAirFrameEndReceive(frame);
        return;
    }

    //smaller zero means don't give it to me again
    if(nextHandleTime < 0)
    {
        nextHandleTime = signalEndTime;
        frame->setState(END_RECEIVE);
    }
    //invalid point in time
    else if(nextHandleTime < omnetpp::simTime() || nextHandleTime > signalEndTime)
        throw omnetpp::cRuntimeError("Invalid next handle time returned by Decider. Expected a value between current simulation time (%.2f) and end of signal (%.2f) but got %.2f",
                SIMTIME_DBL(omnetpp::simTime()), SIMTIME_DBL(signalEndTime), SIMTIME_DBL(nextHandleTime));

    coreEV << "Handed AirFrame with ID " << frame->getId() << " to Decider. Next handling in " << nextHandleTime - omnetpp::simTime() << "s." << std::endl;

    scheduleAt(nextHandleTime, frame);
}


void BasePhyLayer::handleAirFrameEndReceive(AirFrame* frame)
{
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


void BasePhyLayer::handleUpperMessage(omnetpp::cMessage* msg)
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

    // build the AirFrame to send
    assert(dynamic_cast<omnetpp::cPacket*>(msg) != 0);

    AirFrame* frame = encapsMsg(static_cast<omnetpp::cPacket*>(msg));

    // make sure there is no self message of kind TX_OVER scheduled
    // and schedule the actual one
    assert (!txOverTimer->isScheduled());
    scheduleAt(omnetpp::simTime() + frame->getDuration(), txOverTimer);

    sendToChannel(frame);
}


AirFrame *BasePhyLayer::encapsMsg(omnetpp::cPacket *macPkt)
{
    // ...and must always have a ControlInfo attached (contains Signal)
    cObject* ctrlInfo = macPkt->removeControlInfo();
    assert(ctrlInfo);

    // Retrieve the pointer to the Signal-instance from the ControlInfo-instance.
    // We are now the new owner of this instance.
    Signal* s = MacToPhyControlInfo::getSignalFromControlInfo(ctrlInfo);
    // make sure we really obtained a pointer to an instance
    assert(s);

    // create the new AirFrame
    AirFrame* frame = new AirFrame(macPkt->getName(), AIR_FRAME);

    assert(s->getDuration() > 0);
    frame->setDuration(s->getDuration());
    // copy the signal into the AirFrame
    frame->setSignal(*s);
    //set priority of AirFrames above the normal priority to ensure
    //channel consistency (before any thing else happens at a time
    //point t make sure that the channel has removed every AirFrame
    //ended at t and added every AirFrame started at t)
    frame->setSchedulingPriority(airFramePriority());
    frame->setProtocolId(myProtocolId());
    frame->setBitLength(headerLength);
    frame->setId(world->getUniqueAirFrameId());
    frame->setChannel(radio->getCurrentChannel());

    // pointer and Signal not needed anymore
    delete s;
    s = 0;

    // delete the Control info
    delete ctrlInfo;
    ctrlInfo = 0;

    frame->encapsulate(macPkt);

    // from here on, the AirFrame is the owner of the MacPacket
    macPkt = 0;
    coreEV << "AirFrame encapsulated, length: " << frame->getBitLength() << "\n";

    return frame;
}


void BasePhyLayer::filterSignal(AirFrame *frame)
{
    if (analogueModels.empty())
        return;

    ChannelAccess *const senderModule   = dynamic_cast<ChannelAccess *const>(frame->getSenderModule());
    ChannelAccess *const receiverModule = dynamic_cast<ChannelAccess *const>(frame->getArrivalModule());

    assert(senderModule);
    assert(receiverModule);

    /** claim the Move pattern of the sender from the Signal */
    ChannelMobilityPtrType sendersMobility  = senderModule   ? senderModule->getMobilityModule()   : NULL;
    ChannelMobilityPtrType receiverMobility = receiverModule ? receiverModule->getMobilityModule() : NULL;

    const Coord sendersPos  = sendersMobility  ? sendersMobility->getCurrentPosition() : NoMobiltyPos;
    const Coord receiverPos = receiverMobility ? receiverMobility->getCurrentPosition(): NoMobiltyPos;

    for(auto &it : analogueModels)
        it->filterSignal(frame, sendersPos, receiverPos);
}


void BasePhyLayer::handleChannelSenseRequest(omnetpp::cMessage* msg)
{
    ChannelSenseRequest* senseReq = static_cast<ChannelSenseRequest*>(msg);

    omnetpp::simtime_t nextHandleTime = decider->handleChannelSenseRequest(senseReq);

    //schedule request for next handling
    if(nextHandleTime >= omnetpp::simTime())
    {
        scheduleAt(nextHandleTime, msg);

        //don't throw away any AirFrames while ChannelSenseRequest is active
        if(!channelInfo.isRecording())
            channelInfo.startRecording(omnetpp::simTime());
    }
    else if(nextHandleTime >= 0.0)
        throw omnetpp::cRuntimeError("Next handle time of ChannelSenseRequest returned by the Decider is smaller then current simulation time: %.2f",
                SIMTIME_DBL(nextHandleTime));

    // else, i.e. nextHandleTime < 0.0, the Decider doesn't want to handle
    // the request again
}


void BasePhyLayer::handleUpperControlMessage(omnetpp::cMessage* msg)
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


void BasePhyLayer::handleSelfMessage(omnetpp::cMessage* msg)
{
    switch(msg->getKind())
    {
    //transmission over
    case TX_OVER:
        assert(msg == txOverTimer);
        sendControlMsgToMac(new omnetpp::cMessage("Transmission over", TX_OVER));
        break;
        //radio switch over
    case RADIO_SWITCHING_OVER:
        assert(msg == radioSwitchingOverTimer);
        finishRadioSwitching();
        break;
        //AirFrame
    case AIR_FRAME:
        handleAirFrame(static_cast<AirFrame*>(msg));
        break;
        //ChannelSenseRequest
    case CHANNEL_SENSE_REQUEST:
        handleChannelSenseRequest(msg);
        break;
    default:
        break;
    }
}


//--Destruction--------------------------------

BasePhyLayer::~BasePhyLayer()
{
    //get AirFrames from ChannelInfo and delete
    //(although ChannelInfo normally owns the AirFrames it
    //is not able to cancel and delete them itself
    AirFrameVector channel;
    channelInfo.getAirFrames(0, omnetpp::simTime(), channel);

    for(AirFrameVector::iterator it = channel.begin(); it != channel.end(); ++it)
        cancelAndDelete(*it);

    //free timer messages
    if(txOverTimer)
        cancelAndDelete(txOverTimer);

    if(radioSwitchingOverTimer)
        cancelAndDelete(radioSwitchingOverTimer);

    //free thermal noise mapping
    if(thermalNoise)
        delete thermalNoise;

    //free Decider
    if(decider != 0)
        delete decider;

    /*
     * get a pointer to the radios RSAM again to avoid deleting it,
     * it is not created by calling new (BasePhyLayer is not the owner)!
     */
    AnalogueModel* rsamPointer = radio ? radio->getAnalogueModel() : NULL;

    //free AnalogueModels
    for(AnalogueModelList::iterator it = analogueModels.begin(); it != analogueModels.end(); it++)
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


//--MacToPhyInterface implementation-----------------------

int BasePhyLayer::getRadioState()
{
    Enter_Method_Silent();

    assert(radio);
    return radio->getCurrentState();
}


void BasePhyLayer::finishRadioSwitching()
{
    radio->endSwitch(omnetpp::simTime());
    sendControlMsgToMac(new omnetpp::cMessage("Radio switching over", RADIO_SWITCHING_OVER));
}


omnetpp::simtime_t BasePhyLayer::setRadioState(int rs)
{
    Enter_Method_Silent();

    assert(radio);

    if(txOverTimer && txOverTimer->isScheduled())
        EV_WARN << "Switched radio while sending an AirFrame. The effects this would have on the transmission are not simulated by the BasePhyLayer!";

    omnetpp::simtime_t switchTime = radio->switchTo(rs, omnetpp::simTime());

    //invalid switch time, we are probably already switching
    if(switchTime < 0)
        return switchTime;

    // if switching is done in exactly zero-time no extra self-message is scheduled
    if (switchTime == 0.0)
    {
        // TODO: in case of zero-time-switch, send no control-message to mac!
        // maybe call a method finishRadioSwitchingSilent()
        finishRadioSwitching();
    }
    else
        scheduleAt(omnetpp::simTime() + switchTime, radioSwitchingOverTimer);

    return switchTime;
}


ChannelState BasePhyLayer::getChannelState()
{
    Enter_Method_Silent();

    assert(decider);
    return decider->getChannelState();
}


int BasePhyLayer::getPhyHeaderLength()
{
    Enter_Method_Silent();

    if (headerLength < 0)
        return par("headerLength").longValue();

    return headerLength;
}


void BasePhyLayer::setCurrentRadioChannel(int newRadioChannel)
{
    if(txOverTimer && txOverTimer->isScheduled())
        EV_WARN << "Switched channel while sending an AirFrame. The effects this would have on the transmission are not simulated by the BasePhyLayer!";

    radio->setCurrentChannel(newRadioChannel);
    decider->channelChanged(newRadioChannel);
    coreEV << "Switched radio to channel " << newRadioChannel << std::endl;
}


int BasePhyLayer::getCurrentRadioChannel()
{
    return radio->getCurrentChannel();
}


int BasePhyLayer::getNbRadioChannels()
{
    return par("nbRadioChannels");
}


//--DeciderToPhyInterface implementation------------

void BasePhyLayer::getChannelInfo(omnetpp::simtime_t_cref from, omnetpp::simtime_t_cref to, AirFrameVector& out)
{
    channelInfo.getAirFrames(from, to, out);
}


ConstMapping* BasePhyLayer::getThermalNoise(omnetpp::simtime_t_cref from, omnetpp::simtime_t_cref to)
{
    if(thermalNoise)
        thermalNoise->initializeArguments(Argument(from));

    return thermalNoise;
}


void BasePhyLayer::sendControlMsgToMac(omnetpp::cMessage* msg)
{
    if(msg->getKind() == CHANNEL_SENSE_REQUEST)
    {
        if(channelInfo.isRecording())
            channelInfo.stopRecording();
    }

    send(msg, upperControlOut);
}


void BasePhyLayer::sendUp(AirFrame* frame, DeciderResult* result)
{
    coreEV << "Decapsulating MacPacket from Airframe with ID " << frame->getId() << " and sending it up to MAC." << std::endl;

    omnetpp::cMessage* packet = frame->decapsulate();
    assert(packet);

    setUpControlInfo(packet, result);

    send(packet, upperLayerOut);
}


omnetpp::simtime_t BasePhyLayer::getSimTime()
{

    return omnetpp::simTime();
}


void BasePhyLayer::cancelScheduledMessage(omnetpp::cMessage* msg)
{
    if(msg->isScheduled())
        cancelEvent(msg);
    else
    {
        EV << "Warning: Decider wanted to cancel a scheduled message but message"
                << " wasn't actually scheduled. Message is: " << msg << std::endl;
    }
}


void BasePhyLayer::rescheduleMessage(omnetpp::cMessage* msg, omnetpp::simtime_t_cref t)
{
    cancelScheduledMessage(msg);
    scheduleAt(t, msg);
}


void BasePhyLayer::drawCurrent(double amount, int activity)
{

}


BaseWorldUtility* BasePhyLayer::getWorldUtility()
{
    return world;
}


void BasePhyLayer::recordScalar(const char *name, double value, const char *unit)
{
    ChannelAccess::recordScalar(name, value, unit);
}


/**
 * Attaches a "control info" (PhyToMac) structure (object) to the message pMsg.
 */
omnetpp::cObject *const BasePhyLayer::setUpControlInfo(omnetpp::cMessage *const pMsg, DeciderResult *const pDeciderResult)
{
    return PhyToMacControlInfo::setControlInfo(pMsg, pDeciderResult);
}
