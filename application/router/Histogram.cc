/****************************************************************************/
/// @file    Histogram.cc
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

#include "Histogram.h"

namespace VENTOS {

Histogram::Histogram()
{
    average = 0;
    count = 0;
    minimum = 100000;
}

void Histogram::insert(int d, int LaneCostsMode)
{
    if(LaneCostsMode == 1)
    {
    average = ((average * data.size()) + d)/ (data.size() + 1);
    count++;
    if(d < minimum)
        minimum = d;

    if(data.find(d) == data.end())
        data[d] = 1;
    else
        data[d]++;
    }
    else if(LaneCostsMode == 2)
    {
        average = average * 0.8 + (double)d * .2;
    }
}

double Histogram::percentAt(int d)
{
    return (double)data[d] / (double)count;
}

std::ostream& operator<<(ostream& os, Histogram& h)
{
    os << h.data.size() << endl;
    for(map<int, int>::iterator it = h.data.begin(); it != h.data.end(); it++)
        os << it->first << " ";
    return os;
}

}
