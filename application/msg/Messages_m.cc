//
// Generated file, do not edit! Created by opp_msgc 4.3 from msg/Messages.msg.
//

// Disable warnings about unused variables, empty switch stmts, etc:
#ifdef _MSC_VER
#  pragma warning(disable:4101)
#  pragma warning(disable:4065)
#endif

#include <iostream>
#include <sstream>
#include "Messages_m.h"

// Template rule which fires if a struct or class doesn't have operator<<
template<typename T>
std::ostream& operator<<(std::ostream& out,const T&) {return out;}

// Another default rule (prevents compiler from choosing base class' doPacking())
template<typename T>
void doPacking(cCommBuffer *, T& t) {
    throw cRuntimeError("Parsim error: no doPacking() function for type %s or its base class (check .msg and _m.cc/h files!)",opp_typename(typeid(t)));
}

template<typename T>
void doUnpacking(cCommBuffer *, T& t) {
    throw cRuntimeError("Parsim error: no doUnpacking() function for type %s or its base class (check .msg and _m.cc/h files!)",opp_typename(typeid(t)));
}




Register_Class(WaveShortMessage);

WaveShortMessage::WaveShortMessage(const char *name, int kind) : cPacket(name,kind)
{
    this->wsmVersion_var = 0;
    this->securityType_var = 0;
    this->channelNumber_var = 0;
    this->dataRate_var = 1;
    this->priority_var = 3;
    this->psid_var = 0;
    this->psc_var = "Service with some Data";
    this->wsmLength_var = 0;
    this->wsmData_var = "Some Data";
}

WaveShortMessage::WaveShortMessage(const WaveShortMessage& other) : cPacket(other)
{
    copy(other);
}

WaveShortMessage::~WaveShortMessage()
{
}

WaveShortMessage& WaveShortMessage::operator=(const WaveShortMessage& other)
{
    if (this==&other) return *this;
    cPacket::operator=(other);
    copy(other);
    return *this;
}

void WaveShortMessage::copy(const WaveShortMessage& other)
{
    this->wsmVersion_var = other.wsmVersion_var;
    this->securityType_var = other.securityType_var;
    this->channelNumber_var = other.channelNumber_var;
    this->dataRate_var = other.dataRate_var;
    this->priority_var = other.priority_var;
    this->psid_var = other.psid_var;
    this->psc_var = other.psc_var;
    this->wsmLength_var = other.wsmLength_var;
    this->wsmData_var = other.wsmData_var;
}

void WaveShortMessage::parsimPack(cCommBuffer *b)
{
    cPacket::parsimPack(b);
    doPacking(b,this->wsmVersion_var);
    doPacking(b,this->securityType_var);
    doPacking(b,this->channelNumber_var);
    doPacking(b,this->dataRate_var);
    doPacking(b,this->priority_var);
    doPacking(b,this->psid_var);
    doPacking(b,this->psc_var);
    doPacking(b,this->wsmLength_var);
    doPacking(b,this->wsmData_var);
}

void WaveShortMessage::parsimUnpack(cCommBuffer *b)
{
    cPacket::parsimUnpack(b);
    doUnpacking(b,this->wsmVersion_var);
    doUnpacking(b,this->securityType_var);
    doUnpacking(b,this->channelNumber_var);
    doUnpacking(b,this->dataRate_var);
    doUnpacking(b,this->priority_var);
    doUnpacking(b,this->psid_var);
    doUnpacking(b,this->psc_var);
    doUnpacking(b,this->wsmLength_var);
    doUnpacking(b,this->wsmData_var);
}

int WaveShortMessage::getWsmVersion() const
{
    return wsmVersion_var;
}

void WaveShortMessage::setWsmVersion(int wsmVersion)
{
    this->wsmVersion_var = wsmVersion;
}

int WaveShortMessage::getSecurityType() const
{
    return securityType_var;
}

void WaveShortMessage::setSecurityType(int securityType)
{
    this->securityType_var = securityType;
}

int WaveShortMessage::getChannelNumber() const
{
    return channelNumber_var;
}

void WaveShortMessage::setChannelNumber(int channelNumber)
{
    this->channelNumber_var = channelNumber;
}

int WaveShortMessage::getDataRate() const
{
    return dataRate_var;
}

void WaveShortMessage::setDataRate(int dataRate)
{
    this->dataRate_var = dataRate;
}

int WaveShortMessage::getPriority() const
{
    return priority_var;
}

void WaveShortMessage::setPriority(int priority)
{
    this->priority_var = priority;
}

int WaveShortMessage::getPsid() const
{
    return psid_var;
}

void WaveShortMessage::setPsid(int psid)
{
    this->psid_var = psid;
}

const char * WaveShortMessage::getPsc() const
{
    return psc_var.c_str();
}

void WaveShortMessage::setPsc(const char * psc)
{
    this->psc_var = psc;
}

int WaveShortMessage::getWsmLength() const
{
    return wsmLength_var;
}

void WaveShortMessage::setWsmLength(int wsmLength)
{
    this->wsmLength_var = wsmLength;
}

const char * WaveShortMessage::getWsmData() const
{
    return wsmData_var.c_str();
}

void WaveShortMessage::setWsmData(const char * wsmData)
{
    this->wsmData_var = wsmData;
}

class WaveShortMessageDescriptor : public cClassDescriptor
{
  public:
    WaveShortMessageDescriptor();
    virtual ~WaveShortMessageDescriptor();

    virtual bool doesSupport(cObject *obj) const;
    virtual const char *getProperty(const char *propertyname) const;
    virtual int getFieldCount(void *object) const;
    virtual const char *getFieldName(void *object, int field) const;
    virtual int findField(void *object, const char *fieldName) const;
    virtual unsigned int getFieldTypeFlags(void *object, int field) const;
    virtual const char *getFieldTypeString(void *object, int field) const;
    virtual const char *getFieldProperty(void *object, int field, const char *propertyname) const;
    virtual int getArraySize(void *object, int field) const;

