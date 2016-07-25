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
#include "vlog.h"
#include "dev.h"

#include <thread>
#include <fstream>

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
        active = par("active").boolValue();
        if(!active)
            return;

        // get a pointer to the TraCI module
        cModule *module = omnetpp::getSimulation()->getSystemModule()->getSubmodule("TraCI");
        ASSERT(module);
        TraCI = static_cast<TraCI_Commands *>(module);
        ASSERT(TraCI);

        Signal_initialize_withTraCI = registerSignal("initialize_withTraCI");
        omnetpp::getSimulation()->getSystemModule()->subscribe("initialize_withTraCI", this);

        Signal_executeEachTS = registerSignal("executeEachTS");
        omnetpp::getSimulation()->getSystemModule()->subscribe("executeEachTS", this);

        // construct the path to redpineAppl folder
        boost::filesystem::path VENTOS_FullPath = omnetpp::getEnvir()->getConfig()->getConfigEntry("network").getBaseDirectory();
        redpineAppl_FullPath = VENTOS_FullPath / "src" / "redpineAppl";

        remoteDir_Driver = par("remoteDir_Driver").stringValue();
        if(remoteDir_Driver == "")
            throw omnetpp::cRuntimeError("remoteDir_Driver is empty!");

        remoteDir_SourceCode = par("remoteDir_SourceCode").stringValue();
        if(remoteDir_SourceCode == "")
            throw omnetpp::cRuntimeError("remoteDir_SourceCode is empty!");

        numDev = par("numDev").longValue();
        if(numDev < 0)
            throw omnetpp::cRuntimeError("numDev should be >= 0");

        // add remote device modules into omnet
        addDevs();
    }
}


void codeLoader::finish()
{
    // delete all active SSH sessions
    for(auto &i : active_SSH)
    {
        if(i.second)
            delete i.second;
    }
}


void codeLoader::handleMessage(omnetpp::cMessage *msg)
{

}


void codeLoader::receiveSignal(omnetpp::cComponent *source, omnetpp::simsignal_t signalID, long i, cObject* details)
{
    if(!active)
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
    if (active && !wasExecuted)
    {
        make_connection();
        wasExecuted = true;
    }
}


void codeLoader::addDevs()
{
    if(numDev == 0)
        return;

    cModule* parentMod = getParentModule();
    if (!parentMod)
        throw omnetpp::cRuntimeError("Parent Module not found");

    omnetpp::cModuleType* nodeType = omnetpp::cModuleType::get("VENTOS.src.redpineAppl.dev");

    for(int i = 0; i < numDev; i++)
    {
        cModule* mod = nodeType->create("dev", parentMod, numDev, i);

        mod->finalizeParameters();
        mod->getDisplayString().updateWith("i=old/x_yellow");
        mod->getDisplayString().updateWith("p=0,0");
        mod->buildInside();
        mod->scheduleStart(omnetpp::simTime());
        // mod->callInitialize();   // no need to call initialize at this point
    }
}


