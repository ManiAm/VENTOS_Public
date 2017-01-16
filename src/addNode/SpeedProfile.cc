/****************************************************************************/
/// @file    SpeedProfile.cc
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

#include "addNode/SpeedProfile.h"
#include "logging/vlog.h"

namespace VENTOS {

Define_Module(VENTOS::SpeedProfile);

void SpeedProfile::initialize(int stage)
{
    if(stage == 0)
    {
        active = par("active").boolValue();
        if(!active)
            return;

        mode = par("mode").longValue();
        laneId = par("laneId").stringValue();
        vehId = par("vehId").stringValue();
        startTime = par("startTime").doubleValue();

        if(startTime < 0 && startTime != -1)
            throw omnetpp::cRuntimeError("startTime in SpeedProfile is not correct!");

        // get total vehicles from addMobileNode module
        cModule *module = omnetpp::getSimulation()->getSystemModule()->getSubmodule("addMobileNode");
        ASSERT(module);
        numVehicles = module->par("numVehicles").longValue();

        warmUp = par("warmUp").boolValue();
        stopPosition = par("stopPosition").doubleValue() * numVehicles;
        warmUpSpeed = par("warmUpSpeed").doubleValue();
        waitingTime = par("waitingTime").doubleValue();

        minSpeed = par("minSpeed").doubleValue();
        normalSpeed = par("normalSpeed").doubleValue();
        maxSpeed = par("maxSpeed").doubleValue();
        switchTime = par("switchTime").doubleValue();
        trajectoryPath = par("trajectoryPath").stringValue();

        // get a pointer to the TraCI module
        module = omnetpp::getSimulation()->getSystemModule()->getSubmodule("TraCI");
        TraCI = static_cast<TraCI_Commands *>(module);
        ASSERT(TraCI);

        Signal_executeEachTS = registerSignal("executeEachTS");
        omnetpp::getSimulation()->getSystemModule()->subscribe("executeEachTS", this);

        IsWarmUpFinished = false;

        if(warmUp)
            finishingWarmup = new omnetpp::cMessage("finishingWarmup", 1);

        old_speed = -1;
        lastProfileVehicle = "";
        fileOpened = false;
        endOfFile = false;
    }
}


void SpeedProfile::finish()
{
    // unsubscribe
    if(active)
        omnetpp::getSimulation()->getSystemModule()->unsubscribe("executeEachTS", this);
}


void SpeedProfile::handleMessage(omnetpp::cMessage *msg)
{
    if (warmUp && msg == finishingWarmup)
    {
        IsWarmUpFinished = true;
        LOG_EVENT << "t=" << omnetpp::simTime().dbl() << ": Warm-up phase finished. \n" << std::flush;
        delete msg;
    }
}


void SpeedProfile::receiveSignal(omnetpp::cComponent *source, omnetpp::simsignal_t signalID, long i, cObject* details)
{
    Enter_Method_Silent();

    if(signalID == Signal_executeEachTS)
    {
        // wait for startTime (-1: simulation start time)
        if(startTime == -1 || startTime <= omnetpp::simTime().dbl())
            eachTimeStep();
    }
}


void SpeedProfile::eachTimeStep()
{
    if(vehId == "")
    {
        // get a list of all vehicles on lane 'laneId'
        std::list<std::string> veh = TraCI->laneGetLastStepVehicleIDs(laneId);

        // return if no vehicle exist
        if(veh.empty())
            return;

        // get the first leading vehicle
        profileVehicle = veh.back();
    }
    else
    {
        // wait for the vehicle to enter the simulation
        static bool vehEntered = false;
        if(!vehEntered)
        {
            std::list<std::string> all = TraCI->vehicleGetIDList();

            // iterate over all vehicles
            for(auto &ii: all)
            {
                if(ii == vehId)
                {
                    vehEntered = true;
                    break;
                }
            }
        }

        if(!vehEntered)
            return;

        profileVehicle = vehId;
    }

    // if warm-up is active
    if(warmUp && !IsWarmUpFinished)
    {
        if(!finishingWarmup->isScheduled())
        {
            // perform warm-up for this time-step
            bool isFinished = DoWarmup();

            // if warm-up is finished, start speed-profiling
            if (isFinished)
                DoSpeedProfile();
        }
    }
    // do speed-profiling
    else
        DoSpeedProfile();
}


bool SpeedProfile::DoWarmup()
{
    static bool wasExecuted = false;
    if (!wasExecuted)
    {
        LOG_EVENT << "t=" << omnetpp::simTime().dbl() << ": Warm-up phase is started ... \n" << std::flush;
        wasExecuted = true;
    }

    double pos = TraCI->vehicleGetLanePosition(profileVehicle);

    // profileVehicle is at stop position
    if(pos >= stopPosition)
    {
        // set speed to warmupSpeed and wait for other vehicles
        TraCI->vehicleSetSpeed(profileVehicle, warmUpSpeed);

        // get # of vehicles that have entered simulation so far
        int n = TraCI->vehicleGetIDCount();

        // if all vehicles are in the simulation, then wait for waitingTime before finishing warm-up
        if(n == numVehicles)
        {
            scheduleAt(omnetpp::simTime() + waitingTime, finishingWarmup);
            LOG_EVENT << "t=" << omnetpp::simTime().dbl() << ": Waiting for " << waitingTime << "s before finishing warm-up ... \n" << std::flush;
        }
    }

    return false;
}


void SpeedProfile::DoSpeedProfile()
{
    static bool wasExecuted = false;
    if (!wasExecuted)
    {
        startTimeTrajectory = omnetpp::simTime().dbl();
        LOG_EVENT << "t=" << startTimeTrajectory << ": Speed profiling is started ... \n" << std::flush;
        wasExecuted = true;
    }

    // when the profileVehicle leaves the current lane, for the new profileVehicle,
    // speed profiling should re-start from the current simulation time.
    if(lastProfileVehicle != "" && profileVehicle != lastProfileVehicle)
        startTimeTrajectory = omnetpp::simTime().dbl();

    lastProfileVehicle = profileVehicle;

    if(omnetpp::simTime().dbl() == startTimeTrajectory)
    {
        // todo: profileVehicle is the first vehicle and is not running the car-following code
        // thus comfAccel and comfDeccel do not apply to it and uses the max accel/decel.
        TraCI->vehicleSetMaxAccel(profileVehicle, 2.);
        TraCI->vehicleSetMaxDecel(profileVehicle, 5.);
    }

    // #############################################
    // checking which SpeedProfile mode is selected?
    // #############################################

    // fixed speed
    if(mode == 0)
    {
        TraCI->vehicleSetMaxDecel(profileVehicle, 5.);
        TraCI->vehicleSetSpeed(profileVehicle, normalSpeed);
    }
    else if(mode == 1)
    {
        TraCI->vehicleSetMaxDecel(profileVehicle, 20.);
        TraCI->vehicleSetSpeed(profileVehicle, 0.);

        // todo: change vehicle color to gray! -- for HIL
        RGB newColor = Color::colorNameToRGB("gray");
        TraCI->vehicleSetColor(profileVehicle, newColor);
    }
    else if(mode == 2)
    {
        // start time, min speed, max speed
        AccelDecel(startTimeTrajectory, minSpeed, normalSpeed);
    }
    // extreme case
    else if(mode == 3)
    {
        AccelDecel(startTimeTrajectory, 0., maxSpeed);
    }
    // stability test
    else if(mode == 4)
    {
        // we change the maxDecel and maxAccel of trajectory to lower values (it does not touch the boundaries)
        // note1: when we change maxDecel or maxAccel of a vehicle, its type will change!
        // note2: we have to make sure the trajectory entered into the simulation, before changing MaxAccel and MaxDecel
        // note3: and we have to change maxDeccel and maxAccel only once!
        if( omnetpp::simTime().dbl() == startTimeTrajectory )
        {
            TraCI->vehicleSetMaxAccel(profileVehicle, 1.5);
            TraCI->vehicleSetMaxDecel(profileVehicle, 2.);
            return;
        }

        AccelDecel(startTimeTrajectory+5, minSpeed, normalSpeed);
    }
    else if(mode == 5)
    {
        AccelDecelZikZak(startTimeTrajectory, minSpeed, normalSpeed);
    }
    else if(mode == 6)
    {
        // A sin(Wt) + C
        // start time, offset (C), amplitude (A), omega (W)
        AccelDecelPeriodic(startTimeTrajectory, 10., 10., 0.2);
    }
    else if(mode == 7)
    {
        ExTrajectory(startTimeTrajectory);
    }
    // hysteresis
    else if(mode == 8)
    {
        if( omnetpp::simTime().dbl() == startTimeTrajectory )
        {
            TraCI->vehicleSetMaxAccel(profileVehicle, 1);
            TraCI->vehicleSetMaxDecel(profileVehicle, 1);
            return;
        }

        // after 5 seconds start AccelDecel
        AccelDecel(startTimeTrajectory+5, minSpeed, normalSpeed);
    }
    else
        throw omnetpp::cRuntimeError("not a valid speed profile mode!");
}


// trajectory like a pulse
void SpeedProfile::AccelDecel(double startT, double minV, double maxV)
{
    if( omnetpp::simTime().dbl() == startT )
    {
        TraCI->vehicleSetSpeed(profileVehicle, maxV);
        return;
    }

    double v = TraCI->vehicleGetSpeed(profileVehicle);

    if( old_speed != v )
    {
        old_speed = v;
        old_time = omnetpp::simTime().dbl();
    }
    // as soon as current speed is equal to old speed
    else
    {
        if(v == maxV)
        {
            // waiting time between speed change
            if(omnetpp::simTime().dbl() - old_time >= switchTime)
                TraCI->vehicleSetSpeed(profileVehicle, minV);
        }
        else if(v == minV)
        {
            // waiting time between speed change
            if(omnetpp::simTime().dbl() - old_time >= switchTime)
                TraCI->vehicleSetSpeed(profileVehicle, maxV);
        }
    }
}


void SpeedProfile::AccelDecelZikZak(double startT, double minV, double maxV)
{
    if( omnetpp::simTime().dbl() == startT )
    {
        TraCI->vehicleSetSpeed(profileVehicle, maxV);
        return;
    }

    double v = TraCI->vehicleGetSpeed(profileVehicle);

    if( old_speed != v )
        old_speed = v;
    // as soon as current speed is equal to old speed
    else
    {
        if(v == maxV)
            TraCI->vehicleSetSpeed(profileVehicle, minV);
        else if(v == minV)
            TraCI->vehicleSetSpeed(profileVehicle, maxV);
    }
}


void SpeedProfile::AccelDecelPeriodic(double startT, double offset, double A, double w)
{
    if( omnetpp::simTime().dbl() == startT )
        TraCI->vehicleSetSpeed(profileVehicle, offset);
    else if( omnetpp::simTime().dbl() < (startT + 10) )
        return;

    double t = omnetpp::simTime().dbl();
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
            throw omnetpp::cRuntimeError("external trajectory file does not exists! Check trajectoryPath variable.");

        fileOpened = true;
    }

    // if we have read all lines of the file, then return
    if(endOfFile)
        return;

    if( omnetpp::simTime().dbl() == startT )
    {
        // set the leading vehicle speed equal with the first speed of real trajectory data
        TraCI->vehicleSetSpeed(profileVehicle, 13.8);
        return;
    }
    else if( omnetpp::simTime().dbl() < 120 )
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
