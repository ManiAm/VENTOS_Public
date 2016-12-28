/****************************************************************************/
/// @file    EdgeCosts.h
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

#ifndef HISTOGRAM_H
#define HISTOGRAM_H

#include <vector>
#include <map>
#include <iostream>
#include <map>

#include "global/GlobalConsts.h"

namespace VENTOS {

class EdgeCosts
{
public:
    EdgeCosts();
    EdgeCosts(std::map<int, int> dataSet);

    void insert(int d); //Inserts a new value into the LaneCosts structure.
    double percentAt(int d);

public:
    std::map<int, int> data; //Map from each travel time to its number of occurrences
    int count;
    double average;
    double EWMARate;
    LaneCostsMode laneCostsMode;
};

}

#endif
