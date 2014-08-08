//
// Generated file, do not edit! Created by opp_msgc 4.5 from msg/Messages.msg.
//

// Disable warnings about unused variables, empty switch stmts, etc:
#ifdef _MSC_VER
#  pragma warning(disable:4101)
#  pragma warning(disable:4065)
#endif

#include <iostream>
#include <sstream>
#include "Messages_m.h"

USING_NAMESPACE


// Another default rule (prevents compiler from choosing base class' doPacking())
template<typename T>
void doPacking(cCommBuffer *, T& t) {
    throw cRuntimeError("Parsim error: no doPacking() function for type %s or its base class (check .msg and _m.cc/h files!)",opp_typename(typeid(t)));
}

template<typename T>
void doUnpacking(cCommBuffer *, T& t) {
    throw cRuntimeError("Parsim error: no doUnpacking() function for type %s or its base class (check .msg and _m.cc/h files!)",opp_typename(typeid(t)));
}



namespace VENTOS {

// Template rule for outputting std::vector<T> types
template<typename T, typename A>
inline std::ostream& operator<<(std::ostream& out, const std::vector<T,A>& vec)
{
    out.put('{');
    for(typename std::vector<T,A>::const_iterator it = vec.begin(); it != vec.end(); ++it)
    {
        if (it != vec.begin()) {
            out.put(','); out.put(' ');
        }
        out << *it;
    }
    out.put('}');
    
    char buf[32];
    sprintf(buf, " (size=%u)", (unsigned int)vec.size());
    out.write(buf, strlen(buf));
    return out;
}

// Template rule which fires if a struct or class doesn't have operator<<
template<typename T>
inline std::ostream& operator<<(std::ostream& out,const T&) {return out;}

Register_Class(BeaconVehicle);

BeaconVehicle::BeaconVehicle(const char *name, int kind) : ::WaveShortMessage(name,kind)
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

BeaconVehicle::BeaconVehicle(const BeaconVehicle& other) : ::WaveShortMessage(other)
{
    copy(other);
}

BeaconVehicle::~BeaconVehicle()
{
}

BeaconVehicle& BeaconVehicle::operator=(const BeaconVehicle& other)
{
    if (this==&other) return *this;
    ::WaveShortMessage::operator=(other);
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
    ::WaveShortMessage::parsimPack(b);
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
    ::WaveShortMessage::parsimUnpack(b);
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

BeaconVehicleDescriptor::BeaconVehicleDescriptor() : cClassDescriptor("VENTOS::BeaconVehicle", "WaveShortMessage")
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
    switch (field) {
        case 2: return opp_typename(typeid(Coord));
        default: return NULL;
    };
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

BeaconRSU::BeaconRSU(const char *name, int kind) : ::WaveShortMessage(name,kind)
{
    this->sender_var = 0;
    this->recipient_var = 0;
}

BeaconRSU::BeaconRSU(const BeaconRSU& other) : ::WaveShortMessage(other)
{
    copy(other);
}

BeaconRSU::~BeaconRSU()
{
}

BeaconRSU& BeaconRSU::operator=(const BeaconRSU& other)
{
    if (this==&other) return *this;
    ::WaveShortMessage::operator=(other);
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
    ::WaveShortMessage::parsimPack(b);
    doPacking(b,this->sender_var);
    doPacking(b,this->recipient_var);
    doPacking(b,this->pos_var);
}

void BeaconRSU::parsimUnpack(cCommBuffer *b)
{
    ::WaveShortMessage::parsimUnpack(b);
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

BeaconRSUDescriptor::BeaconRSUDescriptor() : cClassDescriptor("VENTOS::BeaconRSU", "WaveShortMessage")
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
    switch (field) {
        case 2: return opp_typename(typeid(Coord));
        default: return NULL;
    };
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

Register_Class(LaneChangeMsg);

LaneChangeMsg::LaneChangeMsg(const char *name, int kind) : ::WaveShortMessage(name,kind)
{
    this->sender_var = 0;
    this->recipient_var = 0;
}

LaneChangeMsg::LaneChangeMsg(const LaneChangeMsg& other) : ::WaveShortMessage(other)
{
    copy(other);
}

LaneChangeMsg::~LaneChangeMsg()
{
}

LaneChangeMsg& LaneChangeMsg::operator=(const LaneChangeMsg& other)
{
    if (this==&other) return *this;
    ::WaveShortMessage::operator=(other);
    copy(other);
    return *this;
}

void LaneChangeMsg::copy(const LaneChangeMsg& other)
{
    this->sender_var = other.sender_var;
    this->recipient_var = other.recipient_var;
    this->laneChange_var = other.laneChange_var;
}

void LaneChangeMsg::parsimPack(cCommBuffer *b)
{
    ::WaveShortMessage::parsimPack(b);
    doPacking(b,this->sender_var);
    doPacking(b,this->recipient_var);
    doPacking(b,this->laneChange_var);
}

void LaneChangeMsg::parsimUnpack(cCommBuffer *b)
{
    ::WaveShortMessage::parsimUnpack(b);
    doUnpacking(b,this->sender_var);
    doUnpacking(b,this->recipient_var);
    doUnpacking(b,this->laneChange_var);
}

const char * LaneChangeMsg::getSender() const
{
    return sender_var.c_str();
}

void LaneChangeMsg::setSender(const char * sender)
{
    this->sender_var = sender;
}

const char * LaneChangeMsg::getRecipient() const
{
    return recipient_var.c_str();
}

void LaneChangeMsg::setRecipient(const char * recipient)
{
    this->recipient_var = recipient;
}

stringQueue& LaneChangeMsg::getLaneChange()
{
    return laneChange_var;
}

void LaneChangeMsg::setLaneChange(const stringQueue& laneChange)
{
    this->laneChange_var = laneChange;
}

class LaneChangeMsgDescriptor : public cClassDescriptor
{
  public:
    LaneChangeMsgDescriptor();
    virtual ~LaneChangeMsgDescriptor();

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

Register_ClassDescriptor(LaneChangeMsgDescriptor);

LaneChangeMsgDescriptor::LaneChangeMsgDescriptor() : cClassDescriptor("VENTOS::LaneChangeMsg", "WaveShortMessage")
{
}

LaneChangeMsgDescriptor::~LaneChangeMsgDescriptor()
{
}

bool LaneChangeMsgDescriptor::doesSupport(cObject *obj) const
{
    return dynamic_cast<LaneChangeMsg *>(obj)!=NULL;
}

const char *LaneChangeMsgDescriptor::getProperty(const char *propertyname) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : NULL;
}

int LaneChangeMsgDescriptor::getFieldCount(void *object) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 3+basedesc->getFieldCount(object) : 3;
}

unsigned int LaneChangeMsgDescriptor::getFieldTypeFlags(void *object, int field) const
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

const char *LaneChangeMsgDescriptor::getFieldName(void *object, int field) const
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
        "laneChange",
    };
    return (field>=0 && field<3) ? fieldNames[field] : NULL;
}

