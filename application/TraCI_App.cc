
#include "TraCI_App.h"

#include <sstream>
#include <iostream>
#include <fstream>

Define_Module(TraCI_App);


TraCI_App::~TraCI_App()
{

}


void TraCI_App::initialize(int stage)
{
    TraCI_Extend::initialize(stage);

    if(stage ==0)
    {
        // get the ptr of the current module
        nodePtr = FindModule<>::findHost(this);
        if(nodePtr == NULL)
            error("can not get a pointer to the module.");

        trajectoryMode = par("trajectoryMode").longValue();
        trajectory = par("trajectory").stringValue();
        terminate = par("terminate").doubleValue();

        f2 = fopen ("sumo/EX_Trajectory.txt", "r");

        if ( f2 == NULL )
            error("external trajectory file does not exists!");

        reached = false;
        index = 1;
        endOfFile = false;
        old_speed = -1;
    }
}


void TraCI_App::init_traci()
{
    TraCI_Extend::init_traci();

    // making sure that platoonLeader exists in the sumo


    // ---------------------------------------------

    char fName [50];

    if( ev.isGUI() )
    {
        sprintf (fName, "%s.txt", "results/gui/speed-gap");
    }
    else
    {
        // get the current run number
        int currentRun = ev.getConfigEx()->getActiveRunNumber();
        sprintf (fName, "%s_%d.txt", "results/cmd/speed-gap", currentRun);
    }

    f1 = fopen (fName, "w");

    // write header
    fprintf (f1, "%-10s","index");
    fprintf (f1, "%-10s","vehicle");
    fprintf (f1, "%-12s","timeStep");
    fprintf (f1, "%-10s","speed");
    fprintf (f1, "%-12s","accel");
    fprintf (f1, "%-12s","pos");
    fprintf (f1, "%-10s","gap");
    fprintf (f1, "%-10s\n\n","timeGap");

    fflush(f1);
}


void TraCI_App::executeOneTimestep()
{
    EV << "### Sending Command to SUMO to perform simulation for TS = " << (getCurrentTimeMs()/1000.) << std::endl;

    TraCIScenarioManager::executeOneTimestep();

    EV << "### SUMO completed simulation for TS = " << (getCurrentTimeMs()/1000.) << std::endl;

    // We write the parameters of all vehicles that are present now into the file
    writeToFile();

    if(simTime().dbl() >= terminate)
    {
        // send termination signal to statistics
        // statistic will perform some post-processing and
        // then terminates the simulation
        simsignal_t Signal_terminate = registerSignal("terminate");
        nodePtr->emit(Signal_terminate, 1);
    }

    // do the trajectory
    Trajectory();
}


void TraCI_App::writeToFile()
{
    std::string vleaderID = "";

    // get all lanes in the network
    std::list<std::string> list = commandGetLaneList();

    for(std::list<std::string>::iterator i = list.begin(); i != list.end(); ++i)
    {
        // get all vehicles on lane i
        std::list<std::string> list2 = commandGetVehicleLaneList( i->c_str() );

        for(std::list<std::string>::reverse_iterator k = list2.rbegin(); k != list2.rend(); ++k)
        {
            std::string vID = k->c_str();
            writeToFilePerVehicle(vID, vleaderID);
            vleaderID = k->c_str();
        }
    }

    // increase index after writing data for all vehicles
    if (commandGetNoVehicles() > 0)
        index++;
}


// vID is current vehicle, vleaderID is the leader (if present)
void TraCI_App::writeToFilePerVehicle(std::string vID, std::string vleaderID)
{
    double speed = commandGetVehicleSpeed(vID);
    double pos = commandGetLanePosition(vID);
    double accel = commandGetVehicleAccel(vID);

    // calculate gap (if leading is present)
    double gap = -1;

    if(vleaderID != "")
    {
        gap = commandGetLanePosition(vleaderID) - commandGetLanePosition(vID) - commandGetVehicleLength(vleaderID);
    }

    // calculate timeGap (if leading is present)
    double timeGap = -1;

    if(vleaderID != "" && speed != 0)
        timeGap = gap / speed;

    // write the current vehicle data into file
    fprintf (f1, "%-10d ", index);
    fprintf (f1, "%-10s ", vID.c_str());
    fprintf (f1, "%-10.2f ", ( simTime()-updateInterval).dbl() );
    fprintf (f1, "%-10.2f ", speed);
    fprintf (f1, "%-10.2f ", accel);
    fprintf (f1, "%-10.2f ", pos);
    fprintf (f1, "%-10.2f ", gap);
    fprintf (f1, "%-10.2f \n", timeGap);

    fflush(f1);
}


