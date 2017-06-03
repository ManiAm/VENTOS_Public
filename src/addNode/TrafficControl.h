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
    omnetpp::simsignal_t Signal_executeEachTS;

    const std::string speed_tag = "speed";
    const std::string optSize_tag = "optSize";
    const std::string pltMerge_tag = "pltMerge";
    const std::string pltSplit_tag = "pltSplit";

    std::string id;

    typedef struct speedEntry
    {
        std::string id_str;
        double begin;
        double end;
        double duration;
        std::string edgeId_str;
        double edgePos;
        std::string laneId_str;
        double lanePos;
        std::string value_str;
        double maxDecel;
        double maxAccel;

        bool processingStarted;
        bool processingEnded;
        double oldSpeed;
    } speedEntry_t;

    std::map<uint32_t, speedEntry_t> allSpeed;

    typedef struct optSizeEntry
    {
        std::string pltId_str;
        double begin;
        int value;
    } optSizeEntry_t;

    std::map<uint32_t, optSizeEntry_t> allOptSize;

    typedef struct pltMergeEntry
    {
        std::string pltId_str;
        double begin;
        std::string mode_str;
    } pltMergeEntry_t;

    std::map<uint32_t, pltMergeEntry_t> allPltMerge;

    typedef struct pltSplitEntry
    {
        std::string pltId_str;
        double begin;
        int splitIndex;
        std::string splitVehId_str;
    } pltSplitEntry_t;

    std::map<uint32_t, pltSplitEntry_t> allPltSplit;

public:
    virtual ~TrafficControl();
    virtual void initialize(int stage);
    virtual void handleMessage(omnetpp::cMessage *msg);
    virtual void finish();
    virtual void receiveSignal(omnetpp::cComponent *, omnetpp::simsignal_t, long, cObject* details);

private:
    void readInsertion(std::string);
    void printLoadedStatistics();

    void parseSpeed(rapidxml::xml_node<> *);
    void vehicleSetSpeed(speedEntry_t &, std::string);
    void controlSpeed();
    void controlSpeed_begin(speedEntry_t &);
    void controlSpeed_edgeId(speedEntry_t &);
    void controlSpeed_laneId(speedEntry_t &);

    void parseOptSize(rapidxml::xml_node<> *);
    void controlOptSize();

    void parsePltMerge(rapidxml::xml_node<> *);
    void controlPltMerge();

    void parsePltSplit(rapidxml::xml_node<> *);
    void controlPltSplit();
};

}

#endif