int LaneChangeMsgDescriptor::findField(void *object, const char *fieldName) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    int base = basedesc ? basedesc->getFieldCount(object) : 0;
    if (fieldName[0]=='s' && strcmp(fieldName, "sender")==0) return base+0;
    if (fieldName[0]=='r' && strcmp(fieldName, "recipient")==0) return base+1;
    if (fieldName[0]=='l' && strcmp(fieldName, "laneChange")==0) return base+2;
    return basedesc ? basedesc->findField(object, fieldName) : -1;
}

const char *LaneChangeMsgDescriptor::getFieldTypeString(void *object, int field) const
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

const char *LaneChangeMsgDescriptor::getFieldProperty(void *object, int field, const char *propertyname) const
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

int LaneChangeMsgDescriptor::getArraySize(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getArraySize(object, field);
        field -= basedesc->getFieldCount(object);
    }
    LaneChangeMsg *pp = (LaneChangeMsg *)object; (void)pp;
    switch (field) {
        default: return 0;
    }
}

std::string LaneChangeMsgDescriptor::getFieldAsString(void *object, int field, int i) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldAsString(object,field,i);
        field -= basedesc->getFieldCount(object);
    }
    LaneChangeMsg *pp = (LaneChangeMsg *)object; (void)pp;
    switch (field) {
        case 0: return oppstring2string(pp->getSender());
        case 1: return oppstring2string(pp->getRecipient());
        case 2: {std::stringstream out; out << pp->getLaneChange(); return out.str();}
        default: return "";
    }
}

