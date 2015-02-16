
#include <Statistics.h>
#include <ApplRSU_03_Manager.h>

namespace VENTOS {

Define_Module(VENTOS::Statistics);

class ApplRSUManager;


Statistics::~Statistics()
{

}


void Statistics::initialize(int stage)
{
    if(stage == 0)
	{
        // get a pointer to the TraCI module
        cModule *module = simulation.getSystemModule()->getSubmodule("TraCI");
        TraCI = static_cast<TraCI_Extend *>(module);
        terminate = module->par("terminate").doubleValue();
        updateInterval = module->par("updateInterval").doubleValue();

        collectMAClayerData = par("collectMAClayerData").boolValue();
        collectPlnManagerData = par("collectPlnManagerData").boolValue();
        printBeaconsStatistics = par("printBeaconsStatistics").boolValue();

        // register signals
        Signal_executeEachTS = registerSignal("executeEachTS");

        Signal_beaconP = registerSignal("beaconP");
        Signal_beaconO = registerSignal("beaconO");
        Signal_beaconD = registerSignal("beaconD");

        Signal_MacStats = registerSignal("MacStats");

        Signal_SentPlatoonMsg = registerSignal("SentPlatoonMsg");
        Signal_VehicleState = registerSignal("VehicleState");
        Signal_PlnManeuver = registerSignal("PlnManeuver");

        // now subscribe locally to all these signals
        simulation.getSystemModule()->subscribe("executeEachTS", this);
        simulation.getSystemModule()->subscribe("beaconP", this);
        simulation.getSystemModule()->subscribe("beaconO", this);
        simulation.getSystemModule()->subscribe("beaconD", this);
        simulation.getSystemModule()->subscribe("MacStats", this);
        simulation.getSystemModule()->subscribe("SentPlatoonMsg", this);
        simulation.getSystemModule()->subscribe("VehicleState", this);
        simulation.getSystemModule()->subscribe("PlnManeuver", this);
    }
}


void Statistics::finish()
{

}


void Statistics::handleMessage(cMessage *msg)
{

}


// todo: make all beacons object
void Statistics::receiveSignal(cComponent *source, simsignal_t signalID, long i)
{
    Enter_Method_Silent();

    EV << "*** Statistics module received signal " << signalID;
    EV << " from module " << source->getFullName();
    EV << " with value " << i << endl;

    int nodeIndex = getNodeIndex(source->getFullName());

    if(signalID == Signal_executeEachTS)
    {
        Statistics::executeEachTimestep(i);
    }
    else if(signalID == Signal_beaconD)
    {
        // from preceding
        if(i==1)
        {
            NodeEntry *tmp = new NodeEntry(source->getFullName(), "-", nodeIndex, -1, simTime());
            Vec_BeaconsDP.push_back(tmp);
        }
        // from other vehicles
        else if(i==2)
        {
            NodeEntry *tmp = new NodeEntry(source->getFullName(), "-", nodeIndex, -1, simTime());
            Vec_BeaconsDO.push_back(tmp);
        }
    }
}


void Statistics::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj)
{
    Enter_Method_Silent();

    EV << "*** Statistics module received signal " << signalID;
    EV << " from module " << source->getFullName() << endl;

    int nodeIndex = getNodeIndex(source->getFullName());

    if(collectMAClayerData && signalID == Signal_MacStats)
    {
        MacStat *m = static_cast<MacStat *>(obj);
        if (m == NULL) return;

        int counter = findInVector(Vec_MacStat, source->getFullName());

        // its a new entry, so we add it.
        if(counter == -1)
        {
            MacStatEntry *tmp = new MacStatEntry(source->getFullName(), nodeIndex, simTime().dbl(), m->vec);
            Vec_MacStat.push_back(tmp);
        }
        // if found, just update the existing fields
        else
        {
            Vec_MacStat[counter]->time = simTime().dbl();
            Vec_MacStat[counter]->MacStatsVec = m->vec;
        }
    }
    else if(collectPlnManagerData && signalID == Signal_VehicleState)
    {
        CurrentVehicleState *state = dynamic_cast<CurrentVehicleState*>(obj);
        ASSERT(state);

        plnManagement *tmp = new plnManagement(simTime().dbl(), state->name, "-", state->state, "-", "-");
        Vec_plnManagement.push_back(tmp);
    }
    else if(collectPlnManagerData && signalID == Signal_SentPlatoonMsg)
    {
        CurrentPlnMsg* plnMsg = dynamic_cast<CurrentPlnMsg*>(obj);
        ASSERT(plnMsg);

        plnManagement *tmp = new plnManagement(simTime().dbl(), plnMsg->msg->getSender(), plnMsg->msg->getRecipient(), plnMsg->type, plnMsg->msg->getSendingPlatoonID(), plnMsg->msg->getReceivingPlatoonID());
        Vec_plnManagement.push_back(tmp);
    }
    else if(collectPlnManagerData && signalID == Signal_PlnManeuver)
    {
        PlnManeuver* com = dynamic_cast<PlnManeuver*>(obj);
        ASSERT(com);

        plnStat *tmp = new plnStat(simTime().dbl(), com->from, com->to, com->maneuver);
        Vec_plnStat.push_back(tmp);
    }

    // todo
    else if(signalID == Signal_beaconP)
    {
        data *m = static_cast<data *>(obj);
        if (m == NULL) return;

        NodeEntry *tmp = new NodeEntry(source->getFullName(), m->name, nodeIndex, -1, simTime());
        Vec_BeaconsP.push_back(tmp);
    }
    else if(signalID == Signal_beaconO)
    {
        data *m = static_cast<data *>(obj);
        if (m == NULL) return;

        NodeEntry *tmp = new NodeEntry(source->getFullName(), m->name, nodeIndex, -1, simTime());
        Vec_BeaconsO.push_back(tmp);
    }
}


