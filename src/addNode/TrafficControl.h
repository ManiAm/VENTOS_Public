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

#include "exprtk.hpp"

#include "traci/TraCICommands.h"
#include "baseAppl/03_BaseApplLayer.h"
#include "nodes/vehicle/Manager.h"


namespace VENTOS {

class TrafficControl : public BaseApplLayer
{
private:
    typedef BaseApplLayer super;

    TraCI_Commands *TraCI;
    omnetpp::simsignal_t Signal_initialize_withTraCI;
    omnetpp::simsignal_t Signal_executeEachTS;

    double SUMO_timeStep = -1;

    const std::string speed_tag = "speed";
    const std::string optSize_tag = "optSize";
    const std::string pltMerge_tag = "pltMerge";
    const std::string pltSplit_tag = "pltSplit";
    const std::string pltLeave_tag = "pltLeave";
    const std::string maneuver_tag = "maneuver";

    std::string id;

    typedef exprtk::symbol_table<double> symbol_table_t;
    typedef exprtk::expression<double>   expression_t;
    typedef exprtk::parser<double>       parser_t;

    struct format_t
    {
        double time = -1;
        double speed = -1;
    };

    struct speedEntry_t
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
        std::string file_str;
        int headerLines;
        std::string columns;

        // for internal use
        bool processingStarted = false;
        bool processingEnded = false;
        double oldSpeed = -1;
        bool checkRouteEdge = false;
        bool onEdge = false;
        bool onPos = false;
        bool checkRouteLane = false;
        bool onLane = false;
        // used for expression evaluation
        bool expressionEvaluationRequired = false;
        double expressionVariable = 0;
        symbol_table_t symbol_table;
        expression_t expression;
        // used in reading the external file
        std::vector<format_t> fileContent;
        bool endOfFile = false;
    };

    std::map<uint32_t, speedEntry_t> allSpeed;

    struct optSizeEntry_t
    {
        std::string pltId_str;
        double begin;
        int value;

        // for internal use
        bool processingStarted = false;
        bool processingEnded = false;
    };

    std::map<uint32_t, optSizeEntry_t> allOptSize;

    struct pltMergeEntry_t
    {
        std::string pltId_str;
        double begin;

        // for internal use
        bool processingStarted = false;
        bool processingEnded = false;
    };

    std::map<uint32_t, pltMergeEntry_t> allPltMerge;

    struct pltSplitEntry_t
    {
        std::string pltId_str;
        int index;
        std::string vehId_str;
        double begin;

        // for internal use
        bool processingStarted = false;
        bool processingEnded = false;
    };

    std::map<uint32_t, pltSplitEntry_t> allPltSplit;

    struct pltLeaveEntry_t
    {
        std::string pltId_str;
        int index;
        std::string vehId_str;
        double begin;
        std::string leaveDirection;

        // for internal use
        bool processingStarted = false;
        bool processingEnded = false;
    };

    std::map<uint32_t, pltLeaveEntry_t> allPltLeave;

    struct pltManeuver_t
    {
        std::string pltId_str;
        double begin;
        int merge_active;
        int split_active;
        int leaderLeave_active;
        int followerLeave_active;
        int entry_active;

        // for internal use
        bool processingStarted = false;
        bool processingEnded = false;
    };

    std::map<uint32_t, pltManeuver_t> allPltManeuver;

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
    void checkSpeedConflicts(speedEntry_t &, uint32_t);
    uint32_t checkSpeedConflicts_begin(speedEntry_t &, uint32_t);
    uint32_t checkSpeedConflicts_edgeId(speedEntry_t &, uint32_t);
    uint32_t checkSpeedConflicts_laneId(speedEntry_t &, uint32_t);
    void controlSpeed();
    void controlSpeed_begin(speedEntry_t &);
    void controlSpeed_edgeId(speedEntry_t &);
    void controlSpeed_laneId(speedEntry_t &);

    void readFile(speedEntry_t &);
    void processFileTokens(std::vector<std::string> &, uint32_t &, speedEntry_t &speedEntry);
    void verifyFile(speedEntry_t &);
    bool isCSV(std::string);
    void CSV2TXT(std::string, std::vector<std::vector<std::string>> &);

    void parseOptSize(rapidxml::xml_node<> *);
    uint32_t checkOptSizeConflicts(optSizeEntry_t &, uint32_t);
    void controlOptSize();

    void parsePltMerge(rapidxml::xml_node<> *);
    void controlPltMerge();

    void parsePltSplit(rapidxml::xml_node<> *);
    void controlPltSplit();
    typedef struct pltIndex
    {
        std::string platoonId = "";
        int index = -1;
    }pltIndex_t;
    template <typename T> pltIndex_t getPlatoonIdAndIndex(T &entry);

    void parsePltLeave(rapidxml::xml_node<> *);
    void controlPltLeave();

    void parsePltManeuver(rapidxml::xml_node<> *);
    void controlPltManeuver();

    ApplVManager* getApplPtr(std::string vehId);
};

}

#endif
