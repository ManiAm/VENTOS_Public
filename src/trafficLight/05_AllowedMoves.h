/****************************************************************************/
/// @file    AllowedMoves.h
/// @author  Mani Amoozadeh <maniam@ucdavis.edu>
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

#ifndef TRAFFICLIGHTALLOWEDMOVES_H
#define TRAFFICLIGHTALLOWEDMOVES_H

#undef ev
#include "boost/filesystem.hpp"

#include "trafficLight/04_IntersectionDelay.h"

namespace VENTOS {

class TrafficLightAllowedMoves : public IntersectionDelay
{
private:
    typedef IntersectionDelay super;

    int LINKSIZE;
    boost::filesystem::path movementsFilePath;

    int rightTurns[8] = {0, 2, 5, 7, 10, 12, 15, 17};

    std::vector< std::vector<int> > allMovements;

public:
    virtual ~TrafficLightAllowedMoves();
    virtual void initialize(int);
    virtual void finish();
    virtual void handleMessage(omnetpp::cMessage *);

protected:
    void virtual initialize_withTraCI();
    void virtual executeEachTimeStep();
    std::vector<std::vector<int>>& getMovements(std::string);
    bool isRightTurn(unsigned int);

private:
    void generateAllAllowedMovements();
    void allMovementBatch(unsigned int linkNumber);
};

}

#endif
