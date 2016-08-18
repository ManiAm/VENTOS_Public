/****************************************************************************/
/// @file    SpeedProfile.h
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

#ifndef SPEEDPROFILE
#define SPEEDPROFILE

#include "BaseApplLayer.h"
#include "TraCICommands.h"

namespace VENTOS {

class SpeedProfile : public BaseApplLayer
{
public:
    virtual void initialize(int stage);
    virtual void handleMessage(omnetpp::cMessage *msg);
    virtual void finish();
    virtual void receiveSignal(omnetpp::cComponent *, omnetpp::simsignal_t, long, cObject* details);

private:
    void eachTimeStep();
    bool DoWarmup();
    void DoSpeedProfile();

    void AccelDecel(double, double, double);
    void AccelDecelZikZak(double, double, double);
    void AccelDecelPeriodic(double, double, double, double);
    void ExTrajectory(double);

private:

    // NED variables
    bool active;
    int mode;
    std::string laneId;
    std::string vehId;
    double startTime;  // the time that speed profiling starts
    int numVehicles;

    bool warmUp;
    double stopPosition;  // the position that first vehicle should stop waiting for others
    double warmUpSpeed;
    double waitingTime;

    double minSpeed;
    double normalSpeed;
    double maxSpeed;
    double switchTime;
    std::string trajectoryPath;

    // class variables
    TraCI_Commands *TraCI;
    omnetpp::simsignal_t Signal_executeEachTS;

    bool IsWarmUpFinished;
    omnetpp::cMessage* finishingWarmup;

    std::string profileVehicle;
    std::string lastProfileVehicle;
    double startTimeTrajectory;
    double old_speed;
    double old_time;

    bool fileOpened;
    bool endOfFile;
    FILE *f2;
};

}

#endif