void Statistics::executeEachTimestep(bool simulationDone)
{
    if(collectMAClayerData)
        MAClayerToFile();

    if(collectPlnManagerData)
    {
        if(ev.isGUI()) plnManageToFile();  // write what we have collected so far
        if(ev.isGUI()) plnStatToFile();
    }

    // todo:
    if(simulationDone)
    {
        if(collectPlnManagerData && !ev.isGUI())
        {
            plnManageToFile();
            plnStatToFile();
        }

        // sort the vectors by node ID:
        // Vec_BeaconsP = SortByID(Vec_BeaconsP);
        // Vec_BeaconsO = SortByID(Vec_BeaconsO);
        // Vec_BeaconsDP = SortByID(Vec_BeaconsDP);
        // Vec_BeaconsDO = SortByID(Vec_BeaconsDO);

        postProcess();

        if(printBeaconsStatistics)
            printToFile();
    }
}


void Statistics::MAClayerToFile()
{
    boost::filesystem::path filePath;

    if( ev.isGUI() )
    {
        filePath = "results/gui/MACdata.txt";
    }
    else
    {
        // get the current run number
        int currentRun = ev.getConfigEx()->getActiveRunNumber();
        ostringstream fileName;
        fileName << currentRun << "_MACdata.txt";
        filePath = "results/cmd/" + fileName.str();
    }

    FILE *filePtr = fopen (filePath.string().c_str(), "w");

    // write header
    fprintf (filePtr, "%-20s","timeStep");
    fprintf (filePtr, "%-20s","vehicleName");

    fprintf (filePtr, "%-20s","DroppedPackets");
    fprintf (filePtr, "%-20s","NumTooLittleTime");
    fprintf (filePtr, "%-30s","NumInternalContention");
    fprintf (filePtr, "%-20s","NumBackoff");
    fprintf (filePtr, "%-20s","SlotsBackoff");
    fprintf (filePtr, "%-20s","TotalBusyTime");
    fprintf (filePtr, "%-20s","SentPackets");
    fprintf (filePtr, "%-20s","SNIRLostPackets");
    fprintf (filePtr, "%-20s","TXRXLostPackets");
    fprintf (filePtr, "%-20s","ReceivedPackets");
    fprintf (filePtr, "%-20s","ReceivedBroadcasts\n\n");

    // write body
    for(unsigned int k=0; k<Vec_MacStat.size(); k++)
    {
        fprintf (filePtr, "%-20.2f ", Vec_MacStat[k]->time);
        fprintf (filePtr, "%-20s ", Vec_MacStat[k]->name);

        fprintf (filePtr, "%-20ld ", Vec_MacStat[k]->MacStatsVec[0]);
        fprintf (filePtr, "%-20ld ", Vec_MacStat[k]->MacStatsVec[1]);
        fprintf (filePtr, "%-30ld ", Vec_MacStat[k]->MacStatsVec[2]);
        fprintf (filePtr, "%-20ld ", Vec_MacStat[k]->MacStatsVec[3]);
        fprintf (filePtr, "%-20ld ", Vec_MacStat[k]->MacStatsVec[4]);
        fprintf (filePtr, "%-20ld ", Vec_MacStat[k]->MacStatsVec[5]);
        fprintf (filePtr, "%-20ld ", Vec_MacStat[k]->MacStatsVec[6]);
        fprintf (filePtr, "%-20ld ", Vec_MacStat[k]->MacStatsVec[7]);
        fprintf (filePtr, "%-20ld ", Vec_MacStat[k]->MacStatsVec[8]);
        fprintf (filePtr, "%-20ld ", Vec_MacStat[k]->MacStatsVec[9]);
        fprintf (filePtr, "%-20ld ", Vec_MacStat[k]->MacStatsVec[10]);

        fprintf (filePtr, "\n" );
    }

    fclose(filePtr);
}


