/****************************************************************************/
/// @file    TrafficLightControl.cc
/// @author  Philip Vo <foxvo@ucdavis.edu>
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

#include <TrafficLightControl.h>

namespace VENTOS {

Define_Module(VENTOS::TrafficLightControl);


TrafficLightControl::~TrafficLightControl()
{

}


void TrafficLightControl::initialize(int stage)
{
    TrafficLightBase::initialize(stage);

    if(stage == 0)
    {
        TLControlMode = par("TLControlMode");
        updateInterval = TraCI->par("updateInterval").doubleValue();
    }
}


void TrafficLightControl::finish()
{

}


void TrafficLightControl::handleMessage(cMessage *msg)
{

}


void TrafficLightControl::executeFirstTimeStep()
{
    string program;

    // Set with TL program to use.
    // Actuated TL program (via loop detectors):
    if (TLControlMode == 2)
        program = "actuated";
    // All other programs:
    else
        program = "static";

    // set program for all TLs
    TLlidLst = TraCI->commandGetTLIDList();
    for (list<string>::iterator TL = TLlidLst.begin(); TL != TLlidLst.end(); TL++)
    {
        TraCI->commandSetTLProgram(*TL, program);
    }

    nextTime = simTime().dbl() + updateInterval;
}


void TrafficLightControl::executeEachTimeStep()
{
    cout << "time: " << simTime().dbl() << endl;

    VehicleLst = TraCI->commandGetVehicleList();

    if (TLControlMode == 3)
    {
        // For each traffic light:
        for (list<string>::iterator TL = TLlidLst.begin(); TL != TLlidLst.end(); TL++)
        {
            if (simTime().dbl() == nextTime)
            {
                // Look for vehicles within the radius of that traffic light
                // (this will be simpler with RSU):
                vector<string> VehInRange;
                for(list<string>::iterator V = VehicleLst.begin(); V != VehicleLst.end(); V++)
                {
                    //cout << *V << " Edge: " << TraCI->commandGetVehicleEdgeId(*V) << " Lane" << TraCI->commandGetVehicleLaneId(*V) << " LaneIndex: " << TraCI->commandGetVehicleLanePosition(*V) << endl;
                    // If within 50m of traffic light and heading towards TL:
                    Coord pos = TraCI->commandGetVehiclePos(*V);
                    string edge = TraCI->commandGetVehicleEdgeId(*V);
                    if (pos.x > 50 && pos.x < 150 && pos.y > 50 && pos.y < 150 &&
                            (edge == "NC" || edge == "EC" || edge == "SC" || edge == "WC"))
                    {
                        if (    ((edge == "NC" || edge == "SC") && (prevEdge == "NC" || prevEdge == "SC")) ||
                                ((edge == "WC" || edge == "EC") && (prevEdge == "WC" || prevEdge == "EC"))  )
                        {
                            cout << "\nEdge:" << edge << " Prev edge:" << prevEdge << endl;
                            TraCI->commandSetTLPhaseDurationRemaining(*TL, passTime*1000);
                            nextTime = simTime().dbl() + passTime;
                            goto EndTLForLoop2;
                        }
                        VehInRange.push_back(*V);
                    }
                }

                if (!VehInRange.empty())
                {
                    vector<string>::iterator VIR;
                    VIR = VehInRange.begin();
                    string edge = TraCI->commandGetVehicleEdgeId(*VIR);
                    int currentphase = TraCI->commandGetCurrentPhase(*TL);

                    if (edge == "NC" || edge == "SC")
                    {
                        cout << "\t" << "current phase: " << currentphase << endl;
                        cout<<"\tDuration: "<<TraCI->commandGetCurrentPhaseDuration(*TL)<<" Switch: "<< TraCI->commandGetNextSwitchTime(*TL) << endl;

                        if (currentphase == 0)
                        {
                            TraCI->commandSetTLPhaseDurationRemaining(*TL, 0);
                            nextTime = simTime().dbl() + 5 + passTime;
                        }
                        prevEdge = edge;
                        cout<<"New:"<<endl;
                        cout<<"\tDuration: "<<TraCI->commandGetCurrentPhaseDuration(*TL)<<" Switch: "<< TraCI->commandGetNextSwitchTime(*TL) << endl;
                        cout << endl;
                    }
                    else if (edge == "EC" || edge == "WC")
                    {
                        if (currentphase == 3)
                        {
                            TraCI->commandSetTLPhaseDurationRemaining(*TL, 0);
                            nextTime = simTime().dbl() + 5 + passTime;
                        }
                        prevEdge = edge;
                    }
                    cout << "cphase" << currentphase << endl;

                }
                else
                    nextTime = simTime().dbl() + updateInterval;
            }
            EndTLForLoop2:;
            cout << nextTime << endl;
        }

    }
    // Network-controlled TL program with minimum queue size set:
    else if (TLControlMode == 4)
    {
        cout << "Current phase: " << TraCI->commandGetCurrentPhase("C") << endl;
        // For each traffic light:
        list<string>::iterator TL;
        for (TL = TLlidLst.begin(); TL != TLlidLst.end(); TL++)
        {
            if (simTime().dbl() == 1 || simTime().dbl() == nextTime)
            {
                // Look for vehicles within the radius of that traffic light
                // (this will be simpler with RSU):
                vector<string> vertical;
                vector<string> horizontal;
                list<string>::iterator V;
                for(V = VehicleLst.begin(); V != VehicleLst.end(); V++)
                {
                    // If within 50m of traffic light and heading towards TL:
                    Coord pos = TraCI->commandGetVehiclePos(*V);
                    string edge = TraCI->commandGetVehicleEdgeId(*V);
                    if (pos.x > 50 && pos.x < 150 && pos.y > 50 && pos.y < 150 &&
                            (edge == "NC" || edge == "EC" || edge == "SC" || edge == "WC"))
                    {
                        /*  if (edge == "NC" || edge == "SC")
                                    vertical.pushback(*V);
                                else if (edge == "WC" || edge == "EC")
                                    horizontal.pushback(*V);
                                {
                                    //                            cout << "\nEdge:" << edge << " Prev edge:" << prevEdge << endl;
                                    //                            TraCI->commandSetTLPhaseDurationRemaining(*TL, passTime*1000);
                                    //                            nextTime = simTime().dbl() + passTime;
                                    //                            goto EndTLForLoop3;
                                }*/
                    }
                }

                /*
                        if (!VehInRange.empty())
                        {
                            vector<string>::iterator VIR;
                            VIR = VehInRange.begin();
                            string edge = TraCI->commandGetVehicleEdgeId(*VIR);
                            int currentphase = TraCI->commandGetCurrentPhase(*TL);

                            if (edge == "NC" || edge == "SC")
                            {
                                cout << "\t" << "current phase: " << currentphase << endl;
                                cout<<"\tDuration: "<<TraCI->commandGetCurrentPhaseDuration(*TL)<<" Switch: "<< TraCI->commandGetNextSwitchTime(*TL) << endl;

                                if (currentphase == 0)
                                {
                                    TraCI->commandSetTLPhaseDurationRemaining(*TL, 0);
                                    nextTime = simTime().dbl() + 5 + passTime;
                                }
                                prevEdge = edge;
                                cout<<"New:"<<endl;
                                cout<<"\tDuration: "<<TraCI->commandGetCurrentPhaseDuration(*TL)<<" Switch: "<< TraCI->commandGetNextSwitchTime(*TL) << endl;
                                cout << endl;
                            }
                            else if (edge == "EC" || edge == "WC")
                            {
                                if (currentphase == 3)
                                {
                                    TraCI->commandSetTLPhaseDurationRemaining(*TL, 0);
                                    nextTime = simTime().dbl() + 5 + passTime;
                                }
                                prevEdge = edge;
                            }
                            cout << "cphase" << currentphase << endl;

                        }
                 */
                if ((prevEdge == "NC" || prevEdge == "SC"))
                {
                    if (!vertical.empty())
                    {
                        TraCI->commandSetTLPhaseDurationRemaining(*TL, passTime*1000);
                        nextTime = simTime().dbl() + passTime;
                    }
                    else if (!horizontal.empty())
                    {
                        TraCI->commandSetTLPhaseDurationRemaining(*TL, 0);
                        nextTime = simTime().dbl() + 5 + passTime;
                        prevEdge = TraCI->commandGetVehicleEdgeId(*horizontal.begin());
                    }
                }
                else if (prevEdge == "EC" || prevEdge == "WC")
                {

                }
                else
                    nextTime = simTime().dbl() + updateInterval;
            }
            EndTLForLoop3:;
            cout << nextTime << endl;
        }
    }

    // Network-controlled TL program with minimum wait time:
    else if (TLControlMode == 5)
    {
    }
}

}
