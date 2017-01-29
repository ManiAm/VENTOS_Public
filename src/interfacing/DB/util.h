/****************************************************************************/
/// @file    util.h
/// @author  Mani Amoozadeh <maniam@ucdavis.edu>
/// @author  second author name
/// @date    Jan 2017
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

#ifndef UTIL_H
#define UTIL_H

#include <string>
#include <map>


namespace VENTOS {

class util
{
public:
    virtual ~util();
    util();

    static std::string MACaddrTostr(const u_int8_t MACData[]);
    static std::string IPaddrTostr(const u_int8_t addr[]);
    static std::string IPaddrTostr(uint32_t addr);
    static std::string serverPortTostr(int port);
    static std::string OUITostr(const u_int8_t MACData[]);

private:
    static std::map<std::string, std::string> OUI;
    static std::map<int, std::string> portNumber;
};

}

#endif
