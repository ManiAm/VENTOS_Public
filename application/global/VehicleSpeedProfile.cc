/****************************************************************************/
/// @file    VehicleSpeedProfile.cc
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

#include "VehicleSpeedProfile.h"


namespace VENTOS {

Define_Module(VENTOS::SpeedProfile);


SpeedProfile::~SpeedProfile()
{

}


void SpeedProfile::initialize(int stage)
{
    if(stage ==0)
    {
        // get a pointer to the TraCI module
        cModule *module = simulation.getSystemModule()->getSubmodule("TraCI");
        TraCI = static_cast<TraCI_Extend *>(module);
        ASSERT(TraCI);

        on = par("on").boolValue();
        startTime = par("startTime").doubleValue();
        laneId = par("laneId").stringValue();
        mode = par("mode").longValue();
        minSpeed = par("minSpeed").doubleValue();
        normalSpeed = par("normalSpeed").doubleValue();
        maxSpeed = par("maxSpeed").doubleValue();
        switchTime = par("switchTime").doubleValue();
        trajectoryPath = par("trajectoryPath").stringValue();

        if(startTime < 0 && startTime != -1)
            error("startTime in SpeedProfile is not correct!");

        old_speed = -1;
        lastProfileVehicle = "";
        fileOpened = false;
        endOfFile = false;
    }
}


void SpeedProfile::finish()
{

}


void SpeedProfile::handleMessage(cMessage *msg)
{

}


void SpeedProfile::Change()
{
    // if speedProfiling is not on, return
    if (!on)
        return;

    // get a list of all vehicles
    std::list<std::string> veh = TraCI->laneGetLastStepVehicleIDs(laneId);

    // as long as there is no vehicles, return
    if(veh.empty())
        return;

    // if startTime is not specified, then store the current simulation time as startTime
    if(startTime == -1)
    {
        startTime = simTime().dbl();
        std::cout << "t=" << simTime().dbl() << ": Speed profiling phase is started ..." << endl;
    }
    // if user specifies a startTime, we should wait for it
    else if(startTime > simTime().dbl())
        return;

    // get the first leading vehicle
    profileVehicle = veh.back();

    // when the profileVehicle leaves the current lane, for the new profileVehicle,
    // speed profiling should re-start from current simulation time.
    if(lastProfileVehicle != "" && profileVehicle != lastProfileVehicle)
        startTime = simTime().dbl();

    lastProfileVehicle = profileVehicle;

    // profileVehicle is the first vehicle and is not running the car-following code
    // thus comfAccel and comfDeccel do not apply to it and uses the max accel/decel.
    TraCI->vehicleSetMaxAccel(profileVehicle, 2.);
    TraCI->vehicleSetMaxDecel(profileVehicle, 5.);

    // #############################################
    // checking which SpeedProfile mode is selected?
    // #############################################

    // fixed speed
    if(mode == 0)
    {
        TraCI->vehicleSetSpeed(profileVehicle, normalSpeed);
    }
    else if(mode == 1)
    {
        // start time, min speed, max speed
        AccelDecel(startTime, minSpeed, normalSpeed);
    }
    // extreme case
    else if(mode == 2)
    {
        AccelDecel(startTime, 0., maxSpeed);
    }
    // stability test
    else if(mode == 3)
    {
        // we change the maxDecel and maxAccel of trajectory to lower values (it does not touch the boundaries)
        // note1: when we change maxDecel or maxAccel of a vehicle, its type will change!
        // note2: we have to make sure the trajectory entered into the simulation, before changing MaxAccel and MaxDecel
        // note3: and we have to change maxDeccel and maxAccel only once!
        if( simTime().dbl() == startTime )
        {
            TraCI->vehicleSetMaxAccel(profileVehicle, 1.5);
            TraCI->vehicleSetMaxDecel(profileVehicle, 2.);
            return;
        }

        AccelDecel(startTime+5, minSpeed, normalSpeed);
    }
    else if(mode == 4)
    {
        AccelDecelZikZak(startTime, minSpeed, normalSpeed);
    }
    else if(mode == 5)
    {
        // A sin(Wt) + C
        // start time, offset (C), amplitude (A), omega (W)
        AccelDecelPeriodic(startTime, 10., 10., 0.2);
    }
    else if(mode == 6)
    {
        ExTrajectory(startTime);
    }
    // hysteresis
    else if(mode == 7)
    {
        if( simTime().dbl() == startTime )
        {
            TraCI->vehicleSetMaxAccel(profileVehicle, 1);
            TraCI->vehicleSetMaxDecel(profileVehicle, 1);
            return;
        }

        // after 5 seconds start AccelDecel
        AccelDecel(startTime+5, minSpeed, normalSpeed);
    }
    else
        error("not a valid speed profile mode!");
}


// trajectory like a pulse
void SpeedProfile::AccelDecel(double startT, double minV, double maxV)
{
    if( simTime().dbl() == startT )
    {
        TraCI->vehicleSetSpeed(profileVehicle, maxV);
        return;
    }

    double v = TraCI->vehicleGetSpeed(profileVehicle);

    if( old_speed != v )
    {
        old_speed = v;
        old_time = simTime().dbl();
    }
    // as soon as current speed is equal to old speed
    else
    {
        if(v == maxV)
        {
            // waiting time between speed change
            if(simTime().dbl() - old_time >= switchTime)
            {
                TraCI->vehicleSetSpeed(profileVehicle, minV);
            }
        }
        else if(v == minV)
        {
            // waiting time between speed change
            if(simTime().dbl() - old_time >= switchTime)
            {
                TraCI->vehicleSetSpeed(profileVehicle, maxV);
            }
        }
    }
}


void SpeedProfile::AccelDecelZikZak(double startT, double minV, double maxV)
{
    if( simTime().dbl() == startT )
    {
        TraCI->vehicleSetSpeed(profileVehicle, maxV);
        return;
    }

    double v = TraCI->vehicleGetSpeed(profileVehicle);

    if( old_speed != v )
    {
        old_speed = v;
    }
    // as soon as current speed is equal to old speed
    else
    {
        if(v == maxV)
        {
            TraCI->vehicleSetSpeed(profileVehicle, minV);
        }
        else if(v == minV)
        {
            TraCI->vehicleSetSpeed(profileVehicle, maxV);
        }
    }
}


void SpeedProfile::AccelDecelPeriodic(double startT, double offset, double A, double w)
{
    if( simTime().dbl() == startT )
    {
        TraCI->vehicleSetSpeed(profileVehicle, offset);
    }
    else if( simTime().dbl() < (startT + 10) )
    {
        return;
    }

    double t = simTime().dbl();
    double newSpeed = offset + A * sin(w * t);

    TraCI->vehicleSetSpeed(profileVehicle, newSpeed);
}


void SpeedProfile::ExTrajectory(double startT)
{
    // open the file for reading
    if(!fileOpened)
    {
        f2 = fopen (trajectoryPath.c_str(), "r");

        if (f2 == NULL)
            error("external trajectory file does not exists! Check trajectoryPath variable.");

        fileOpened = true;
    }

    // if we have read all lines of the file, then return
    if(endOfFile)
        return;

    if( simTime().dbl() == startT )
    {
        // set the leading vehicle speed equal with the first speed of real trajectory data
        TraCI->vehicleSetSpeed(profileVehicle, 13.8);
        return;
    }
    else if( simTime().dbl() < 120 )
    {
        return;
    }

    char line [20];  // maximum line size
    char *result = fgets (line, sizeof line, f2);

    if (result == NULL)
    {
        fclose(f2);  // close the file
        endOfFile = true;
        return;
    }

    TraCI->vehicleSetSpeed(profileVehicle, atof(line));
}

} // end of namespace