    virtual std::string getFieldAsString(void *object, int field, int i) const;
    virtual bool setFieldAsString(void *object, int field, int i, const char *value) const;

    virtual const char *getFieldStructName(void *object, int field) const;
    virtual void *getFieldStructPointer(void *object, int field, int i) const;
};

Register_ClassDescriptor(WaveShortMessageDescriptor);

WaveShortMessageDescriptor::WaveShortMessageDescriptor() : cClassDescriptor("WaveShortMessage", "cPacket")
{
}

WaveShortMessageDescriptor::~WaveShortMessageDescriptor()
{
}

bool WaveShortMessageDescriptor::doesSupport(cObject *obj) const
{
    return dynamic_cast<WaveShortMessage *>(obj)!=NULL;
}

const char *WaveShortMessageDescriptor::getProperty(const char *propertyname) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : NULL;
}

int WaveShortMessageDescriptor::getFieldCount(void *object) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 9+basedesc->getFieldCount(object) : 9;
}

unsigned int WaveShortMessageDescriptor::getFieldTypeFlags(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldTypeFlags(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
    };
    return (field>=0 && field<9) ? fieldTypeFlags[field] : 0;
}

const char *WaveShortMessageDescriptor::getFieldName(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldName(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static const char *fieldNames[] = {
        "wsmVersion",
        "securityType",
        "channelNumber",
        "dataRate",
        "priority",
        "psid",
        "psc",
        "wsmLength",
        "wsmData",
    };
    return (field>=0 && field<9) ? fieldNames[field] : NULL;
}

int WaveShortMessageDescriptor::findField(void *object, const char *fieldName) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    int base = basedesc ? basedesc->getFieldCount(object) : 0;
    if (fieldName[0]=='w' && strcmp(fieldName, "wsmVersion")==0) return base+0;
    if (fieldName[0]=='s' && strcmp(fieldName, "securityType")==0) return base+1;
    if (fieldName[0]=='c' && strcmp(fieldName, "channelNumber")==0) return base+2;
    if (fieldName[0]=='d' && strcmp(fieldName, "dataRate")==0) return base+3;
    if (fieldName[0]=='p' && strcmp(fieldName, "priority")==0) return base+4;
    if (fieldName[0]=='p' && strcmp(fieldName, "psid")==0) return base+5;
    if (fieldName[0]=='p' && strcmp(fieldName, "psc")==0) return base+6;
    if (fieldName[0]=='w' && strcmp(fieldName, "wsmLength")==0) return base+7;
    if (fieldName[0]=='w' && strcmp(fieldName, "wsmData")==0) return base+8;
    return basedesc ? basedesc->findField(object, fieldName) : -1;
}

const char *WaveShortMessageDescriptor::getFieldTypeString(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldTypeString(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static const char *fieldTypeStrings[] = {
        "int",
        "int",
        "int",
        "int",
        "int",
        "int",
        "string",
        "int",
        "string",
    };
    return (field>=0 && field<9) ? fieldTypeStrings[field] : NULL;
}

const char *WaveShortMessageDescriptor::getFieldProperty(void *object, int field, const char *propertyname) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldProperty(object, field, propertyname);
        field -= basedesc->getFieldCount(object);
    }
    switch (field) {
        default: return NULL;
    }
}

int WaveShortMessageDescriptor::getArraySize(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getArraySize(object, field);
        field -= basedesc->getFieldCount(object);
    }
    WaveShortMessage *pp = (WaveShortMessage *)object; (void)pp;
    switch (field) {
        default: return 0;
    }
}

std::string WaveShortMessageDescriptor::getFieldAsString(void *object, int field, int i) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldAsString(object,field,i);
        field -= basedesc->getFieldCount(object);
    }
    WaveShortMessage *pp = (WaveShortMessage *)object; (void)pp;
    switch (field) {
        case 0: return long2string(pp->getWsmVersion());
        case 1: return long2string(pp->getSecurityType());
        case 2: return long2string(pp->getChannelNumber());
        case 3: return long2string(pp->getDataRate());
        case 4: return long2string(pp->getPriority());
        case 5: return long2string(pp->getPsid());
        case 6: return oppstring2string(pp->getPsc());
        case 7: return long2string(pp->getWsmLength());
        case 8: return oppstring2string(pp->getWsmData());
        default: return "";
    }
}

bool WaveShortMessageDescriptor::setFieldAsString(void *object, int field, int i, const char *value) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->setFieldAsString(object,field,i,value);
        field -= basedesc->getFieldCount(object);
    }
    WaveShortMessage *pp = (WaveShortMessage *)object; (void)pp;
    switch (field) {
        case 0: pp->setWsmVersion(string2long(value)); return true;
        case 1: pp->setSecurityType(string2long(value)); return true;
        case 2: pp->setChannelNumber(string2long(value)); return true;
        case 3: pp->setDataRate(string2long(value)); return true;
        case 4: pp->setPriority(string2long(value)); return true;
        case 5: pp->setPsid(string2long(value)); return true;
        case 6: pp->setPsc((value)); return true;
        case 7: pp->setWsmLength(string2long(value)); return true;
        case 8: pp->setWsmData((value)); return true;
        default: return false;
    }
}

const char *WaveShortMessageDescriptor::getFieldStructName(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldStructName(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static const char *fieldStructNames[] = {
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
    };
    return (field>=0 && field<9) ? fieldStructNames[field] : NULL;
}

void *WaveShortMessageDescriptor::getFieldStructPointer(void *object, int field, int i) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldStructPointer(object, field, i);
        field -= basedesc->getFieldCount(object);
    }
    WaveShortMessage *pp = (WaveShortMessage *)object; (void)pp;
    switch (field) {
        default: return NULL;
    }
}

Register_Class(BeaconVehicle);

