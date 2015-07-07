/****************************************************************************/
/// @file    TraCI_App.h
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

#ifndef TraCI_APP
#define TraCI_APP

#include "TraCI_Extend.h"

namespace VENTOS {

class TraCI_App : public TraCI_Extend
{
public:
    virtual ~TraCI_App();
    virtual void initialize(int stage);
    virtual int numInitStages() const
    {
        return 3;
    }
    virtual void handleSelfMsg(cMessage *msg);
    virtual void finish();

private:
    virtual void init_traci();
    virtual void executeOneTimestep();
    void addPedestriansToOMNET();
    virtual void addModule(std::string nodeId, std::string type, std::string name, std::string displayString, const Coord& position, std::string road_id = "", double speed = -1, double angle = -1);

private:
    // NED variables
    double terminate;

    // NED (bicycles)
    std::string bikeModuleType;
    std::string bikeModuleName;
    std::string bikeModuleDisplayString;

    // NED (pedestrians)
    std::string pedModuleType;
    std::string pedModuleName;
    std::string pedModuleDisplayString;

    // class variables
    std::set<std::string> subscribedPedestrians; /**< all pedestrians we have already subscribed to */
    std::list<std::string> allPedestrians;
};

}

#endif
