/****************************************************************************/
/// @file    AddFixNode.h
/// @author  Mani Amoozadeh <maniam@ucdavis.edu>
/// @author  second author name
/// @date    Apr 2016
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

#ifndef AddFixNode_H_
#define AddFixNode_H_

#include "TraCICommands.h"
#include <BaseApplLayer.h>

namespace VENTOS {

class RSUEntry
{
public:
    std::string type;
    double coordX;
    double coordY;

    RSUEntry(std::string str, double x, double y)
    {
        this->type = str;
        this->coordX = x;
        this->coordY = y;
    }
};


class AddFixNode : public BaseApplLayer
{
public:
    virtual ~AddFixNode();
    virtual void initialize(int stage);
    virtual void handleMessage(cMessage *msg);
    virtual void finish();
    virtual void receiveSignal(cComponent *, simsignal_t, long);

private:
    void beginLoading();
    void printLoadedStatistics();

    void addAdversary(int num = 1);
    void addCA(int num = 1);

    void addRSU(int num = 1);
    void commandAddCirclePoly(std::string, std::string, const RGB color, Coord*, double);

private:
    typedef BaseApplLayer super;

    TraCI_Commands *TraCI;
    simsignal_t Signal_executeFirstTS;
    std::map<int, cModule*> RSUhosts; /**< vector of all RSUs managed by us */
};

}

#endif