BeaconVehicle::BeaconVehicle(const char *name, int kind) : WaveShortMessage(name,kind)
{
    this->sender_var = 0;
    this->recipient_var = 0;
    this->speed_var = 0;
    this->accel_var = 0;
    this->maxDecel_var = 0;
    this->lane_var = 0;
    this->platoonID_var = 0;
    this->platoonDepth_var = 0;
}

BeaconVehicle::BeaconVehicle(const BeaconVehicle& other) : WaveShortMessage(other)
{
    copy(other);
}

BeaconVehicle::~BeaconVehicle()
{
}

BeaconVehicle& BeaconVehicle::operator=(const BeaconVehicle& other)
{
    if (this==&other) return *this;
    WaveShortMessage::operator=(other);
    copy(other);
    return *this;
}

void BeaconVehicle::copy(const BeaconVehicle& other)
{
    this->sender_var = other.sender_var;
    this->recipient_var = other.recipient_var;
    this->pos_var = other.pos_var;
    this->speed_var = other.speed_var;
    this->accel_var = other.accel_var;
    this->maxDecel_var = other.maxDecel_var;
    this->lane_var = other.lane_var;
    this->platoonID_var = other.platoonID_var;
    this->platoonDepth_var = other.platoonDepth_var;
}

void BeaconVehicle::parsimPack(cCommBuffer *b)
{
    WaveShortMessage::parsimPack(b);
    doPacking(b,this->sender_var);
    doPacking(b,this->recipient_var);
    doPacking(b,this->pos_var);
    doPacking(b,this->speed_var);
    doPacking(b,this->accel_var);
    doPacking(b,this->maxDecel_var);
    doPacking(b,this->lane_var);
    doPacking(b,this->platoonID_var);
    doPacking(b,this->platoonDepth_var);
}

void BeaconVehicle::parsimUnpack(cCommBuffer *b)
{
    WaveShortMessage::parsimUnpack(b);
    doUnpacking(b,this->sender_var);
    doUnpacking(b,this->recipient_var);
    doUnpacking(b,this->pos_var);
    doUnpacking(b,this->speed_var);
    doUnpacking(b,this->accel_var);
    doUnpacking(b,this->maxDecel_var);
    doUnpacking(b,this->lane_var);
    doUnpacking(b,this->platoonID_var);
    doUnpacking(b,this->platoonDepth_var);
}

const char * BeaconVehicle::getSender() const
{
    return sender_var.c_str();
}

void BeaconVehicle::setSender(const char * sender)
{
    this->sender_var = sender;
}

const char * BeaconVehicle::getRecipient() const
{
    return recipient_var.c_str();
}

void BeaconVehicle::setRecipient(const char * recipient)
{
    this->recipient_var = recipient;
}

Coord& BeaconVehicle::getPos()
{
    return pos_var;
}

void BeaconVehicle::setPos(const Coord& pos)
{
    this->pos_var = pos;
}

double BeaconVehicle::getSpeed() const
{
    return speed_var;
}

void BeaconVehicle::setSpeed(double speed)
{
    this->speed_var = speed;
}

double BeaconVehicle::getAccel() const
{
    return accel_var;
}

void BeaconVehicle::setAccel(double accel)
{
    this->accel_var = accel;
}

double BeaconVehicle::getMaxDecel() const
{
    return maxDecel_var;
}

void BeaconVehicle::setMaxDecel(double maxDecel)
{
    this->maxDecel_var = maxDecel;
}

const char * BeaconVehicle::getLane() const
{
    return lane_var.c_str();
}

void BeaconVehicle::setLane(const char * lane)
{
    this->lane_var = lane;
}

const char * BeaconVehicle::getPlatoonID() const
{
    return platoonID_var.c_str();
}

void BeaconVehicle::setPlatoonID(const char * platoonID)
{
    this->platoonID_var = platoonID;
}

int BeaconVehicle::getPlatoonDepth() const
{
    return platoonDepth_var;
}

void BeaconVehicle::setPlatoonDepth(int platoonDepth)
{
    this->platoonDepth_var = platoonDepth;
}

class BeaconVehicleDescriptor : public cClassDescriptor
{
  public:
    BeaconVehicleDescriptor();
    virtual ~BeaconVehicleDescriptor();

    virtual bool doesSupport(cObject *obj) const;
    virtual const char *getProperty(const char *propertyname) const;
    virtual int getFieldCount(void *object) const;
    virtual const char *getFieldName(void *object, int field) const;
    virtual int findField(void *object, const char *fieldName) const;
    virtual unsigned int getFieldTypeFlags(void *object, int field) const;
    virtual const char *getFieldTypeString(void *object, int field) const;
    virtual const char *getFieldProperty(void *object, int field, const char *propertyname) const;
    virtual int getArraySize(void *object, int field) const;

    virtual std::string getFieldAsString(void *object, int field, int i) const;
    virtual bool setFieldAsString(void *object, int field, int i, const char *value) const;

    virtual const char *getFieldStructName(void *object, int field) const;
    virtual void *getFieldStructPointer(void *object, int field, int i) const;
};

Register_ClassDescriptor(BeaconVehicleDescriptor);

BeaconVehicleDescriptor::BeaconVehicleDescriptor() : cClassDescriptor("BeaconVehicle", "WaveShortMessage")
{
}

BeaconVehicleDescriptor::~BeaconVehicleDescriptor()
{
}

bool BeaconVehicleDescriptor::doesSupport(cObject *obj) const
{
    return dynamic_cast<BeaconVehicle *>(obj)!=NULL;
}

const char *BeaconVehicleDescriptor::getProperty(const char *propertyname) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : NULL;
}

int BeaconVehicleDescriptor::getFieldCount(void *object) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 9+basedesc->getFieldCount(object) : 9;
}

unsigned int BeaconVehicleDescriptor::getFieldTypeFlags(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldTypeFlags(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISCOMPOUND,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
    };
    return (field>=0 && field<9) ? fieldTypeFlags[field] : 0;
}

