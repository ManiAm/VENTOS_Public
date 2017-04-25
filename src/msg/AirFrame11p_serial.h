
#ifndef __AIRFRMAE11pSERIAL_H
#define __AIRFRMAE11pSERIAL_H

#include "AirFrame11p_m.h"

namespace Veins {

class AirFrame11p : public AirFrame11p_Base
{
public:

    AirFrame11p(const char *name=nullptr) : AirFrame11p_Base(name) {}
    AirFrame11p(const char *name=nullptr, int kind=0) : AirFrame11p_Base(name, kind) {}
    AirFrame11p(const AirFrame11p& other) : AirFrame11p_Base(other) {}
    AirFrame11p& operator=(const AirFrame11p& other) {AirFrame_Base::operator=(other); return *this;}
    virtual AirFrame11p *dup() const {return new AirFrame11p(*this);}

    // This method lets cereal know which data members to serialize
    template<class Archive>
    void serialize(Archive & archive, const unsigned int version)
    {
        archive & underSensitivity;
        archive & wasTransmitting;
    }
};

}

#endif
