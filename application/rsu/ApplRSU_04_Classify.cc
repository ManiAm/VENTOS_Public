/****************************************************************************/
/// @file    ApplRSU_05_Classify.cc
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

#include "ApplRSU_05_Classify.h"
#include <Plotter.h>

#undef ev
//###begin<skeleton>
#include <shark/Data/Dataset.h>
#include <shark/Data/Csv.h>
#include <shark/ObjectiveFunctions/Loss/ZeroOneLoss.h>
//###end<skeleton>

//###begin<SVM-includes>
#include <shark/Models/Kernels/GaussianRbfKernel.h>
#include <shark/Models/Kernels/KernelExpansion.h>
#include <shark/Algorithms/Trainers/CSvmTrainer.h>
//###end<SVM-includes>
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
        if(!classifier)
            return;

        pipe = NULL;

        if(ev.isGUI())
        {
            // get a pointer to the plotter module
            cModule *pmodule = simulation.getSystemModule()->getSubmodule("plotter");

            if(pmodule != NULL)
            {
                // check if plotter in on
                if(pmodule->par("on").boolValue())
                {
                    // get a pointer to the class
                    Plotter *plotterPtr = static_cast<Plotter *>(pmodule);

                    if(plotterPtr != NULL)
                        pipe = plotterPtr->pipe;
                }
            }

            if(pipe != NULL)
            {
                // set title name
                fprintf(pipe, "set title 'Sample Points' \n");

                // set axis labels
                fprintf(pipe, "set xlabel 'X pos' offset -5 \n");
                fprintf(pipe, "set ylabel 'Y pos' offset 3 \n");
                fprintf(pipe, "set zlabel 'Approach Speed' offset -2 rotate left \n");

                // change ticks
                fprintf(pipe, "set xtics 20 \n");
                fprintf(pipe, "set ytics 20 \n");

                // set grid and border
                fprintf(pipe, "set grid \n");
                fprintf(pipe, "set border 4095 \n");

                // set agenda location
                fprintf(pipe, "set key outside right top box \n");

                // define line style
                fprintf(pipe, "set style line 1 pointtype 7 pointsize 1 lc rgb 'red'  \n");
                fprintf(pipe, "set style line 2 pointtype 7 pointsize 1 lc rgb 'green' \n");
                fprintf(pipe, "set style line 3 pointtype 7 pointsize 1 lc rgb 'blue' \n");

                fflush(pipe);
            }
        }

        classifierF();
    }
}


void ApplRSUCLASSIFY::finish()
{
    ApplRSUTLVANET::finish();
}


void ApplRSUCLASSIFY::handleSelfMsg(cMessage* msg)
{
    ApplRSUTLVANET::handleSelfMsg(msg);
}


void ApplRSUCLASSIFY::executeEachTimeStep(bool simulationDone)
{
    ApplRSUTLVANET::executeEachTimeStep(simulationDone);
}


void ApplRSUCLASSIFY::onBeaconVehicle(BeaconVehicle* wsm)
{
    ApplRSUTLVANET::onBeaconVehicle(wsm);
}


void ApplRSUCLASSIFY::onBeaconBicycle(BeaconBicycle* wsm)
{
    ApplRSUTLVANET::onBeaconBicycle(wsm);
}


void ApplRSUCLASSIFY::onBeaconPedestrian(BeaconPedestrian* wsm)
{
    ApplRSUTLVANET::onBeaconPedestrian(wsm);
}


void ApplRSUCLASSIFY::onBeaconRSU(BeaconRSU* wsm)
{
    ApplRSUTLVANET::onBeaconRSU(wsm);
}


void ApplRSUCLASSIFY::onData(LaneChangeMsg* wsm)
{
    ApplRSUTLVANET::onData(wsm);
}


// add each instance of input vector (posX, posY, speed) to the dataset and plot it
void ApplRSUCLASSIFY::addInputToClassifier(std::string type, Coord pos, double speed)
{
    // draw a new plot from the updated vehApproach file
    if(pipe != NULL && ev.isGUI() && boost::filesystem::exists(vehApproachFilePath))
    {
        fprintf(pipe, "splot '%s' index 0 using 4:5:8 with points ls 1 title 'Class 1',", vehApproachFilePath.string().c_str());
        fprintf(pipe, "      ''   index 1 using 4:5:8 with points ls 2 title 'Class 2',");
        fprintf(pipe, "      ''   index 2 using 4:5:8 with points ls 3 title 'Class 3' \n");

        fflush(pipe);
    }


}


