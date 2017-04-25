
#ifndef __AIRFRMAESERIAL_H
#define __AIRFRMAESERIAL_H

#include <boost/serialization/access.hpp>

#include "AirFrame_m.h"

class AirFrame : public AirFrame_Base
{
public:

    AirFrame(const char *name=nullptr) : AirFrame_Base(name) {}
    AirFrame(const char *name=nullptr, int kind=0) : AirFrame_Base(name, kind) {}
    AirFrame(const AirFrame& other) : AirFrame_Base(other) {}
    AirFrame& operator=(const AirFrame& other) {AirFrame_Base::operator=(other); return *this;}
    virtual AirFrame *dup() const {return new AirFrame(*this);}

    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & archive, const unsigned int version)
    {
        archive & signal;
        archive & state;
        archive & type;
        archive & id;
        archive & protocolId;
        archive & channel;
    }
};

#endif
