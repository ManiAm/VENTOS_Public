/****************************************************************************/
/// @file    TL_AdaptiveQueue.cc
/// @author  Mani Amoozadeh <maniam@ucdavis.edu>
/// @date    August 2013
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

#include <05_TL_Adaptive_Queue.h>

namespace VENTOS {

Define_Module(VENTOS::TrafficLightAdaptiveQueue);


TrafficLightAdaptiveQueue::~TrafficLightAdaptiveQueue()
{

}


void TrafficLightAdaptiveQueue::initialize(int stage)
{
    TrafficLightAdaptive::initialize(stage);

    if(TLControlMode != TL_Adaptive_Time_Queue)
        return;

    if(stage == 0)
    {
        // set initial values
        intervalOffSet = minGreenTime;
        intervalElapseTime = 0;
        currentInterval = phase1_5;

        ChangeEvt = new cMessage("ChangeEvt", 1);
        scheduleAt(simTime().dbl() + intervalOffSet, ChangeEvt);

        // collect queue information
        measureIntersectionQueue = true;
    }
}


void TrafficLightAdaptiveQueue::finish()
{
    TrafficLightAdaptive::finish();

}


void TrafficLightAdaptiveQueue::handleMessage(cMessage *msg)
{
    TrafficLightAdaptive::handleMessage(msg);

    if(TLControlMode != TL_Adaptive_Time_Queue)
        return;

    if (msg == ChangeEvt)
    {
        chooseNextInterval();

        // Schedule next light change event:
        scheduleAt(simTime().dbl() + intervalOffSet, ChangeEvt);
    }
}


void TrafficLightAdaptiveQueue::executeFirstTimeStep()
{
    TrafficLightAdaptive::executeFirstTimeStep();

    if(TLControlMode != TL_Adaptive_Time_Queue)
        return;

    std::cout << "Adaptive-time with queue traffic signal control ..." << endl << endl;

    getMovements();

    for (std::list<std::string>::iterator TL = TLList.begin(); TL != TLList.end(); ++TL)
    {
        TraCI->TLSetProgram(*TL, "adaptive-time");
        TraCI->TLSetState(*TL, phase1_5);
    }

    char buff[300];
    sprintf(buff, "SimTime: %4.2f | Planned interval: %s | Start time: %4.2f | End time: %4.2f", simTime().dbl(), currentInterval.c_str(), simTime().dbl(), simTime().dbl() + intervalOffSet);
    std::cout << buff << endl;
}


void TrafficLightAdaptiveQueue::executeEachTimeStep(bool simulationDone)
{
    TrafficLightAdaptive::executeEachTimeStep(simulationDone);

    if(TLControlMode != TL_Adaptive_Time_Queue)
        return;

    intervalElapseTime += updateInterval;
}


void TrafficLightAdaptiveQueue::getMovements()
{
    // Get all links for this TL
    std::map<int,std::string> allLinks = TraCI->TLGetControlledLinks("C");

    LINKSIZE = allLinks.size();

    if(LINKSIZE == 0)
        error("LINKSIZE is zero!");

    movementsFilePath = SUMO_FullPath / "allMovements.txt";

    // check if this file exists?
    if( !boost::filesystem::exists( movementsFilePath ) )
        generateAllAllowedMovements();
    else
    {
        std::cout << "Reading movements from file ... " << std::flush;
        FILE *filePtr = fopen (movementsFilePath.string().c_str(), "r");

        std::vector<int> temp;
        char c;
        allMovements.clear();

        while ((c = getc(filePtr)) != EOF)
        {
            if(c != '\n')
            {
                int val = c - '0';
                temp.push_back(val);
            }
            else
            {
                allMovements.push_back(temp);
                temp.clear();
            }
        }

        fclose(filePtr);
        std::cout << "Done!" << endl << endl;
    }

    if(allMovements.size() == 0)
        error("size of allMovements is zero!");
}


void TrafficLightAdaptiveQueue::generateAllAllowedMovements()
{
    // Create a graph type
    typedef boost::adjacency_list<boost::mapS /*OutEdgeList*/, boost::vecS /*VertexListS*/, boost::undirectedS /*graph type*/> Graph;

    // Create the graph object
    Graph conflictGraph(LINKSIZE);

    // Add edges for link 3 (vehNS)
    add_edge(3, 8, conflictGraph);
    add_edge(3, 9, conflictGraph);
    add_edge(3, 14, conflictGraph);
    add_edge(3, 18, conflictGraph);
    add_edge(3, 19, conflictGraph);
    add_edge(3, 6, conflictGraph);
    add_edge(3, 16, conflictGraph);
    add_edge(3, 20, conflictGraph);
    add_edge(3, 22, conflictGraph);

    // Add edges for link 4 (vehNE)
    add_edge(4, 8, conflictGraph);
    add_edge(4, 9, conflictGraph);
    add_edge(4, 13, conflictGraph);
    add_edge(4, 18, conflictGraph);
    add_edge(4, 19, conflictGraph);
    add_edge(4, 6, conflictGraph);
    add_edge(4, 11, conflictGraph);
    add_edge(4, 16, conflictGraph);
    add_edge(4, 20, conflictGraph);
    add_edge(4, 21, conflictGraph);

    // Add edges for link 8 (vehEW)
    add_edge(8, 3, conflictGraph);
    add_edge(8, 4, conflictGraph);
    add_edge(8, 13, conflictGraph);
    add_edge(8, 14, conflictGraph);
    add_edge(8, 19, conflictGraph);
    add_edge(8, 1, conflictGraph);
    add_edge(8, 11, conflictGraph);
    add_edge(8, 21, conflictGraph);
    add_edge(8, 23, conflictGraph);

    // Add edges for link 9 (vehES)
    add_edge(9, 3, conflictGraph);
    add_edge(9, 4, conflictGraph);
    add_edge(9, 13, conflictGraph);
    add_edge(9, 14, conflictGraph);
    add_edge(9, 18, conflictGraph);
    add_edge(9, 1, conflictGraph);
    add_edge(9, 11, conflictGraph);
    add_edge(9, 16, conflictGraph);
    add_edge(9, 21, conflictGraph);
    add_edge(9, 22, conflictGraph);

    // Add edges for link 13 (vehSN)
    add_edge(13, 4, conflictGraph);
    add_edge(13, 8, conflictGraph);
    add_edge(13, 9, conflictGraph);
    add_edge(13, 18, conflictGraph);
    add_edge(13, 19, conflictGraph);
    add_edge(13, 6, conflictGraph);
    add_edge(13, 16, conflictGraph);
    add_edge(13, 20, conflictGraph);
    add_edge(13, 22, conflictGraph);

    // Add edges for link 14 (vehSW)
    add_edge(14, 3, conflictGraph);
    add_edge(14, 8, conflictGraph);
    add_edge(14, 9, conflictGraph);
    add_edge(14, 18, conflictGraph);
    add_edge(14, 19, conflictGraph);
    add_edge(14, 1, conflictGraph);
    add_edge(14, 6, conflictGraph);
    add_edge(14, 16, conflictGraph);
    add_edge(14, 22, conflictGraph);
    add_edge(14, 23, conflictGraph);

    // Add edges for link 18 (vehWE)
    add_edge(18, 3, conflictGraph);
    add_edge(18, 4, conflictGraph);
    add_edge(18, 9, conflictGraph);
    add_edge(18, 13, conflictGraph);
    add_edge(18, 14, conflictGraph);
    add_edge(18, 1, conflictGraph);
    add_edge(18, 11, conflictGraph);
    add_edge(18, 21, conflictGraph);
    add_edge(18, 23, conflictGraph);

    // Add edges for link 19 (vehWN)
    add_edge(19, 3, conflictGraph);
    add_edge(19, 4, conflictGraph);
    add_edge(19, 8, conflictGraph);
    add_edge(19, 13, conflictGraph);
    add_edge(19, 14, conflictGraph);
    add_edge(19, 1, conflictGraph);
    add_edge(19, 6, conflictGraph);
    add_edge(19, 11, conflictGraph);
    add_edge(19, 20, conflictGraph);
    add_edge(19, 23, conflictGraph);

    // Add edges for link 1 (bikeNS)
    add_edge(1, 8, conflictGraph);
    add_edge(1, 9, conflictGraph);
    add_edge(1, 14, conflictGraph);
    add_edge(1, 18, conflictGraph);
    add_edge(1, 19, conflictGraph);
    add_edge(1, 6, conflictGraph);
    add_edge(1, 16, conflictGraph);
    add_edge(1, 20, conflictGraph);
    add_edge(1, 22, conflictGraph);

    // Add edges for link 6 (bikeEW)
    add_edge(6, 3, conflictGraph);
    add_edge(6, 4, conflictGraph);
    add_edge(6, 13, conflictGraph);
    add_edge(6, 14, conflictGraph);
    add_edge(6, 19, conflictGraph);
    add_edge(6, 1, conflictGraph);
    add_edge(6, 11, conflictGraph);
    add_edge(6, 21, conflictGraph);
    add_edge(6, 23, conflictGraph);

    // Add edges for link 11 (bikeSN)
    add_edge(11, 4, conflictGraph);
    add_edge(11, 8, conflictGraph);
    add_edge(11, 9, conflictGraph);
    add_edge(11, 18, conflictGraph);
    add_edge(11, 19, conflictGraph);
    add_edge(11, 6, conflictGraph);
    add_edge(11, 16, conflictGraph);
    add_edge(11, 20, conflictGraph);
    add_edge(11, 22, conflictGraph);

    // Add edges for link 16 (bikeWE)
    add_edge(16, 3, conflictGraph);
    add_edge(16, 4, conflictGraph);
    add_edge(16, 9, conflictGraph);
    add_edge(16, 13, conflictGraph);
    add_edge(16, 14, conflictGraph);
    add_edge(16, 1, conflictGraph);
    add_edge(16, 11, conflictGraph);
    add_edge(16, 21, conflictGraph);
    add_edge(16, 23, conflictGraph);

    // Add edges for link 20 (pedN)
    add_edge(20, 3, conflictGraph);
    add_edge(20, 4, conflictGraph);
    add_edge(20, 13, conflictGraph);
    add_edge(20, 19, conflictGraph);
    add_edge(20, 1, conflictGraph);
    add_edge(20, 11, conflictGraph);

    // Add edges for link 21 (pedE)
    add_edge(21, 4, conflictGraph);
    add_edge(21, 8, conflictGraph);
    add_edge(21, 9, conflictGraph);
    add_edge(21, 18, conflictGraph);
    add_edge(21, 6, conflictGraph);
    add_edge(21, 16, conflictGraph);

    // Add edges for link 22 (pedS)
    add_edge(22, 3, conflictGraph);
    add_edge(22, 9, conflictGraph);
    add_edge(22, 13, conflictGraph);
    add_edge(22, 14, conflictGraph);
    add_edge(22, 1, conflictGraph);
    add_edge(22, 11, conflictGraph);

    // Add edges for link 23 (pedW)
    add_edge(23, 8, conflictGraph);
    add_edge(23, 14, conflictGraph);
    add_edge(23, 18, conflictGraph);
    add_edge(23, 19, conflictGraph);
    add_edge(23, 6, conflictGraph);
    add_edge(23, 16, conflictGraph);

    std::cout << "Generating all possible non-conflicting movements (out of 2^" << LINKSIZE << ") ... " << std::flush;

    // generate truth table from all links
    // check here: http://stackoverflow.com/questions/3504642/generate-a-truth-table-given-an-input
    std::vector< std::vector<int> > truthTable(LINKSIZE, std::vector<int>(1 << LINKSIZE));
    unsigned num_to_fill = 1U << (LINKSIZE - 1);
    for(int col = 0; col < LINKSIZE; ++col, num_to_fill >>= 1U)
    {
        for(unsigned row = num_to_fill; row < (1U << LINKSIZE); row += (num_to_fill * 2))
        {
            std::fill_n(&truthTable[col][row], num_to_fill, 1);
        }
    }

    std::vector<int> temp;

    // iterate over each row of the truth table (skip the first row which is all-zero)
    for(int x = 1; x < (1 << LINKSIZE); ++x)   // row
    {
        bool conflictFound = false;
        temp.clear();

        for(int y = 0; y < LINKSIZE; ++y)  // column
        {
            temp.push_back(truthTable[y][x]);

            if( truthTable[y][x] != 1 )
                continue;

            for(int z=y+1; z < LINKSIZE; ++z)
            {
                if( truthTable[z][x] != 1 )
                    continue;

                // check if link y and link z are in conflict
                if( edge(y, z, conflictGraph).second )
                {
                    conflictFound = true;
                    temp.clear();
                    break;
                }
            }

            if(conflictFound)
                break;
        }

        // store this row as a non-conflicting movement
        if(!conflictFound)
            allMovements.push_back(temp);
    }

    std::cout << allMovements.size() << " movements found!" << endl;

    // write it to file
    std::cout << "Writing to file ... " << std::flush;
    FILE *filePtr = fopen (movementsFilePath.string().c_str(), "w");
    for(std::vector< std::vector<int> >::iterator it = allMovements.begin(); it != allMovements.end(); ++it)
    {
        for(std::vector<int>::iterator it2 = (*it).begin(); it2 != (*it).end(); ++it2)
        {
            fprintf(filePtr, "%d", *it2);
        }
        fprintf (filePtr, "\n");
    }

    fclose(filePtr);
    std::cout << "Done!" << endl << endl;
}


void TrafficLightAdaptiveQueue::chooseNextInterval()
{
    if (currentInterval == "yellow")
    {
        currentInterval = "red";

        // change all 'y' to 'r'
        std::string str = TraCI->TLGetState("C");
        std::string nextInterval = "";
        for(char& c : str) {
            if (c == 'y')
                nextInterval += 'r';
            else
                nextInterval += c;
        }

        // set the new state
        TraCI->TLSetState("C", nextInterval);
        intervalElapseTime = 0.0;
        intervalOffSet = redTime;
    }
    else if (currentInterval == "red")
    {
        currentInterval = nextGreenInterval;

        // set the new state
        TraCI->TLSetState("C", nextGreenInterval);
        intervalElapseTime = 0.0;
        intervalOffSet = minGreenTime;
    }
    else
    {
        chooseNextGreenInterval();
    }

    char buff[300];
    sprintf(buff, "SimTime: %4.2f | Planned interval: %s | Start time: %4.2f | End time: %4.2f", simTime().dbl(), currentInterval.c_str(), simTime().dbl(), simTime().dbl() + intervalOffSet);
    std::cout << buff << endl;
}


void TrafficLightAdaptiveQueue::chooseNextGreenInterval()
{
    int maxQueue = -1;
    int numMoves = -1;
    int row = -1;

    // get which row has the highest queue length
    for(unsigned int i = 0; i < allMovements.size(); ++i)  // row
    {
        int totalQueueRow = 0;
        int oneCount = 0;

        for(int j = 0; j < LINKSIZE; ++j)  // column
        {
            if(allMovements[i][j] == 1)
            {
                totalQueueRow = totalQueueRow + linkQueueSize[std::make_pair("C",j)];
                oneCount++;
            }
        }

        // if we find a row with higher total queue size or
        // equal queue size but with higher number of allowed non-conflicting movements
        if( (totalQueueRow > maxQueue) || (totalQueueRow == maxQueue && oneCount > numMoves) )
        {
            maxQueue = totalQueueRow;
            numMoves = oneCount;
            row = i;
        }
    }

    if(maxQueue == -1 || row == -1)
        error("something is wrong!");

    // Calculate the next green interval.
    // right-turns are all permissive thus using 'g'
    nextGreenInterval = "";
    int rightTurns[] = {2, 7, 12, 17, 0, 5, 10, 15};
    for(int j = 0; j < LINKSIZE; ++j)
    {
        // if a right turn
        bool exists = std::find(std::begin(rightTurns), std::end(rightTurns), j) != std::end(rightTurns);

        if(allMovements[row][j] == 0)
            nextGreenInterval += 'r';
        else if(allMovements[row][j] == 1 && exists)
            nextGreenInterval += 'g';
        else
            nextGreenInterval += 'G';
    }

    // Calculate 'next interval'
    std::string nextInterval = "";
    bool needYellowInterval = false;  // if we have at least one yellow interval
    for(int i = 0; i < LINKSIZE; ++i)
    {
        if( (currentInterval[i] == 'G' || currentInterval[i] == 'g') && nextGreenInterval[i] == 'r')
        {
            nextInterval += 'y';
            needYellowInterval = true;
        }
        else
            nextInterval += currentInterval[i];
    }

    std::cout << endl;
    std::cout << "set of links with max q     ";
    for(int k =0; k < LINKSIZE; ++k)
        if(allMovements[row][k] == 1)
            std::cout << k << " (" << linkQueueSize[std::make_pair("C",k)] << "), ";
    std::cout << endl;
    std::cout << "current interval            " << currentInterval << endl;
    std::cout << "next green interval         " << nextGreenInterval << endl;
    std::cout << "next interval               " << nextInterval << endl;

    if(needYellowInterval)
    {
        currentInterval = "yellow";
        TraCI->TLSetState("C", nextInterval);

        intervalElapseTime = 0.0;
        intervalOffSet =  yellowTime;
    }
    else
    {
        intervalOffSet = minGreenTime;
        std::cout << "Continue the last green interval." << endl;
    }
}

}
