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

    char vehicleName[20];
    char vehicleType[20];

    char lane[20];
    double pos;

    double speed;
    double accel;
    char CFMode[30];

    double timeGapSetting;
    double spaceGap;
    double timeGap;

    VehicleData(int i, double d1,
                 const char *str1, const char *str2,
                 const char *str3, double d2,
                 double d3, double d4, const char *str4,
                 double d3a, double d5, double d6)
    {
        this->index = i;
        this->time = d1;

        strcpy(this->vehicleName, str1);
        strcpy(this->vehicleType, str2);

        strcpy(this->lane, str3);
        this->pos = d2;

        this->speed = d3;
        this->accel = d4;
        strcpy(this->CFMode, str4);

        this->timeGapSetting = d3a;
        this->spaceGap = d5;
        this->timeGap = d6;
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
        string bikeModuleType;
        string bikeModuleName;
        string bikeModuleDisplayString;

        // NED (pedestrians)
        string pedModuleType;
        string pedModuleName;
        string pedModuleDisplayString;

        vector<VehicleData *> Vec_vehiclesData;

        int index;

    private:
        virtual void init_traci();
        virtual void executeOneTimestep();
        void addPedestriansToOMNET();
        virtual void addModule(std::string nodeId, std::string type, std::string name, std::string displayString, const Coord& position, std::string road_id = "", double speed = -1, double angle = -1);

        void vehiclesData();
        void saveVehicleData(string);
        void vehiclesDataToFile();
};

}

#endif