void Statistics::plnManageToFile()
{
    boost::filesystem::path filePath;

    if( ev.isGUI() )
    {
        filePath = "results/gui/plnManage.txt";
    }
    else
    {
        // get the current run number
        int currentRun = ev.getConfigEx()->getActiveRunNumber();
        ostringstream fileName;
        fileName << currentRun << "_plnManage.txt";
        filePath = "results/cmd/" + fileName.str();
    }

    FILE *filePtr = fopen (filePath.string().c_str(), "w");

    // write header
    fprintf (filePtr, "%-12s","timeStep");
    fprintf (filePtr, "%-15s","sender");
    fprintf (filePtr, "%-17s","receiver");
    fprintf (filePtr, "%-25s","type");
    fprintf (filePtr, "%-20s","sendingPlnID");
    fprintf (filePtr, "%-20s\n\n","recPlnID");

    string oldSender = "";
    double oldTime = -1;

    // write body
    for(unsigned int k=0; k<Vec_plnManagement.size(); k++)
    {
        // make the log more readable :)
        if(string(Vec_plnManagement[k]->sender) != oldSender || Vec_plnManagement[k]->time != oldTime)
        {
            fprintf(filePtr, "\n");
            oldSender = Vec_plnManagement[k]->sender;
            oldTime = Vec_plnManagement[k]->time;
        }

        fprintf (filePtr, "%-10.2f ", Vec_plnManagement[k]->time);
        fprintf (filePtr, "%-15s ", Vec_plnManagement[k]->sender);
        fprintf (filePtr, "%-17s ", Vec_plnManagement[k]->receiver);
        fprintf (filePtr, "%-30s ", Vec_plnManagement[k]->type);
        fprintf (filePtr, "%-18s ", Vec_plnManagement[k]->sendingPlnID);
        fprintf (filePtr, "%-20s\n", Vec_plnManagement[k]->receivingPlnID);
    }

    fclose(filePtr);
}


void Statistics::plnStatToFile()
{
    boost::filesystem::path filePath;

    if( ev.isGUI() )
    {
        filePath = "results/gui/plnStat.txt";
    }
    else
    {
        // get the current run number
        int currentRun = ev.getConfigEx()->getActiveRunNumber();
        ostringstream fileName;
        fileName << currentRun << "_plnStat.txt";
        filePath = "results/cmd/" + fileName.str();
    }

    FILE *filePtr = fopen (filePath.string().c_str(), "w");

    // write header
    fprintf (filePtr, "%-12s","timeStep");
    fprintf (filePtr, "%-20s","from platoon");
    fprintf (filePtr, "%-20s","to platoon");
    fprintf (filePtr, "%-20s\n\n","comment");

    string oldPln = "";

    // write body
    for(unsigned int k=0; k<Vec_plnStat.size(); k++)
    {
        if(Vec_plnStat[k]->from != oldPln)
        {
            fprintf(filePtr, "\n");
            oldPln = Vec_plnStat[k]->from;
        }

        fprintf (filePtr, "%-10.2f ", Vec_plnStat[k]->time);
        fprintf (filePtr, "%-20s ", Vec_plnStat[k]->from);
        fprintf (filePtr, "%-20s ", Vec_plnStat[k]->to);
        fprintf (filePtr, "%-20s\n", Vec_plnStat[k]->maneuver);
    }

    fclose(filePtr);
}


