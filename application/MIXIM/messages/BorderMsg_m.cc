//
// Generated file, do not edit! Created by nedtool 4.6 from messages/BorderMsg.msg.
//

// Disable warnings about unused variables, empty switch stmts, etc:
#ifdef _MSC_VER
#  pragma warning(disable:4101)
#  pragma warning(disable:4065)
#endif

#include <iostream>
#include <sstream>
#include "BorderMsg_m.h"

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

Register_Class(BorderMsg);

BorderMsg::BorderMsg(const char *name, int kind) : ::cPacket(name,kind)
{
    this->policy_var = 0;
}

BorderMsg::BorderMsg(const BorderMsg& other) : ::cPacket(other)
{
    copy(other);
}

BorderMsg::~BorderMsg()
{
}

BorderMsg& BorderMsg::operator=(const BorderMsg& other)
{
    if (this==&other) return *this;
    ::cPacket::operator=(other);
    copy(other);
    return *this;
}

void BorderMsg::copy(const BorderMsg& other)
{
    this->policy_var = other.policy_var;
    this->startPos_var = other.startPos_var;
    this->direction_var = other.direction_var;
}

void BorderMsg::parsimPack(cCommBuffer *b)
{
    ::cPacket::parsimPack(b);
    doPacking(b,this->policy_var);
    doPacking(b,this->startPos_var);
    doPacking(b,this->direction_var);
}

void BorderMsg::parsimUnpack(cCommBuffer *b)
{
    ::cPacket::parsimUnpack(b);
    doUnpacking(b,this->policy_var);
    doUnpacking(b,this->startPos_var);
    doUnpacking(b,this->direction_var);
}

int BorderMsg::getPolicy() const
{
    return policy_var;
}

void BorderMsg::setPolicy(int policy)
{
    this->policy_var = policy;
}

Coord& BorderMsg::getStartPos()
{
    return startPos_var;
}

void BorderMsg::setStartPos(const Coord& startPos)
{
    this->startPos_var = startPos;
}

Coord& BorderMsg::getDirection()
{
    return direction_var;
}

void BorderMsg::setDirection(const Coord& direction)
{
    this->direction_var = direction;
}

class BorderMsgDescriptor : public cClassDescriptor
{
  public:
    BorderMsgDescriptor();
    virtual ~BorderMsgDescriptor();

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

Register_ClassDescriptor(BorderMsgDescriptor);

BorderMsgDescriptor::BorderMsgDescriptor() : cClassDescriptor("BorderMsg", "cPacket")
{
}

BorderMsgDescriptor::~BorderMsgDescriptor()
{
}

bool BorderMsgDescriptor::doesSupport(cObject *obj) const
{
    return dynamic_cast<BorderMsg *>(obj)!=NULL;
}

const char *BorderMsgDescriptor::getProperty(const char *propertyname) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : NULL;
}

int BorderMsgDescriptor::getFieldCount(void *object) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 3+basedesc->getFieldCount(object) : 3;
}

unsigned int BorderMsgDescriptor::getFieldTypeFlags(void *object, int field) const
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
        FD_ISCOMPOUND,
    };
    return (field>=0 && field<3) ? fieldTypeFlags[field] : 0;
}

const char *BorderMsgDescriptor::getFieldName(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldName(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static const char *fieldNames[] = {
        "policy",
        "startPos",
        "direction",
    };
    return (field>=0 && field<3) ? fieldNames[field] : NULL;
}

int BorderMsgDescriptor::findField(void *object, const char *fieldName) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    int base = basedesc ? basedesc->getFieldCount(object) : 0;
    if (fieldName[0]=='p' && strcmp(fieldName, "policy")==0) return base+0;
    if (fieldName[0]=='s' && strcmp(fieldName, "startPos")==0) return base+1;
    if (fieldName[0]=='d' && strcmp(fieldName, "direction")==0) return base+2;
    return basedesc ? basedesc->findField(object, fieldName) : -1;
}

const char *BorderMsgDescriptor::getFieldTypeString(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldTypeString(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static const char *fieldTypeStrings[] = {
        "int",
        "Coord",
        "Coord",
    };
    return (field>=0 && field<3) ? fieldTypeStrings[field] : NULL;
}

const char *BorderMsgDescriptor::getFieldProperty(void *object, int field, const char *propertyname) const
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

int BorderMsgDescriptor::getArraySize(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getArraySize(object, field);
        field -= basedesc->getFieldCount(object);
    }
    BorderMsg *pp = (BorderMsg *)object; (void)pp;
    switch (field) {
        default: return 0;
    }
}

std::string BorderMsgDescriptor::getFieldAsString(void *object, int field, int i) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldAsString(object,field,i);
        field -= basedesc->getFieldCount(object);
    }
    BorderMsg *pp = (BorderMsg *)object; (void)pp;
    switch (field) {
        case 0: return long2string(pp->getPolicy());
        case 1: {std::stringstream out; out << pp->getStartPos(); return out.str();}
        case 2: {std::stringstream out; out << pp->getDirection(); return out.str();}
        default: return "";
    }
}

bool BorderMsgDescriptor::setFieldAsString(void *object, int field, int i, const char *value) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->setFieldAsString(object,field,i,value);
        field -= basedesc->getFieldCount(object);
    }
    BorderMsg *pp = (BorderMsg *)object; (void)pp;
    switch (field) {
        case 0: pp->setPolicy(string2long(value)); return true;
        default: return false;
    }
}

const char *BorderMsgDescriptor::getFieldStructName(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldStructName(object, field);
        field -= basedesc->getFieldCount(object);
    }
    switch (field) {
        case 1: return opp_typename(typeid(Coord));
        case 2: return opp_typename(typeid(Coord));
        default: return NULL;
    };
}

void *BorderMsgDescriptor::getFieldStructPointer(void *object, int field, int i) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldStructPointer(object, field, i);
        field -= basedesc->getFieldCount(object);
    }
    BorderMsg *pp = (BorderMsg *)object; (void)pp;
    switch (field) {
        case 1: return (void *)(&pp->getStartPos()); break;
        case 2: return (void *)(&pp->getDirection()); break;
        default: return NULL;
    }
}


