/****************************************************************************/
/// @file    LoopDetectors.h
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

#ifndef LOOPDETECTORS_H
#define LOOPDETECTORS_H

#include <unordered_map>

#include "baseAppl/03_BaseApplLayer.h"
#include "traci/TraCICommands.h"

namespace VENTOS {

class LoopDetectors : public BaseApplLayer
{
private:
    TraCI_Commands *TraCI;
    bool record_stat;

    omnetpp::simsignal_t Signal_initialize_withTraCI;
    omnetpp::simsignal_t Signal_executeEachTS;

    // loop detector ids with their lanes
    std::unordered_map<std::string /*id*/, std::string /*lane*/> LDs;

    typedef struct LoopDetectorData
    {
        std::string detectorName;
        std::string lane;
        std::string vehicleName;
        double entryTime;
        double leaveTime;
        double entrySpeed;
        double leaveSpeed;
    } LoopDetectorData_t;

    std::vector<LoopDetectorData_t> Vec_loopDetectors;

public:
    virtual ~LoopDetectors();
    virtual void initialize(int);
    virtual void finish();
    virtual void handleMessage(omnetpp::cMessage *);
    virtual void receiveSignal(omnetpp::cComponent *, omnetpp::simsignal_t, long, cObject* details);

protected:
    void virtual initialize_withTraCI();
    void virtual executeEachTimeStep();

private:
    void checkLoopDetectors();
    void collectLDsData();
    void saveLDsData();
};

}

#endif
