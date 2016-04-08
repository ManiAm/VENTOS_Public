/****************************************************************************/
/// @file    TL_Webster.h
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

#ifndef TRAFFICLIGHTWEBSTER_H
#define TRAFFICLIGHTWEBSTER_H

#include <01_TL_Fixed.h>

namespace VENTOS {

class TrafficLightWebster : public TrafficLightFixed
{
  public:
    virtual ~TrafficLightWebster();
    virtual void initialize(int);
    virtual void finish();
    virtual void handleMessage(cMessage *);

  protected:
    void virtual executeFirstTimeStep();
    void virtual executeEachTimeStep();

  private:
    void chooseNextInterval();
    void chooseNextGreenInterval();
    void calculateGreenSplits();

  protected:
    // class variables
    double intervalDuration;
    std::string nextGreenInterval;
    std::string currentInterval;

    cMessage* intervalChangeEVT;

  private:
    std::string phase1_5 = "grgrGgrgrrgrgrGgrgrrrrrr";
    std::string phase2_6 = "gGgGrgrgrrgGgGrgrgrrrGrG";
    std::string phase3_7 = "grgrrgrgrGgrgrrgrgrGrrrr";
    std::string phase4_8 = "grgrrgGgGrgrgrrgGgGrGrGr";

    std::vector<std::string> phases = {phase1_5, phase2_6, phase3_7, phase4_8};

    std::map<std::string /*phase*/, double /*green split*/> greenSplit;
    double alpha;
};

}

#endif
