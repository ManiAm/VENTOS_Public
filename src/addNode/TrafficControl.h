/****************************************************************************/
/// @file    TrafficControl.h
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

#ifndef TRAFFICCONTROL_H_
#define TRAFFICCONTROL_H_

#include <rapidxml.hpp>
#include <rapidxml_utils.hpp>
#include <rapidxml_print.hpp>

#include "traci/TraCICommands.h"
#include "baseAppl/03_BaseApplLayer.h"


namespace VENTOS {

class TrafficControl : public BaseApplLayer
{
private:
    typedef BaseApplLayer super;

    TraCI_Commands *TraCI;
    omnetpp::simsignal_t Signal_initialize_withTraCI;

    const std::string adversary_tag = "adversary";

    std::string id;

    typedef struct adversaryEntry
    {
        std::string id_str;
        TraCICoord pos;
        bool drawMaxIntfDist;
        std::string color_str;
        bool filled;
        cModule* module;
    } adversaryEntry_t;

    std::map<std::string, adversaryEntry_t> allAdversary;

public:
    virtual ~TrafficControl();
    virtual void initialize(int stage);
    virtual void handleMessage(omnetpp::cMessage *msg);
    virtual void finish();
    virtual void receiveSignal(omnetpp::cComponent *, omnetpp::simsignal_t, long, cObject* details);

private:
    void readInsertion(std::string);
    void printLoadedStatistics();

    void parseAdversary(rapidxml::xml_node<> *);
    void addAdversary();
};

}

#endif