const char *BeaconVehicleDescriptor::getFieldName(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldName(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static const char *fieldNames[] = {
        "sender",
        "recipient",
        "pos",
        "speed",
        "accel",
        "maxDecel",
        "lane",
        "platoonID",
        "platoonDepth",
    };
    return (field>=0 && field<9) ? fieldNames[field] : NULL;
}

int BeaconVehicleDescriptor::findField(void *object, const char *fieldName) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    int base = basedesc ? basedesc->getFieldCount(object) : 0;
    if (fieldName[0]=='s' && strcmp(fieldName, "sender")==0) return base+0;
    if (fieldName[0]=='r' && strcmp(fieldName, "recipient")==0) return base+1;
    if (fieldName[0]=='p' && strcmp(fieldName, "pos")==0) return base+2;
    if (fieldName[0]=='s' && strcmp(fieldName, "speed")==0) return base+3;
    if (fieldName[0]=='a' && strcmp(fieldName, "accel")==0) return base+4;
    if (fieldName[0]=='m' && strcmp(fieldName, "maxDecel")==0) return base+5;
    if (fieldName[0]=='l' && strcmp(fieldName, "lane")==0) return base+6;
    if (fieldName[0]=='p' && strcmp(fieldName, "platoonID")==0) return base+7;
    if (fieldName[0]=='p' && strcmp(fieldName, "platoonDepth")==0) return base+8;
    return basedesc ? basedesc->findField(object, fieldName) : -1;
}

const char *BeaconVehicleDescriptor::getFieldTypeString(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldTypeString(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static const char *fieldTypeStrings[] = {
        "string",
        "string",
        "Coord",
        "double",
        "double",
        "double",
        "string",
        "string",
        "int",
    };
    return (field>=0 && field<9) ? fieldTypeStrings[field] : NULL;
}

const char *BeaconVehicleDescriptor::getFieldProperty(void *object, int field, const char *propertyname) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldProperty(object, field, propertyname);
        field -= basedesc->getFieldCount(object);
    }
    switch (field) {
        default: return NULL;
    }
}

int BeaconVehicleDescriptor::getArraySize(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getArraySize(object, field);
        field -= basedesc->getFieldCount(object);
    }
    BeaconVehicle *pp = (BeaconVehicle *)object; (void)pp;
    switch (field) {
        default: return 0;
    }
}

std::string BeaconVehicleDescriptor::getFieldAsString(void *object, int field, int i) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldAsString(object,field,i);
        field -= basedesc->getFieldCount(object);
    }
    BeaconVehicle *pp = (BeaconVehicle *)object; (void)pp;
    switch (field) {
        case 0: return oppstring2string(pp->getSender());
        case 1: return oppstring2string(pp->getRecipient());
        case 2: {std::stringstream out; out << pp->getPos(); return out.str();}
        case 3: return double2string(pp->getSpeed());
        case 4: return double2string(pp->getAccel());
        case 5: return double2string(pp->getMaxDecel());
        case 6: return oppstring2string(pp->getLane());
        case 7: return oppstring2string(pp->getPlatoonID());
        case 8: return long2string(pp->getPlatoonDepth());
        default: return "";
    }
}

bool BeaconVehicleDescriptor::setFieldAsString(void *object, int field, int i, const char *value) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->setFieldAsString(object,field,i,value);
        field -= basedesc->getFieldCount(object);
    }
    BeaconVehicle *pp = (BeaconVehicle *)object; (void)pp;
    switch (field) {
        case 0: pp->setSender((value)); return true;
        case 1: pp->setRecipient((value)); return true;
        case 3: pp->setSpeed(string2double(value)); return true;
        case 4: pp->setAccel(string2double(value)); return true;
        case 5: pp->setMaxDecel(string2double(value)); return true;
        case 6: pp->setLane((value)); return true;
        case 7: pp->setPlatoonID((value)); return true;
        case 8: pp->setPlatoonDepth(string2long(value)); return true;
        default: return false;
    }
}

const char *BeaconVehicleDescriptor::getFieldStructName(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldStructName(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static const char *fieldStructNames[] = {
        NULL,
        NULL,
        "Coord",
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
    };
    return (field>=0 && field<9) ? fieldStructNames[field] : NULL;
}

void *BeaconVehicleDescriptor::getFieldStructPointer(void *object, int field, int i) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldStructPointer(object, field, i);
        field -= basedesc->getFieldCount(object);
    }
    BeaconVehicle *pp = (BeaconVehicle *)object; (void)pp;
    switch (field) {
        case 2: return (void *)(&pp->getPos()); break;
        default: return NULL;
    }
}

Register_Class(BeaconRSU);

BeaconRSU::BeaconRSU(const char *name, int kind) : WaveShortMessage(name,kind)
{
    this->sender_var = 0;
    this->recipient_var = 0;
}

BeaconRSU::BeaconRSU(const BeaconRSU& other) : WaveShortMessage(other)
{
    copy(other);
}

BeaconRSU::~BeaconRSU()
{
}

BeaconRSU& BeaconRSU::operator=(const BeaconRSU& other)
{
    if (this==&other) return *this;
    WaveShortMessage::operator=(other);
    copy(other);
    return *this;
}

void BeaconRSU::copy(const BeaconRSU& other)
{
    this->sender_var = other.sender_var;
    this->recipient_var = other.recipient_var;
    this->pos_var = other.pos_var;
}

void BeaconRSU::parsimPack(cCommBuffer *b)
{
    WaveShortMessage::parsimPack(b);
    doPacking(b,this->sender_var);
    doPacking(b,this->recipient_var);
    doPacking(b,this->pos_var);
}

void BeaconRSU::parsimUnpack(cCommBuffer *b)
{
    WaveShortMessage::parsimUnpack(b);
    doUnpacking(b,this->sender_var);
    doUnpacking(b,this->recipient_var);
    doUnpacking(b,this->pos_var);
}

