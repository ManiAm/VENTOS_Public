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

        on = par("on").boolValue();
        mode = par("mode").longValue();

        VENTOS_FullPath = cSimulation::getActiveSimulation()->getEnvir()->getConfig()->getConfigEntry("network").getBaseDirectory();
        SUMO_Path = simulation.getSystemModule()->par("SUMODirectory").stringValue();
        SUMO_FullPath = VENTOS_FullPath / SUMO_Path;
        if( !boost::filesystem::exists( SUMO_FullPath ) )
            error("SUMO directory is not valid! Check it again.");
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

    std::string RSUfile = par("RSUfile").stringValue();
    boost::filesystem::path RSUfilePath = SUMO_FullPath / RSUfile;

    // check if this file is valid?
    if( !boost::filesystem::exists( RSUfilePath ) )
        error("RSU file does not exist in %s", RSUfilePath.string().c_str());

    std::deque<RSUEntry*> RSUs = commandReadRSUsCoord(RSUfilePath.string());

    // ##############################
    // Step 2: create RSUs in OMNET++
    // ##############################

    int NoRSUs = RSUs.size();

    cModule* parentMod = getParentModule();
    if (!parentMod) error("Parent Module not found");

    cModuleType* nodeType = cModuleType::get("c3po.ned.RSU");

    // We only create RSUs in OMNET++ without moving them to
    // the correct position
    for(int i = 0; i < NoRSUs; i++)
    {
        cModule* mod = nodeType->create("RSU", parentMod, NoRSUs, i);
        mod->finalizeParameters();
        mod->getDisplayString().updateWith("i=device/antennatower");
        mod->buildInside();
        mod->scheduleStart(simTime());
        mod->callInitialize();
    }

    // ###############################################################
    // Step 3: draw RSU in SUMO (using a circle to show radio coverage)
    // ###############################################################

    // get the radius of an RSU
    cModule *module = simulation.getSystemModule()->getSubmodule("RSU", 0);
    double radius = atof( module->getDisplayString().getTagArg("r",0) );

    for(int i = 0; i < NoRSUs; i++)
    {
        Coord *center = new Coord(RSUs[i]->coordX, RSUs[i]->coordY);
        commandAddCirclePoly(RSUs[i]->name, "RSU", TraCIColor::fromTkColor("blue"), center, radius);
    }
}


std::deque<RSUEntry*> AddRSU::commandReadRSUsCoord(std::string RSUfilePath)
{
    rapidxml::file<> xmlFile( RSUfilePath.c_str() );        // Convert our file to a rapid-xml readable object
    rapidxml::xml_document<> doc;                           // Build a rapidxml doc
    doc.parse<0>(xmlFile.data());                 // Fill it with data from our file
    rapidxml::xml_node<> *node = doc.first_node("RSUs");    // Parse up to the "RSUs" declaration

    for(node = node->first_node("poly"); node; node = node->next_sibling())
    {
        std::string RSUname = "";
        std::string RSUtype = "";
        std::string RSUcoordinates = "";
        int readCount = 1;

        // For each node, iterate over its attributes until we reach "shape"
        for(rapidxml::xml_attribute<> *attr = node->first_attribute(); attr; attr = attr->next_attribute())
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
        boost::char_separator<char> sep(",");
        boost::tokenizer< boost::char_separator<char> > tokens(RSUcoordinates, sep);

        for(boost::tokenizer< boost::char_separator<char> >::iterator beg=tokens.begin(); beg!=tokens.end();++beg)
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


void AddRSU::commandAddCirclePoly(std::string name, std::string type, const TraCIColor& color, Coord *center, double radius)
{
    std::list<TraCICoord> circlePoints;

    // Convert from degrees to radians via multiplication by PI/180
    for(int angleInDegrees = 0; angleInDegrees <= 360; angleInDegrees = angleInDegrees + 10)
    {
        double x = (double)( radius * cos(angleInDegrees * 3.14 / 180) ) + center->x;
        double y = (double)( radius * sin(angleInDegrees * 3.14 / 180) ) + center->y;

        circlePoints.push_back(TraCICoord(x, y));
    }

    // create polygon in SUMO
    TraCI->polygonAdd(name, type, color, 0, 1, circlePoints);
}


}

