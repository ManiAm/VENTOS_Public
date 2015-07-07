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

namespace VENTOS {

Define_Module(VENTOS::AddFlow);

AddFlow::~AddFlow()
{

}


void AddFlow::initialize(int stage)
{
    if(stage ==0)
    {
        // get the ptr of the current module
        nodePtr = FindModule<>::findHost(this);
        if(nodePtr == NULL)
            error("can not get a pointer to the module.");

        // get a pointer to the TraCI module
        cModule *module = simulation.getSystemModule()->getSubmodule("TraCI");
        TraCI = static_cast<TraCI_Extend *>(module);

        // get path to the launchFile
        std::string launchFile = module->par("launchFile").stringValue();
        launchFullPath = SUMO_FullPath / launchFile;

        on = par("on").boolValue();
        flowSet = par("flowSet").longValue();

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

    if(signalID == Signal_addFlow)
    {
        AddFlow::Add();
    }
}


void AddFlow::Add()
{
    // if dynamic adding is off, return
    if (!on)
        return;

    if(flowSet == 1)
    {
        Scenario1();
    }
    else
    {
        error("not a valid mode!");
    }
}


void AddFlow::Scenario1()
{
    rapidxml::file<> xmlFile( launchFullPath.string().c_str() );
    rapidxml::xml_document<> doc;
    doc.parse<0>(xmlFile.data());

}

}
