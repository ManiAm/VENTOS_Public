
#include "RSUMobility.h"

#include <FWMath.h>


Define_Module(RSUMobility);


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
        module = simulation.getSystemModule()->getSubmodule("RSUAdd");
        RSUAddPtr = static_cast<RSUAdd *>(module);
        if(RSUAddPtr == NULL)
            error("can not get a pointer to the RSUAdd module.");

        // vehicle id in omnet++
        myId = getParentModule()->getIndex();
        myFullId = getParentModule()->getFullName();

        // get my TraCI coordinates
        Coord *SUMOpos = RSUAddPtr->commandGetRSUsCoord(myId);

        Coord pos = world->getRandomPosition();

        // read coordinates from parameters if available
        pos.x = SUMOpos->x;
        pos.y = SUMOpos->x;
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