/*
The classes are as follows:
    - class 1: points very close to the origin
    - class 2: points on the circle of radius 10 around the origin
    - class 3: points that are on a circle of radius 4 but not around the origin at all */
//void ApplRSUCLASSIFY::generate_data (std::vector<sample_type_2D>& samples, std::vector<double>& labels)
//{
//    if(pipe != NULL)
//    {
//        fprintf(pipe, "set title 'Sample Points' \n");
//        fprintf(pipe, "set xlabel 'X' \n");
//        fprintf(pipe, "set ylabel 'Y' \n");
//        fprintf(pipe, "set key outside right top box \n");
//
//        // The "-" is used to specify that the data follows the plot command
//        fprintf(pipe, "plot '-' using 1:2 with points pointtype 7 pointsize 0.5 title 'type 1' ,");
//        fprintf(pipe, "'-' using 1:2 with points pointtype 6 pointsize 0.5 title 'type 2' ,");
//        fprintf(pipe, "'-' using 1:2 with points pointtype 6 pointsize 0.5 title 'type 3' \n");
//    }
//
//    const long num = 50;
//
//    sample_type_2D m;
//
//    dlib::rand rnd;
//
//    // make some samples near the origin
//    double radius = 4;
//    for (long i = 0; i < num+10; ++i)
//    {
//        double sign = 1;
//        if (rnd.get_random_double() < 0.5)
//            sign = -1;
//
//        m(0) = 2*radius*rnd.get_random_double()-radius;
//        m(1) = sign*sqrt(radius*radius - m(0)*m(0));
//
//        // add this sample to our set of training samples
//        samples.push_back(m);
//        labels.push_back(1);
//
//        if(pipe != NULL)
//            fprintf(pipe, "%f %f\n", m(0), m(1));
//    }
//
//    fflush(pipe);
//    fprintf(pipe, "e\n"); // termination character
//
//    // make some samples in a circle around the origin but far away
//    radius = 10.0;
//    for (long i = 0; i < num+20; ++i)
//    {
//        double sign = 1;
//        if (rnd.get_random_double() < 0.5)
//            sign = -1;
//
//        m(0) = 2*radius*rnd.get_random_double()-radius;
//        m(1) = sign*sqrt(radius*radius - m(0)*m(0));
//
//        // move points to the right
//        m(0) += 25;
//
//        // add this sample to our set of training samples
//        samples.push_back(m);
//        labels.push_back(2);
//
//        if(pipe != NULL)
//            fprintf(pipe, "%f %f\n", m(0), m(1));
//    }
//
//    fflush(pipe);
//    fprintf(pipe, "e\n"); // termination character
//
//    // make some samples in a circle around the point (25,25)
//    radius = 4.0;
//    for (long i = 0; i < num+30; ++i)
//    {
//        double sign = 1;
//        if (rnd.get_random_double() < 0.5)
//            sign = -1;
//        m(0) = 2*radius*rnd.get_random_double()-radius;
//        m(1) = sign*sqrt(radius*radius - m(0)*m(0));
//
//        // translate this point away from the origin
//        m(0) += 25;
//        m(1) += 25;
//
//        // add this sample to our set of training samples
//        samples.push_back(m);
//        labels.push_back(3);
//
//        if(pipe != NULL)
//            fprintf(pipe, "%f %f\n", m(0), m(1));
//    }
//
//    std::cout << "samples.size(): " << samples.size() << std::endl << endl;
//
//    fflush(pipe);
//    fprintf(pipe, "e\n"); // termination character
//}


