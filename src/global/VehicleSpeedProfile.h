/****************************************************************************/
/// @file    VehicleSpeedProfile.h
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

#ifndef SPEEDPROFILE
#define SPEEDPROFILE

#include "BaseApplLayer.h"
#include "TraCICommands.h"

namespace VENTOS {

class SpeedProfile : public BaseApplLayer
{
	public:
		virtual ~SpeedProfile();
		virtual void initialize(int stage);
        virtual void handleMessage(omnetpp::cMessage *msg);
		virtual void finish();

	public:
        void Change();

	private:

        // NED variables
        TraCI_Commands *TraCI;
        bool on;
        double startTime;  // the time that speed profiling starts
	    int mode;
	    std::string laneId;
	    double minSpeed;
        double normalSpeed;
        double maxSpeed;
        double switchTime;
        std::string trajectoryPath;

	    // class variables
        std::string profileVehicle;
        std::string lastProfileVehicle;
	    double old_speed;
	    double old_time;

        bool fileOpened;
        bool endOfFile;
        FILE *f2;

	    // method
	    void AccelDecel(double, double, double);
	    void AccelDecelZikZak(double, double, double);
        void AccelDecelPeriodic(double, double, double, double);
	    void ExTrajectory(double);
};
}

#endif