bool LaneChangeMsgDescriptor::setFieldAsString(void *object, int field, int i, const char *value) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->setFieldAsString(object,field,i,value);
        field -= basedesc->getFieldCount(object);
    }
    LaneChangeMsg *pp = (LaneChangeMsg *)object; (void)pp;
    switch (field) {
        case 0: pp->setSender((value)); return true;
        case 1: pp->setRecipient((value)); return true;
        default: return false;
    }
}

const char *LaneChangeMsgDescriptor::getFieldStructName(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldStructName(object, field);
        field -= basedesc->getFieldCount(object);
    }
    switch (field) {
        case 2: return opp_typename(typeid(stringQueue));
        default: return NULL;
    };
}

void *LaneChangeMsgDescriptor::getFieldStructPointer(void *object, int field, int i) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldStructPointer(object, field, i);
        field -= basedesc->getFieldCount(object);
    }
    LaneChangeMsg *pp = (LaneChangeMsg *)object; (void)pp;
    switch (field) {
        case 2: return (void *)(&pp->getLaneChange()); break;
        default: return NULL;
    }
}

Register_Class(PlatoonMsg);

PlatoonMsg::PlatoonMsg(const char *name, int kind) : ::WaveShortMessage(name,kind)
{
    this->sender_var = 0;
    this->recipient_var = 0;
    this->type_var = 0;
    this->sendingPlatoonID_var = 0;
    this->receivingPlatoonID_var = 0;
    this->dblValue_var = 0;
    this->strValue_var = 0;
}

PlatoonMsg::PlatoonMsg(const PlatoonMsg& other) : ::WaveShortMessage(other)
{
    copy(other);
}

PlatoonMsg::~PlatoonMsg()
{
}

PlatoonMsg& PlatoonMsg::operator=(const PlatoonMsg& other)
{
    if (this==&other) return *this;
    ::WaveShortMessage::operator=(other);
    copy(other);
    return *this;
}

void PlatoonMsg::copy(const PlatoonMsg& other)
{
    this->sender_var = other.sender_var;
    this->recipient_var = other.recipient_var;
    this->type_var = other.type_var;
    this->sendingPlatoonID_var = other.sendingPlatoonID_var;
    this->receivingPlatoonID_var = other.receivingPlatoonID_var;
    this->dblValue_var = other.dblValue_var;
    this->strValue_var = other.strValue_var;
    this->queueValue_var = other.queueValue_var;
}

void PlatoonMsg::parsimPack(cCommBuffer *b)
{
    ::WaveShortMessage::parsimPack(b);
    doPacking(b,this->sender_var);
    doPacking(b,this->recipient_var);
    doPacking(b,this->type_var);
    doPacking(b,this->sendingPlatoonID_var);
    doPacking(b,this->receivingPlatoonID_var);
    doPacking(b,this->dblValue_var);
    doPacking(b,this->strValue_var);
    doPacking(b,this->queueValue_var);
}

