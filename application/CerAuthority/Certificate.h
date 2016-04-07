#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/vector.hpp>

#ifndef CERTIFICATE_H_
#define CERTIFICATE_H_

namespace VENTOS {

class Certificate
{
public:
    std::string CerName;
    std::string CAname;
    int CAid;
    std::string NodeName;
    int NodeID;

    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & CerName;
        ar & CAname;
        ar & CAid;
        ar & NodeName;
        ar & NodeID;
    }
};

}

#endif
