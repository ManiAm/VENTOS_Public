/****************************************************************************/
/// @file    ApplRSU_04_Classify.h
/// @author  Mani Amoozadeh <maniam@ucdavis.edu>
/// @author  second author name
/// @date    Nov 2015
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

#ifndef APPLRSUCLASSIFY_H_
#define APPLRSUCLASSIFY_H_

#include "ApplRSU_03_ActiveTL.h"

//#undef ev
//#include "dlib/svm_threaded.h"
//#include "dlib/rand.h"
//#define ev  (*cSimulation::getActiveEnvir())

namespace VENTOS {

class feature
{
public:
    double xPos;
    double yPos;
    double speed;

    feature(double x, double y, double z)
    {
        this->xPos = x;
        this->yPos = y;
        this->speed = z;
    }
};

//// data type for 2-dimensional data
//typedef dlib::matrix<double,2,1> sample_type_2D;
//
//// data type for 3-dimensional data
//typedef dlib::matrix<double,3,1> sample_type_3D;

class ApplRSUCLASSIFY : public ApplRSUTLVANET
{
	public:
		~ApplRSUCLASSIFY();
		virtual void initialize(int stage);
		virtual void finish();
        virtual void handleSelfMsg(cMessage* msg);

	protected:
        void virtual executeEachTimeStep();

        virtual void onBeaconVehicle(BeaconVehicle*);
        virtual void onBeaconBicycle(BeaconBicycle*);
        virtual void onBeaconPedestrian(BeaconPedestrian*);
        virtual void onBeaconRSU(BeaconRSU*);
        virtual void onData(LaneChangeMsg*);

	private:
        void initializeGnuPlot();
        template <typename T> void onBeaconAny(T wsm);
      //  void generate_data(std::vector<sample_type_2D>& samples, std::vector<double>& labels);
      //  void generate_data_3D(std::vector<sample_type_3D>& samples, std::vector<double>& labels);
        void classifierF();

	private:
        bool classifier;
        FILE *pipe;
        std::vector<feature> dataSet [3];
};

}

#endif