void PlatoonMsg::parsimUnpack(cCommBuffer *b)
{
    ::WaveShortMessage::parsimUnpack(b);
    doUnpacking(b,this->sender_var);
    doUnpacking(b,this->recipient_var);
    doUnpacking(b,this->type_var);
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

int PlatoonMsg::getType() const
{
    return type_var;
}

void PlatoonMsg::setType(int type)
{
    this->type_var = type;
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

PlatoonMsgDescriptor::PlatoonMsgDescriptor() : cClassDescriptor("VENTOS::PlatoonMsg", "WaveShortMessage")
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
        "type",
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
    if (fieldName[0]=='t' && strcmp(fieldName, "type")==0) return base+2;
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
        case 2: return long2string(pp->getType());
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
        case 2: pp->setType(string2long(value)); return true;
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
    switch (field) {
        case 7: return opp_typename(typeid(stringQueue));
        default: return NULL;
    };
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

Register_Class(SystemMsg);

SystemMsg::SystemMsg(const char *name, int kind) : ::WaveShortMessage(name,kind)
{
    this->sender_var = 0;
    this->recipient_var = 0;
    this->requestType_var = 0;
    this->edge_var = 0;
    this->target_var = 0;
}

SystemMsg::SystemMsg(const SystemMsg& other) : ::WaveShortMessage(other)
{
    copy(other);
}

SystemMsg::~SystemMsg()
{
}

SystemMsg& SystemMsg::operator=(const SystemMsg& other)
{
    if (this==&other) return *this;
    ::WaveShortMessage::operator=(other);
    copy(other);
    return *this;
}

void SystemMsg::copy(const SystemMsg& other)
{
    this->sender_var = other.sender_var;
    this->recipient_var = other.recipient_var;
    this->requestType_var = other.requestType_var;
    this->edge_var = other.edge_var;
    this->target_var = other.target_var;
}

void SystemMsg::parsimPack(cCommBuffer *b)
{
    ::WaveShortMessage::parsimPack(b);
    doPacking(b,this->sender_var);
    doPacking(b,this->recipient_var);
    doPacking(b,this->requestType_var);
    doPacking(b,this->edge_var);
    doPacking(b,this->target_var);
}

void SystemMsg::parsimUnpack(cCommBuffer *b)
{
    ::WaveShortMessage::parsimUnpack(b);
    doUnpacking(b,this->sender_var);
    doUnpacking(b,this->recipient_var);
    doUnpacking(b,this->requestType_var);
    doUnpacking(b,this->edge_var);
    doUnpacking(b,this->target_var);
}

const char * SystemMsg::getSender() const
{
    return sender_var.c_str();
}

void SystemMsg::setSender(const char * sender)
{
    this->sender_var = sender;
}

const char * SystemMsg::getRecipient() const
{
    return recipient_var.c_str();
}

void SystemMsg::setRecipient(const char * recipient)
{
    this->recipient_var = recipient;
}

int SystemMsg::getRequestType() const
{
    return requestType_var;
}

void SystemMsg::setRequestType(int requestType)
{
    this->requestType_var = requestType;
}

const char * SystemMsg::getEdge() const
{
    return edge_var.c_str();
}

void SystemMsg::setEdge(const char * edge)
{
    this->edge_var = edge;
}

int SystemMsg::getTarget() const
{
    return target_var;
}

void SystemMsg::setTarget(int target)
{
    this->target_var = target;
}

class SystemMsgDescriptor : public cClassDescriptor
{
  public:
    SystemMsgDescriptor();
    virtual ~SystemMsgDescriptor();

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

Register_ClassDescriptor(SystemMsgDescriptor);

SystemMsgDescriptor::SystemMsgDescriptor() : cClassDescriptor("VENTOS::SystemMsg", "WaveShortMessage")
{
}

SystemMsgDescriptor::~SystemMsgDescriptor()
{
}

bool SystemMsgDescriptor::doesSupport(cObject *obj) const
{
    return dynamic_cast<SystemMsg *>(obj)!=NULL;
}

const char *SystemMsgDescriptor::getProperty(const char *propertyname) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : NULL;
}

int SystemMsgDescriptor::getFieldCount(void *object) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 5+basedesc->getFieldCount(object) : 5;
}

unsigned int SystemMsgDescriptor::getFieldTypeFlags(void *object, int field) const
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
    };
    return (field>=0 && field<5) ? fieldTypeFlags[field] : 0;
}

const char *SystemMsgDescriptor::getFieldName(void *object, int field) const
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
        "requestType",
        "edge",
        "target",
    };
    return (field>=0 && field<5) ? fieldNames[field] : NULL;
}