// todo
void Statistics::postProcess()
{
    for(unsigned int k=0; k<Vec_BeaconsP.size(); k++)
    {
        int counter = findInVector(totalBeaconsP, Vec_BeaconsP[k]->name1);

        // its a new entry, so we add it.
        if(counter == -1)
        {
            NodeEntry *tmp = new NodeEntry(Vec_BeaconsP[k]->name1, "-", Vec_BeaconsP[k]->nodeID, 1, -1);
            totalBeaconsP.push_back(tmp);
        }
        // if found, just update the existing fields
        else
        {
            totalBeaconsP[counter]->count = totalBeaconsP[counter]->count + 1;
        }
    }

    for(unsigned int k=0; k<Vec_BeaconsO.size(); k++)
    {
        int counter = findInVector(totalBeaconsO, Vec_BeaconsO[k]->name1);

        // its a new entry, so we add it.
        if(counter == -1)
        {
            NodeEntry *tmp = new NodeEntry(Vec_BeaconsO[k]->name1, "-", Vec_BeaconsO[k]->nodeID, 1, -1);
            totalBeaconsO.push_back(tmp);
        }
        // if found, just update the existing fields
        else
        {
            totalBeaconsO[counter]->count = totalBeaconsO[counter]->count + 1;
        }
    }

    for(unsigned int k=0; k<Vec_BeaconsDP.size(); k++)
    {
        int counter = findInVector(totalBeaconsDP, Vec_BeaconsDP[k]->name1);

        // its a new entry, so we add it.
        if(counter == -1)
        {
            NodeEntry *tmp = new NodeEntry(Vec_BeaconsDP[k]->name1, "-", Vec_BeaconsDP[k]->nodeID, 1, -1);
            totalBeaconsDP.push_back(tmp);
        }
        // if found, just update the existing fields
        else
        {
            totalBeaconsDP[counter]->count = totalBeaconsDP[counter]->count + 1;
        }
    }

    for(unsigned int k=0; k<Vec_BeaconsDO.size(); k++)
    {
        int counter = findInVector(totalBeaconsDO, Vec_BeaconsDO[k]->name1);

        // its a new entry, so we add it.
        if(counter == -1)
        {
            NodeEntry *tmp = new NodeEntry(Vec_BeaconsDO[k]->name1, "-", Vec_BeaconsDO[k]->nodeID, 1, -1);
            totalBeaconsDO.push_back(tmp);
        }
        // if found, just update the existing fields
        else
        {
            totalBeaconsDO[counter]->count = totalBeaconsDO[counter]->count + 1;
        }
    }

    double intervalS = 0;
    double intervalE = updateInterval;
    int val = 0;

    for(int i=1; i<(terminate/updateInterval); i++)
    {
        // it should not be sorted
        for(unsigned int k=0; k<Vec_BeaconsDO.size(); k++)
        {
            if(Vec_BeaconsDO[k]->time.dbl() >= intervalS && Vec_BeaconsDO[k]->time.dbl() <= intervalE)
                val++;
        }

        NodeEntry *tmp = new NodeEntry("-", "-", -1, val, intervalE);
        beaconsDO_interval.push_back(tmp);

        val = 0;
        intervalS = intervalS + updateInterval;
        intervalE = intervalE + updateInterval;
    }

    intervalS = 0;
    intervalE = updateInterval;
    val = 0;

    for(int i=1; i<(terminate/updateInterval); i++)
    {
        // it should not be sorted
        for(unsigned int k=0; k<Vec_BeaconsDP.size(); k++)
        {
            if(Vec_BeaconsDP[k]->time.dbl() >= intervalS && Vec_BeaconsDP[k]->time.dbl() <= intervalE)
                val++;
        }

        NodeEntry *tmp = new NodeEntry("-", "-", -1, val, intervalE);
        beaconsDP_interval.push_back(tmp);

        val = 0;
        intervalS = intervalS + updateInterval;
        intervalE = intervalE + updateInterval;
    }
}


