
#ifndef RSUMOBILITY_H
#define RSUMOBILITY_H

#include "MiXiMDefs.h"
#include "BaseMobility.h"
#include "Global_01_TraCI_Extend.h"

class MIXIM_API RSUMobility : public BaseMobility
{
  public:
    /** @brief Initializes mobility model parameters.*/
    virtual void initialize(int);

  protected:
    /** @brief Calculate the target position to move to*/
    virtual void setTargetPosition();

    /** @brief Move the host*/
    virtual void makeMove();

    // NED variables
    cModule *nodePtr;   // pointer to the Node
    mutable TraCI_Extend* TraCI;

    // Class variables
    int myId;
    const char *myFullId;
};

#endif
