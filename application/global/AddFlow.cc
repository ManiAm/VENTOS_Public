/****************************************************************************/
/// @file    AddFlow.cc
/// @author  Mani Amoozadeh <maniam@ucdavis.edu>
/// @author  second author name
/// @date    Jul 2015
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

#include "AddFlow.h"

#include <rapidxml.hpp>
#include <rapidxml_utils.hpp>
#include <rapidxml_print.hpp>

namespace VENTOS {

Define_Module(VENTOS::AddFlow);

AddFlow::~AddFlow()
{

}


void AddFlow::initialize(int stage)
{
    if(stage ==0)
    {
        // get a pointer to the TraCI module
        cModule *module = simulation.getSystemModule()->getSubmodule("TraCI");
        TraCI = static_cast<TraCI_Commands *>(module);
        ASSERT(TraCI);

        // get path to the launchFile
        std::string launchFile = module->par("launchFile").stringValue();
        launchFullPath = SUMO_FullPath / launchFile;

        on = par("on").boolValue();
        flowSetId = par("flowSetId").stringValue();

        Signal_addFlow = registerSignal("addFlow");
        simulation.getSystemModule()->subscribe("addFlow", this);

        boost::filesystem::path VENTOS_FullPath = cSimulation::getActiveSimulation()->getEnvir()->getConfig()->getConfigEntry("network").getBaseDirectory();
        boost::filesystem::path SUMO_Path = simulation.getSystemModule()->par("SUMODirectory").stringValue();
        SUMO_FullPath = VENTOS_FullPath / SUMO_Path;
        if( !boost::filesystem::exists( SUMO_FullPath ) )
            error("SUMO directory is not valid! Check it again.");
    }
}


void AddFlow::finish()
{

}


void AddFlow::handleMessage(cMessage *msg)
{

}


void AddFlow::receiveSignal(cComponent *source, simsignal_t signalID, long i)
{
    Enter_Method_Silent();

    if(signalID == Signal_addFlow && on)
    {
        // read launch file and get path to the sumo.cfg file
        std::string sumoConfig = getFullPathToSumoConfig(launchFullPath.string());

        // read sumo.cfg file and get the path to rou file
        std::string sumoRou = getFullPathToSumoRou(sumoConfig);

        // read flows.xml file and extract a flow set
        //     getFlowSet();

        // copy the flow set into a new rou file in %TMP%
        //     addFlowSetToNewRou();

        // add a new entry to copy the new rou, and also modify sumo.cfg
        //    applyChanges();
    }
}


std::string AddFlow::getFullPathToSumoConfig(std::string launchFullPath)
{
    rapidxml::file<> xmlFile(launchFullPath.c_str());
    rapidxml::xml_document<> doc;
    doc.parse<0>(xmlFile.data());

}


std::string AddFlow::getFullPathToSumoRou(std::string sumoConfigFullPath)
{


}


}
