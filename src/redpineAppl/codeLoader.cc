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
        // get a pointer to the TraCI module
        cModule *module = simulation.getSystemModule()->getSubmodule("TraCI");
        ASSERT(module);
        TraCI = static_cast<TraCI_Commands *>(module);
        ASSERT(TraCI);

        Signal_initialize_withTraCI = registerSignal("initialize_withTraCI");
        simulation.getSystemModule()->subscribe("initialize_withTraCI", this);

        Signal_executeEachTS = registerSignal("executeEachTS");
        simulation.getSystemModule()->subscribe("executeEachTS", this);

        // construct the path to VENTOS_Start_WAVE
        boost::filesystem::path VENTOS_FullPath = cSimulation::getActiveSimulation()->getEnvir()->getConfig()->getConfigEntry("network").getBaseDirectory();
        redpineAppl_FullPath = VENTOS_FullPath / "src" / "redpineAppl";

        on = par("on").boolValue();

        if(on)
        {
            make_connection();
            init_board();
        }
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
        //mod->callInitialize();  do not call initialize at this point!
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

    // vector container stores threads
    std::vector<std::thread> workers;

    for(unsigned int i = 0; i < remoteDevs.size(); i++)
    {
        workers.push_back(std::thread([=]() {  // pass by value
            // delay each new thread
            std::this_thread::sleep_for(std::chrono::milliseconds(i*50));

            // create SSH connection to the remote device
            SSH *new_session = new SSH(remoteDevs[i].host, remoteDevs[i].port, remoteDevs[i].username, remoteDevs[i].password);
            ASSERT(new_session);
            IMX_board.push_back(new_session);
        }));
    }

    // wait for all threads to finish
    std::for_each(workers.begin(), workers.end(), [](std::thread &t) {
        t.join();
    });

    std::cout << "  Done! \n\n";
    std::cout.flush();
}


void codeLoader::init_board()
{
    // first make sure all pointers are valid
    for(auto &board : IMX_board)
        ASSERT(board);

    // copy the init script to all boards
    boost::filesystem::path script_FullPath = redpineAppl_FullPath / "VENTOS_Start_WAVE";
    boost::filesystem::path destDir = "/home/dsrc/release";
    for(auto &board : IMX_board)
    {
        printf(">>> Copying the init script to %s:%s ... ", board->getHost().c_str(), destDir.c_str());
        std::cout.flush();
        board->copyFile_SFTP(script_FullPath, destDir);
        printf("Done!\n");
        std::cout.flush();
    }

    std::cout << std::endl;

    // vector container stores threads
    std::vector<std::thread> workers;

    // run the script in all boards
    for(auto &board : IMX_board)
    {
        workers.push_back(std::thread([=]() {  // pass by value
            printf(">>> Running the init script at %s ... \n", board->getHost().c_str());
            std::cout.flush();
            board->run_command("sudo /home/dsrc/release/VENTOS_Start_WAVE", true);
        }));
    }

    // wait for all threads to finish
    std::for_each(workers.begin(), workers.end(), [](std::thread &t) {
        t.join();
    });





    std::cout << "yeayy! \n" << std::flush;

    // start 1609 stack in WAVE mode
    //IMX_board[0]->run_command("sudo /home/dsrc/release/rsi_1609");


    //    std::vector<sftp_attributes> dirListings = IMX_board[0]->listDir("/home/dsrc/source/sample_apps");
    //
    //    for(auto &i : dirListings)
    //    {
    //        printf("%-30s  %u  %10llu  %.8o  %s(%d)  %s(%d)  %-15u  %-15lu  %-15u \n",
    //                i->name,
    //                i->type,
    //                (long long unsigned int) i->size,
    //                i->permissions,
    //                i->owner,
    //                i->uid,
    //                i->group,
    //                i->gid,
    //                i->atime,
    //                i->createtime,
    //                i->mtime);
    //
    //        sftp_attributes_free(i);
    //    }
    //    std::cout.flush();


    // copy all new/modified files in local directory to remote directory
    //boost::filesystem::path sampleAppl_FullPath = redpineAppl_FullPath / "sampleAppl";
    //IMX_board[0]->syncDir(sampleAppl_FullPath, "/home/dsrc/source/sample_apps");
}

}