const char * BeaconRSU::getSender() const
{
    return sender_var.c_str();
}

void BeaconRSU::setSender(const char * sender)
{
    this->sender_var = sender;
}

const char * BeaconRSU::getRecipient() const
{
    return recipient_var.c_str();
}

void BeaconRSU::setRecipient(const char * recipient)
{
    this->recipient_var = recipient;
}

Coord& BeaconRSU::getPos()
{
    return pos_var;
}

void BeaconRSU::setPos(const Coord& pos)
{
    this->pos_var = pos;
}

class BeaconRSUDescriptor : public cClassDescriptor
{
  public:
    BeaconRSUDescriptor();
    virtual ~BeaconRSUDescriptor();

    virtual bool doesSupport(cObject *obj) const;
    virtual const char *getProperty(const char *propertyname) const;
    virtual int getFieldCount(void *object) const;
    virtual const char *getFieldName(void *object, int field) const;
    virtual int findField(void *object, const char *fieldName) const;
    virtual unsigned int getFieldTypeFlags(void *object, int field) const;
    virtual const char *getFieldTypeString(void *object, int field) const;
    virtual const char *getFieldProperty(void *object, int field, const char *propertyname) const;
    virtual int getArraySize(void *object, int field) const;

    virtual std::string getFieldAsString(void *object, int field, int i) const;
    virtual bool setFieldAsString(void *object, int field, int i, const char *value) const;

    virtual const char *getFieldStructName(void *object, int field) const;
    virtual void *getFieldStructPointer(void *object, int field, int i) const;
};

Register_ClassDescriptor(BeaconRSUDescriptor);

BeaconRSUDescriptor::BeaconRSUDescriptor() : cClassDescriptor("BeaconRSU", "WaveShortMessage")
{
}

BeaconRSUDescriptor::~BeaconRSUDescriptor()
{
}

bool BeaconRSUDescriptor::doesSupport(cObject *obj) const
{
    return dynamic_cast<BeaconRSU *>(obj)!=NULL;
}

const char *BeaconRSUDescriptor::getProperty(const char *propertyname) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : NULL;
}

int BeaconRSUDescriptor::getFieldCount(void *object) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 3+basedesc->getFieldCount(object) : 3;
}

unsigned int BeaconRSUDescriptor::getFieldTypeFlags(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldTypeFlags(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISCOMPOUND,
    };
    return (field>=0 && field<3) ? fieldTypeFlags[field] : 0;
}

const char *BeaconRSUDescriptor::getFieldName(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldName(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static const char *fieldNames[] = {
        "sender",
        "recipient",
        "pos",
    };
    return (field>=0 && field<3) ? fieldNames[field] : NULL;
}

int BeaconRSUDescriptor::findField(void *object, const char *fieldName) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    int base = basedesc ? basedesc->getFieldCount(object) : 0;
    if (fieldName[0]=='s' && strcmp(fieldName, "sender")==0) return base+0;
    if (fieldName[0]=='r' && strcmp(fieldName, "recipient")==0) return base+1;
    if (fieldName[0]=='p' && strcmp(fieldName, "pos")==0) return base+2;
    return basedesc ? basedesc->findField(object, fieldName) : -1;
}

const char *BeaconRSUDescriptor::getFieldTypeString(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldTypeString(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static const char *fieldTypeStrings[] = {
        "string",
        "string",
        "Coord",
    };
    return (field>=0 && field<3) ? fieldTypeStrings[field] : NULL;
}

const char *BeaconRSUDescriptor::getFieldProperty(void *object, int field, const char *propertyname) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldProperty(object, field, propertyname);
        field -= basedesc->getFieldCount(object);
    }
    switch (field) {
        default: return NULL;
    }
}

int BeaconRSUDescriptor::getArraySize(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getArraySize(object, field);
        field -= basedesc->getFieldCount(object);
    }
    BeaconRSU *pp = (BeaconRSU *)object; (void)pp;
    switch (field) {
        default: return 0;
    }
}

std::string BeaconRSUDescriptor::getFieldAsString(void *object, int field, int i) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldAsString(object,field,i);
        field -= basedesc->getFieldCount(object);
    }
    BeaconRSU *pp = (BeaconRSU *)object; (void)pp;
    switch (field) {
        case 0: return oppstring2string(pp->getSender());
        case 1: return oppstring2string(pp->getRecipient());
        case 2: {std::stringstream out; out << pp->getPos(); return out.str();}
        default: return "";
    }
}

bool BeaconRSUDescriptor::setFieldAsString(void *object, int field, int i, const char *value) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->setFieldAsString(object,field,i,value);
        field -= basedesc->getFieldCount(object);
    }
    BeaconRSU *pp = (BeaconRSU *)object; (void)pp;
    switch (field) {
        case 0: pp->setSender((value)); return true;
        case 1: pp->setRecipient((value)); return true;
        default: return false;
    }
}

const char *BeaconRSUDescriptor::getFieldStructName(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldStructName(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static const char *fieldStructNames[] = {
        NULL,
        NULL,
        "Coord",
    };
    return (field>=0 && field<3) ? fieldStructNames[field] : NULL;
}

void *BeaconRSUDescriptor::getFieldStructPointer(void *object, int field, int i) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldStructPointer(object, field, i);
        field -= basedesc->getFieldCount(object);
    }
    BeaconRSU *pp = (BeaconRSU *)object; (void)pp;
    switch (field) {
        case 2: return (void *)(&pp->getPos()); break;
        default: return NULL;
    }
}

Register_Class(LaneChangeCountMsg);

LaneChangeCountMsg::LaneChangeCountMsg(const char *name, int kind) : WaveShortMessage(name,kind)
{
    this->sender_var = 0;
    this->recipient_var = 0;
}