// todo
void Statistics::printToFile()
{
    char fName [50];
    FILE *f1;

    if( ev.isGUI() )
    {
        sprintf (fName, "%s.txt", "results/gui/01.beacons_total_p");
    }
    else
    {
        // get the current run number
        int currentRun = ev.getConfigEx()->getActiveRunNumber();
        sprintf (fName, "%s_%d.txt", "results/cmd/01.beacons_total_p", currentRun);
    }

    f1 = fopen (fName, "w");

    fprintf (f1, "%s\n\n","Total Number of received beacons from preceding vehicle");

    // write header
    fprintf (f1, "%-10s","vehicle");
    fprintf (f1, "%-10s\n","beacons");  // beacon from preceding

    for(unsigned int k=0; k<totalBeaconsP.size(); k++)
    {
        fprintf (f1, "%-10s ", totalBeaconsP[k]->name1);
        fprintf (f1, "%-10d ", totalBeaconsP[k]->count);
        fprintf (f1, "\n");
    }

    fclose(f1);

    // ##########################################################
    // ##########################################################

    if( ev.isGUI() )
    {
        sprintf (fName, "%s.txt", "results/gui/02.beacons_total_o");
    }
    else
    {
        // get the current run number
        int currentRun = ev.getConfigEx()->getActiveRunNumber();
        sprintf (fName, "%s_%d.txt", "results/cmd/02.beacons_total_o", currentRun);
    }

    f1 = fopen (fName, "w");

    fprintf (f1, "%s\n\n","Total Number of received beacons from other vehicle");

    // write header
    fprintf (f1, "%-10s","vehicle");
    fprintf (f1, "%-10s\n","beacons");  // beacon from preceding

    for(unsigned int k=0; k<totalBeaconsO.size(); k++)
    {
        fprintf (f1, "%-10s ", totalBeaconsO[k]->name1);
        fprintf (f1, "%-10d ", totalBeaconsO[k]->count);
        fprintf (f1, "\n");
    }

    fclose(f1);

    // ##########################################################
    // ##########################################################

    if( ev.isGUI() )
    {
        sprintf (fName, "%s.txt", "results/gui/03.beacons_total_droped_p");
    }
    else
    {
        // get the current run number
        int currentRun = ev.getConfigEx()->getActiveRunNumber();
        sprintf (fName, "%s_%d.txt", "results/cmd/03.beacons_total_droped_p", currentRun);
    }

    f1 = fopen (fName, "w");

    fprintf (f1, "%s\n\n","Total Number of dropped beacons from preceding vehicle");

    // write header
    fprintf (f1, "%-10s","vehicle");
    fprintf (f1, "%-10s\n","beacons");  // beacon from preceding

    for(unsigned int k=0; k<totalBeaconsDP.size(); k++)
    {
        fprintf (f1, "%-10s ", totalBeaconsDP[k]->name1);
        fprintf (f1, "%-10d ", totalBeaconsDP[k]->count);
        fprintf (f1, "\n");
    }

    fclose(f1);

    // ##########################################################
    // ##########################################################

    if( ev.isGUI() )
    {
        sprintf (fName, "%s.txt", "results/gui/04.beacons_total_droped_o");
    }
    else
    {
        // get the current run number
        int currentRun = ev.getConfigEx()->getActiveRunNumber();
        sprintf (fName, "%s_%d.txt", "results/cmd/04.beacons_total_droped_o", currentRun);
    }

    f1 = fopen (fName, "w");

    fprintf (f1, "%s\n\n","Total Number of dropped beacons from other vehicles");

    // write header
    fprintf (f1, "%-10s","vehicle");
    fprintf (f1, "%-10s\n","beacons");  // beacon from preceding

    for(unsigned int k=0; k<totalBeaconsDO.size(); k++)
    {
        fprintf (f1, "%-10s ", totalBeaconsDO[k]->name1);
        fprintf (f1, "%-10d ", totalBeaconsDO[k]->count);
        fprintf (f1, "\n");
    }

    fclose(f1);

    // ##########################################################
    // ##########################################################

    if( ev.isGUI() )
    {
        sprintf (fName, "%s.txt", "results/gui/05.beacon_interval_droped_o");
    }
    else
    {
        // get the current run number
        int currentRun = ev.getConfigEx()->getActiveRunNumber();
        sprintf (fName, "%s_%d.txt", "results/cmd/05.beacon_interval_droped_o", currentRun);
    }

    f1 = fopen (fName, "w");

    fprintf (f1, "%s\n\n","Number of dropped beacons (from other vehicles) in each interval");

    // write header
    fprintf (f1, "%-10s","vehicle");
    fprintf (f1, "%-10s","delta");
    fprintf (f1, "%-10s\n","beacons");  // beacon from preceding

    for(unsigned int k=0; k<beaconsDO_interval.size(); k++)
    {
        fprintf (f1, "%-10s ", beaconsDO_interval[k]->name1);
        fprintf (f1, "%-10f ", beaconsDO_interval[k]->time.dbl());
        fprintf (f1, "%-10d ", beaconsDO_interval[k]->count);
        fprintf (f1, "\n");
    }

    fclose(f1);

    // ##########################################################
    // ##########################################################

    if( ev.isGUI() )
    {
        sprintf (fName, "%s.txt", "results/gui/06.beacon_interval_droped_p");
    }
    else
    {
        // get the current run number
        int currentRun = ev.getConfigEx()->getActiveRunNumber();
        sprintf (fName, "%s_%d.txt", "results/cmd/06.beacon_interval_droped_p", currentRun);
    }

    f1 = fopen (fName, "w");

    fprintf (f1, "%s\n\n","Number of dropped beacons (from proceeding vehicle) in each interval");

    // write header
    fprintf (f1, "%-10s","vehicle");
    fprintf (f1, "%-10s","delta");
    fprintf (f1, "%-10s\n","beacons");  // beacon from preceding

    for(unsigned int k=0; k<beaconsDP_interval.size(); k++)
    {
        fprintf (f1, "%-10s ", beaconsDP_interval[k]->name1);
        fprintf (f1, "%-10f ", beaconsDP_interval[k]->time.dbl());
        fprintf (f1, "%-10d ", beaconsDP_interval[k]->count);
        fprintf (f1, "\n");
    }

    fclose(f1);
}


