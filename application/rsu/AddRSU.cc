/****************************************************************************/
/// @file    AddRSU.cc
/// @author  Mani Amoozadeh <maniam@ucdavis.edu>
/// @author  second author name
/// @date    August 2013
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

#include "AddRSU.h"

namespace VENTOS {

Define_Module(VENTOS::AddRSU);


AddRSU::~AddRSU()
{

}


void AddRSU::initialize(int stage)
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

        Signal_executeFirstTS = registerSignal("executeFirstTS");
        simulation.getSystemModule()->subscribe("executeFirstTS", this);

        VENTOS_FullPath = cSimulation::getActiveSimulation()->getEnvir()->getConfig()->getConfigEntry("network").getBaseDirectory();
        SUMO_Path = simulation.getSystemModule()->par("SUMODirectory").stringValue();
        SUMO_FullPath = VENTOS_FullPath / SUMO_Path;
        // check if this directory is valid?
        if( !exists( SUMO_FullPath ) )
        {
            error("SUMO directory is not valid! Check it again.");
        }

        on = par("on").boolValue();
        mode = par("mode").longValue();
    }
}


void AddRSU::finish()
{

}


void AddRSU::handleMessage(cMessage *msg)
{

}


void AddRSU::receiveSignal(cComponent *source, simsignal_t signalID, long i)
{
    Enter_Method_Silent();

    if(signalID == Signal_executeFirstTS)
    {
        AddRSU::Add();
    }
}


void AddRSU::Add()
{
    // if dynamic adding is off, return
    if (!on)
        return;

    if(mode == 1)
    {
        Scenario1();
    }
}


void AddRSU::Scenario1()
{
    // ####################################
    // Step 1: read RSU locations from file
    // ####################################

    string RSUfile = par("RSUfile").stringValue();
    boost::filesystem::path RSUfilePath = SUMO_FullPath / RSUfile;

    // check if this file is valid?
    if( !exists( RSUfilePath ) )
    {
        error("RSU file does not exist in %s", RSUfilePath.string().c_str());
    }

    deque<RSUEntry*> RSUs = commandReadRSUsCoord(RSUfilePath.string());

    // ################################
    // Step 2: create RSUs into OMNET++
    // ################################

    int NoRSUs = RSUs.size();

    cModule* parentMod = getParentModule();
    if (!parentMod) error("Parent Module not found");

    cModuleType* nodeType = cModuleType::get("c3po.ned.RSU");

    // We only create RSUs in OMNET++ without moving them to
    // the correct coordinate
    for(int i = 0; i < NoRSUs; i++)
    {
        cModule* mod = nodeType->create("RSU", parentMod, NoRSUs, i);
        mod->finalizeParameters();
        mod->getDisplayString().updateWith("i=device/antennatower");
        mod->buildInside();
        mod->scheduleStart(simTime());
        mod->callInitialize();
    }

    // ##################################################
    // Step 3: create a polygon for radio circle in SUMO
    // ##################################################

    // get the radius of an RSU
    cModule *module = simulation.getSystemModule()->getSubmodule("RSU", 0);
    double radius = atof( module->getDisplayString().getTagArg("r",0) );

    for(int i = 0; i < NoRSUs; i++)
    {
        Coord *center = new Coord(RSUs[i]->coordX, RSUs[i]->coordY);
        commandAddCirclePoly(RSUs[i]->name, "RSU", TraCIColor::fromTkColor("blue"), center, radius);
    }
}


deque<RSUEntry*> AddRSU::commandReadRSUsCoord(string RSUfilePath)
{
    file<> xmlFile( RSUfilePath.c_str() );        // Convert our file to a rapid-xml readable object
    xml_document<> doc;                           // Build a rapidxml doc
    doc.parse<0>(xmlFile.data());                 // Fill it with data from our file
    xml_node<> *node = doc.first_node("RSUs");    // Parse up to the "RSUs" declaration

    for(node = node->first_node("poly"); node; node = node->next_sibling())
    {
        string RSUname = "";
        string RSUtype = "";
        string RSUcoordinates = "";
        int readCount = 1;

        // For each node, iterate over its attributes until we reach "shape"
        for(xml_attribute<> *attr = node->first_attribute(); attr; attr = attr->next_attribute())
        {
            if(readCount == 1)
            {
                RSUname = attr->value();
            }
            else if(readCount == 2)
            {
                RSUtype = attr->value();
            }
            else if(readCount == 3)
            {
                RSUcoordinates = attr->value();
            }

            readCount++;
        }

        // tokenize coordinates
        int readCount2 = 1;
        double x;
        double y;
        char_separator<char> sep(",");
        tokenizer< char_separator<char> > tokens(RSUcoordinates, sep);

        for(tokenizer< char_separator<char> >::iterator beg=tokens.begin(); beg!=tokens.end();++beg)
        {
            if(readCount2 == 1)
            {
                x = atof( (*beg).c_str() );
            }
            else if(readCount2 == 2)
            {
                y = atof( (*beg).c_str() );
            }

            readCount2++;
        }

        // add it into queue (with TraCI coordinates)
        RSUEntry *entry = new RSUEntry(RSUname.c_str(), x, y);
        RSUs.push_back(entry);
    }

    return RSUs;
}


void AddRSU::commandAddCirclePoly(string name, string type, const TraCIColor& color, Coord *center, double radius)
{
    list<TraCICoord> circlePoints;

    // Convert from degrees to radians via multiplication by PI/180
    for(int angleInDegrees = 0; angleInDegrees <= 360; angleInDegrees = angleInDegrees + 10)
    {
        double x = (double)( radius * cos(angleInDegrees * 3.14 / 180) ) + center->x;
        double y = (double)( radius * sin(angleInDegrees * 3.14 / 180) ) + center->y;

        circlePoints.push_back(TraCICoord(x, y));
    }

    // create polygon in SUMO
    TraCI->commandAddPolygon(name, type, color, 0, 1, circlePoints);
}


}