LaneChangeCountMsg::LaneChangeCountMsg(const LaneChangeCountMsg& other) : WaveShortMessage(other)
{
    copy(other);
}

LaneChangeCountMsg::~LaneChangeCountMsg()
{
}

LaneChangeCountMsg& LaneChangeCountMsg::operator=(const LaneChangeCountMsg& other)
{
    if (this==&other) return *this;
    WaveShortMessage::operator=(other);
    copy(other);
    return *this;
}

void LaneChangeCountMsg::copy(const LaneChangeCountMsg& other)
{
    this->sender_var = other.sender_var;
    this->recipient_var = other.recipient_var;
    this->queueValue_var = other.queueValue_var;
}

void LaneChangeCountMsg::parsimPack(cCommBuffer *b)
{
    WaveShortMessage::parsimPack(b);
    doPacking(b,this->sender_var);
    doPacking(b,this->recipient_var);
    doPacking(b,this->queueValue_var);
}

void LaneChangeCountMsg::parsimUnpack(cCommBuffer *b)
{
    WaveShortMessage::parsimUnpack(b);
    doUnpacking(b,this->sender_var);
    doUnpacking(b,this->recipient_var);
    doUnpacking(b,this->queueValue_var);
}

const char * LaneChangeCountMsg::getSender() const
{
    return sender_var.c_str();
}

void LaneChangeCountMsg::setSender(const char * sender)
{
    this->sender_var = sender;
}

const char * LaneChangeCountMsg::getRecipient() const
{
    return recipient_var.c_str();
}

void LaneChangeCountMsg::setRecipient(const char * recipient)
{
    this->recipient_var = recipient;
}

stringQueue& LaneChangeCountMsg::getQueueValue()
{
    return queueValue_var;
}

void LaneChangeCountMsg::setQueueValue(const stringQueue& queueValue)
{
    this->queueValue_var = queueValue;
}

class LaneChangeCountMsgDescriptor : public cClassDescriptor
{
  public:
    LaneChangeCountMsgDescriptor();
    virtual ~LaneChangeCountMsgDescriptor();

    virtual bool doesSupport(cObject *obj) const;
    virtual const char *getProperty(const char *propertyname) const;
    virtual int getFieldCount(void *object) const;
    virtual const char *getFieldName(void *object, int field) const;
    virtual int findField(void *object, const char *fieldName) const;
    virtual unsigned int getFieldTypeFlags(void *object, int field) const;
    virtual const char *getFieldTypeString(void *object, int field) const;
    virtual const char *getFieldProperty(void *object, int field, const char *propertyname) const;
    virtual int getArraySize(void *object, int field) const;

    virtual std::string getFieldAsString(void *object, int field, int i) const;
    virtual bool setFieldAsString(void *object, int field, int i, const char *value) const;

    virtual const char *getFieldStructName(void *object, int field) const;
    virtual void *getFieldStructPointer(void *object, int field, int i) const;
};

Register_ClassDescriptor(LaneChangeCountMsgDescriptor);

LaneChangeCountMsgDescriptor::LaneChangeCountMsgDescriptor() : cClassDescriptor("LaneChangeCountMsg", "WaveShortMessage")
{
}

LaneChangeCountMsgDescriptor::~LaneChangeCountMsgDescriptor()
{
}

bool LaneChangeCountMsgDescriptor::doesSupport(cObject *obj) const
{
    return dynamic_cast<LaneChangeCountMsg *>(obj)!=NULL;
}

const char *LaneChangeCountMsgDescriptor::getProperty(const char *propertyname) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : NULL;
}

int LaneChangeCountMsgDescriptor::getFieldCount(void *object) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 3+basedesc->getFieldCount(object) : 3;
}

unsigned int LaneChangeCountMsgDescriptor::getFieldTypeFlags(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldTypeFlags(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISCOMPOUND,
    };
    return (field>=0 && field<3) ? fieldTypeFlags[field] : 0;
}

const char *LaneChangeCountMsgDescriptor::getFieldName(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldName(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static const char *fieldNames[] = {
        "sender",
        "recipient",
        "queueValue",
    };
    return (field>=0 && field<3) ? fieldNames[field] : NULL;
}

int LaneChangeCountMsgDescriptor::findField(void *object, const char *fieldName) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    int base = basedesc ? basedesc->getFieldCount(object) : 0;
    if (fieldName[0]=='s' && strcmp(fieldName, "sender")==0) return base+0;
    if (fieldName[0]=='r' && strcmp(fieldName, "recipient")==0) return base+1;
    if (fieldName[0]=='q' && strcmp(fieldName, "queueValue")==0) return base+2;
    return basedesc ? basedesc->findField(object, fieldName) : -1;
}

const char *LaneChangeCountMsgDescriptor::getFieldTypeString(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldTypeString(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static const char *fieldTypeStrings[] = {
        "string",
        "string",
        "stringQueue",
    };
    return (field>=0 && field<3) ? fieldTypeStrings[field] : NULL;
}

const char *LaneChangeCountMsgDescriptor::getFieldProperty(void *object, int field, const char *propertyname) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldProperty(object, field, propertyname);
        field -= basedesc->getFieldCount(object);
    }
    switch (field) {
        default: return NULL;
    }
}

int LaneChangeCountMsgDescriptor::getArraySize(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getArraySize(object, field);
        field -= basedesc->getFieldCount(object);
    }
    LaneChangeCountMsg *pp = (LaneChangeCountMsg *)object; (void)pp;
    switch (field) {
        default: return 0;
    }
}

std::string LaneChangeCountMsgDescriptor::getFieldAsString(void *object, int field, int i) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldAsString(object,field,i);
        field -= basedesc->getFieldCount(object);
    }
    LaneChangeCountMsg *pp = (LaneChangeCountMsg *)object; (void)pp;
    switch (field) {
        case 0: return oppstring2string(pp->getSender());
        case 1: return oppstring2string(pp->getRecipient());
        case 2: {std::stringstream out; out << pp->getQueueValue(); return out.str();}
        default: return "";
    }
}