int SystemMsgDescriptor::findField(void *object, const char *fieldName) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    int base = basedesc ? basedesc->getFieldCount(object) : 0;
    if (fieldName[0]=='s' && strcmp(fieldName, "sender")==0) return base+0;
    if (fieldName[0]=='r' && strcmp(fieldName, "recipient")==0) return base+1;
    if (fieldName[0]=='r' && strcmp(fieldName, "requestType")==0) return base+2;
    if (fieldName[0]=='e' && strcmp(fieldName, "edge")==0) return base+3;
    if (fieldName[0]=='t' && strcmp(fieldName, "target")==0) return base+4;
    return basedesc ? basedesc->findField(object, fieldName) : -1;
}

const char *SystemMsgDescriptor::getFieldTypeString(void *object, int field) const
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
        "int",
    };
    return (field>=0 && field<5) ? fieldTypeStrings[field] : NULL;
}

const char *SystemMsgDescriptor::getFieldProperty(void *object, int field, const char *propertyname) const
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

int SystemMsgDescriptor::getArraySize(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getArraySize(object, field);
        field -= basedesc->getFieldCount(object);
    }
    SystemMsg *pp = (SystemMsg *)object; (void)pp;
    switch (field) {
        default: return 0;
    }
}

std::string SystemMsgDescriptor::getFieldAsString(void *object, int field, int i) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldAsString(object,field,i);
        field -= basedesc->getFieldCount(object);
    }
    SystemMsg *pp = (SystemMsg *)object; (void)pp;
    switch (field) {
        case 0: return oppstring2string(pp->getSender());
        case 1: return oppstring2string(pp->getRecipient());
        case 2: return long2string(pp->getRequestType());
        case 3: return oppstring2string(pp->getEdge());
        case 4: return long2string(pp->getTarget());
        default: return "";
    }
}

bool SystemMsgDescriptor::setFieldAsString(void *object, int field, int i, const char *value) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->setFieldAsString(object,field,i,value);
        field -= basedesc->getFieldCount(object);
    }
    SystemMsg *pp = (SystemMsg *)object; (void)pp;
    switch (field) {
        case 0: pp->setSender((value)); return true;
        case 1: pp->setRecipient((value)); return true;
        case 2: pp->setRequestType(string2long(value)); return true;
        case 3: pp->setEdge((value)); return true;
        case 4: pp->setTarget(string2long(value)); return true;
        default: return false;
    }
}

const char *SystemMsgDescriptor::getFieldStructName(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldStructName(object, field);
        field -= basedesc->getFieldCount(object);
    }
    switch (field) {
        default: return NULL;
    };
}

void *SystemMsgDescriptor::getFieldStructPointer(void *object, int field, int i) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldStructPointer(object, field, i);
        field -= basedesc->getFieldCount(object);
    }
    SystemMsg *pp = (SystemMsg *)object; (void)pp;
    switch (field) {
        default: return NULL;
    }
}

Register_Class(RouterMsg);

RouterMsg::RouterMsg(const char *name, int kind) : ::WaveShortMessage(name,kind)
{
    this->recipient_var = 0;
}

RouterMsg::RouterMsg(const RouterMsg& other) : ::WaveShortMessage(other)
{
    copy(other);
}

RouterMsg::~RouterMsg()
{
}

RouterMsg& RouterMsg::operator=(const RouterMsg& other)
{
    if (this==&other) return *this;
    ::WaveShortMessage::operator=(other);
    copy(other);
    return *this;
}

void RouterMsg::copy(const RouterMsg& other)
{
    this->recipient_var = other.recipient_var;
    this->info_var = other.info_var;
}

void RouterMsg::parsimPack(cCommBuffer *b)
{
    ::WaveShortMessage::parsimPack(b);
    doPacking(b,this->recipient_var);
    doPacking(b,this->info_var);
}

void RouterMsg::parsimUnpack(cCommBuffer *b)
{
    ::WaveShortMessage::parsimUnpack(b);
    doUnpacking(b,this->recipient_var);
    doUnpacking(b,this->info_var);
}

const char * RouterMsg::getRecipient() const
{
    return recipient_var.c_str();
}

