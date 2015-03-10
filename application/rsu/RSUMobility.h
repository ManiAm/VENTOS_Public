/****************************************************************************/
/// @file    RSUMobility.h
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

#ifndef RSUMOBILITY_H
#define RSUMOBILITY_H

#include "MiXiMDefs.h"
#include "BaseMobility.h"
#include "TraCI_Extend.h"
#include "AddRSU.h"

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
    boost::filesystem::path RSUfilePath;

    // Class variables
    int myId;
    const char *myFullId;
    deque<RSUEntry*> RSUs;
};

}

#endif
