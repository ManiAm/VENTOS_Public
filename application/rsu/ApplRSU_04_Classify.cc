/****************************************************************************/
/// @file    ApplRSU_04_Classify.cc
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

#include "ApplRSU_04_Classify.h"
#include <Plotter.h>

// un-defining ev!
// why? http://stackoverflow.com/questions/24103469/cant-include-the-boost-filesystem-header
#undef ev
#include "boost/filesystem.hpp"
#define ev  (*cSimulation::getActiveEnvir())

namespace VENTOS {

Define_Module(VENTOS::ApplRSUCLASSIFY);

ApplRSUCLASSIFY::~ApplRSUCLASSIFY()
{

}


void ApplRSUCLASSIFY::initialize(int stage)
{
    ApplRSUTLVANET::initialize(stage);

    if (stage==0)
    {
        classifier = par("classifier").boolValue();
        if(!classifier)
            return;

        collectTrainingData = par("collectTrainingData").boolValue();

        // we need this RSU to be associated with a TL
        if(myTLid == "")
            error("The id of %s does not match with any TL. Check RSUsLocation.xml file!", myFullId);

        // for each incoming lane in this TL
        std::list<std::string> lan = TraCI->TLGetControlledLanes(myTLid);

        // remove duplicate entries
        lan.unique();

        // for each incoming lane
        for(std::list<std::string>::iterator it2 = lan.begin(); it2 != lan.end(); ++it2)
            lanesTL[*it2] = myTLid;

        // 3 is number of features
        shark_sample.resize(1,3);

        if(ev.isGUI())
            initializeGnuPlot();

        if(!collectTrainingData)
            trainClassifier();
    }
}


void ApplRSUCLASSIFY::finish()
{
    ApplRSUTLVANET::finish();

    if(collectTrainingData)
        saveSampleToFile();
}


void ApplRSUCLASSIFY::handleSelfMsg(cMessage* msg)
{
    ApplRSUTLVANET::handleSelfMsg(msg);
}


void ApplRSUCLASSIFY::executeEachTimeStep()
{
    ApplRSUTLVANET::executeEachTimeStep();
}


void ApplRSUCLASSIFY::onBeaconVehicle(BeaconVehicle* wsm)
{
    ApplRSUTLVANET::onBeaconVehicle(wsm);

    if (classifier)
        onBeaconAny(wsm);
}


void ApplRSUCLASSIFY::onBeaconBicycle(BeaconBicycle* wsm)
{
    ApplRSUTLVANET::onBeaconBicycle(wsm);

    if (classifier)
        onBeaconAny(wsm);
}


void ApplRSUCLASSIFY::onBeaconPedestrian(BeaconPedestrian* wsm)
{
    ApplRSUTLVANET::onBeaconPedestrian(wsm);

    if (classifier)
        onBeaconAny(wsm);
}


void ApplRSUCLASSIFY::onBeaconRSU(BeaconRSU* wsm)
{
    ApplRSUTLVANET::onBeaconRSU(wsm);
}


void ApplRSUCLASSIFY::onData(LaneChangeMsg* wsm)
{
    ApplRSUTLVANET::onData(wsm);
}


void ApplRSUCLASSIFY::initializeGnuPlot()
{
    // get a pointer to the plotter module
    cModule *pmodule = simulation.getSystemModule()->getSubmodule("plotter");

    if(pmodule == NULL)
        error("plotter module is not found!");

    // return if plotter is off
    if(!pmodule->par("on").boolValue())
        return;

    // get a pointer to the class
    Plotter *pltPtr = static_cast<Plotter *>(pmodule);
    ASSERT(pltPtr);

    plotterPtr = pltPtr->pipeGnuPlot;
    ASSERT(plotterPtr);

    // We need feature in GNUPLOT 5.0 and above
    if(pltPtr->vers < 5)
        error("GNUPLOT version should be >= 5");

    // set title name
    fprintf(plotterPtr, "set title 'Sample Points' \n");

    // set axis labels
    fprintf(plotterPtr, "set xlabel 'X pos' offset -5 \n");
    fprintf(plotterPtr, "set ylabel 'Y pos' offset 3 \n");
    fprintf(plotterPtr, "set zlabel 'Speed' offset -2 rotate left \n");

    // change ticks
    //   fprintf(pipe, "set xtics 20 \n");
    //   fprintf(pipe, "set ytics 20 \n");

    // set range
    //   fprintf(pipe, "set yrange [885:902] \n");

    // set grid and border
    fprintf(plotterPtr, "set grid \n");
    fprintf(plotterPtr, "set border 4095 \n");

    // set agenda location
    fprintf(plotterPtr, "set key outside right top box \n");

    // define line style
    fprintf(plotterPtr, "set style line 1 pointtype 7 pointsize 1 lc rgb 'red'  \n");
    fprintf(plotterPtr, "set style line 2 pointtype 7 pointsize 1 lc rgb 'green' \n");
    fprintf(plotterPtr, "set style line 3 pointtype 7 pointsize 1 lc rgb 'blue' \n");

    fflush(plotterPtr);
}


// update variables upon reception of any beacon (vehicle, bike, pedestrian)
template <typename beaconGeneral>
void ApplRSUCLASSIFY::onBeaconAny(beaconGeneral wsm)
{
    if(collectTrainingData)
    {
        collectSample(wsm);
        return;
    }

    // on one of the incoming lanes?
    auto it = lanesTL.find(wsm->getLane());

    // not on any incoming lanes
    if(it == lanesTL.end())
        return;

    // I do not control this lane!
    if(it->second != myTLid)
        return;

    Coord pos = wsm->getPos();
    shark_sample(0,0) = (double) pos.x;            // xPos
    shark_sample(0,1) = (double) pos.y;            // yPos
    shark_sample(0,2) = (double) wsm->getSpeed();  // entry speed

    // print debug information
    printf("%0.3f, %0.3f, %06.3f --> predicted label: %d, sender: %s \n",
            shark_sample(0,0),
            shark_sample(0,1),
            shark_sample(0,2),
            kc_model(shark_sample)[0],
            wsm->getSender());

    std::cout.flush();
}


template <typename beaconGeneral>
void ApplRSUCLASSIFY::collectSample(beaconGeneral wsm)
{
    std::string lane = wsm->getLane();

    auto it = lanesTL.find(lane);

    // not on any incoming lanes
    if(it == lanesTL.end())
        return;

    // I do not control this lane!
    if(it->second != myTLid)
        return;

    // make an instance
    Coord pos = wsm->getPos();
    sample_type *m = new sample_type(pos.x, pos.y, wsm->getSpeed());

    // check instance's class
    auto it2 = entityClasses.find(lane);
    if(it2 == entityClasses.end())
    {
        int classNum = entityClasses.size();
        entityClasses[lane] = classNum;

        samples.push_back(*m);
        labels.push_back(classNum);
    }
    else
    {
        samples.push_back(*m);
        labels.push_back(it2->second);
    }

    // plot each sample, live on gnuplot
    //    if(plotterPtr != NULL)
    //    {
    //        // create a data blocks out of dataset (only gnuplot > 5.0 supports this)
    //        fprintf(plotterPtr, "$data << EOD \n");
    //
    //        // data block 1
    //        //    for(auto &i : dataSet[0])
    //        //            fprintf(plotterPtr, "%f  %f  %f \n", i.xPos, i.yPos, i.speed);
    //
    //        // two blank lines as data block separator
    //        fprintf(plotterPtr, "\n\n \n");
    //
    //        // data block 2
    //        //      for(auto &i : dataSet[1])
    //        //           fprintf(plotterPtr, "%f  %f  %f \n", i.xPos, i.yPos, i.speed);
    //
    //        // two blank lines as data block separator
    //        fprintf(plotterPtr, "\n\n \n");
    //
    //        // data block 3
    //        //        for(auto &i : dataSet[2])
    //        //            fprintf(plotterPtr, "%f  %f  %f \n", i.xPos, i.yPos, i.speed);
    //
    //        // data block terminator
    //        fprintf(plotterPtr, "EOD \n");
    //
    //        // make plot
    //        fprintf(plotterPtr, "splot '$data' index 0 using 1:2:3 with points ls 1 title 'passenger',");
    //        fprintf(plotterPtr, "      ''      index 1 using 1:2:3 with points ls 2 title 'bicycle',");
    //        fprintf(plotterPtr, "      ''      index 2 using 1:2:3 with points ls 3 title 'pedestrian' \n");
    //
    //        fflush(plotterPtr);
    //    }
}


void ApplRSUCLASSIFY::saveSampleToFile()
{
    boost::filesystem::path filePath = "application/rsu/trainData.txt";
    FILE *filePtr = fopen (filePath.string().c_str(), "w");

    for(unsigned int i = 0; i < samples.size(); ++i)
        fprintf (filePtr, "%f %f %f %d \n", samples[i].xPos, samples[i].yPos, samples[i].speed, labels[i]);

    fclose(filePtr);
}


void ApplRSUCLASSIFY::trainClassifier()
{
    shark::ClassificationDataset data;

    try
    {
        std::cout << "reading training samples... " << std::flush;
        // Load data from external file
        shark::importCSV(data, "application/rsu/trainData.txt", shark::LAST_COLUMN /*label position*/, ' ' /*separator*/);
        std::cout << data.elements().size() << " samples fetched! \n" << std::flush;
    }
    catch (std::exception& e)
    {
        std::cout << std::endl << e.what() << std::endl;
        endSimulation();
    }

    std::cout << "number of classes: " << numberOfClasses(data) << std::endl;
    int classCount = 0;
    for(auto i : classSizes(data))
    {
        std::cout << "  class " << classCount << ": " << i << std::endl;
        classCount++;
    }

    //    std::cout << "shuffling training samples... " << std::flush;
    //    // shuffle data
    //    data.shuffle();
    //    std::cout << "done \n" << std::flush;

    shark::GaussianRbfKernel<> *kernel = new shark::GaussianRbfKernel<shark::RealVector> (0.5 /*kernel bandwidth parameter*/);
    // shark::CSvmTrainer<shark::RealVector> trainer(kernel, 1000.0 /*regularization parameter*/, true /*use bias/offset*/);

    double C = 10.0;

    // There are 9 trainers for multi-class SVMs in Shark which can train with or without bias:
    shark::AbstractSvmTrainer<shark::RealVector, unsigned int> *trainer[18];

    trainer[0]  = new shark::McSvmOVATrainer<shark::RealVector>(kernel, C, false);
    trainer[1]  = new shark::McSvmCSTrainer<shark::RealVector>(kernel, C, false);
    trainer[2]  = new shark::McSvmWWTrainer<shark::RealVector>(kernel, C, false);
    trainer[3]  = new shark::McSvmLLWTrainer<shark::RealVector>(kernel, C, false);
    trainer[4]  = new shark::McSvmADMTrainer<shark::RealVector>(kernel, C, false);
    trainer[5]  = new shark::McSvmATSTrainer<shark::RealVector>(kernel, C, false);
    trainer[6]  = new shark::McSvmATMTrainer<shark::RealVector>(kernel, C, false);
    trainer[7]  = new shark::McSvmMMRTrainer<shark::RealVector>(kernel, C, false);
    trainer[8]  = new shark::McReinforcedSvmTrainer<shark::RealVector>(kernel, C, false);

    trainer[9]  = new shark::McSvmOVATrainer<shark::RealVector>(kernel, C, true);
    trainer[10] = new shark::McSvmCSTrainer<shark::RealVector>(kernel, C, true);
    trainer[11] = new shark::McSvmWWTrainer<shark::RealVector>(kernel, C, true);
    trainer[12] = new shark::McSvmLLWTrainer<shark::RealVector>(kernel, C, true);
    trainer[13] = new shark::McSvmADMTrainer<shark::RealVector>(kernel, C, true);
    trainer[14] = new shark::McSvmATSTrainer<shark::RealVector>(kernel, C, true);
    trainer[15] = new shark::McSvmATMTrainer<shark::RealVector>(kernel, C, true);
    trainer[16] = new shark::McSvmMMRTrainer<shark::RealVector>(kernel, C, true);
    trainer[17] = new shark::McReinforcedSvmTrainer<shark::RealVector>(kernel, C, true);

    for (int i = 0; i < 18; i++)
    {
        std::printf("[%2d] training model %10s %s... \n",
                i,
                trainer[i]->name().c_str(),
                trainer[i]->trainOffset()? "with bias   ":"without bias");
        std::cout.flush();

        // start training
        trainer[i]->train(kc_model, data);

        std::printf("  iterations=%10d, accuracy=%f, time=%9.4g seconds \n",
                (int)trainer[i]->solutionProperties().iterations,
                trainer[i]->solutionProperties().accuracy,
                trainer[i]->solutionProperties().seconds);

        std::cout << "  training error= ";
        std::cout.flush();

        // evaluate the model on training set
        shark::ZeroOneLoss<unsigned int> loss; // 0-1 loss
        shark::Data<unsigned int> output = kc_model(data.inputs());
        double train_error = loss.eval(data.labels(), output);

        std::cout << train_error << std::endl;
        std::cout.flush();
    }
}

}
