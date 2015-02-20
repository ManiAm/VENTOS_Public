
#include "SumoServer.h"

namespace VENTOS {

Define_Module(VENTOS::SumoServer);

void SumoServer::initialize(int stage)
{
    if(stage == 2)
    {
        // get a pointer to the TraCI module
        cModule *module = simulation.getSystemModule()->getSubmodule("SumoBinary");

        SUMO_GUI_FileName = module->par("SUMO_GUI_FileName").stringValue();
        SUMO_CMD_FileName = module->par("SUMO_CMD_FileName").stringValue();

        VENTOS_FullPath = cSimulation::getActiveSimulation()->getEnvir()->getConfig()->getConfigEntry("network").getBaseDirectory();
        SUMO_Binary_FullPath = VENTOS_FullPath / "sumoBinary";

        SUMO_GUI_Binary_FullPath = SUMO_Binary_FullPath / SUMO_GUI_FileName;
        SUMO_CMD_Binary_FullPath = SUMO_Binary_FullPath / SUMO_CMD_FileName;

        createServerSocket();
    }
}


void SumoServer::finish()
{

}


void SumoServer::handleMessage(cMessage *msg)
{

}


void SumoServer::createServerSocket()
{

}

} // namespace


