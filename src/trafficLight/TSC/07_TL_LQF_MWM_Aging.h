/****************************************************************************/
/// @file    TL_LQF_MWM_Aging.h
/// @author  Mani Amoozadeh <maniam@ucdavis.edu>
/// @date    Jul 2015
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

#ifndef TRAFFICLIGHTLQFMWMAGING_H
#define TRAFFICLIGHTLQFMWMAGING_H

#include <06_TL_LQF_MWM.h>
#include <queue>

namespace VENTOS {

class TrafficLight_LQF_MWM_Aging : public TrafficLight_LQF_MWM
{
public:
    virtual ~TrafficLight_LQF_MWM_Aging();
    virtual void initialize(int);
    virtual void finish();
    virtual void handleMessage(cMessage *);

protected:
    void virtual executeFirstTimeStep();
    void virtual executeEachTimeStep();

private:
    void chooseNextInterval();
    void chooseNextGreenInterval();

private:
    typedef TrafficLight_LQF_MWM super;
    double nextGreenTime;
};

}

#endif
