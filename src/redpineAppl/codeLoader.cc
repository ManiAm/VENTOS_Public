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
        start();
        wasExecuted = true;
    }
}


void codeLoader::start()
{
    make_connection();

    // call init_board for each board in a separate child thread
    std::vector<std::thread> workers;
    for(auto &ii : IMX_board)
        workers.push_back( std::thread (&codeLoader::init_board, this, ii) );

    // wait for all threads to finish
    std::for_each(workers.begin(), workers.end(), [](std::thread &t) {
        t.join();
    });



    std::cout << "yeayy! \n" << std::flush;
}


void codeLoader::make_connection()
{
    int numDev = par("numDev").longValue();
    if(numDev < 0)
        error("numDev should be >= 0");
    else if(numDev == 0)
        return;

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

    printf("\n");
    printf(">>> Connecting to '%u' remote devices ... \n", numDev);
    std::cout.flush();

    // vector container to store threads
    std::vector<std::thread> workers;

    // create SSH connection to all remote devices at the same time
    for(int i = 0; i < numDev; i++)
    {
        // get a reference to dev
        cModule *module = simulation.getSystemModule()->getSubmodule("dev", i);
        ASSERT(module);

        workers.push_back(std::thread([=]() {  // pass by value
            SSH *new_session = new SSH(module->par("host").stringValue(),
                    module->par("port").longValue(),
                    module->par("username").stringValue(),
                    module->par("password").stringValue());

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

    //######################################################
    // Step 0: reboot the device -- todo: remove this later!
    //######################################################
    printf(">>> Re-booting device @%s ... Please wait \n\n", board->getHostName().c_str());
    std::cout.flush();

    // create a shell
    ssh_channel rebootShell = board->openShell();

    double duration_ms = board->rebootDev(rebootShell, 40000 /*timeout in ms*/);

    // close the shell
    board->closeShell(rebootShell);

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

    // get a class pointer
    cModule *module = findDev(board);
    ASSERT(module);
    dev *devPtr = static_cast<dev *>(module);
    ASSERT(devPtr);

    // ask dev to substitute its parameters in the init script
    devPtr->substituteParams(content);

    // do the copying
    board->copyFileStr_SFTP(initScriptName, content, remoteDir_Driver);

    //##################################################################
    // Step 2: copying new/modified source codes to remoteDir_SourceCode
    //##################################################################
    printf(">>> Syncing source codes with %s ... \n\n", board->getHostName().c_str());
    std::cout.flush();

    boost::filesystem::path sampleAppl_FullPath = redpineAppl_FullPath / "sampleAppl";
    //board->syncDir(sampleAppl_FullPath, remoteDir_SourceCode);  // todo

    //##################################
    // Step 3: remotely compile the code
    //##################################
    // get the application name
    module = findDev(board);
    ASSERT(module);
    std::string applName = module->par("applName").stringValue();
    if(applName == "")
        throw cRuntimeError("applName is empty!");

    printf(">>> Compiling %s @%s ... \n\n", applName.c_str(), board->getHostName().c_str());
    std::cout.flush();

    // create a shell for compiling the code
    ssh_channel compileShell = board->openShell();

    // compile the application source code
    board->run_command(compileShell, "cd " + remoteDir_SourceCode.string(), false);
    board->run_command(compileShell, "gcc " + applName + ".c " + "-o " + applName + " ./rsi_wave_api/lib_rsi_wave_api.a" + " ./dsrc/libdsrc.a" + " -I ./dsrc/includes/" + " -lpthread", true);

    // close the shell
    board->closeShell(compileShell);

    //########################################################
    // Step 4: remotely run the script in the remoteDir_Driver
    //########################################################
    printf(">>> Running the init script @%s ... \n\n", board->getHostName().c_str());
    std::cout.flush();

    // create a shell for running the init script
    ssh_channel driverShell = board->openShell();

    board->run_command(driverShell, "sudo su", false);
    board->run_command(driverShell, "cd " + remoteDir_Driver.string(), false);
    board->run_command(driverShell, "./" + initScriptName, false);

    // close the shell
    board->closeShell(driverShell);

    //###############################################
    // Step 5: remotely start 1609 stack in WAVE mode
    //###############################################
    printf(">>> Start 1609 stack in WAVE mode @%s ... \n\n", board->getHostName().c_str());
    std::cout.flush();

    // create a shell for running the 1609
    ssh_channel rsi1609Shell = board->openShell();

    board->run_command(rsi1609Shell, "sudo su", false);
    board->run_command(rsi1609Shell, "cd " + remoteDir_Driver.string(), false);
    board->run_command(rsi1609Shell, "if ! pgrep rsi_1609 > /dev/null; then ./rsi_1609; fi", false);  // should not close the shell

    //##############################
    // Step 6: remotely run the code
    //##############################
    printf(">>> Running %s @%s ... \n\n", applName.c_str(), board->getHostName().c_str());
    std::cout.flush();

    // create a shell for running the code
    ssh_channel applShell = board->openShell();

    board->run_command(applShell, "sudo su", false);
    board->run_command(applShell, "cd " + remoteDir_SourceCode.string(), false);
    board->run_command(applShell, "./" + applName, true);  // should not close the shell
}


cModule * codeLoader::findDev(SSH *board)
{
    ASSERT(board);

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
        if(devHost == board->getHostName())
            return module;
    }

    return NULL;
}

}