void codeLoader::make_connection()
{
    if(numDev == 0)
        return;

    // make connection to each device in a separate child thread
    std::vector<std::thread> workers;
    for(int ii = 0; ii < numDev; ii++)
    {
        cModule *module = omnetpp::getSimulation()->getSystemModule()->getSubmodule("dev", ii);
        ASSERT(module);

        std::string host = module->par("host").stringValue();
        int port = module->par("port").longValue();
        std::string username = module->par("username").stringValue();
        std::string password = module->par("password").stringValue();

        if(host == "")
            throw omnetpp::cRuntimeError("host is empty for dev %d", ii);

        if(port <= 0)
            throw omnetpp::cRuntimeError("port number is invalid for dev %d", ii);

        if(username == "")
            throw omnetpp::cRuntimeError("username is empty for dev %d", ii);

        workers.push_back(std::thread([=]() {  // pass by value

            LOG_EVENT_C(host, "default") << boost::format("===[ Connecting to %1% ... ]=== \n\n") % host << std::flush;

            // create SSH connection to the dev
            SSH_Helper *board = new SSH_Helper(host, port, username, password, true, host, "default");
            ASSERT(board);

            {
                std::lock_guard<std::mutex> lock(lock_vector);
                active_SSH.push_back( std::make_pair(module, board) );
            }

            // adding a new line to improve readability
            LOG_EVENT_C(host, "default") << "\n" << std::flush;

        }));
    }

    // wait for all threads to finish
    std::for_each(workers.begin(), workers.end(), [](std::thread &t) {
        t.join();
    });

    // make sure all dev are connected
    ASSERT(active_SSH.size() == (unsigned int)numDev);

    // call init_board for each device in a separate child thread
    workers.clear();
    for(auto &ii : active_SSH)
    {
        // todo: make sure that the remote board is redpine!
        workers.push_back( std::thread (&codeLoader::init_board, this, ii.first, ii.second) );
    }

    // wait for all threads to finish
    std::for_each(workers.begin(), workers.end(), [](std::thread &t) {
        t.join();
    });

    LOG_EVENT << ">>> Number of active SSH connections is " << active_SSH.size() << "\n";

    // loop through all active SSH connections
    for(auto &ii : active_SSH)
    {
        LOG_EVENT << "    " << ii.second->getHostName() << ": ";

        // delete this SSH sessions if it has no active threads
        if(ii.second->getNumActiveThreads() == 0)
        {
            LOG_EVENT << "No running command. Terminating this SSH session... ";

            delete ii.second;
            ii.second = NULL;  // make SSH pointer NULL

            LOG_EVENT << "Done! \n";
        }
        else
            LOG_EVENT << ii.second->getNumActiveThreads() << " commands are running. \n";
    }

    LOG_FLUSH;
}