bool LaneChangeCountMsgDescriptor::setFieldAsString(void *object, int field, int i, const char *value) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->setFieldAsString(object,field,i,value);
        field -= basedesc->getFieldCount(object);
    }
    LaneChangeCountMsg *pp = (LaneChangeCountMsg *)object; (void)pp;
    switch (field) {
        case 0: pp->setSender((value)); return true;
        case 1: pp->setRecipient((value)); return true;
        default: return false;
    }
}

const char *LaneChangeCountMsgDescriptor::getFieldStructName(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldStructName(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static const char *fieldStructNames[] = {
        NULL,
        NULL,
        "stringQueue",
    };
    return (field>=0 && field<3) ? fieldStructNames[field] : NULL;
}

void *LaneChangeCountMsgDescriptor::getFieldStructPointer(void *object, int field, int i) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldStructPointer(object, field, i);
        field -= basedesc->getFieldCount(object);
    }
    LaneChangeCountMsg *pp = (LaneChangeCountMsg *)object; (void)pp;
    switch (field) {
        case 2: return (void *)(&pp->getQueueValue()); break;
        default: return NULL;
    }
}

Register_Class(PlatoonMsg);

PlatoonMsg::PlatoonMsg(const char *name, int kind) : WaveShortMessage(name,kind)
{
    this->sender_var = 0;
    this->recipient_var = 0;
    this->req_res_type_var = 0;
    this->sendingPlatoonID_var = 0;
    this->receivingPlatoonID_var = 0;
    this->dblValue_var = 0;
    this->strValue_var = 0;
}

PlatoonMsg::PlatoonMsg(const PlatoonMsg& other) : WaveShortMessage(other)
{
    copy(other);
}

PlatoonMsg::~PlatoonMsg()
{
}

PlatoonMsg& PlatoonMsg::operator=(const PlatoonMsg& other)
{
    if (this==&other) return *this;
    WaveShortMessage::operator=(other);
    copy(other);
    return *this;
}

void PlatoonMsg::copy(const PlatoonMsg& other)
{
    this->sender_var = other.sender_var;
    this->recipient_var = other.recipient_var;
    this->req_res_type_var = other.req_res_type_var;
    this->sendingPlatoonID_var = other.sendingPlatoonID_var;
    this->receivingPlatoonID_var = other.receivingPlatoonID_var;
    this->dblValue_var = other.dblValue_var;
    this->strValue_var = other.strValue_var;
    this->queueValue_var = other.queueValue_var;
}

void PlatoonMsg::parsimPack(cCommBuffer *b)
{
    WaveShortMessage::parsimPack(b);
    doPacking(b,this->sender_var);
    doPacking(b,this->recipient_var);
    doPacking(b,this->req_res_type_var);
    doPacking(b,this->sendingPlatoonID_var);
    doPacking(b,this->receivingPlatoonID_var);
    doPacking(b,this->dblValue_var);
    doPacking(b,this->strValue_var);
    doPacking(b,this->queueValue_var);
}

void PlatoonMsg::parsimUnpack(cCommBuffer *b)
{
    WaveShortMessage::parsimUnpack(b);
    doUnpacking(b,this->sender_var);
    doUnpacking(b,this->recipient_var);
    doUnpacking(b,this->req_res_type_var);
    doUnpacking(b,this->sendingPlatoonID_var);
    doUnpacking(b,this->receivingPlatoonID_var);
    doUnpacking(b,this->dblValue_var);
    doUnpacking(b,this->strValue_var);
    doUnpacking(b,this->queueValue_var);
}

const char * PlatoonMsg::getSender() const
{
    return sender_var.c_str();
}

void PlatoonMsg::setSender(const char * sender)
{
    this->sender_var = sender;
}

const char * PlatoonMsg::getRecipient() const
{
    return recipient_var.c_str();
}

void PlatoonMsg::setRecipient(const char * recipient)
{
    this->recipient_var = recipient;
}

int PlatoonMsg::getReq_res_type() const
{
    return req_res_type_var;
}

void PlatoonMsg::setReq_res_type(int req_res_type)
{
    this->req_res_type_var = req_res_type;
}

const char * PlatoonMsg::getSendingPlatoonID() const
{
    return sendingPlatoonID_var.c_str();
}

void PlatoonMsg::setSendingPlatoonID(const char * sendingPlatoonID)
{
    this->sendingPlatoonID_var = sendingPlatoonID;
}

const char * PlatoonMsg::getReceivingPlatoonID() const
{
    return receivingPlatoonID_var.c_str();
}

void PlatoonMsg::setReceivingPlatoonID(const char * receivingPlatoonID)
{
    this->receivingPlatoonID_var = receivingPlatoonID;
}

double PlatoonMsg::getDblValue() const
{
    return dblValue_var;
}

void PlatoonMsg::setDblValue(double dblValue)
{
    this->dblValue_var = dblValue;
}

const char * PlatoonMsg::getStrValue() const
{
    return strValue_var.c_str();
}

void PlatoonMsg::setStrValue(const char * strValue)
{
    this->strValue_var = strValue;
}

stringQueue& PlatoonMsg::getQueueValue()
{
    return queueValue_var;
}

void PlatoonMsg::setQueueValue(const stringQueue& queueValue)
{
    this->queueValue_var = queueValue;
}

class PlatoonMsgDescriptor : public cClassDescriptor
{
  public:
    PlatoonMsgDescriptor();
    virtual ~PlatoonMsgDescriptor();

    virtual bool doesSupport(cObject *obj) const;
    virtual const char *getProperty(const char *propertyname) const;
    virtual int getFieldCount(void *object) const;
    virtual const char *getFieldName(void *object, int field) const;
    virtual int findField(void *object, const char *fieldName) const;
    virtual unsigned int getFieldTypeFlags(void *object, int field) const;
    virtual const char *getFieldTypeString(void *object, int field) const;
    virtual const char *getFieldProperty(void *object, int field, const char *propertyname) const;
    virtual int getArraySize(void *object, int field) const;

