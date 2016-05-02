
#include "Signal_.h"
#include <cassert>

Signal::Signal(omnetpp::simtime_t_cref sendingStart, omnetpp::simtime_t_cref duration):
	senderModuleID(-1), senderFromGateID(-1), receiverModuleID(-1), receiverToGateID(-1),
	sendingStart(sendingStart), duration(duration),
	propagationDelay(0),
	power(NULL), bitrate(NULL),
	txBitrate(NULL),
	attenuations(), rcvPower(NULL)
{}

Signal::Signal(const Signal & o):
	senderModuleID(o.senderModuleID), senderFromGateID(o.senderFromGateID), receiverModuleID(o.receiverModuleID), receiverToGateID(o.receiverToGateID),
	sendingStart(o.sendingStart), duration(o.duration),
	propagationDelay(o.propagationDelay),
	power(NULL), bitrate(NULL),
	txBitrate(NULL),
	attenuations(), rcvPower(NULL)
{
	if (o.power) {
		power = o.power->constClone();
	}

	if (o.bitrate) {
		bitrate = o.bitrate->clone();
	}

	if (o.txBitrate) {
		txBitrate = o.txBitrate->clone();
	}

	for(ConstMappingList::const_iterator it = o.attenuations.begin();
		it != o.attenuations.end(); it++){
		attenuations.push_back((*it)->constClone());
	}
}

const Signal& Signal::operator=(const Signal& o) {
	sendingStart     = o.sendingStart;
	duration         = o.duration;
	propagationDelay = o.propagationDelay;
	senderModuleID   = o.senderModuleID;
	senderFromGateID = o.senderFromGateID;
	receiverModuleID = o.receiverModuleID;
	receiverToGateID = o.receiverToGateID;

	markRcvPowerOutdated();

	if(power){
		delete power;
		power = NULL;
	}

	if(bitrate){
		delete bitrate;
		bitrate = NULL;
	}

	if(txBitrate){
		delete txBitrate;
		txBitrate = NULL;
	}

	if(o.power)
		power = o.power->constClone();

	if(o.bitrate)
		bitrate = o.bitrate->clone();

	if(o.txBitrate)
		txBitrate = o.txBitrate->clone();

	for(ConstMappingList::const_iterator it = attenuations.begin();
		it != attenuations.end(); ++it){
		delete(*it);
	}

	attenuations.clear();

	for(ConstMappingList::const_iterator it = o.attenuations.begin();
		it != o.attenuations.end(); ++it){
		attenuations.push_back((*it)->constClone());
	}

	return *this;
}

void Signal::swap(Signal& s) {
	std::swap(senderModuleID,   s.senderModuleID);
	std::swap(senderFromGateID, s.senderFromGateID);
	std::swap(receiverModuleID, s.receiverModuleID);
	std::swap(receiverToGateID, s.receiverToGateID);
	std::swap(sendingStart,     s.sendingStart);
	std::swap(duration,         s.duration);
	std::swap(propagationDelay, s.propagationDelay);
	std::swap(power,            s.power);
	std::swap(bitrate,          s.bitrate);
	std::swap(txBitrate,        s.txBitrate);
	std::swap(attenuations,     s.attenuations);
	std::swap(rcvPower,         s.rcvPower);
}

Signal::~Signal()
{
	if(rcvPower){
		if(propagationDelay != 0){
			assert(rcvPower->getRefMapping() != power);
			delete rcvPower->getRefMapping();
		}

		delete rcvPower;
	}

	if(power)
		delete power;

	if(bitrate)
		delete bitrate;

	if(txBitrate)
		delete txBitrate;

	for(ConstMappingList::iterator it = attenuations.begin();
		it != attenuations.end(); it++) {

		delete *it;
	}
}

omnetpp::simtime_t_cref Signal::getSendingStart() const {
	return sendingStart;
}

omnetpp::simtime_t Signal::getSendingEnd() const {
	return sendingStart + duration;
}

omnetpp::simtime_t Signal::getReceptionStart() const {
	return sendingStart + propagationDelay;
}

omnetpp::simtime_t Signal::getReceptionEnd() const {
	return sendingStart + propagationDelay + duration;
}

omnetpp::simtime_t_cref Signal::getDuration() const{
	return duration;
}

omnetpp::simtime_t_cref Signal::getPropagationDelay() const {
	return propagationDelay;
}

void Signal::setPropagationDelay(omnetpp::simtime_t_cref delay) {
	assert(propagationDelay == 0);
	assert(!txBitrate);

	markRcvPowerOutdated();

	propagationDelay = delay;

	if(bitrate) {
		txBitrate = bitrate;
		bitrate = new DelayedMapping(txBitrate, propagationDelay);
	}
}

void Signal::setTransmissionPower(ConstMapping *power)
{
	if(this->power){
		markRcvPowerOutdated();
		delete this->power;
	}

	this->power = power;
}

void Signal::setBitrate(Mapping *bitrate)
{
	assert(!txBitrate);

	if(this->bitrate)
		delete this->bitrate;

	this->bitrate = bitrate;
}

omnetpp::cGate *Signal::getSendingGate() const
{
    if (senderFromGateID < 0) return NULL;
    omnetpp::cModule *const mod = getSendingModule();
    return !mod ? NULL : mod->gate(senderFromGateID);
}

omnetpp::cGate *Signal::getReceptionGate() const
{
    if (receiverToGateID < 0) return NULL;
    omnetpp::cModule *const mod = getReceptionModule();
    return !mod ? NULL : mod->gate(receiverToGateID);
}

void Signal::setReceptionSenderInfo(const omnetpp::cMessage *const pMsg)
{
	if (!pMsg)
		return;

	assert(senderModuleID < 0);

	senderModuleID   = pMsg->getSenderModuleId();
	senderFromGateID = pMsg->getSenderGateId();

	receiverModuleID = pMsg->getArrivalModuleId();
	receiverToGateID = pMsg->getArrivalGateId();
}
