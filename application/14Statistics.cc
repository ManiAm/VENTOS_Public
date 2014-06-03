#include <14Statistics.h>
#include <stdlib.h>

Define_Module(Statistics);


Statistics::Statistics()
{

}


void Statistics::initialize(int stage)
{
    if(stage == 0)
	{
        // get a pointer to the manager module
        cModule *module = simulation.getSystemModule()->getSubmodule("manager");
        manager = static_cast<TraCI_Extend *>(module);
        terminate = module->par("terminate").doubleValue();
        updateInterval = module->par("updateInterval").doubleValue();

        printBeaconsStatistics = par("printBeaconsStatistics").boolValue();


        // register signals
        Signal_terminate = registerSignal("terminate");

        Signal_beaconP = registerSignal("beaconP");
        Signal_beaconO = registerSignal("beaconO");
        Signal_beaconD = registerSignal("beaconD");

        Signal_MacStats = registerSignal("MacStats");

        // now subscribe locally to all these signals
        simulation.getSystemModule()->subscribe("terminate", this);
        simulation.getSystemModule()->subscribe("beaconP", this);
        simulation.getSystemModule()->subscribe("beaconO", this);
        simulation.getSystemModule()->subscribe("beaconD", this);
        simulation.getSystemModule()->subscribe("MacStats", this);
    }
}


void Statistics::handleMessage(cMessage *msg)
{


}


void Statistics::receiveSignal(cComponent *source, simsignal_t signalID, long i)
{
	Enter_Method_Silent();

	EV << "*** Statistics module received signal " << signalID;
    EV << " from module " << source->getFullName();
	EV << " with value " << i << endl;

    int nodeIndex = getNodeIndex(source ->getFullName());

    // signal from MyTraCI
	if(signalID == Signal_terminate)
	{
	    // sort the vectors by node ID:
	    //Vec_BeaconsP = SortByID(Vec_BeaconsP);
	    //Vec_BeaconsO = SortByID(Vec_BeaconsO);
	    //Vec_BeaconsDP = SortByID(Vec_BeaconsDP);
	    //Vec_BeaconsDO = SortByID(Vec_BeaconsDO);

	    postProcess();

	    printStatistics();

	    if(printBeaconsStatistics)
	        printToFile();

	    // close TraCI connection
	   // manager->commandTerminate();

	    // then terminate
	    endSimulation();
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

    int nodeIndex = getNodeIndex(source ->getFullName());

    if(signalID == Signal_beaconP)
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
    else if(signalID == Signal_MacStats)
    {
        MacStat *m = static_cast<MacStat *>(obj);
        if (m == NULL) return;

        int counter = findInVector(Vec_MacStat, source->getFullName());

        // its a new entry, so we add it.
        if(counter == -1)
        {
            MacStatEntry *tmp = new MacStatEntry(source->getFullName(), nodeIndex, simTime(), m->vec);
            Vec_MacStat.push_back(tmp);
        }
        // if found, just update the existing fields
        else
        {
            Vec_MacStat[counter]->time = simTime();
            Vec_MacStat[counter]->MacStatsVec = m->vec;
        }
    }
}


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


void Statistics::printStatistics()
{
    EV << endl;

    EV << "******************************************************" << endl;
    EV << "The simulation completed at " << simTime().dbl() << endl;
    EV << "******************************************************" << endl;

    for(unsigned int i=0; i<Vec_MacStat.size(); i++)
    {
        EV << "Mac statistics for " << Vec_MacStat[i]->name1 << ": " << endl;

        for(unsigned int j=0; j<Vec_MacStat[i]->MacStatsVec.size(); j++)
        {
            EV << Vec_MacStat[i]->MacStatsVec[j] << endl;
        }

        EV << endl;
    }
}


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
    std::ostringstream oss;

    for(int h=0 ; ModName[h] != NULL ; h++)
    {
        if ( std::isdigit(ModName[h]) )
        {
            oss << ModName[h];
        }
    }

    int nodeID = atoi(oss.str().c_str());

    return nodeID;
}


int Statistics::findInVector(std::vector<NodeEntry *> Vec, const char *name)
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


int Statistics::findInVector(std::vector<MacStatEntry *> Vec, const char *name)
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


std::vector<NodeEntry *> Statistics::SortByID(std::vector<NodeEntry *> vec)
{
    if(vec.size() == 0)
        return vec;

    for(unsigned int i = 0; i<vec.size()-1; i++)
    {
        for(unsigned int j = i+1; j<vec.size(); j++)
        {
            if( vec[i]->nodeID > vec[j]->nodeID)
                std::swap(vec[i], vec[j]);
        }
    }

    return vec;
}


void Statistics::finish()
{

}


Statistics::~Statistics()
{

}
