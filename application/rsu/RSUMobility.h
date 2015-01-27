
#ifndef RSUMOBILITY_H
#define RSUMOBILITY_H

#include "MiXiMDefs.h"
#include "BaseMobility.h"
#include "TraCI_Extend.h"
#include "Global_04_AddRSU.h"

namespace VENTOS {

class TraCI_Extend;

class MIXIM_API RSUMobility : public BaseMobility
{
  public:
    /** @brief Initializes mobility model parameters.*/
    virtual void initialize(int);

  protected:
    bool isInBoundary(Coord c, Coord lowerBound, Coord upperBound);

    /** @brief Calculate the target position to move to*/
    virtual void setTargetPosition();

    /** @brief Move the host*/
    virtual void makeMove();

    Coord *getRSUCoord(unsigned int);

    // NED variables
    cModule *nodePtr;   // pointer to the Node
    TraCI_Extend *TraCI;
    AddRSU *AddRSUPtr;

    // Class variables
    int myId;
    const char *myFullId;
    deque<RSUEntry*> RSUs;
};

}

#endif
