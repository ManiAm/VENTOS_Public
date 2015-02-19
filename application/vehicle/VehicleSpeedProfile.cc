
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
        // get the ptr of the current module
        nodePtr = FindModule<>::findHost(this);
        if(nodePtr == NULL)
            error("can not get a pointer to the module.");

        // get a pointer to the TraCI module
        cModule *module = simulation.getSystemModule()->getSubmodule("TraCI");
        TraCI = static_cast<TraCI_Extend *>(module);

        on = par("on").boolValue();
        laneId = par("laneId").stringValue();
        mode = par("mode").longValue();
        minSpeed = par("minSpeed").doubleValue();
        normalSpeed = par("normalSpeed").doubleValue();
        maxSpeed = par("maxSpeed").doubleValue();
        switchTime = par("switchTime").doubleValue();
        startTime = par("startTime").doubleValue();
        trajectoryPath = par("trajectoryPath").stringValue();

        old_speed = -1;
        lastProfileVehicle = "";

        if(mode == 6)
        {
            f2 = fopen (trajectoryPath.c_str(), "r");

            if ( f2 == NULL )
                error("external trajectory file does not exists! Check trajectoryPath variable.");

            endOfFile = false;
        }
    }
}


void SpeedProfile::finish()
{
    if(mode == 6)
        fclose(f2);
}


void SpeedProfile::handleMessage(cMessage *msg)
{

}


void SpeedProfile::Change()
{
    // if speedProfiling is not on, return
    if (!on)
        return;

    // upon first call:
    // a) if startTime is not specified, then store the current simulation time as startTime
    if(startTime == -1)
    {
        startTime = simTime().dbl();
    }
    // b) if user specifies a startTime, then wait for it!
    else if(simTime().dbl() < startTime)
        return;

    if(startTime < 0)
        error("startTime is less than 0 in SpeedProfile.");

    // who is leading?
    list<string> veh = TraCI->commandGetLaneVehicleList(laneId);

    if(veh.empty())
        return;

    profileVehicle = veh.back();

    // when the profileVehicle leaves the current lane, for the new profileVehicle,
    // speed profiling should re-start from current simulation time.
    if(lastProfileVehicle != "" && profileVehicle != lastProfileVehicle)
    {
        startTime = simTime().dbl();
    }

    lastProfileVehicle = profileVehicle;


    // #############################################
    // checking which SpeedProfile mode is selected?
    // #############################################

    // fixed speed
    if(mode == 0)
    {
        TraCI->commandChangeVehicleSpeed(profileVehicle, normalSpeed);
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
            TraCI->commandSetVehicleMaxAccel(profileVehicle, 1.5);
            TraCI->commandSetVehicleMaxDecel(profileVehicle, 2.);
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
    else
    {
        error("not a valid mode!");
    }
}


// trajectory like a pulse
void SpeedProfile::AccelDecel(double startT, double minV, double maxV)
{
    if( simTime().dbl() < startT )
    {
        return;
    }
    else if( simTime().dbl() == startT )
    {
        TraCI->commandChangeVehicleSpeed(profileVehicle, maxV);
        return;
    }

    double v = TraCI->commandGetVehicleSpeed(profileVehicle);

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
            // waiting time between speed change (default is 40 s)
            if(simTime().dbl() - old_time >= switchTime)
            {
                TraCI->commandChangeVehicleSpeed(profileVehicle, minV);
            }
        }
        else if(v == minV)
        {
            // waiting time between speed change (default is 40 s)
            if(simTime().dbl() - old_time >= switchTime)
            {
                TraCI->commandChangeVehicleSpeed(profileVehicle, maxV);
            }
        }
    }
}


void SpeedProfile::AccelDecelZikZak(double startT, double minV, double maxV)
{
    if( simTime().dbl() < startT )
    {
        return;
    }
    else if( simTime().dbl() == startT )
    {
        TraCI->commandChangeVehicleSpeed(profileVehicle, maxV);
        return;
    }

    double v = TraCI->commandGetVehicleSpeed(profileVehicle);

    if( old_speed != v )
    {
        old_speed = v;
    }
    // as soon as current speed is equal to old speed
    else
    {
        if(v == maxV)
        {
            TraCI->commandChangeVehicleSpeed(profileVehicle, minV);
        }
        else if(v == minV)
        {
            TraCI->commandChangeVehicleSpeed(profileVehicle, maxV);
        }
    }
}


void SpeedProfile::AccelDecelPeriodic(double startT, double offset, double A, double w)
{
    if( simTime().dbl() < startT )
    {
        return;
    }
    else if( simTime().dbl() == startT )
    {
        TraCI->commandChangeVehicleSpeed(profileVehicle, offset);
    }
    else if( simTime().dbl() < (startT + 10) )
    {
        return;
    }

    double t = simTime().dbl();
    double newSpeed = offset + A * sin(w * t);

    TraCI->commandChangeVehicleSpeed(profileVehicle, newSpeed);
}


void SpeedProfile::ExTrajectory(double startT)
{
    // if we have read all lines of the file, then return
    if(endOfFile)
        return;

    if( simTime().dbl() < startT )
    {
        return;
    }
    else if( simTime().dbl() == startT )
    {
        // TraCI->commandSetSpeed(profileVehicle, 13.86);
        TraCI->commandChangeVehicleSpeed("CACC1", 20.);
        return;
    }
    else if( simTime().dbl() < 80 )
    {
        return;
    }

    char line [20];  // maximum line size
    char *result = fgets (line, sizeof line, f2);

    if (result == NULL)
    {
        endOfFile = true;
        return;
    }

    TraCI->commandChangeVehicleSpeed(profileVehicle, atof(line));
}

} // end of namespace

