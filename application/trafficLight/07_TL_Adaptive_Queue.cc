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

#include <07_TL_Adaptive_Queue.h>
#include <boost/graph/adjacency_list.hpp>

std::vector<int> bestMovement;


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
        ChangeEvt = new cMessage("ChangeEvt", 1);

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
        if(phases.empty())
        {
            calculatePhases();
        }

        chooseNextInterval();

        if(intervalOffSet <= 0)
            error("intervalOffSet is <= 0");

        // Schedule next light change event:
        scheduleAt(simTime().dbl() + intervalOffSet, ChangeEvt);
    }
}


void TrafficLightAdaptiveQueue::executeFirstTimeStep()
{
    TrafficLightAdaptive::executeFirstTimeStep();

    if(TLControlMode != TL_Adaptive_Time_Queue)
        return;

    std::cout << endl << "Adaptive-time with queue traffic signal control ..." << endl << endl;

    // set initial values
    currentInterval = phase1_5;
    intervalElapseTime = 0;
    intervalOffSet = minGreenTime;

    scheduleAt(simTime().dbl() + intervalOffSet, ChangeEvt);

    getMovements();

    calculatePhases();

    currentInterval = phases.front();

    for (std::list<std::string>::iterator TL = TLList.begin(); TL != TLList.end(); ++TL)
    {
        TraCI->TLSetProgram(*TL, "adaptive-time");
        TraCI->TLSetState(*TL, currentInterval);
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
    std::map<int,std::vector<std::string>> allLinks = TraCI->TLGetControlledLinks("C");

    LINKSIZE = allLinks.size();

    if(LINKSIZE == 0)
        error("LINKSIZE can not be zero for this TL!");

    movementsFilePath = SUMO_FullPath / "allMovements.txt";

    // check if this file exists?
    if( boost::filesystem::exists( movementsFilePath ) )
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
    else
        generateAllAllowedMovements();

    if(allMovements.size() == 0)
        error("size of allMovements is zero!");
}


// if allMovements.txt is not found then we should generate it.
// this process is CPU-intensive and lengthy, but hopefully this
// needs to be executed once to obtain allMovements.txt
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

        // if this row has no conflicts
        if(!conflictFound)
        {
            // now we check if this row covers all the right turns
            // note that right turns are all permissive
            bool coversRightTurns = true;
            for(int j = 0; j < 8; ++j)
            {
                // if not a right turn
                if ( temp[rightTurns[j]] == 0 )
                {
                    coversRightTurns = false;
                    break;
                }
            }

            if(coversRightTurns)
            {
                // store this row as a non-conflicting movement
                allMovements.push_back(temp);
            }
        }
    }

    std::cout << allMovements.size() << " movements found!" << endl;

    // write allMovements to file
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


bool pred(std::vector<int> v)
{
    for (int i = 0; i < v.size(); i++)
    {
        // Ignore all permissive right turns since these are always green:
        if (i == 0 || i == 2 || i == 5 || i == 7 ||
                i == 10 || i == 12 || i == 15 || i == 17)
            continue;

        // Want to remove movements that have already been done:
        if (v[i] && bestMovement[i])
        {
            return true;
        }
    }
    return false;
}


