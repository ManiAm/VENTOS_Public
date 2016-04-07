/****************************************************************************/
/// @file    Certificate.h
/// @author  Mani Amoozadeh <maniam@ucdavis.edu>
/// @author  second author name
/// @date    Apr 2016
///
/****************************************************************************/
// VENTOS, Vehicular Network Open Simulator; see http:?
// Copyright (C) 2013-2015
/****************************************************************************/
//
// This file is part of VENTOS.
// VENTOS is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

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
