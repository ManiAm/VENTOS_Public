/****************************************************************************/
/// @file    Classify.h
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

#include <boost/circular_buffer.hpp>

#undef ev
#include <shark/Data/Dataset.h>
#include <shark/Data/Csv.h>
#include <shark/ObjectiveFunctions/Loss/ZeroOneLoss.h>

#include <shark/LinAlg/Base.h>
#include <shark/Models/Kernels/GaussianRbfKernel.h>
#include <shark/Algorithms/Trainers/CSvmTrainer.h>

#undef ev
#include "boost/filesystem.hpp"

#include "global/gnuplot.h"
#include "nodes/rsu/02_Monitor.h"

namespace VENTOS {

class ApplRSUCLASSIFY : public ApplRSUMonitor
{
private:
    typedef ApplRSUMonitor super;

    bool classifier;
    bool collectTrainingData = false;
    double trainError_position;
    double trainError_speed;
    double GPSError_position;
    double GPSError_speed;
    int debugLevel;

    boost::filesystem::path trainingDataFilePath = "";
    boost::filesystem::path trainingModelFilePath = "";

    shark::CSvmTrainer<shark::RealVector, unsigned int> *trainer;

    typedef struct sample
    {
        TraCICoord pos;
        double speed;
        double accel;
        double angle;
    } sample_t;

    std::vector<sample_t> samples;

    std::vector<int> labels;

    // all incoming lanes for the intersection that this RSU belongs to
    std::map<std::string /*lane*/, std::string /*TLid*/> lanesTL;

    std::map< std::string /*class name*/, unsigned int /*label*/> classLabel =
    {
            {"NC_2", 0},
            {"NC_3", 1},
            {"NC_4", 2},

            {"SC_2", 3},
            {"SC_3", 4},
            {"SC_4", 5},

            {"WC_2", 6},
            {"WC_3", 7},
            {"WC_4", 8},

            {"EC_2", 9},
            {"EC_3", 10},
            {"EC_4", 11}
    };

    shark::ClassificationDataset trainingData;
    shark::KernelClassifier<shark::RealVector> *kc_model;
    std::map< std::string /*entity id*/, boost::circular_buffer<unsigned int> > predictionQueue;

    typedef struct resultEntry
    {
        unsigned int label_predicted;
        unsigned int label_true;
        double time;
    } resultEntry_t;

    std::map< std::string /*entity id*/, std::vector<resultEntry_t> > classifyResults;

    gnuplot *gnuplotPtr = NULL;

    typedef struct dataBlockEntry
    {
        unsigned int counter;
        HSV color;
    } dataBlockEntry_t;

    /* two-level categorization for plotting:
     *     level 1: beacons of the same class
     *     level 2: different entities within each class */
    std::map< unsigned int /*class label*/, std::map< std::string /*entity id*/, dataBlockEntry_t > > dataBlockCounter;

public:
    ~ApplRSUCLASSIFY();
    virtual void initialize(int stage);
    virtual void finish();
    virtual void handleSelfMsg(omnetpp::cMessage* msg);

protected:
    void virtual executeEachTimeStep();

    virtual void onBeaconVehicle(BeaconVehicle*);
    virtual void onBeaconBicycle(BeaconBicycle*);
    virtual void onBeaconPedestrian(BeaconPedestrian*);
    virtual void onBeaconRSU(BeaconRSU*);

private:
    void init_gnuplot();
    void loadTrainer();
    void readTrainingSamples();
    void trainClassifier();
    template <typename beaconGeneral> void onBeaconAny(beaconGeneral);
    template <typename beaconGeneral> unsigned int makePrediction(beaconGeneral);
    template <typename beaconGeneral> void addError(beaconGeneral &, double, double);

    void saveTrainingDataToFile();
    void saveClassificationResults();

    template <typename beaconGeneral> void draw(beaconGeneral &, unsigned int);
};

}

#endif