    virtual std::string getFieldAsString(void *object, int field, int i) const;
    virtual bool setFieldAsString(void *object, int field, int i, const char *value) const;

    virtual const char *getFieldStructName(void *object, int field) const;
    virtual void *getFieldStructPointer(void *object, int field, int i) const;
};

Register_ClassDescriptor(PlatoonMsgDescriptor);

PlatoonMsgDescriptor::PlatoonMsgDescriptor() : cClassDescriptor("PlatoonMsg", "WaveShortMessage")
{
}

PlatoonMsgDescriptor::~PlatoonMsgDescriptor()
{
}

bool PlatoonMsgDescriptor::doesSupport(cObject *obj) const
{
    return dynamic_cast<PlatoonMsg *>(obj)!=NULL;
}

const char *PlatoonMsgDescriptor::getProperty(const char *propertyname) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : NULL;
}

int PlatoonMsgDescriptor::getFieldCount(void *object) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 8+basedesc->getFieldCount(object) : 8;
}

unsigned int PlatoonMsgDescriptor::getFieldTypeFlags(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldTypeFlags(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISCOMPOUND,
    };
    return (field>=0 && field<8) ? fieldTypeFlags[field] : 0;
}

const char *PlatoonMsgDescriptor::getFieldName(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldName(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static const char *fieldNames[] = {
        "sender",
        "recipient",
        "req_res_type",
        "sendingPlatoonID",
        "receivingPlatoonID",
        "dblValue",
        "strValue",
        "queueValue",
    };
    return (field>=0 && field<8) ? fieldNames[field] : NULL;
}

int PlatoonMsgDescriptor::findField(void *object, const char *fieldName) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    int base = basedesc ? basedesc->getFieldCount(object) : 0;
    if (fieldName[0]=='s' && strcmp(fieldName, "sender")==0) return base+0;
    if (fieldName[0]=='r' && strcmp(fieldName, "recipient")==0) return base+1;
    if (fieldName[0]=='r' && strcmp(fieldName, "req_res_type")==0) return base+2;
    if (fieldName[0]=='s' && strcmp(fieldName, "sendingPlatoonID")==0) return base+3;
    if (fieldName[0]=='r' && strcmp(fieldName, "receivingPlatoonID")==0) return base+4;
    if (fieldName[0]=='d' && strcmp(fieldName, "dblValue")==0) return base+5;
    if (fieldName[0]=='s' && strcmp(fieldName, "strValue")==0) return base+6;
    if (fieldName[0]=='q' && strcmp(fieldName, "queueValue")==0) return base+7;
    return basedesc ? basedesc->findField(object, fieldName) : -1;
}

const char *PlatoonMsgDescriptor::getFieldTypeString(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldTypeString(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static const char *fieldTypeStrings[] = {
        "string",
        "string",
        "int",
        "string",
        "string",
        "double",
        "string",
        "stringQueue",
    };
    return (field>=0 && field<8) ? fieldTypeStrings[field] : NULL;
}

const char *PlatoonMsgDescriptor::getFieldProperty(void *object, int field, const char *propertyname) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldProperty(object, field, propertyname);
        field -= basedesc->getFieldCount(object);
    }
    switch (field) {
        default: return NULL;
    }
}

int PlatoonMsgDescriptor::getArraySize(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getArraySize(object, field);
        field -= basedesc->getFieldCount(object);
    }
    PlatoonMsg *pp = (PlatoonMsg *)object; (void)pp;
    switch (field) {
        default: return 0;
    }
}

std::string PlatoonMsgDescriptor::getFieldAsString(void *object, int field, int i) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldAsString(object,field,i);
        field -= basedesc->getFieldCount(object);
    }
    PlatoonMsg *pp = (PlatoonMsg *)object; (void)pp;
    switch (field) {
        case 0: return oppstring2string(pp->getSender());
        case 1: return oppstring2string(pp->getRecipient());
        case 2: return long2string(pp->getReq_res_type());
        case 3: return oppstring2string(pp->getSendingPlatoonID());
        case 4: return oppstring2string(pp->getReceivingPlatoonID());
        case 5: return double2string(pp->getDblValue());
        case 6: return oppstring2string(pp->getStrValue());
        case 7: {std::stringstream out; out << pp->getQueueValue(); return out.str();}
        default: return "";
    }
}

bool PlatoonMsgDescriptor::setFieldAsString(void *object, int field, int i, const char *value) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->setFieldAsString(object,field,i,value);
        field -= basedesc->getFieldCount(object);
    }
    PlatoonMsg *pp = (PlatoonMsg *)object; (void)pp;
    switch (field) {
        case 0: pp->setSender((value)); return true;
        case 1: pp->setRecipient((value)); return true;
        case 2: pp->setReq_res_type(string2long(value)); return true;
        case 3: pp->setSendingPlatoonID((value)); return true;
        case 4: pp->setReceivingPlatoonID((value)); return true;
        case 5: pp->setDblValue(string2double(value)); return true;
        case 6: pp->setStrValue((value)); return true;
        default: return false;
    }
}

const char *PlatoonMsgDescriptor::getFieldStructName(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldStructName(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static const char *fieldStructNames[] = {
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        "stringQueue",
    };
    return (field>=0 && field<8) ? fieldStructNames[field] : NULL;
}

void *PlatoonMsgDescriptor::getFieldStructPointer(void *object, int field, int i) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldStructPointer(object, field, i);
        field -= basedesc->getFieldCount(object);
    }
    PlatoonMsg *pp = (PlatoonMsg *)object; (void)pp;
    switch (field) {
        case 7: return (void *)(&pp->getQueueValue()); break;
        default: return NULL;
    }
}