void ApplRSUCLASSIFY::classifierF()
{

    // Load data, use 70% for training and 30% for testing.
    // The path is hard coded; make sure to invoke the executable
    // from a place where the data file can be found. It is located
    // under [shark]/examples/Supervised/data.
    shark::ClassificationDataset traindata, testdata;
    shark::importCSV(traindata, "/home/mani/Desktop/dataset/quickstartData.csv", shark::LAST_COLUMN, ' ');
    testdata = shark::splitAtElement(traindata, 70 * traindata.numberOfElements() / 100);
    //###end<skeleton>

    //###begin<SVM>
    double gamma = 1.0;         // kernel bandwidth parameter
    double C = 10.0;            // regularization parameter
    shark::GaussianRbfKernel<shark::RealVector> kernel(gamma);
    shark::KernelClassifier<shark::RealVector> model(&kernel);
    shark::CSvmTrainer<shark::RealVector> trainer(
            &kernel,
            C,
            true); /* true: train model with offset */
    //###end<SVM>

    //###begin<skeleton>
    trainer.train(model, traindata);

    shark::Data<unsigned int> prediction = model(testdata.inputs());

    shark::ZeroOneLoss<unsigned int> loss;
    double error_rate = loss(testdata.labels(), prediction);

    std::cout << "model: " << model.name() << std::endl
            << "trainer: " << trainer.name() << std::endl
            << "test error rate: " << error_rate << std::endl;



    //    std::vector<sample_type_2D> samples;
    //    std::vector<double> labels;
    //
    //    // First, get our labeled set of training data
    //    generate_data(samples, labels);
    //
    //    try
    //    {
    //        // Program we will work with a one_vs_one_trainer object which stores any
    //        // kind of trainer that uses our sample_type samples.
    //        typedef dlib::one_vs_one_trainer<dlib::any_trainer<sample_type_2D> > ovo_trainer;
    //
    //        // Finally, make the one_vs_one_trainer.
    //        ovo_trainer trainer;
    //
    //        // making the second binary classification trainer object.
    //        // this uses 'kernel ridge regression' and 'RBF kernels'
    //        typedef dlib::radial_basis_kernel<sample_type_2D> rbf_kernel;
    //        dlib::krr_trainer<rbf_kernel> rbf_trainer;  // make the binary trainer
    //        rbf_trainer.set_kernel(rbf_kernel(0.1));    // set some parameters
    //
    //        // making the first binary classification trainer object.
    //        // this uses 'support vector machine' and 'polynomial kernels'
    //        typedef dlib::polynomial_kernel<sample_type_2D> poly_kernel;
    //        dlib::svm_nu_trainer<poly_kernel> poly_trainer;   // make the binary trainer
    //        poly_trainer.set_kernel(poly_kernel(0.1, 1, 2));  // set some parameters
    //
    //        // Now tell the one_vs_one_trainer that, by default, it should use the rbf_trainer
    //        // to solve the individual binary classification subproblems.
    //        trainer.set_trainer(rbf_trainer);
    //
    //        // We can also get more specific. Here we tell the one_vs_one_trainer to use the
    //        // poly_trainer to solve the class 1 vs class 2 subproblem. All the others will
    //        // still be solved with the rbf_trainer.
    //        trainer.set_trainer(poly_trainer, 1, 2);
    //
    //        // Now let's do 5-fold cross-validation using the one_vs_one_trainer we just setup.
    //        // As an aside, always shuffle the order of the samples before doing cross validation.
    //        // For a discussion of why this is a good idea see the svm_ex.cpp example.
    //        //randomize_samples(samples, labels);
    //        //std::cout << "cross validation: \n" << cross_validate_multiclass_trainer(trainer, samples, labels, 5) << endl;
    //
    //        // The output is shown below.  It is the confusion matrix which describes the results.  Each row
    //        // corresponds to a class of data and each column to a prediction.  Reading from top to bottom,
    //        // the rows correspond to the class labels if the labels have been listed in sorted order.  So the
    //        // top row corresponds to class 1, the middle row to class 2, and the bottom row to class 3.  The
    //        // columns are organized similarly, with the left most column showing how many samples were predicted
    //        // as members of class 1.
    //        //
    //        // So in the results below we can see that, for the class 1 samples, 60 of them were correctly predicted
    //        // to be members of class 1 and 0 were incorrectly classified.  Similarly, the other two classes of data
    //        // are perfectly classified.
    //        /*
    //            cross validation:
    //            60  0  0
    //            0 70  0
    //            0  0 80
    //        */
    //
    //        // Next, if you wanted to obtain the decision rule learned by a one_vs_one_trainer you
    //        // would store it into a one_vs_one_decision_function.
    //        dlib::one_vs_one_decision_function<ovo_trainer> df = trainer.train(samples, labels);
    //
    //        sample_type_2D newPoint;
    //        newPoint(0) = 15;
    //        newPoint(1) = 30;
    //
    //        std::cout << "predicted label: " << df(newPoint) << endl << endl;
    //    }
    //    catch (std::exception& e)
    //    {
    //        std::cout << "exception thrown!" << endl;
    //        std::cout << e.what() << endl;
    //    }
}

}
