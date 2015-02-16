
#include "RSUMobility.h"
#include <FWMath.h>

namespace VENTOS {

Define_Module(VENTOS::RSUMobility);


void RSUMobility::initialize(int stage)
{
    BaseMobility::initialize(stage);

    if (stage == 0)
    {
        // get the ptr of the current module
        nodePtr = FindModule<>::findHost(this);
        if(nodePtr == NULL)
            error("can not get a pointer to the module.");

        // get a pointer to the TraCI module
        cModule *module = simulation.getSystemModule()->getSubmodule("TraCI");
        TraCI = static_cast<TraCI_Extend *>(module);

        // get a pointer to the RSUAdd module
        cModule *module2 = simulation.getSystemModule()->getSubmodule("addRSU");
        AddRSUPtr = static_cast<AddRSU *>(module2);

        // vehicle id in omnet++
        myId = getParentModule()->getIndex();
        myFullId = getParentModule()->getFullName();

        Coord pos = world->getRandomPosition();

        // get my TraCI coordinates
        Coord *SUMOpos = getRSUCoord(myId);

        // read coordinates from parameters if available
        pos.x = SUMOpos->x;
        pos.y = SUMOpos->y;
        pos.z = 0;

        // set start-position and start-time (i.e. current simulation-time) of the Move
        move.setStart(pos);

        //check whether position is within the playground
        if ( !isInBoundary(move.getStartPos(), Coord::ZERO, *world->getPgs()) ) {
            error("node position specified in omnetpp.ini exceeds playgroundsize");
        }

        // set speed and direction of the Move
        move.setSpeed(0);
        move.setDirectionByVector(Coord::ZERO);
    }
}


Coord *RSUMobility::getRSUCoord(unsigned int index)
{
    boost::filesystem::path VENTOS_FullPath = cSimulation::getActiveSimulation()->getEnvir()->getConfig()->getConfigEntry("network").getBaseDirectory();
    boost::filesystem::path SUMO_Path = simulation.getSystemModule()->par("SUMODirectory").stringValue();
    boost::filesystem::path SUMO_FullPath = VENTOS_FullPath / SUMO_Path;
    // check if this directory is valid?
    if( !exists( SUMO_FullPath ) )
    {
        error("SUMO directory is not valid! Check it again.");
    }

    string RSUfile = AddRSUPtr->par("RSUfile").stringValue();
    boost::filesystem::path RSUfilePath = SUMO_FullPath / RSUfile;
    // check if this file is valid?
    if( !exists( RSUfilePath ) )
    {
        error("RSU file does not exist in %s", RSUfilePath.string().c_str());
    }

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

    if( RSUs.size() == 0 )
        error("No RSUs have been initialized!");

    if( index < 0 || index >= RSUs.size() )
        error("index out of bound!");

    Coord *point = new Coord(RSUs[index]->coordX, RSUs[index]->coordY);

    return point;
}


bool RSUMobility::isInBoundary(Coord c, Coord lowerBound, Coord upperBound)
{
    return  lowerBound.x <= c.x && c.x <= upperBound.x &&
            lowerBound.y <= c.y && c.y <= upperBound.y &&
            lowerBound.z <= c.z && c.z <= upperBound.z;
}


/**
 * Calculate a new random position and the number of steps the host
 * needs to reach this position
 */
void RSUMobility::setTargetPosition()
{

}


/**
 * Move the host if the destination is not reached yet. Otherwise
 * calculate a new random position
 */
void RSUMobility::makeMove()
{

}


}

