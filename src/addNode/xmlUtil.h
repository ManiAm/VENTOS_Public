/****************************************************************************/
/// @file    xmlUtil.h
/// @author  Mani Amoozadeh <maniam@ucdavis.edu>
/// @author  second author name
/// @date    Jun 2017
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

#ifndef XMLUTIL_H_
#define XMLUTIL_H_

#include <rapidxml.hpp>
#include <rapidxml_utils.hpp>
#include <rapidxml_print.hpp>

#include "mobility/TraCICoord.h"

namespace VENTOS {

class xmlUtil
{
public:
    static void validityCheck(rapidxml::xml_node<> *, std::vector<std::string>);
    static std::string getAttrValue_string(rapidxml::xml_node<> *, std::string, bool = true, std::string = "");
    static TraCICoord getAttrValue_coord(rapidxml::xml_node<> *, std::string, bool = true, TraCICoord = TraCICoord(0,0,0));
    static TraCICoord getAttrValue_coord_rnd(rapidxml::xml_node<> *, std::string, bool = true, TraCICoord = TraCICoord(0,0,0));
    static bool getAttrValue_bool(rapidxml::xml_node<> *, std::string, bool = true, bool = false);
    static int getAttrValue_int(rapidxml::xml_node<> *, std::string, bool = true, int = 0);
    static double getAttrValue_double(rapidxml::xml_node<> *, std::string, bool = true, double = 0);
    static std::vector<std::string> getAttrValue_stringVector(rapidxml::xml_node<> *, std::string, bool = true, std::vector<std::string> = std::vector<std::string>());
    static std::vector<double> getAttrValue_doubleVector(rapidxml::xml_node<> *, std::string, bool = true, std::vector<double> = std::vector<double>());
};

}

#endif
