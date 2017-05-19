/****************************************************************************/
/// @file    AllowedMoves.cc
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

#include <boost/graph/adjacency_list.hpp>
#include "trafficLight/05_AllowedMoves.h"

namespace VENTOS {

Define_Module(VENTOS::TrafficLightAllowedMoves);


TrafficLightAllowedMoves::~TrafficLightAllowedMoves()
{

}


void TrafficLightAllowedMoves::initialize(int stage)
{
    super::initialize(stage);

    if(stage == 0)
    {

    }
}


void TrafficLightAllowedMoves::finish()
{
    super::finish();
}


void TrafficLightAllowedMoves::handleMessage(omnetpp::cMessage *msg)
{
    super::handleMessage(msg);
}


void TrafficLightAllowedMoves::initialize_withTraCI()
{
    super::initialize_withTraCI();
}


void TrafficLightAllowedMoves::executeEachTimeStep()
{
    super::executeEachTimeStep();
}


std::vector<std::vector<int>>& TrafficLightAllowedMoves::getMovements(std::string TLid)
{
    // Get all links for this TL
    auto allLinks = TraCI->TLGetControlledLinks(TLid);

    LINKSIZE = allLinks.size();

    if(LINKSIZE == 0)
        throw omnetpp::cRuntimeError("LINKSIZE can not be zero for this TL!");

    boost::filesystem::path dir (TraCI->getFullPath_SUMOConfig().parent_path());
    movementsFilePath = dir / "allMovements.txt";

    // check if this file exists?
    if( boost::filesystem::exists( movementsFilePath ) )
    {
        LOG_INFO << "\nReading movements from file ... " << std::flush;
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
        LOG_INFO << "Done! \n";
    }
    else
        generateAllAllowedMovements();

    if(allMovements.empty())
        throw omnetpp::cRuntimeError("allMovements vector is empty!");

    if(omnetpp::cSimulation::getActiveEnvir()->isGUI() && debugLevel > 2)
        allMovementBatch(14);

    return allMovements;
}


// if allMovements.txt is not found then we should generate it.
// this process is CPU-intensive and lengthy, but hopefully this
// needs to be executed once to obtain allMovements.txt
void TrafficLightAllowedMoves::generateAllAllowedMovements()
{
    // Create a graph type
    typedef boost::adjacency_list<boost::mapS /*OutEdgeList*/, boost::vecS /*VertexListS*/, boost::undirectedS /*graph type*/> Graph;

    // Create the graph object
    Graph conflictGraph(LINKSIZE);

    /* these are the list of all movements in our intersection
        Movements from north:
        0: NW, bike
        1: NS, bike
        2: NW, veh
        3: NS, veh
        4: NE, veh

        Movements from east:
        5: EN, bike
        6: EW, bike
        7: EN, veh
        8: EW, veh
        9: ES, veh

        Movements from south:
        10: SE, bike
        11: SN, bike
        12: SE, veh
        13: SN, veh
        14: SW, veh

        Movements from west:
        15: WS, bike
        16: WE, bike
        17: WS, veh
        18: WE, veh
        19: WN, veh

        Pedestrian crossings:
        20: N
        21: E
        22: S
        23: W
     */

    // Add edges for link 3 (vehNS)
    add_edge(3, 8, conflictGraph);  // movement 3 (NS) and movement 8 (EW) are conflicting
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

    LOG_INFO << "\nGenerating all possible non-conflicting movements (out of 2^" << LINKSIZE << ") ... " << std::flush;

    // generate truth table from all links
    // check here: http://stackoverflow.com/questions/3504642/generate-a-truth-table-given-an-input
    std::vector< std::vector<int> > truthTable(LINKSIZE, std::vector<int>(1 << LINKSIZE));
    unsigned num_to_fill = 1U << (LINKSIZE - 1);
    for(int col = 0; col < LINKSIZE; ++col, num_to_fill >>= 1U)
    {
        for(unsigned row = num_to_fill; row < (1U << LINKSIZE); row += (num_to_fill * 2))
            std::fill_n(&truthTable[col][row], num_to_fill, 1);
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

        // if this row contains no conflicts
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

            // store this row as a non-conflicting movement
            if(coversRightTurns)
                allMovements.push_back(temp);
        }
    }

    std::cout << allMovements.size() << " movements found!" << std::endl;

    // write allMovements to file
    std::cout << "Writing to file ... " << std::flush;
    FILE *filePtr = fopen (movementsFilePath.string().c_str(), "w");
    for(auto &it : allMovements)
    {
        for(auto &it2 : it)
            fprintf(filePtr, "%d", it2);

        fprintf (filePtr, "\n");
    }

    fclose(filePtr);
    std::cout << "Done!" << std::endl << std::endl;
}


void TrafficLightAllowedMoves::allMovementBatch(unsigned int linkNumber)
{
    std::map<unsigned int, std::string> linkStr =
    {
            // Movements from north
            {0, "NW_b"},
            {1, "NS_b"},
            {2, "NW_v"},
            {3, "NS_v"},
            {4, "NE_v"},

            // Movements from east
            {5, "EN_b"},
            {6, "EW_b"},
            {7, "EN_v"},
            {8, "EW_v"},
            {9, "ES_v"},

            // Movements from south
            {10, "SE_b"},
            {11, "SN_b"},
            {12, "SE_v"},
            {13, "SN_v"},
            {14, "SW_v"},

            // Movements from west
            {15, "WS_b"},
            {16, "WE_b"},
            {17, "WS_v"},
            {18, "WE_v"},
            {19, "WN_v"},

            // Pedestrian crossings
            {20, "N_p"},
            {21, "E_p"},
            {22, "S_p"},
            {23, "W_p"},
    };

    printf("Non-conflicting movements with movement %d (%s): \n", linkNumber, linkStr[linkNumber].c_str());

    std::vector<std::string> linkName;
    for(auto row : allMovements)
    {
        if(row[linkNumber] == 0)
            continue;

        linkName.clear();
        for(unsigned int j = 0; j < row.size(); j++)
        {
            bool rightTurn = std::find(std::begin(rightTurns), std::end(rightTurns), j) != std::end(rightTurns);

            if(rightTurn)
                std::cout << "* ";
            else
            {
                std::cout << row[j] << " ";
                if(row[j] == 1 && j != linkNumber)
                    linkName.push_back(linkStr[j]);
            }
        }

        std::cout << " --> ";

        for(auto k : linkName)
            std::cout << k << ", ";

        std::cout << std::endl;
    }

    std::cout << std::endl;
    std::cout.flush();
}


bool TrafficLightAllowedMoves::isRightTurn(unsigned int linkNumber)
{
    auto it = std::find(std::begin(rightTurns), std::end(rightTurns), linkNumber);

    if(it != std::end(rightTurns))
        return true;

    return false;
}

}
