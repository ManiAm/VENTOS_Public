/****************************************************************************/
/// @file    TraCI_App.h
/// @author  Mani Amoozadeh <maniam@ucdavis.edu>
/// @author  second author name
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

#ifndef TraCI_APP
#define TraCI_APP

#include "TraCI_Extend.h"
#include "Router.h"

namespace VENTOS {

class VehicleData
{
  public:
    int index;
    double time;
    std::string vehicleName;
    std::string vehicleType;
    std::string lane;
    double pos;
    double speed;
    double accel;
    std::string CFMode;
    double timeGapSetting;
    double spaceGap;
    double timeGap;
    std::string TLid;  // TLid that controls this vehicle
    int YorR;          // if the TL state ahead is yellow or red

    VehicleData(int i, double d1, std::string str1, std::string str2, std::string str3, double d2, double d3, double d4, std::string str4, double d3a, double d5, double d6, std::string str5, int YR)
    {
        this->index = i;
        this->time = d1;
        this->vehicleName = str1;
        this->vehicleType = str2;
        this->lane = str3;
        this->pos = d2;
        this->speed = d3;
        this->accel = d4;
        this->CFMode = str4;
        this->timeGapSetting = d3a;
        this->spaceGap = d5;
        this->timeGap = d6;
        this->TLid = str5;
        this->YorR = YR;
    }
};


class Router;   //Forward-declaration so TraCI_App may hold a Router*

class TraCI_App : public TraCI_Extend
{
    public:
        virtual ~TraCI_App();
        virtual void initialize(int stage);
        virtual int numInitStages() const
        {
            return 3;
        }
        virtual void handleSelfMsg(cMessage *msg);
        virtual void finish();

    private:
        // NED variables
        cModule *nodePtr;   // pointer to the Node
        Router* router;
        double terminate;
        bool collectVehiclesData;
        bool useDetailedFilenames;

        std::set<std::string> subscribedPedestrians; /**< all pedestrians we have already subscribed to */

        // NED (bicycles)
        std::string bikeModuleType;
        std::string bikeModuleName;
        std::string bikeModuleDisplayString;

        // NED (pedestrians)
        std::string pedModuleType;
        std::string pedModuleName;
        std::string pedModuleDisplayString;

        std::list<std::string> TLList;   // list of traffic-lights in the network
        std::vector<VehicleData> Vec_vehiclesData;

        int index;

    private:
        virtual void init_traci();
        virtual void executeOneTimestep();
        void addPedestriansToOMNET();
        virtual void addModule(std::string nodeId, std::string type, std::string name, std::string displayString, const Coord& position, std::string road_id = "", double speed = -1, double angle = -1);

        void vehiclesData();
        void saveVehicleData(std::string);
        void vehiclesDataToFile();
};

}

#endif
