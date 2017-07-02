/****************************************************************************/
/// @file    gui.h
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

#ifndef GUI_H
#define GUI_H

#include <rapidxml.hpp>
#include <rapidxml_utils.hpp>
#include <rapidxml_print.hpp>

#include "baseAppl/03_BaseApplLayer.h"
#include "traci/TraCICommands.h"

namespace VENTOS {

class gui : public BaseApplLayer
{
private:
    // NED variables
    TraCI_Commands *TraCI;
    omnetpp::simsignal_t Signal_initialize_withTraCI;
    omnetpp::simsignal_t Signal_executeEachTS;

    double SUMO_timeStep = -1;

    const std::string viewport_tag = "viewport";
    const std::string track_tag = "track";

    std::string id;

    typedef struct viewportEntry
    {
        std::string viewId_str;
        double zoom;
        double offsetX;
        double offsetY;
        double begin;
        int steps;

        // for internal use
        bool processingStarted = false;
        bool processingEnded = false;
        double baseZoom = 100;
        double zoomSteps = -1;
    } viewportEntry_t;

    std::map<uint32_t, viewportEntry> allViewport;

    typedef struct trackEntry
    {
        std::string viewId_str;
        std::string vehId_str;
        double begin;
        double updateRate;

        // for internal use
        bool processingStarted = false;
        bool processingEnded = false;
    } trackEntry_t;

    std::map<uint32_t, trackEntry_t> allTracking;

public:
    virtual ~gui();
    virtual void initialize(int stage);
    virtual void handleMessage(omnetpp::cMessage *msg);
    virtual void finish();
    virtual void receiveSignal(omnetpp::cComponent *, omnetpp::simsignal_t, long, cObject* details);

private:
    void readInsertion(std::string);

    void parseViewport(rapidxml::xml_node<> *);
    void controlViewport();

    void parseTracking(rapidxml::xml_node<> *);
    void controlTracking();
};

}

#endif