void RouterMsg::setRecipient(const char * recipient)
{
    this->recipient_var = recipient;
}

stringList& RouterMsg::getInfo()
{
    return info_var;
}

void RouterMsg::setInfo(const stringList& info)
{
    this->info_var = info;
}

class RouterMsgDescriptor : public cClassDescriptor
{
  public:
    RouterMsgDescriptor();
    virtual ~RouterMsgDescriptor();

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

Register_ClassDescriptor(RouterMsgDescriptor);

RouterMsgDescriptor::RouterMsgDescriptor() : cClassDescriptor("VENTOS::RouterMsg", "WaveShortMessage")
{
}

RouterMsgDescriptor::~RouterMsgDescriptor()
{
}

bool RouterMsgDescriptor::doesSupport(cObject *obj) const
{
    return dynamic_cast<RouterMsg *>(obj)!=NULL;
}

const char *RouterMsgDescriptor::getProperty(const char *propertyname) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : NULL;
}

int RouterMsgDescriptor::getFieldCount(void *object) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 2+basedesc->getFieldCount(object) : 2;
}

unsigned int RouterMsgDescriptor::getFieldTypeFlags(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldTypeFlags(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISEDITABLE,
        FD_ISCOMPOUND,
    };
    return (field>=0 && field<2) ? fieldTypeFlags[field] : 0;
}

const char *RouterMsgDescriptor::getFieldName(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldName(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static const char *fieldNames[] = {
        "recipient",
        "info",
    };
    return (field>=0 && field<2) ? fieldNames[field] : NULL;
}

int RouterMsgDescriptor::findField(void *object, const char *fieldName) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    int base = basedesc ? basedesc->getFieldCount(object) : 0;
    if (fieldName[0]=='r' && strcmp(fieldName, "recipient")==0) return base+0;
    if (fieldName[0]=='i' && strcmp(fieldName, "info")==0) return base+1;
    return basedesc ? basedesc->findField(object, fieldName) : -1;
}

const char *RouterMsgDescriptor::getFieldTypeString(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldTypeString(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static const char *fieldTypeStrings[] = {
        "string",
        "stringList",
    };
    return (field>=0 && field<2) ? fieldTypeStrings[field] : NULL;
}

const char *RouterMsgDescriptor::getFieldProperty(void *object, int field, const char *propertyname) const
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

int RouterMsgDescriptor::getArraySize(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getArraySize(object, field);
        field -= basedesc->getFieldCount(object);
    }
    RouterMsg *pp = (RouterMsg *)object; (void)pp;
    switch (field) {
        default: return 0;
    }
}

std::string RouterMsgDescriptor::getFieldAsString(void *object, int field, int i) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldAsString(object,field,i);
        field -= basedesc->getFieldCount(object);
    }
    RouterMsg *pp = (RouterMsg *)object; (void)pp;
    switch (field) {
        case 0: return oppstring2string(pp->getRecipient());
        case 1: {std::stringstream out; out << pp->getInfo(); return out.str();}
        default: return "";
    }
}

bool RouterMsgDescriptor::setFieldAsString(void *object, int field, int i, const char *value) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->setFieldAsString(object,field,i,value);
        field -= basedesc->getFieldCount(object);
    }
    RouterMsg *pp = (RouterMsg *)object; (void)pp;
    switch (field) {
        case 0: pp->setRecipient((value)); return true;
        default: return false;
    }
}

const char *RouterMsgDescriptor::getFieldStructName(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldStructName(object, field);
        field -= basedesc->getFieldCount(object);
    }
    switch (field) {
        case 1: return opp_typename(typeid(stringList));
        default: return NULL;
    };
}

void *RouterMsgDescriptor::getFieldStructPointer(void *object, int field, int i) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldStructPointer(object, field, i);
        field -= basedesc->getFieldCount(object);
    }
    RouterMsg *pp = (RouterMsg *)object; (void)pp;
    switch (field) {
        case 1: return (void *)(&pp->getInfo()); break;
        default: return NULL;
    }
}

}; // end namespace VENTOS

