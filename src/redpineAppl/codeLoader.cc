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

#include <vlog.h>
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

        initScriptName = par("initScriptName").stringValue();
        if(initScriptName == "")
            throw omnetpp::cRuntimeError("initScriptName is empty!");

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

        workers.push_back(std::thread([=]() {  // pass by value

            // create SSH connection to the dev
            SSH_Helper *board = new SSH_Helper(host, port, username, password, true);
            ASSERT(board);

            {
                std::lock_guard<std::mutex> lock(lock_vector);
                active_SSH.push_back( std::make_pair(module, board) );
            }

            // adding a new line to improve readability
            {
                std::lock_guard<std::mutex> lock(vlog::lock_log);
                vlog::EVENT(board->getHostName()) << "\n";
                vlog::flush();
            }

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
        // todo: make sure it is redpine board
        //workers.push_back( std::thread (&codeLoader::init_test, this, ii.first, ii.second) );

        workers.push_back( std::thread (&codeLoader::init_board, this, ii.first, ii.second) );
    }

    // wait for all threads to finish
    std::for_each(workers.begin(), workers.end(), [](std::thread &t) {
        t.join();
    });

    std::cout << "All done! \n" << std::flush;

    // delete all SSH sessions that have no active threads
    for(auto &ii : active_SSH)
    {
        if(ii.second->getNumActiveThreads() == 0)
        {
            delete ii.second;
            ii.second = NULL;  // make SSH pointer NULL
        }
    }
}


void codeLoader::init_board(cModule *module, SSH_Helper *board)
{
    ASSERT(module);
    ASSERT(board);

    // should we reboot this dev before proceeding?
    if(module->par("rebootAtStart").boolValue())
    {
        ssh_channel rebootShell = board->openShell("rebootShell");  // open a shell

        {
            std::lock_guard<std::mutex> lock(vlog::lock_log);
            vlog::EVENT(board->getHostName()) << ">>> Re-booting device ... Please wait \n\n";
            vlog::flush();
        }

        double duration_ms = board->rebootDev(rebootShell, 40000);
        board->closeShell(rebootShell);  // close the shell

        {
            std::lock_guard<std::mutex> lock(vlog::lock_log);
            vlog::EVENT(board->getHostName()) << boost::format(">>> Device is up and running! Boot time ~ %1% seconds. Reconnecting ... \n\n") % (duration_ms / 1000.);
            vlog::flush();
        }

        // previous SSH connection is lost. Re-connect to the dev
        board = new SSH_Helper(board->getHostName(), board->getPort(), board->getUsername(), board->getPassword());
        ASSERT(board);
    }

    // open as many shells as we need
    ssh_channel shell1 = board->openShell("shell1", true);
    ssh_channel shell2 = board->openShell("shell2", true);

    //#################################################
    // Step 1: copy the init script to remoteDir_Driver
    //#################################################
    boost::filesystem::path script_FullPath = redpineAppl_FullPath / initScriptName;

    {
        std::lock_guard<std::mutex> lock(vlog::lock_log);
        vlog::EVENT(board->getHostName()) << boost::format(">>> Copying the init script to %1% ... \n\n") % remoteDir_Driver;
        vlog::flush();
    }

    // read file contents into a string
    std::ifstream ifs(script_FullPath.c_str());
    std::string content( (std::istreambuf_iterator<char>(ifs) ),
            (std::istreambuf_iterator<char>()    ) );

    // get a pointer to the dev class
    dev *devPtr = static_cast<dev *>(module);
    ASSERT(devPtr);

    // ask dev to substitute its parameters in the init script
    devPtr->substituteParams(content);

    // do the copying
    board->copyFileStr_SFTP(initScriptName, content, remoteDir_Driver);

    //##################################################################
    // Step 2: copying new/modified source codes to remoteDir_SourceCode
    //##################################################################

    {
        std::lock_guard<std::mutex> lock(vlog::lock_log);
        vlog::EVENT(board->getHostName()) << boost::format(">>> Syncing source codes with %1% ... \n\n") % remoteDir_SourceCode;
        vlog::flush();
    }

    board->createDir(remoteDir_SourceCode);  // create a directory to store all our source codes

    board->syncDir(redpineAppl_FullPath / "headers", remoteDir_SourceCode); // copy local folder 'headers'
    board->syncDir(redpineAppl_FullPath / "sampleAppl", remoteDir_SourceCode); // copy local folder 'sampleAppl'
    board->syncDir(redpineAppl_FullPath / "libs", remoteDir_SourceCode); // create local folder 'libs'

    //##################################
    // Step 3: remotely compile the code
    //##################################
    std::string applName = module->par("applName").stringValue();
    if(applName == "")
        throw omnetpp::cRuntimeError("applName is empty!");

    {
        std::lock_guard<std::mutex> lock(vlog::lock_log);
        vlog::EVENT(board->getHostName()) << boost::format(">>> Compiling application %1% ... \n\n") % applName;
        vlog::flush();
    }

    board->run_command_blocking(shell1, "cd " + (remoteDir_SourceCode / "sampleAppl").string());
    board->run_command_blocking(shell1, "make " + applName, true, board->getHostName());

    //########################################################
    // Step 4: remotely run the script in the remoteDir_Driver
    //########################################################

    {
        std::lock_guard<std::mutex> lock(vlog::lock_log);
        vlog::EVENT(board->getHostName()) << boost::format(">>> Running the init script %1% ... \n\n") % initScriptName;
        vlog::flush();
    }

    board->run_command_blocking(shell1, "cd " + remoteDir_Driver.string());
    board->run_command_blocking(shell1, "sudo ./" + initScriptName, true, board->getHostName());

    //###############################################
    // Step 5: remotely start 1609 stack in WAVE mode
    //###############################################

    {
        std::lock_guard<std::mutex> lock(vlog::lock_log);
        vlog::EVENT(board->getHostName()) << ">>> Start 1609 stack in WAVE mode ... \n\n";
        vlog::flush();
    }

    board->run_command_blocking(shell1, "cd " + remoteDir_Driver.string());
    board->run_command_nonblocking(shell1, "if ! pgrep rsi_1609 > /dev/null; then sudo ./rsi_1609; fi", true, board->getHostName());

    // put the main thread to sleep -- let the last non-blocking command to run for a while
    std::this_thread::sleep_for(std::chrono::milliseconds(5000));

    //##############################
    // Step 6: remotely run the code
    //##############################

    {
        std::lock_guard<std::mutex> lock(vlog::lock_log);
        vlog::EVENT(board->getHostName(), "shell2") << boost::format(">>> Running application %1% ... \n\n") % applName;
        vlog::flush();
    }

    board->run_command_blocking(shell2, "cd " + (remoteDir_SourceCode / "sampleAppl").string());
    board->run_command_nonblocking(shell2, "sudo ./" + applName, true, board->getHostName(), "shell2");
}


void codeLoader::init_test(cModule *module, SSH_Helper *board)
{
    ASSERT(module);
    ASSERT(board);

    //ssh_channel shell1 = board->openShell("shell1", true);

    //board->run_command(shell1, "[[ $- == *i* ]] && echo 'Interactive' || echo 'Not interactive'", 5, true);
    //board->run_command(shell1, "shopt -q login_shell && echo 'Login shell' || echo 'Not login shell'", 5, true);

    //board->run_command(shell1, "tmux set -g status off", 10, true);
    //board->run_command(shell1, "tmux", 10, true);

}

}