void TraCI_App::Trajectory()
{
    if(!reached)
    {
        double pos = commandGetLanePosition(trajectory);

        // stop at x = 100, waiting for other vehicles
        if(pos >= 100)
        {
            commandSetSpeed(trajectory, 0.);
            reached = true;
        }
    }
    // trajectory can start after 40 s
    else
    {
        if(trajectoryMode == 0)
        {
            // no trajectory
        }
        else if(trajectoryMode == 1)
        {
            AccelDecel(5.0);
        }
        else if(trajectoryMode == 2)
        {
            AccelDecel(0.);
        }
        // stability test
        else if(trajectoryMode == 3)
        {
            // todo:
            // we change the maxDecel and maxAccel of trajectory to lower values (it does not touch the boundaries)
            // note: when we change maxDecel or maxAccel of a vehicle, its type will be changes!
            // we have to make sure the trajectory entered into the simulation
            // and we have to change maxDeccel and maxAccel only once
            if(simTime().dbl() == 35)
            {
                commandSetMaxAccel(trajectory, 1.5);
                commandSetMaxDecel(trajectory, 2.);
            }

            AccelDecel(5.0);
        }
        else if(trajectoryMode == 4)
        {
            AccelDecelZikZak(5., 20.);
        }
        else if(trajectoryMode == 5)
        {
            // param 1: offset
            // param 2: amplitude
            // param 3: omega
            AccelDecelPeriodic(10., 10., 0.2);
        }
        else if(trajectoryMode == 6)
        {
            ExTrajectory();
        }
    }
}


void TraCI_App::AccelDecel(double speed)
{
    if( simTime().dbl() == 40 )
    {
        commandSetSpeed(trajectory, 20.);
    }
    else if(simTime().dbl() == 80)
    {
        commandSetSpeed(trajectory, speed);
    }
    else if(simTime().dbl() == 110)
    {
        commandSetSpeed(trajectory, 20.);
    }
    else if(simTime().dbl() == 150)
    {
        commandSetSpeed(trajectory, speed);
    }
}


void TraCI_App::AccelDecelZikZak(double down, double up)
{
    if( simTime().dbl() == 40 )
    {
        commandSetSpeed(trajectory, up);
        return;
    }

    double v = commandGetVehicleSpeed(trajectory);

    if( old_speed != v )
    {
        old_speed = v;
    }
    // as soon as current speed is equal to old speed
    //
    else
    {
        if(v == down)
        {
            commandSetSpeed(trajectory, up);
        }
        else if(v == up)
        {
            commandSetSpeed(trajectory, down);
        }
    }
}


void TraCI_App::AccelDecelPeriodic(double offset, double A, double w)
{
    if( simTime().dbl() == 40 )
    {
        commandSetSpeed(trajectory, offset);
    }
    else if( simTime().dbl() < 50 )
    {
        return;
    }

    double t = simTime().dbl();
    double newSpeed = offset + A * sin(w * t);

    commandSetSpeed(trajectory, newSpeed);
}


void TraCI_App::ExTrajectory()
{
    // if we have read all lines of the file
    // return from this method
    if(endOfFile)
        return;

    if( simTime().dbl() == 40 )
    {
        commandSetSpeed(trajectory, 13.86);
    }
    else if(simTime().dbl() < 80)
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

    commandSetSpeed(trajectory, atof(line));
}


void TraCI_App::finish()
{
    TraCI_Extend::finish();

    fclose(f1);
    fclose(f2);
}

