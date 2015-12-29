//
// Generated file, do not edit! Created by nedtool 4.6 from messages/AirFrame.msg.
//

// Disable warnings about unused variables, empty switch stmts, etc:
#ifdef _MSC_VER
#  pragma warning(disable:4101)
#  pragma warning(disable:4065)
#endif

#include <iostream>
#include <sstream>
#include "AirFrame_m.h"

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

Register_Class(AirFrame);

AirFrame::AirFrame(const char *name, int kind) : ::cPacket(name,kind)
{
    this->duration_var = 0;
    this->state_var = 1;
    this->type_var = 0;
    this->id_var = 0;
    this->protocolId_var = 0;
    this->channel_var = 0;
}

AirFrame::AirFrame(const AirFrame& other) : ::cPacket(other)
{
    copy(other);
}

AirFrame::~AirFrame()
{
}

AirFrame& AirFrame::operator=(const AirFrame& other)
{
    if (this==&other) return *this;
    ::cPacket::operator=(other);
    copy(other);
    return *this;
}

void AirFrame::copy(const AirFrame& other)
{
    this->signal_var = other.signal_var;
    this->duration_var = other.duration_var;
    this->state_var = other.state_var;
    this->type_var = other.type_var;
    this->id_var = other.id_var;
    this->protocolId_var = other.protocolId_var;
    this->channel_var = other.channel_var;
}

void AirFrame::parsimPack(cCommBuffer *b)
{
    ::cPacket::parsimPack(b);
    doPacking(b,this->signal_var);
    doPacking(b,this->duration_var);
    doPacking(b,this->state_var);
    doPacking(b,this->type_var);
    doPacking(b,this->id_var);
    doPacking(b,this->protocolId_var);
    doPacking(b,this->channel_var);
}

void AirFrame::parsimUnpack(cCommBuffer *b)
{
    ::cPacket::parsimUnpack(b);
    doUnpacking(b,this->signal_var);
    doUnpacking(b,this->duration_var);
    doUnpacking(b,this->state_var);
    doUnpacking(b,this->type_var);
    doUnpacking(b,this->id_var);
    doUnpacking(b,this->protocolId_var);
    doUnpacking(b,this->channel_var);
}

Signal& AirFrame::getSignal()
{
    return signal_var;
}

void AirFrame::setSignal(const Signal& signal)
{
    this->signal_var = signal;
}

simtime_t AirFrame::getDuration() const
{
    return duration_var;
}

void AirFrame::setDuration(simtime_t duration)
{
    this->duration_var = duration;
}

int AirFrame::getState() const
{
    return state_var;
}

void AirFrame::setState(int state)
{
    this->state_var = state;
}

int AirFrame::getType() const
{
    return type_var;
}

void AirFrame::setType(int type)
{
    this->type_var = type;
}

long AirFrame::getId() const
{
    return id_var;
}

void AirFrame::setId(long id)
{
    this->id_var = id;
}

int AirFrame::getProtocolId() const
{
    return protocolId_var;
}

void AirFrame::setProtocolId(int protocolId)
{
    this->protocolId_var = protocolId;
}

int AirFrame::getChannel() const
{
    return channel_var;
}

void AirFrame::setChannel(int channel)
{
    this->channel_var = channel;
}

class AirFrameDescriptor : public cClassDescriptor
{
  public:
    AirFrameDescriptor();
    virtual ~AirFrameDescriptor();

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

Register_ClassDescriptor(AirFrameDescriptor);

AirFrameDescriptor::AirFrameDescriptor() : cClassDescriptor("AirFrame", "cPacket")
{
}

AirFrameDescriptor::~AirFrameDescriptor()
{
}

bool AirFrameDescriptor::doesSupport(cObject *obj) const
{
    return dynamic_cast<AirFrame *>(obj)!=NULL;
}

const char *AirFrameDescriptor::getProperty(const char *propertyname) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : NULL;
}

int AirFrameDescriptor::getFieldCount(void *object) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 7+basedesc->getFieldCount(object) : 7;
}