// calculate all phases (up to 4)
void TrafficLightAdaptiveQueue::calculatePhases()
{
    // clear the priority queue
    batchMovementQueue = std::priority_queue < batchMovementQueueEntry, std::vector<batchMovementQueueEntry>, movementCompare >();

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

        // add this batch of movements to priority_queue
        batchMovementQueueEntry *entry = new batchMovementQueueEntry(oneCount, totalQueueRow, allMovements[i]);
        batchMovementQueue.push(*entry);
    }

    // Save all batchMovements to a vector for iteration:
    std::vector < std::vector<int> > movements;
    while(!batchMovementQueue.empty())
    {
        batchMovementQueueEntry entry = batchMovementQueue.top();
        movements.push_back(entry.batchMovements);
        batchMovementQueue.pop();
    }

    // Select at most 4 phases for new cycle:
    while(!movements.empty())
    {
        std::cout << "Movements SIZE: " << movements.size() << endl;
        // Always select the first movement because it will be the best(?):
        bestMovement = movements.front();
        movements.erase(movements.begin());

        // Change all 1's to G, 0's to r:
        std::string nextInterval = "";
        for(int i : bestMovement) {
            if (i == 1)
                nextInterval += 'G';
            else
                nextInterval += 'r';
        }
        // Avoid pushing "only permissive right turns" phase:
        if (nextInterval == "GrGrrGrGrrGrGrrGrGrrrrrr")
                continue;
        phases.push_back(nextInterval);

        // Now delete these movements because they should never occur again:
        movements.erase( std::remove_if(movements.begin(), movements.end(), pred), movements.end() );
    }

    std::cout << "SIZE: " << phases.size() << endl;
    for(int i = 0; i < phases.size(); i++)
    {
        std::cout << phases[i] << endl;
    }

    // todo: Include error code if more than 4 phases:
    if (phases.size() > 4)
    {

    }

    std::cout << endl;
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
    // Remove current old phase:
    phases.erase(phases.begin());

    // Assign new green:
    if (phases.empty())
        calculatePhases();

    nextGreenInterval = phases.front();


    currentInterval = "yellow";

    // change all 'G/g' to 'y'
    std::string str = TraCI->TLGetState("C");
    std::string nextInterval = "";
    for(char& c : str) {
        if (c == 'G' || c == 'g')
            nextInterval += 'y';
        else
            nextInterval += c;
    }

    TraCI->TLSetState("C", nextInterval);

    intervalElapseTime = 0.0;
    intervalOffSet =  yellowTime;

    char buff[300];
    sprintf(buff, "SimTime: %4.2f | Planned interval: %s | Start time: %4.2f | End time: %4.2f", simTime().dbl(), currentInterval.c_str(), simTime().dbl(), simTime().dbl() + intervalOffSet);
    std::cout << buff << endl << endl;




    //    // Calculate the next green interval.
    //    // right-turns are all permissive and are given 'g'
    //    nextGreenInterval = "";
    //    for(int j = 0; j < LINKSIZE; ++j)
    //    {
    //        // if a right turn
    //        bool rightTurn = std::find(std::begin(rightTurns), std::end(rightTurns), j) != std::end(rightTurns);
    //
    //        if(allMovements[row][j] == 0)
    //            nextGreenInterval += 'r';
    //        else if(allMovements[row][j] == 1 && rightTurn)
    //            nextGreenInterval += 'g';
    //        else
    //            nextGreenInterval += 'G';
    //    }
    //
    //    // Calculate 'next interval'
    //    std::string nextInterval = "";
    //    bool needYellowInterval = false;  // if we have at least one yellow interval
    //    for(int i = 0; i < LINKSIZE; ++i)
    //    {
    //        if( (currentInterval[i] == 'G' || currentInterval[i] == 'g') && nextGreenInterval[i] == 'r')
    //        {
    //            nextInterval += 'y';
    //            needYellowInterval = true;
    //        }
    //        else
    //            nextInterval += currentInterval[i];
    //    }
    //
    //    // print debugging
    //    std::cout << endl;
    //    std::cout << "set of links with max q     ";
    //    for(int k =0; k < LINKSIZE; ++k)
    //        if(allMovements[row][k] == 1)
    //            std::cout << k << " (" << linkQueueSize[std::make_pair("C",k)] << "), ";
    //    std::cout << endl;
    //    std::cout << "current interval            " << currentInterval << endl;
    //    std::cout << "next green interval         " << nextGreenInterval << endl;
    //    std::cout << "next interval               " << nextInterval << endl;
    //
    //    if(needYellowInterval)
    //    {
    //        currentInterval = "yellow";
    //        TraCI->TLSetState("C", nextInterval);
    //
    //        intervalElapseTime = 0.0;
    //        intervalOffSet =  yellowTime;
    //    }
    //    else
    //    {
    //        intervalOffSet = minGreenTime;
    //        std::cout << "Continue the last green interval." << endl;
    //    }
}

}
