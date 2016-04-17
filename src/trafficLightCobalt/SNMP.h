/****************************************************************************/
/// @file    SNMP.h
/// @author  Mani Amoozadeh <maniam@ucdavis.edu>
/// @author  second author name
/// @date    August 2015
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

#ifndef SNMPCONNECT_H_
#define SNMPCONNECT_H_

#include "vector"

#include <sys/socket.h>        // solve in Mac os x: unknown type name socklen_t
#define STDCXX_98_HEADERS
#include "snmp_pp/snmp_pp.h"

namespace VENTOS {

class SNMP
{
public:
    SNMP(std::string, std::string, std::string);
    virtual ~SNMP();

    Snmp_pp::Vb SNMPget(std::string OID, int instance=0);
    std::vector<Snmp_pp::Vb> SNMPwalk(std::string OID);
    Snmp_pp::Vb SNMPset(std::string OID, std::string value, int instance=0);

private:
    Snmp_pp::Snmp *cobalt = NULL;
    Snmp_pp::CTarget *ctarget = NULL;
};

}

#endif
