
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

        // get the ptr of the AddVehicle module
        cModule *module = simulation.getSystemModule()->getSubmodule("addvehicle");
        AddVehiclePtr = static_cast<AddVehicle *>(module);
        if(AddVehiclePtr == NULL)
            error("can not get a pointer to the AddVehicle module.");

        // get the ptr of the SpeedProfile module
        module = simulation.getSystemModule()->getSubmodule("speedprofile");
        SpeedProfilePtr = static_cast<SpeedProfile *>(module);
        if(SpeedProfilePtr == NULL)
            error("can not get a pointer to the SpeedProfile module.");

        // get the ptr of the Warmup module
        module = simulation.getSystemModule()->getSubmodule("warmup");
        WarmupPtr = static_cast<Warmup *>(module);
        if(WarmupPtr == NULL)
            error("can not get a pointer to the Warmup module.");

        terminate = par("terminate").doubleValue();

        index = 1;
    }
}


void TraCI_App::handleSelfMsg(cMessage *msg)
{
    TraCI_Extend::handleSelfMsg(msg);
}


// this method is called once!
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

    // ---------------------------------------------

    char fName2 [50];

    if( ev.isGUI() )
    {
        sprintf (fName2, "%s.txt", "results/gui/loop-detector");
    }
    else
    {
        // get the current run number
        int currentRun = ev.getConfigEx()->getActiveRunNumber();
        sprintf (fName2, "%s_%d.txt", "results/cmd/loop-detector", currentRun);
    }

    f2 = fopen (fName2, "w");

    // write header
    fprintf (f2, "%-20s","timeStep");
    fprintf (f2, "%-20s","loopDetector");
    fprintf (f2, "%-20s","vehicleName");
    fprintf (f2, "%-20s","vehicleEntryTime");
    fprintf (f2, "%-20s","vehicleExitTime");
    fprintf (f2, "%-22s\n\n","vehicleSpeed");

    fflush(f2);

    // --------------------------------------------

    // add vehicles dynamically into SUMO
    AddVehiclePtr->Add();
}


void TraCI_App::executeOneTimestep()
{
    EV << "### Sending Command to SUMO to perform simulation for TS = " << (getCurrentTimeMs()/1000.) << std::endl;

    TraCIScenarioManager::executeOneTimestep();

    EV << "### SUMO completed simulation for TS = " << (getCurrentTimeMs()/1000.) << std::endl;

    // We write the parameters of all vehicles that are present now into the file
    writeToFile();

    // We write the status of induction loops into the file
    inductionLoops();

    if(simTime().dbl() >= terminate)
    {
        // send termination signal to statistics
        // statistic will perform some post-processing and
        // then terminates the simulation
        simsignal_t Signal_terminate = registerSignal("terminate");
        nodePtr->emit(Signal_terminate, 1);
    }

    bool finished = WarmupPtr->DoWarmup();

    // check if warm-up phase is finished
    if (finished)
    {
        // we can start speed profiling
        SpeedProfilePtr->Change();
    }
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


// todo:
void TraCI_App::inductionLoops()
{
    // get all loop detectors
    std::list<std::string> str = commandGetLoopDetectorList();

    // for each loop detector
    for (std::list<std::string>::iterator it=str.begin(); it != str.end(); ++it)
    {
        uint32_t n1 = commandGetLoopDetectorCount(*it);

        // only if this loop detector detected a vehicle
        if(n1 == 1)
        {
            fprintf ( f2, "%-20.2f ", simTime().dbl() );
            fprintf ( f2, "%-20s ", (*it).c_str() );
            fprintf ( f2, "%-20s ", commandGetLoopDetectorVehicleList(*it).front().c_str() );
            fprintf ( f2, "%-20.2f ", -1. );
            fprintf ( f2, "%-20.2f ", -1. );
            fprintf ( f2, "%-20.2f \n", commandGetLoopDetectorSpeed(*it) );
        }
    }

    fflush(f2);


  //  std::list<std::string> str2 = commandGetLoopDetectorVehicleData("detector1");
  //  for (std::list<std::string>::iterator it=str2.begin(); it != str2.end(); ++it)
  //      EV << "data ###########  " << *it;
}


void TraCI_App::finish()
{
    TraCI_Extend::finish();
    fclose(f1);
    fclose(f2);
}