// returns the index of a node. for example gets V[10] as input and returns 10
int Statistics::getNodeIndex(const char *ModName)
{
    ostringstream oss;

    for(int h=0 ; ModName[h] != NULL ; h++)
    {
        if ( isdigit(ModName[h]) )
        {
            oss << ModName[h];
        }
    }

    int nodeID = atoi(oss.str().c_str());

    return nodeID;
}


// todo: use template
int Statistics::findInVector(vector<NodeEntry *> Vec, const char *name)
{
    unsigned int counter;    // for counter
    bool found = false;

    for(counter=0; counter<Vec.size(); counter++)
    {
        if( strcmp(Vec[counter]->name1, name) == 0 )
        {
            found = true;
            break;
        }
    }

    if(!found)
        return -1;
    else
        return counter;
}


int Statistics::findInVector(vector<MacStatEntry *> Vec, const char *name)
{
    unsigned int counter;    // for counter
    bool found = false;

    for(counter=0; counter<Vec.size(); counter++)
    {
        if( strcmp(Vec[counter]->name, name) == 0 )
        {
            found = true;
            break;
        }
    }

    if(!found)
        return -1;
    else
        return counter;
}


vector<NodeEntry *> Statistics::SortByID(vector<NodeEntry *> vec)
{
    if(vec.size() == 0)
        return vec;

    for(unsigned int i = 0; i<vec.size()-1; i++)
    {
        for(unsigned int j = i+1; j<vec.size(); j++)
        {
            if( vec[i]->nodeID > vec[j]->nodeID)
                swap(vec[i], vec[j]);
        }
    }

    return vec;
}


}
