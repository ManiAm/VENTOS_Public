/****************************************************************************/
/// @file    xmlUtil.cc
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

#undef ev
#include "boost/filesystem.hpp"

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <random>

#include "xmlUtil.h"
#include "logging/VENTOS_logging.h"
#include "traci/TraCICommands.h"

namespace VENTOS {

void xmlUtil::validityCheck(rapidxml::xml_node<> *cNode, std::vector<std::string> names)
{
    std::string tag = cNode->name();

    // format checking: Iterate over all attributes in this node
    for(rapidxml::xml_attribute<> *cAttr1 = cNode->first_attribute(); cAttr1; cAttr1 = cAttr1->next_attribute())
    {
        std::string attName = cAttr1->name();

        if( std::find(names.begin(), names.end(), attName) == names.end() )
            throw omnetpp::cRuntimeError("'%s' is not a valid attribute in element '%s'", attName.c_str(), tag.c_str());
    }
}


std::string xmlUtil::getAttrValue_string(rapidxml::xml_node<> *cNode, std::string attr, bool mandatory, std::string defaultVal)
{
    std::string tag = cNode->name();

    auto cAttr = cNode->first_attribute(attr.c_str());
    if(!cAttr)
    {
        if(mandatory)
            throw omnetpp::cRuntimeError("attribute '%s' is not found in element '%s'", attr.c_str(), tag.c_str());
        else
            return defaultVal;
    }
    // if the attribute exists
    else
    {
        // make sure we do not have duplicate attributes
        auto cAttr2 = cNode->last_attribute(attr.c_str());
        if(cAttr != cAttr2)
        {
            throw omnetpp::cRuntimeError("multiple attribute '%s' is not allowed in element '%s'", attr.c_str(), tag.c_str());
        }
    }

    std::string val_str = cAttr->value();
    boost::trim(val_str);

    return val_str;
}


TraCICoord xmlUtil::getAttrValue_coord(rapidxml::xml_node<> *cNode, std::string attr, bool mandatory, TraCICoord defaultVal)
{
    std::string tag = cNode->name();

    auto cAttr = cNode->first_attribute(attr.c_str());
    if(!cAttr)
    {
        if(mandatory)
            throw omnetpp::cRuntimeError("attribute '%s' is not found in element '%s'", attr.c_str(), tag.c_str());
        else
            return defaultVal;
    }
    // if the attribute exists
    else
    {
        // make sure we do not have duplicate attributes
        auto cAttr2 = cNode->last_attribute(attr.c_str());
        if(cAttr != cAttr2)
        {
            throw omnetpp::cRuntimeError("multiple attribute '%s' is not allowed in element '%s'", attr.c_str(), tag.c_str());
        }
    }

    std::string coord_str = cAttr->value();
    boost::trim(coord_str);

    // coord_str are separated by ','
    std::vector<std::string> pos;
    boost::split(pos, coord_str, boost::is_any_of(","));

    if(pos.size() != 3)
        throw omnetpp::cRuntimeError("attribute '%s' in element '%s' should be in the \"x,y,z\" format", attr.c_str(), tag.c_str());

    try
    {
        std::string pos_x_str = pos[0];
        boost::trim(pos_x_str);
        double pos_x = boost::lexical_cast<double>(pos_x_str);

        std::string pos_y_str = pos[1];
        boost::trim(pos_y_str);
        double pos_y = boost::lexical_cast<double>(pos_y_str);

        std::string pos_z_str = pos[2];
        boost::trim(pos_z_str);
        double pos_z = boost::lexical_cast<double>(pos_z_str);

        return TraCICoord(pos_x, pos_y, pos_z);
    }
    catch (boost::bad_lexical_cast const&)
    {
        throw omnetpp::cRuntimeError("attribute '%s' is badly formatted in element %s: %s", attr.c_str(), tag.c_str(), coord_str.c_str());
    }
}


TraCICoord xmlUtil::getAttrValue_coord_rnd(rapidxml::xml_node<> *cNode, std::string attr, bool mandatory, TraCICoord defaultVal)
{
    std::string tag = cNode->name();

    auto cAttr = cNode->first_attribute(attr.c_str());
    if(!cAttr)
    {
        if(mandatory)
            throw omnetpp::cRuntimeError("attribute '%s' is not found in element '%s'", attr.c_str(), tag.c_str());
        else
            return defaultVal;
    }
    // if the attribute exists
    else
    {
        // make sure we do not have duplicate attributes
        auto cAttr2 = cNode->last_attribute(attr.c_str());
        if(cAttr != cAttr2)
        {
            throw omnetpp::cRuntimeError("multiple attribute '%s' is not allowed in element '%s'", attr.c_str(), tag.c_str());
        }
    }

    std::string coord_str = cAttr->value();
    boost::trim(coord_str);

    // coord_str are separated by ','
    std::vector<std::string> pos;
    boost::split(pos, coord_str, boost::is_any_of(","));

    if(pos.size() != 3)
        throw omnetpp::cRuntimeError("attribute '%s' in element '%s' should be in the \"x,y,z\" format", attr.c_str(), tag.c_str());

    // get a pointer to TraCI (only once)
    static TraCI_Commands *TraCI = NULL;
    static simBoundary_t boundaries = {0};
    if(TraCI == NULL)
    {
        // get a pointer to the TraCI module
        TraCI = TraCI_Commands::getTraCI();

        // get road network boundaries from SUMO
        boundaries = TraCI->simulationGetNetBoundary();
    }

    // mersenne twister engine -- choose a fix seed to make tests reproducible
    static std::mt19937 generator(0);

    // generating a random floating point number uniformly in [x1,x2)
    static std::uniform_real_distribution<> xCoord(boundaries.x1, boundaries.x2);
    // generating a random floating point number uniformly in [y1,y2)
    static std::uniform_real_distribution<> yCoord(boundaries.y1, boundaries.y2);

    try
    {
        double pos_x;
        double pos_y;
        double pos_z;

        std::string pos_x_str = pos[0];
        boost::trim(pos_x_str);

        if(pos_x_str == "rnd")
            pos_x = xCoord(generator);
        else
            pos_x = boost::lexical_cast<double>(pos_x_str);

        std::string pos_y_str = pos[1];
        boost::trim(pos_y_str);

        if(pos_y_str == "rnd")
            pos_y = yCoord(generator);
        else
            pos_y = boost::lexical_cast<double>(pos_y_str);

        std::string pos_z_str = pos[2];
        boost::trim(pos_z_str);

        if(pos_z_str == "rnd")
        {
            throw omnetpp::cRuntimeError("attribute '%s' does not support 'rnd' for z coordinate in element %s: %s", attr.c_str(), tag.c_str(), coord_str.c_str());
        }
        else
            pos_z = boost::lexical_cast<double>(pos_z_str);

        return TraCICoord(pos_x, pos_y, pos_z);
    }
    catch (boost::bad_lexical_cast const&)
    {
        throw omnetpp::cRuntimeError("attribute '%s' is badly formatted in element %s: %s", attr.c_str(), tag.c_str(), coord_str.c_str());
    }
}


bool xmlUtil::getAttrValue_bool(rapidxml::xml_node<> *cNode, std::string attr, bool mandatory, bool defaultVal)
{
    std::string tag = cNode->name();

    auto cAttr = cNode->first_attribute(attr.c_str());
    if(!cAttr)
    {
        if(mandatory)
            throw omnetpp::cRuntimeError("attribute '%s' is not found in element '%s'", attr.c_str(), tag.c_str());
        else
            return defaultVal;
    }
    // if the attribute exists
    else
    {
        // make sure we do not have duplicate attributes
        auto cAttr2 = cNode->last_attribute(attr.c_str());
        if(cAttr != cAttr2)
        {
            throw omnetpp::cRuntimeError("multiple attribute '%s' is not allowed in element '%s'", attr.c_str(), tag.c_str());
        }
    }

    std::string val_str = cAttr->value();
    boost::trim(val_str);

    if(val_str == "true")
        return true;
    else if(val_str == "false")
        return false;
    else
        throw omnetpp::cRuntimeError("attribute '%s' is badly formatted in element %s: %s", attr.c_str(), tag.c_str(), val_str.c_str());
}


int xmlUtil::getAttrValue_int(rapidxml::xml_node<> *cNode, std::string attr, bool mandatory, int defaultVal)
{
    std::string tag = cNode->name();

    auto cAttr = cNode->first_attribute(attr.c_str());
    if(!cAttr)
    {
        if(mandatory)
            throw omnetpp::cRuntimeError("attribute '%s' is not found in element '%s'", attr.c_str(), tag.c_str());
        else
            return defaultVal;
    }
    // if the attribute exists
    else
    {
        // make sure we do not have duplicate attributes
        auto cAttr2 = cNode->last_attribute(attr.c_str());
        if(cAttr != cAttr2)
        {
            throw omnetpp::cRuntimeError("multiple attribute '%s' is not allowed in element '%s'", attr.c_str(), tag.c_str());
        }
    }

    std::string val_str = cAttr->value();
    boost::trim(val_str);

    try
    {
        return boost::lexical_cast<int>(val_str);
    }
    catch (boost::bad_lexical_cast const&)
    {
        throw omnetpp::cRuntimeError("attribute '%s' is badly formatted in element %s: %s", attr.c_str(), tag.c_str(), val_str.c_str());
    }
}


double xmlUtil::getAttrValue_double(rapidxml::xml_node<> *cNode, std::string attr, bool mandatory, double defaultVal)
{
    std::string tag = cNode->name();

    auto cAttr = cNode->first_attribute(attr.c_str());
    if(!cAttr)
    {
        if(mandatory)
            throw omnetpp::cRuntimeError("attribute '%s' is not found in element '%s'", attr.c_str(), tag.c_str());
        else
            return defaultVal;
    }
    // if the attribute exists
    else
    {
        // make sure we do not have duplicate attributes
        auto cAttr2 = cNode->last_attribute(attr.c_str());
        if(cAttr != cAttr2)
        {
            throw omnetpp::cRuntimeError("multiple attribute '%s' is not allowed in element '%s'", attr.c_str(), tag.c_str());
        }
    }

    std::string val_str = cAttr->value();
    boost::trim(val_str);

    try
    {
        return boost::lexical_cast<double>(val_str);
    }
    catch (boost::bad_lexical_cast const&)
    {
        throw omnetpp::cRuntimeError("attribute '%s' is badly formatted in element %s: %s", attr.c_str(), tag.c_str(), val_str.c_str());
    }
}


std::vector<std::string> xmlUtil::getAttrValue_stringVector(rapidxml::xml_node<> *cNode, std::string attr, bool mandatory, std::vector<std::string> defaultVal)
{
    std::string tag = cNode->name();

    auto cAttr = cNode->first_attribute(attr.c_str());
    if(!cAttr)
    {
        if(mandatory)
            throw omnetpp::cRuntimeError("attribute '%s' is not found in element '%s'", attr.c_str(), tag.c_str());
        else
            return defaultVal;
    }
    // if the attribute exists
    else
    {
        // make sure we do not have duplicate attributes
        auto cAttr2 = cNode->last_attribute(attr.c_str());
        if(cAttr != cAttr2)
        {
            throw omnetpp::cRuntimeError("multiple attribute '%s' is not allowed in element '%s'", attr.c_str(), tag.c_str());
        }
    }

    std::string val_str = cAttr->value();
    boost::trim(val_str);

    std::vector<std::string> val_str_tokenize;

    // tokenize val_str
    boost::split(val_str_tokenize, val_str, boost::is_any_of(","));

    // remove leading/trailing space
    for(auto &entry : val_str_tokenize)
    {
        boost::trim(entry);
        if(entry == "")
            throw omnetpp::cRuntimeError("attribute '%s' is not formatted correctly in element '%s'", attr.c_str(), tag.c_str());
    }

    return val_str_tokenize;
}


std::vector<double> xmlUtil::getAttrValue_doubleVector(rapidxml::xml_node<> *cNode, std::string attr, bool mandatory, std::vector<double> defaultVal)
{
    std::string tag = cNode->name();

    auto cAttr = cNode->first_attribute(attr.c_str());
    if(!cAttr)
    {
        if(mandatory)
            throw omnetpp::cRuntimeError("attribute '%s' is not found in element '%s'", attr.c_str(), tag.c_str());
        else
            return defaultVal;
    }
    // if the attribute exists
    else
    {
        // make sure we do not have duplicate attributes
        auto cAttr2 = cNode->last_attribute(attr.c_str());
        if(cAttr != cAttr2)
        {
            throw omnetpp::cRuntimeError("multiple attribute '%s' is not allowed in element '%s'", attr.c_str(), tag.c_str());
        }
    }

    std::string val_str = cAttr->value();
    boost::trim(val_str);

    std::vector<std::string> val_str_tokenize;
    std::vector<double> val_tokenize;

    // tokenize val_str
    boost::split(val_str_tokenize, val_str, boost::is_any_of(","));

    for(auto &entry : val_str_tokenize)
    {
        // remove leading/trailing space
        boost::trim(entry);
        if(entry == "")
            throw omnetpp::cRuntimeError("attribute '%s' is not formatted correctly in element '%s'", attr.c_str(), tag.c_str());

        try
        {
            double val = boost::lexical_cast<double>(entry);
            val_tokenize.push_back(val);
        }
        catch (boost::bad_lexical_cast const&)
        {
            throw omnetpp::cRuntimeError("attribute '%s' is badly formatted in element %s: %s", attr.c_str(), tag.c_str(), val_str.c_str());
        }
    }

    return val_tokenize;
}

}
