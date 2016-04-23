/****************************************************************************/
/// @file    codeLoader.cc
/// @author  Mani Amoozadeh <maniam@ucdavis.edu>
/// @author  second author name
/// @date    Apr 2016
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

#include "codeLoader.h"
#include <thread>
#include <fstream>
#include "dev.h"

namespace VENTOS {

Define_Module(VENTOS::codeLoader);

class remoteDev
{
public:
    std::string host;
    int port;
    std::string username;
    std::string password;

    remoteDev(std::string h, int p, std::string ur, std::string pass)
    {
        this->host = h;
        this->port = p;
        this->username = ur;
        this->password = pass;
    }
};


codeLoader::~codeLoader()
{

}


void codeLoader::initialize(int stage)
{
    super::initialize(stage);

    if(stage ==0)
    {
        on = par("on").boolValue();
        if(!on)
            return;

        // get a pointer to the TraCI module
        cModule *module = simulation.getSystemModule()->getSubmodule("TraCI");
        ASSERT(module);
        TraCI = static_cast<TraCI_Commands *>(module);
        ASSERT(TraCI);

        Signal_initialize_withTraCI = registerSignal("initialize_withTraCI");
        simulation.getSystemModule()->subscribe("initialize_withTraCI", this);

        Signal_executeEachTS = registerSignal("executeEachTS");
        simulation.getSystemModule()->subscribe("executeEachTS", this);

        // construct the path to redpineAppl folder
        boost::filesystem::path VENTOS_FullPath = cSimulation::getActiveSimulation()->getEnvir()->getConfig()->getConfigEntry("network").getBaseDirectory();
        redpineAppl_FullPath = VENTOS_FullPath / "src" / "redpineAppl";

        initScriptName = par("initScriptName").stringValue();
        if(initScriptName == "")
            error("initScriptName is empty!");

        remoteDir_Driver = par("remoteDir_Driver").stringValue();
        if(remoteDir_Driver == "")
            error("remoteDir_Driver is empty!");

        remoteDir_SourceCode = par("remoteDir_SourceCode").stringValue();
        if(remoteDir_SourceCode == "")
            error("remoteDir_SourceCode is empty!");
    }
}


void codeLoader::finish()
{
    for(auto &i : IMX_board)
        if(i) delete i;
}


void codeLoader::handleMessage(cMessage *msg)
{

}


void codeLoader::receiveSignal(cComponent *source, simsignal_t signalID, long i)
{
    if(!on)
        return;

    Enter_Method_Silent();

    if(signalID == Signal_initialize_withTraCI)
    {
        initialize_withTraCI();
    }
    else if(signalID == Signal_executeEachTS)
    {
        executeEachTimestep();
    }
}


void codeLoader::initialize_withTraCI()
{

}


void codeLoader::executeEachTimestep()
{
    // run this code only once
    static bool wasExecuted = false;
    if (on && !wasExecuted)
    {
        make_connection();

        // launch a child thread for each remote dev
        std::vector<std::thread> workers;
        for(auto &ii : IMX_board)
            workers.push_back( std::thread (&codeLoader::init_board, this, ii) );

        // wait for all threads to finish
        std::for_each(workers.begin(), workers.end(), [](std::thread &t) {
            t.join();
        });


        std::cout << "yeayy! \n" << std::flush;

        wasExecuted = true;
    }
}


void codeLoader::make_connection()
{
    int numDev = par("numDev").longValue();
    if(numDev < 0)
        error("numDev should be >= 0");
    else if(numDev == 0)
        return;

    printf("\n");
    printf(">>> Connecting to '%u' remote devices ... \n", numDev);
    std::cout.flush();

    cModule* parentMod = getParentModule();
    if (!parentMod)
        error("Parent Module not found");

    cModuleType* nodeType = cModuleType::get("VENTOS.src.redpineAppl.dev");

    for(int i = 0; i < numDev; i++)
    {
        cModule* mod = nodeType->create("dev", parentMod, numDev, i);
        mod->finalizeParameters();
        mod->getDisplayString().updateWith("i=old/x_yellow");
        mod->getDisplayString().updateWith("p=0,0");
        mod->buildInside();
        mod->scheduleStart(simTime());
        mod->callInitialize();
    }

    std::vector<remoteDev> remoteDevs;
    for(int i = 0; i < numDev; i++)
    {
        // get a reference to dev
        cModule *module = simulation.getSystemModule()->getSubmodule("dev", i);
        ASSERT(module);

        std::string host = module->par("host");
        int port = module->par("port");
        std::string username = module->par("username");
        std::string password = module->par("password");

        remoteDev *dev = new remoteDev(host, port, username, password);
        remoteDevs.push_back(*dev);
    }

    // vector container to store threads
    std::vector<std::thread> workers;

    // create SSH connection to all remote devices at the same time
    for(auto &ii : remoteDevs)
    {
        workers.push_back(std::thread([=]() {  // pass by value
            SSH *new_session = new SSH(ii.host, ii.port, ii.username, ii.password);
            ASSERT(new_session);
            IMX_board.push_back(new_session);
        }));
    }

    // wait for all threads to finish
    std::for_each(workers.begin(), workers.end(), [](std::thread &t) {
        t.join();
    });

    std::cout << "    Done! \n\n";
    std::cout.flush();
}


void codeLoader::init_board(SSH *board)
{
    // make sure the pointer is valid
    ASSERT(board);

    printf(">>> Re-booting device @%s ... Please wait \n\n", board->getHostName().c_str());
    std::cout.flush();

    double duration_ms = rebootDev(board, 50000 /*timeout in ms*/);

    printf(">>> Device @%s is up and running! Boot time ~ %.2f seconds. Reconnecting ... \n\n", board->getHostName().c_str(), duration_ms / 1000.);
    std::cout.flush();

    // re-connect to the dev
    board = new SSH(board->getHostName(), board->getPort(), board->getUsername(), board->getPassword(), false);
    ASSERT(board);

    //#################################################
    // Step 1: copy the init script to remoteDir_Driver
    //#################################################
    boost::filesystem::path script_FullPath = redpineAppl_FullPath / initScriptName;

    printf(">>> Copying the init script to %s:%s ... \n\n", board->getHostName().c_str(), remoteDir_Driver.c_str());
    std::cout.flush();

    // read file contents into a string
    std::ifstream ifs(script_FullPath.c_str());
    std::string content( (std::istreambuf_iterator<char>(ifs) ),
            (std::istreambuf_iterator<char>()    ) );

    // replace HW parameters in the init script
    substituteParams(board->getHostName(), content);

    // do the copying
    board->copyFileStr_SFTP(initScriptName, content, remoteDir_Driver);

    //########################################################
    // Step 2: run the script remotely in the remoteDir_Driver
    //########################################################
    printf(">>> Running the init script at %s ... \n\n", board->getHostName().c_str());
    std::cout.flush();

    board->run_command("cd " + remoteDir_Driver.string());
    board->run_command("sudo ./" + initScriptName, true);

    //######################################
    // Step 3: start 1609 stack in WAVE mode
    //######################################
    printf(">>> Start 1609 stack in WAVE mode at %s ... \n\n", board->getHostName().c_str());
    std::cout.flush();

    board->run_command("cd " + remoteDir_Driver.string());
    board->run_command("sudo ./rsi_1609", true);



}


double codeLoader::rebootDev(SSH *board, int timeOut)
{
    if(timeOut <= 0)
        throw cRuntimeError("timeOut value is wrong!");

    typedef std::chrono::high_resolution_clock::time_point Htime_t;

    // start measuring boot time here
    Htime_t startBoot = std::chrono::high_resolution_clock::now();

    board->sendReboot();

    Htime_t startPing = std::chrono::high_resolution_clock::now();

    // keep pinging dev
    bool disconnected = false;
    while(true)
    {
        // wait for 100 ms
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        std::string cmd = "ping -c 1 -s 1 " + board->getHostAddress() + " > /dev/null 2>&1";
        int result = system(cmd.c_str());
        if(!disconnected && result != 0)
            disconnected = true;

        if(disconnected && result == 0)
            break;

        // waiting too long for dev to boot?
        Htime_t currentTime = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> fp_ms = currentTime - startPing;
        if(fp_ms.count() > timeOut)
            throw cRuntimeError("dev reboot timeout!");
    }

    // end measuring boot time here
    Htime_t endBoot = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> fp_ms = endBoot - startBoot;

    return fp_ms.count();
}


void codeLoader::substituteParams(std::string host, std::string &content)
{
    // looking for the dev
    cModule *module = simulation.getSystemModule()->getSubmodule("dev", 0);
    if(module == NULL)
        throw cRuntimeError("No dev module exists!");

    // how many devs are in the network?
    int numDev = module->getVectorSize();

    // iterate over modules
    for(int i = 0; i < numDev; ++i)
    {
        // get a pointer to this dev
        module = simulation.getSystemModule()->getSubmodule("dev", i);

        // get host of this dev
        std::string devHost = module->par("host").stringValue();

        // found our dev with matching host
        if(devHost == host)
        {
            // get a class pointer
            dev *devPtr = static_cast<dev *>(module);
            ASSERT(devPtr);

            // ask the dev to substitute its parameters for us
            devPtr->substituteParams(content);

            return;
        }
    }

    throw cRuntimeError("Cannot find dev with host %s", host.c_str());
}

}

