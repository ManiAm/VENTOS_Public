/****************************************************************************/
/// @file    EdgeCosts.cc
/// @author  Dylan Smith <dilsmith@ucdavis.edu>
/// @author  second author here
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

#include "router/EdgeCosts.h"
#include <omnetpp.h>

namespace VENTOS {

EdgeCosts::EdgeCosts()
{
    // get a pointer to router module
    omnetpp::cModule *module = omnetpp::getSimulation()->getSystemModule()->getSubmodule("router");
    EWMARate = module->par("EWMARate").doubleValue();
    laneCostsMode = static_cast<LaneCostsMode>(module->par("LaneCostsMode").intValue());

    average = 0;
    count = 0;
}

EdgeCosts::EdgeCosts(std::map<int, int> dataSet)
{
    // get a pointer to router module
    omnetpp::cModule *module = omnetpp::getSimulation()->getSystemModule()->getSubmodule("router");
    EWMARate = module->par("EWMARate").doubleValue();
    laneCostsMode = static_cast<LaneCostsMode>(module->par("LaneCostsMode").intValue());

    average = 0;
    count = 0;

    data = dataSet;
    for(auto& pair : data)
    {
        average += pair.first * pair.second;
        count += pair.second;
    }
    average /= count;
}

void EdgeCosts::insert(int d)
{
    //If mode is average, add the new data point to the average
    if(laneCostsMode == MODE_RECORD)
    {
        average = (average * count + d) / (count + 1);
        ++count;
        ++data[d];
    }

    //If mode is EWMA, perform that calculation
    else if(laneCostsMode == MODE_EWMA)
    {
        average = average * (1-EWMARate) + (double)d * EWMARate;
    }
}

//Returns the % of readings of a certain travel time
double EdgeCosts::percentAt(int d)
{
    return double(data[d]) / count;
}

}
