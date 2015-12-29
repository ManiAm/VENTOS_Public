//
// Generated file, do not edit! Created by nedtool 4.6 from AirFrame11p.msg.
//

// Disable warnings about unused variables, empty switch stmts, etc:
#ifdef _MSC_VER
#  pragma warning(disable:4101)
#  pragma warning(disable:4065)
#endif

#include <iostream>
#include <sstream>
#include "AirFrame11p_m.h"

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

Register_Class(AirFrame11p);

AirFrame11p::AirFrame11p(const char *name, int kind) : ::AirFrame(name,kind)
{
    this->underSensitivity_var = false;
    this->wasTransmitting_var = false;
}

AirFrame11p::AirFrame11p(const AirFrame11p& other) : ::AirFrame(other)
{
    copy(other);
}

AirFrame11p::~AirFrame11p()
{
}

AirFrame11p& AirFrame11p::operator=(const AirFrame11p& other)
{
    if (this==&other) return *this;
    ::AirFrame::operator=(other);
    copy(other);
    return *this;
}

void AirFrame11p::copy(const AirFrame11p& other)
{
    this->underSensitivity_var = other.underSensitivity_var;
    this->wasTransmitting_var = other.wasTransmitting_var;
}

void AirFrame11p::parsimPack(cCommBuffer *b)
{
    ::AirFrame::parsimPack(b);
    doPacking(b,this->underSensitivity_var);
    doPacking(b,this->wasTransmitting_var);
}

void AirFrame11p::parsimUnpack(cCommBuffer *b)
{
    ::AirFrame::parsimUnpack(b);
    doUnpacking(b,this->underSensitivity_var);
    doUnpacking(b,this->wasTransmitting_var);
}

bool AirFrame11p::getUnderSensitivity() const
{
    return underSensitivity_var;
}

void AirFrame11p::setUnderSensitivity(bool underSensitivity)
{
    this->underSensitivity_var = underSensitivity;
}

bool AirFrame11p::getWasTransmitting() const
{
    return wasTransmitting_var;
}

void AirFrame11p::setWasTransmitting(bool wasTransmitting)
{
    this->wasTransmitting_var = wasTransmitting;
}

class AirFrame11pDescriptor : public cClassDescriptor
{
  public:
    AirFrame11pDescriptor();
    virtual ~AirFrame11pDescriptor();

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

Register_ClassDescriptor(AirFrame11pDescriptor);

AirFrame11pDescriptor::AirFrame11pDescriptor() : cClassDescriptor("AirFrame11p", "AirFrame")
{
}

AirFrame11pDescriptor::~AirFrame11pDescriptor()
{
}

bool AirFrame11pDescriptor::doesSupport(cObject *obj) const
{
    return dynamic_cast<AirFrame11p *>(obj)!=NULL;
}

const char *AirFrame11pDescriptor::getProperty(const char *propertyname) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : NULL;
}

int AirFrame11pDescriptor::getFieldCount(void *object) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 2+basedesc->getFieldCount(object) : 2;
}

unsigned int AirFrame11pDescriptor::getFieldTypeFlags(void *object, int field) const
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
    };
    return (field>=0 && field<2) ? fieldTypeFlags[field] : 0;
}

const char *AirFrame11pDescriptor::getFieldName(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldName(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static const char *fieldNames[] = {
        "underSensitivity",
        "wasTransmitting",
    };
    return (field>=0 && field<2) ? fieldNames[field] : NULL;
}

int AirFrame11pDescriptor::findField(void *object, const char *fieldName) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    int base = basedesc ? basedesc->getFieldCount(object) : 0;
    if (fieldName[0]=='u' && strcmp(fieldName, "underSensitivity")==0) return base+0;
    if (fieldName[0]=='w' && strcmp(fieldName, "wasTransmitting")==0) return base+1;
    return basedesc ? basedesc->findField(object, fieldName) : -1;
}

const char *AirFrame11pDescriptor::getFieldTypeString(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldTypeString(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static const char *fieldTypeStrings[] = {
        "bool",
        "bool",
    };
    return (field>=0 && field<2) ? fieldTypeStrings[field] : NULL;
}

const char *AirFrame11pDescriptor::getFieldProperty(void *object, int field, const char *propertyname) const
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

int AirFrame11pDescriptor::getArraySize(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getArraySize(object, field);
        field -= basedesc->getFieldCount(object);
    }
    AirFrame11p *pp = (AirFrame11p *)object; (void)pp;
    switch (field) {
        default: return 0;
    }
}

std::string AirFrame11pDescriptor::getFieldAsString(void *object, int field, int i) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldAsString(object,field,i);
        field -= basedesc->getFieldCount(object);
    }
    AirFrame11p *pp = (AirFrame11p *)object; (void)pp;
    switch (field) {
        case 0: return bool2string(pp->getUnderSensitivity());
        case 1: return bool2string(pp->getWasTransmitting());
        default: return "";
    }
}

bool AirFrame11pDescriptor::setFieldAsString(void *object, int field, int i, const char *value) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->setFieldAsString(object,field,i,value);
        field -= basedesc->getFieldCount(object);
    }
    AirFrame11p *pp = (AirFrame11p *)object; (void)pp;
    switch (field) {
        case 0: pp->setUnderSensitivity(string2bool(value)); return true;
        case 1: pp->setWasTransmitting(string2bool(value)); return true;
        default: return false;
    }
}

const char *AirFrame11pDescriptor::getFieldStructName(void *object, int field) const
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

void *AirFrame11pDescriptor::getFieldStructPointer(void *object, int field, int i) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldStructPointer(object, field, i);
        field -= basedesc->getFieldCount(object);
    }
    AirFrame11p *pp = (AirFrame11p *)object; (void)pp;
    switch (field) {
        default: return NULL;
    }
}