void codeLoader::init_board(cModule *module, SSH_Helper *board)
{
    ASSERT(module);
    ASSERT(board);

    //###########################
    // should we reboot this dev?
    //###########################

    if(module->par("rebootAtStart").boolValue())
    {
        LOG_EVENT_C(board->getHostName(), "default") << "===[ Re-booting device ... ]=== \n\n" << std::flush;

        // open a shell and get sudo access
        ssh_channel rebootShell = board->openShell("rebootShell");
        board->checkSudo(rebootShell);

        LOG_EVENT_C(board->getHostName(), "default") << "    Sending the reboot command ... Please wait \n" << std::flush;
        double duration_ms = board->rebootDev(rebootShell, 40000 /*max waiting time*/);
        board->closeShell(rebootShell);  // close the shell

        LOG_EVENT_C(board->getHostName(), "default") << boost::format("    Device is up and running! Boot time ~ %1% seconds \n") % (duration_ms / 1000.) << std::flush;

        // previous SSH connection is lost. Re-connect to the dev
        LOG_EVENT_C(board->getHostName(), "default") << boost::format("    Reconnecting to %1% ... \n\n") % board->getHostName() << std::flush;
        LOG_EVENT_C(board->getHostName(), "default") << boost::format("===[ Connecting to %1% ... ]=== \n\n") % board->getHostName() << std::flush;

        board = new SSH_Helper(board->getHostName(), board->getPort(), board->getUsername(), board->getPassword(), true, board->getHostName(), "default");
        ASSERT(board);

        // adding new line for readability
        LOG_EVENT_C(board->getHostName(), "default") << "\n" << std::flush;
    }

    //#########################################
    // opening shells and acquiring sudo access
    //#########################################

    LOG_EVENT_C(board->getHostName(), "default") << "===[ Opening shells and acquiring sudo access ... ]=== \n\n" << std::flush;

    ssh_channel shell1 = board->openShell("shell1");
    board->checkSudo(shell1);

    ssh_channel shell2 = board->openShell("shell2");
    board->checkSudo(shell2);

    // adding new line for readability
    LOG_EVENT_C(board->getHostName(), "default") << "\n" << std::flush;

    //##########################################################
    // copying new/modified source codes to remoteDir_SourceCode
    //##########################################################

    LOG_EVENT_C(board->getHostName(), "default") << boost::format("===[ Syncing source codes with %1% ... ]=== \n\n") % remoteDir_SourceCode << std::flush;

    board->createDir(remoteDir_SourceCode);  // create a directory to store all our source codes

    board->syncDir(redpineAppl_FullPath / "headers", remoteDir_SourceCode);  // copy local folder 'headers'
    board->syncDir(redpineAppl_FullPath / "sampleAppl", remoteDir_SourceCode);  // copy local folder 'sampleAppl'
    board->syncDir(redpineAppl_FullPath / "libs", remoteDir_SourceCode);  // create local folder 'libs'

    //##########################
    // remotely compile the code
    //##########################

    std::string cmdLine = module->par("applName").stringValue();

    // separating application name from command-line arguments
    std::istringstream iss(cmdLine, std::istringstream::in);
    std::string word;
    std::string applName = "";
    std::string arguments = "";
    int counter = 0;
    while(iss >> word)
    {
        counter++;
        if(counter == 1)
            applName = word;
        else
            arguments = arguments + " " + word;
    }

    // make sure application name is not empty
    if(applName == "")
        throw omnetpp::cRuntimeError("applName is empty!");

    LOG_EVENT_C(board->getHostName(), "default") << boost::format("===[ Compiling application %1% ... ]=== \n\n") % applName << std::flush;

    board->run_blocking(shell1, "cd " + (remoteDir_SourceCode / "sampleAppl").string());
    board->run_blocking(shell1, "make -B " + applName, true, board->getHostName());  // todo: forcing make is not good

    //#########################################
    // copy the init folder to remoteDir_Driver
    //#########################################

    LOG_EVENT_C(board->getHostName(), "default") << boost::format("===[ Copying the init folder to %1% ... ]=== \n\n") % remoteDir_Driver << std::flush;

    // copy the local init folder to remoteDir_Driver
    board->syncDir(redpineAppl_FullPath / "init", remoteDir_Driver);

    boost::filesystem::path script_FullPath = redpineAppl_FullPath / "init" / "run_init";

    // read file contents into a string
    std::ifstream ifs(script_FullPath.c_str());
    std::string content( (std::istreambuf_iterator<char>(ifs) ),
            (std::istreambuf_iterator<char>() ) );

    // get a pointer to the dev class
    dev *devPtr = static_cast<dev *>(module);
    ASSERT(devPtr);

    // ask dev to substitute its parameters in the run_init script
    devPtr->substituteParams(content);

    // copy the run_init script to remoteDir_Driver
    board->copyFileStr_SFTP("run_init", content, remoteDir_Driver / "init");

    //#########################################################
    // remotely run the run_init script in the remoteDir_Driver
    //#########################################################

    LOG_EVENT_C(board->getHostName(), "default") << "===[ Running the run_init script ... ]=== \n\n" << std::flush;

    board->run_blocking(shell1, "cd " + (remoteDir_Driver / "init").string());
    board->run_blocking(shell1, "sudo ./run_init", true, board->getHostName());

    //#######################################
    // remotely start 1609 stack in WAVE mode
    //#######################################

    LOG_EVENT_C(board->getHostName(), "default") << "===[ Running the 1609 stack ... ]=== \n\n" << std::flush;

    // check if rsi_1609 is running on the board
    int running = board->run_blocking_NoRunCheck(shell1, "if pgrep rsi_1609 > /dev/null; then true; else false; fi");

    if(running == 0)
    {
        throw omnetpp::cRuntimeError("Invalid return value from the command!");
    }
    else if(running == 1)
    {
        LOG_EVENT_C(board->getHostName(), "default") << "rsi_1609 is already running! \n" << std::flush;

        // close the shell -- we don't need it anymore
        board->closeShell(shell1);
    }
    else if(running == 2)
    {
        // run rsi_1609
        board->run_blocking(shell1, "cd " + (remoteDir_Driver / "init").string());
        board->run_nonblocking(shell1, "sudo ../rsi_1609", true, board->getHostName());

        // and let it run for a while
        std::this_thread::sleep_for(std::chrono::milliseconds(6000));
    }

    //######################
    // remotely run the code
    //######################

    LOG_EVENT_C(board->getHostName(), "shell2") << boost::format("===[ Running application %1% ... ]=== \n\n") % applName << std::flush;

    board->run_blocking(shell2, "cd " + (remoteDir_SourceCode / "sampleAppl").string());
    board->run_nonblocking(shell2, "sudo ./" + applName + arguments, true, board->getHostName(), "shell2");
}

}