unsigned int AirFrameDescriptor::getFieldTypeFlags(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldTypeFlags(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISCOMPOUND,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
    };
    return (field>=0 && field<7) ? fieldTypeFlags[field] : 0;
}

const char *AirFrameDescriptor::getFieldName(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldName(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static const char *fieldNames[] = {
        "signal",
        "duration",
        "state",
        "type",
        "id",
        "protocolId",
        "channel",
    };
    return (field>=0 && field<7) ? fieldNames[field] : NULL;
}

int AirFrameDescriptor::findField(void *object, const char *fieldName) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    int base = basedesc ? basedesc->getFieldCount(object) : 0;
    if (fieldName[0]=='s' && strcmp(fieldName, "signal")==0) return base+0;
    if (fieldName[0]=='d' && strcmp(fieldName, "duration")==0) return base+1;
    if (fieldName[0]=='s' && strcmp(fieldName, "state")==0) return base+2;
    if (fieldName[0]=='t' && strcmp(fieldName, "type")==0) return base+3;
    if (fieldName[0]=='i' && strcmp(fieldName, "id")==0) return base+4;
    if (fieldName[0]=='p' && strcmp(fieldName, "protocolId")==0) return base+5;
    if (fieldName[0]=='c' && strcmp(fieldName, "channel")==0) return base+6;
    return basedesc ? basedesc->findField(object, fieldName) : -1;
}

const char *AirFrameDescriptor::getFieldTypeString(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldTypeString(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static const char *fieldTypeStrings[] = {
        "Signal",
        "simtime_t",
        "int",
        "int",
        "long",
        "int",
        "int",
    };
    return (field>=0 && field<7) ? fieldTypeStrings[field] : NULL;
}

const char *AirFrameDescriptor::getFieldProperty(void *object, int field, const char *propertyname) const
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

int AirFrameDescriptor::getArraySize(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getArraySize(object, field);
        field -= basedesc->getFieldCount(object);
    }
    AirFrame *pp = (AirFrame *)object; (void)pp;
    switch (field) {
        default: return 0;
    }
}

std::string AirFrameDescriptor::getFieldAsString(void *object, int field, int i) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldAsString(object,field,i);
        field -= basedesc->getFieldCount(object);
    }
    AirFrame *pp = (AirFrame *)object; (void)pp;
    switch (field) {
        case 0: {std::stringstream out; out << pp->getSignal(); return out.str();}
        case 1: return double2string(pp->getDuration());
        case 2: return long2string(pp->getState());
        case 3: return long2string(pp->getType());
        case 4: return long2string(pp->getId());
        case 5: return long2string(pp->getProtocolId());
        case 6: return long2string(pp->getChannel());
        default: return "";
    }
}

bool AirFrameDescriptor::setFieldAsString(void *object, int field, int i, const char *value) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->setFieldAsString(object,field,i,value);
        field -= basedesc->getFieldCount(object);
    }
    AirFrame *pp = (AirFrame *)object; (void)pp;
    switch (field) {
        case 1: pp->setDuration(string2double(value)); return true;
        case 2: pp->setState(string2long(value)); return true;
        case 3: pp->setType(string2long(value)); return true;
        case 4: pp->setId(string2long(value)); return true;
        case 5: pp->setProtocolId(string2long(value)); return true;
        case 6: pp->setChannel(string2long(value)); return true;
        default: return false;
    }
}

const char *AirFrameDescriptor::getFieldStructName(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldStructName(object, field);
        field -= basedesc->getFieldCount(object);
    }
    switch (field) {
        case 0: return opp_typename(typeid(Signal));
        default: return NULL;
    };
}

void *AirFrameDescriptor::getFieldStructPointer(void *object, int field, int i) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldStructPointer(object, field, i);
        field -= basedesc->getFieldCount(object);
    }
    AirFrame *pp = (AirFrame *)object; (void)pp;
    switch (field) {
        case 0: return (void *)(&pp->getSignal()); break;
        default: return NULL;
    }
}


