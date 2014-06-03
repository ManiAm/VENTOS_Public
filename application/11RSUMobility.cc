
#include "11RSUMobility.h"

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

        manager = FindModule<TraCI_Extend*>::findGlobalModule();

        // vehicle id in omnet++
        myId = getParentModule()->getIndex();
        myFullId = getParentModule()->getFullName();

        // get TraCI coordinates
        Coord SUMOpos = manager->commandGetRSUsCoord(myId);

        Coord pos = world->getRandomPosition();

        // read coordinates from parameters if available
        pos.x = SUMOpos.x;
        pos.y = SUMOpos.y;
        pos.z = 0;

        // set start-position and start-time (i.e. current simulation-time) of the Move
        move.setStart(pos);

        //check whether position is within the playground
        if (!move.getStartPos().isInBoundary(Coord::ZERO, world->getPgs())) {
            error("node position specified in omnetpp.ini exceeds playgroundsize");
        }

        // set speed and direction of the Move
        move.setSpeed(0);
        move.setDirectionByVector(Coord::ZERO);
    }
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

