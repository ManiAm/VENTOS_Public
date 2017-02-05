/****************************************************************************/
/// @file    TLStateRecord.h
/// @author  Mani Amoozadeh <maniam@ucdavis.edu>
/// @author
/// @date    April 2015
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

#ifndef TLSTATERECORD_H
#define TLSTATERECORD_H

#include "trafficLight/05_AllowedMoves.h"

namespace VENTOS {

class TLStateRecord : public TrafficLightAllowedMoves
{
protected:
    // NED variables
    double minGreenTime;
    double maxGreenTime;
    double yellowTime;
    double redTime;
    double maxCycleLength;

    typedef struct currentStatusTL
    {
        int cycle;
        std::string allowedMovements;
        double greenLength;
        double greenStart;
        double yellowStart;
        double redStart;
        double phaseEnd;
    } currentStatusTL_t;

    std::string phase1_5 = "grgrGgrgrrgrgrGgrgrrrrrr";
    std::string phase2_5 = "gGgGGgrgrrgrgrrgrgrrrrrG";
    std::string phase1_6 = "grgrrgrgrrgGgGGgrgrrrGrr";
    std::string phase2_6 = "gGgGrgrgrrgGgGrgrgrrrGrG";

    std::string phase3_7 = "grgrrgrgrGgrgrrgrgrGrrrr";
    std::string phase3_8 = "grgrrgrgrrgrgrrgGgGGrrGr";
    std::string phase4_7 = "grgrrgGgGGgrgrrgrgrrGrrr";
    std::string phase4_8 = "grgrrgGgGrgrgrrgGgGrGrGr";

    // vehicle type should be identical to vehicle type in beacon field
    std::map<std::string /*vehicleType*/, double /*weight*/> classWeight =
    {
            {"emergency", 50},
            {"bicycle", 40},
            {"pedestrian", 30},
            {"passenger", 20},
            {"bus", 10},
            {"truck", 1}
    };

private:
    typedef TrafficLightAllowedMoves super;

    // current phase in each TL
    std::unordered_map<std::string /*TLid*/, int /*phase number*/> phaseTL;

    // current status of each TL in each phase
    std::map<std::pair<std::string /*TLid*/, int /*phase number*/>, currentStatusTL_t> statusTL;

public:
    virtual ~TLStateRecord();
    virtual void initialize(int);
    virtual void finish();
    virtual void handleMessage(omnetpp::cMessage *);

protected:
    void virtual initialize_withTraCI();
    void virtual executeEachTimeStep();

    void updateTLstate(std::string, std::string, std::string = "", bool = false);
    const currentStatusTL_t TLGetStatus(std::string TLid);

private:
    void saveTLPhasingData();
};

}

#endif
