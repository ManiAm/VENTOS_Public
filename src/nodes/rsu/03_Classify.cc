/****************************************************************************/
/// @file    Classify.cc
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

#include "nodes/rsu/03_Classify.h"

namespace VENTOS {

Define_Module(VENTOS::ApplRSUCLASSIFY);

ApplRSUCLASSIFY::~ApplRSUCLASSIFY()
{

}


void ApplRSUCLASSIFY::initialize(int stage)
{
    super::initialize(stage);

    if (stage==0)
    {
        classifier = par("classifier").boolValue();
        if(!classifier)
            return;

        // we need this RSU to be associated with a TL
        if(myTLid == "")
            throw omnetpp::cRuntimeError("The id of %s does not match with any TL. Check RSUsLocation.xml file!", myFullId);

        trainError = par("trainError").doubleValue();
        if(trainError < 0)
            throw omnetpp::cRuntimeError("trainError value is not correct!");

        GPSerror = par("GPSerror").doubleValue();
        if(GPSerror < 0)
            throw omnetpp::cRuntimeError("GPSerror value is not correct!");

        debugLevel = omnetpp::getSimulation()->getSystemModule()->par("debugLevel").longValue();

        // construct file name for training data
        int currentRun = omnetpp::getEnvir()->getConfigEx()->getActiveRunNumber();
        std::stringstream fileName;
        fileName << boost::format("%03d_trainData_%0.3f.txt") % currentRun % trainError;
        trainingFilePath = boost::filesystem::path("results") / fileName.str();

        // for each incoming lane in this TL
        auto lan = TraCI->TLGetControlledLanes(myTLid);

        // remove duplicate entries
        sort( lan.begin(), lan.end() );
        lan.erase( unique( lan.begin(), lan.end() ), lan.end() );

        // for each incoming lane
        for(auto &it : lan)
            lanesTL[it] = myTLid;

        if(omnetpp::cSimulation::getActiveEnvir()->isGUI())
            initializeGnuPlot();

        loadTrainer();
    }
}


void ApplRSUCLASSIFY::finish()
{
    super::finish();

    if(!classifier)
        return;

    if(collectTrainingData)
        saveTrainingDataToFile();
    else
        saveClassificationResults();

    if(plotterPtr)
    {
#ifdef WIN32
        _pclose(plotterPtr);
#else
        pclose(plotterPtr);
#endif
    }
}


void ApplRSUCLASSIFY::handleSelfMsg(omnetpp::cMessage* msg)
{
    super::handleSelfMsg(msg);
}


void ApplRSUCLASSIFY::executeEachTimeStep()
{
    super::executeEachTimeStep();
}


void ApplRSUCLASSIFY::onBeaconVehicle(BeaconVehicle* wsm)
{
    super::onBeaconVehicle(wsm);

    if (classifier)
        onBeaconAny(wsm);
}


void ApplRSUCLASSIFY::onBeaconBicycle(BeaconBicycle* wsm)
{
    super::onBeaconBicycle(wsm);

    if (classifier)
        onBeaconAny(wsm);
}


void ApplRSUCLASSIFY::onBeaconPedestrian(BeaconPedestrian* wsm)
{
    super::onBeaconPedestrian(wsm);

    if (classifier)
        onBeaconAny(wsm);
}


void ApplRSUCLASSIFY::onBeaconRSU(BeaconRSU* wsm)
{
    super::onBeaconRSU(wsm);
}


void ApplRSUCLASSIFY::initializeGnuPlot()
{
    double version = getGnuPlotVersion();
    // we need feature in GNUPLOT 5.0 and above
    if(version < 5)
        throw omnetpp::cRuntimeError("GNUPLOT version should be >= 5");

#ifdef WIN32
    plotterPtr = _popen("pgnuplot -persist", "w");
#else
    plotterPtr = popen("gnuplot", "w");
#endif

    if(plotterPtr == NULL)
        throw omnetpp::cRuntimeError("Could not open pipe for write!");

    // interactive gnuplot terminals: x11, wxt, qt (wxt and qt offer nicer output and a wider range of features)
    // persist: keep the windows open even after simulation termination
    // noraise: updating is done in the background
    // link: http://gnuplot.sourceforge.net/docs_4.2/node441.html
    // fprintf(plotterPtr, "set term wxt enhanced 0 font 'Helvetica,' noraise\n");

    // set title name
    fprintf(plotterPtr, "set title 'Sample Points' \n");

    // set axis labels
    fprintf(plotterPtr, "set xlabel 'X pos' offset -5 \n");
    fprintf(plotterPtr, "set ylabel 'Y pos' offset 3 \n");
    fprintf(plotterPtr, "set zlabel 'Speed' offset -2 rotate left \n");

    // change ticks
    // fprintf(pipe, "set xtics 20 \n");
    // fprintf(pipe, "set ytics 20 \n");

    // set range
    // fprintf(pipe, "set yrange [885:902] \n");

    // set grid and border
    fprintf(plotterPtr, "set grid \n");
    fprintf(plotterPtr, "set border 4095 \n");

    // set agenda location
    fprintf(plotterPtr, "set key outside right top box \n");

    fflush(plotterPtr);
}


double ApplRSUCLASSIFY::getGnuPlotVersion()
{
    FILE* pipversion = popen("gnuplot --version", "r");
    if (!pipversion)
        throw omnetpp::cRuntimeError("can not open pipe!");

    char lineversion[128];
    memset (lineversion, 0, sizeof(lineversion));
    if (!fgets(lineversion, sizeof(lineversion), pipversion))
        throw omnetpp::cRuntimeError("fgets error!");

    // now parsing lineversion (gnuplot 5.0 patchlevel 1)
    double vers = 0.0;
    sscanf(lineversion, "gnuplot %lf", &vers);

    int majvers = 0;
    int minvers = 0;

    int pos = -1;
    char* restvers = NULL;
    if (sscanf(lineversion, "gnuplot %d.%d %n", &majvers, &minvers, &pos) >= 2)
    {
        assert(pos>=0);
        restvers = lineversion+pos;
    }

    pclose(pipversion);
    pipversion = NULL;

    return vers;
}


void ApplRSUCLASSIFY::loadTrainer()
{
    auto kernel = new shark::GaussianRbfKernel<shark::RealVector> (0.5 /*gamma: kernel bandwidth parameter*/, false /*unconstrained*/);

    kc_model = new shark::KernelClassifier<shark::RealVector> (kernel);

    // Training of a multi-class SVM by the one-versus-all (OVA) method
    auto trainer = new shark::CSvmTrainer<shark::RealVector, unsigned int>(kernel, 10.0 /*regularization parameter*/, true /*with offset*/);
    trainer->setMcSvmType(shark::McSvm::OVA);

    std::stringstream fileName;
    int currentRun = omnetpp::getEnvir()->getConfigEx()->getActiveRunNumber();
    fileName << boost::format("%03d_%s_%s_%0.3f.model") % currentRun % trainer->name() % (trainer->trainOffset() ? "withOffset" : "withoutOffset") % trainError;
    boost::filesystem::path filePath = boost::filesystem::path("results") / fileName.str();

    std::cout << "\n>>> Looking for '" << fileName.str() << "'... ";

    // check if this model was trained before
    std::ifstream ifs(filePath.string());
    if(!ifs.fail())
    {
        std::cout << "found! \n";

        shark::TextInArchive ia(ifs);
        kc_model->read(ia);
        ifs.close();

        collectTrainingData = false;
        return;
    }

    std::cout << "not found! \n";

    // read training data
    if(trainingData.elements().empty())
    {
        std::cout << "\n>>> Reading training samples... " << std::flush;
        readTrainingSamples();
    }

    // if no training data is present, then collect!
    if(trainingData.elements().empty())
    {
        collectTrainingData = true;
        std::cout << "No training data exists! Keep running the simulation to collect training data. \n";
        return;
    }
    else
    {
        collectTrainingData = false;

        trainClassifier(trainer);

        std::cout << ">>> Saving the training model to file for future runs! \n";

        // save the model to file for future runs
        std::ofstream ofs(filePath.string());
        shark::TextOutArchive oa(ofs);
        kc_model->write(oa);
        ofs.close();
    }
}


void ApplRSUCLASSIFY::readTrainingSamples()
{
    try
    {
        // Load data from external file
        shark::importCSV(trainingData, trainingFilePath.string(), shark::LAST_COLUMN /*label position*/, ' ' /*separator*/);
    }
    catch (std::exception& e)
    {
        std::cout << e.what() << std::endl;
        return;
    }

    std::cout << "done! \n";
    std::cout << trainingData.elements().size() << " samples fetched! \n" << std::flush;

    std::cout << "number of classes: " << numberOfClasses(trainingData) << std::endl;
    int classCount = 0;
    for(auto i : classSizes(trainingData))
    {
        std::printf("  class %-2d: %-5lu, ", classCount, i);
        classCount++;

        if(classCount % 3 == 0)
            std::cout << std::endl;
    }

    std::cout << "shuffling training samples... " << std::flush;
    trainingData.shuffle();   // shuffle trainingData
    std::cout << "done! \n\n" << std::flush;
}


void ApplRSUCLASSIFY::trainClassifier(shark::CSvmTrainer<shark::RealVector, unsigned int> *trainer)
{
    std::cout << ">>> Training the model... Please wait \n" << std::flush;

    // start training --- on Mars server, the training takes around 20 min for 29162 training samples collected
    // during 300s with training error 0m. For training error of 3m, training takes around 50 min!
    trainer->train(*kc_model, trainingData);

    std::printf("  iterations= %d, accuracy= %f, time= %g seconds \n",
            (int)trainer->solutionProperties().iterations,
            trainer->solutionProperties().accuracy,
            trainer->solutionProperties().seconds);

    std::cout << "  training error= " << std::flush;

    // evaluate the model on training set
    shark::ZeroOneLoss<unsigned int> loss; // 0-1 loss
    shark::Data<unsigned int> output = (*kc_model)(trainingData.inputs());
    double train_error = loss.eval(trainingData.labels(), output);

    std::cout << train_error << "\n\n" << std::flush;
}


// update variables upon reception of any beacon (vehicle, bike, pedestrian)
template <typename beaconGeneral>
void ApplRSUCLASSIFY::onBeaconAny(beaconGeneral wsm)
{
    std::string lane = wsm->getLane();

    auto it = lanesTL.find(lane);
    // return if this vehicle is not on any incoming lanes
    if(it == lanesTL.end())
        return;

    // return if I do not control this lane. I might have received
    // this beacon from my nearby intersections
    if(it->second != myTLid)
        return;

    if(collectTrainingData)
    {
        // add trainError to beacon
        if(trainError != 0)
            addError(wsm, trainError);

        // make an instance and push it to samples
        sample_t m = {TraCICoord(wsm->getPos().x, wsm->getPos().y), wsm->getSpeed(), wsm->getAccel(), wsm->getAngle()};
        samples.push_back(m);

        // get class label
        auto it2 = classLabel.find(lane);
        if(it2 == classLabel.end())
            throw omnetpp::cRuntimeError("class %s not found in classLabel!", lane.c_str());

        // push to labels
        labels.push_back(it2->second);

        return;
    }

    // add GPSerror to beacon
    if(GPSerror != 0)
        addError(wsm, GPSerror);

    // get the predicted label
    unsigned int predicted_label = makePrediction(wsm);

    // get the real label
    auto re = classLabel.find(lane);
    if(re == classLabel.end())
        throw omnetpp::cRuntimeError("class %s not found!", lane.c_str());
    unsigned int real_label = re->second;

    // print debug information
    std::string sender = wsm->getSender();
    if(omnetpp::cSimulation::getActiveEnvir()->isGUI() && debugLevel > 1)
    {
        printf("%0.3f, %0.3f, %06.3f, %0.3f --> predicted label: %2d, true label: %2d, sender: %s \n",
                wsm->getPos().x,
                wsm->getPos().y,
                wsm->getSpeed(),
                wsm->getAngle(),
                predicted_label,
                real_label,
                sender.c_str());

        std::cout.flush();
    }

    // save classification results for each entity
    resultEntry_t res = {predicted_label, real_label, omnetpp::simTime().dbl()};
    auto xr = classifyResults.find(sender);
    if(xr == classifyResults.end())
    {
        std::vector<resultEntry_t> tmp {res};
        classifyResults[sender] = tmp;
    }
    else
        xr->second.push_back(res);

    // draw samples on West direction in Gnuplot
    if(omnetpp::cSimulation::getActiveEnvir()->isGUI() && lane.find("WC") != std::string::npos)
        draw(wsm, real_label);
}


template <typename beaconGeneral>
unsigned int ApplRSUCLASSIFY::makePrediction(beaconGeneral wsm)
{
    // we have 5 features, thus shark_sample should be 1 * 5
    shark::blas::matrix<double, shark::blas::row_major> shark_sample(1,5);

    // retrieve info from beacon
    shark_sample(0,0) = wsm->getPos().x;
    shark_sample(0,1) = wsm->getPos().y;
    shark_sample(0,2) = wsm->getSpeed();
    shark_sample(0,3) = wsm->getAccel();
    shark_sample(0,4) = wsm->getAngle();

    // make prediction
    unsigned int predicted_label = (*kc_model)(shark_sample)[0];

    std::string sender = wsm->getSender();

    // search for entity name in predictionQueue
    auto it = predictionQueue.find(sender);
    if(it == predictionQueue.end())
    {
        boost::circular_buffer<unsigned int> CB_class;  // create a circular buffer
        CB_class.set_capacity(10);                      // set max capacity
        CB_class.clear();
        CB_class.push_back(predicted_label);

        predictionQueue[sender] = CB_class;

        return predicted_label;
    }
    else
    {
        it->second.push_back(predicted_label);

        std::vector<int> histogram(21, 0); // we assume that maximum class number is 20
        // iterate over the circular buffer to find the node
        for(auto i : it->second)
            ++histogram[i];

        return std::max_element( histogram.begin(), histogram.end() ) - histogram.begin();
    }
}


template <typename beaconGeneral>
void ApplRSUCLASSIFY::addError(beaconGeneral &wsm, double maxError)
{
    // retrieve info from beacon
    double posX = wsm->getPos().x;
    double posY = wsm->getPos().y;

    double speed = wsm->getSpeed();
   // double accel = wsm->getAccel();
   // double angle = wsm->getAngle();

    double r = 0;

    // Produce a random double in the range [0,1) using generator 0, then scale it to -1 <= r < 1
    r = (dblrand() - 0.5) * 2;
    // add error to posX
    posX += (r * maxError);

    // Produce a random double in the range [0,1) using generator 0, then scale it to -1 <= r < 1
    r = (dblrand() - 0.5) * 2;
    // add error to posY
    posY += (r * maxError);

    // Produce a random double in the range [0,1) using generator 0, then scale it to -1 <= r < 1
    r = (dblrand() - 0.5) * 2;
    // add error to speed
    speed = speed + speed * (r * maxError);  // todo: how to set error for speed?

    // set the changes
    wsm->setPos( TraCICoord(posX, posY) );
}


void ApplRSUCLASSIFY::saveTrainingDataToFile()
{
    printf("\nSaving collected training data into '%s' \n", trainingFilePath.c_str());
    printf("Re-run the simulation to train the model ... \n");

    FILE *filePtr = fopen (trainingFilePath.string().c_str(), "w");

    for(unsigned int i = 0; i < samples.size(); ++i)
        fprintf (filePtr, "%0.3f  %0.3f  %0.3f  %0.3f  %0.3f  %d \n", samples[i].pos.x, samples[i].pos.y, samples[i].speed, samples[i].accel, samples[i].angle, labels[i]);

    fclose(filePtr);
}


void ApplRSUCLASSIFY::saveClassificationResults()
{
    if(classifyResults.empty())
        return;

    int currentRun = omnetpp::getEnvir()->getConfigEx()->getActiveRunNumber();

    std::ostringstream fileName;
    fileName << boost::format("%03d_classificationResults.txt") % currentRun;

    boost::filesystem::path filePath ("results");
    filePath /= fileName.str();

    FILE *filePtr = fopen (filePath.c_str(), "w");
    if (!filePtr)
        throw omnetpp::cRuntimeError("Cannot create file '%s'", filePath.c_str());

    // write simulation parameters at the beginning of the file
    {
        // get the current config name
        std::string configName = omnetpp::getEnvir()->getConfigEx()->getVariable("configname");

        std::string iniFile = omnetpp::getEnvir()->getConfigEx()->getVariable("inifile");

        // PID of the simulation process
        std::string processid = omnetpp::getEnvir()->getConfigEx()->getVariable("processid");

        // globally unique identifier for the run, produced by
        // concatenating the configuration name, run number, date/time, etc.
        std::string runID = omnetpp::getEnvir()->getConfigEx()->getVariable("runid");

        // get number of total runs in this config
        int totalRun = omnetpp::getEnvir()->getConfigEx()->getNumRunsInConfig(configName.c_str());

        // get the current run number
        int currentRun = omnetpp::getEnvir()->getConfigEx()->getActiveRunNumber();

        // get configuration nameables
        std::vector<std::string> iterVar = omnetpp::getEnvir()->getConfigEx()->getConfigChain(configName.c_str());

        // write to file
        fprintf (filePtr, "configName      %s\n", configName.c_str());
        fprintf (filePtr, "iniFile         %s\n", iniFile.c_str());
        fprintf (filePtr, "processID       %s\n", processid.c_str());
        fprintf (filePtr, "runID           %s\n", runID.c_str());
        fprintf (filePtr, "totalRun        %d\n", totalRun);
        fprintf (filePtr, "currentRun      %d\n", currentRun);
        fprintf (filePtr, "currentConfig   %s\n", iterVar[0].c_str());
        fprintf (filePtr, "sim timeStep    %u ms\n", TraCI->simulationGetTimeStep());
        fprintf (filePtr, "startDateTime   %s\n", TraCI->simulationGetStartTime().c_str());
        fprintf (filePtr, "endDateTime     %s\n", TraCI->simulationGetEndTime().c_str());
        fprintf (filePtr, "duration        %s\n\n\n", TraCI->simulationGetDuration().c_str());
    }

    for(auto i : classifyResults)
    {
        fprintf (filePtr, "%s ", i.first.c_str());

        int totalPredictions = 0;
        int correctPredictions = 0;
        for(auto j : i.second)
        {
            fprintf (filePtr, "%0.2f %u %u ", j.time, j.label_predicted, j.label_true);

            totalPredictions++;
            if(j.label_predicted == j.label_true)
                correctPredictions++;
        }

        fprintf (filePtr, "| %u %u \n\n", totalPredictions, correctPredictions);
    }

    fclose(filePtr);
}


template <typename beaconGeneral>
void ApplRSUCLASSIFY::draw(beaconGeneral &wsm, unsigned int real_label)
{
    ASSERT(plotterPtr);

    auto it1 = dataBlockCounter.find(real_label);
    // this label already exists
    if(it1 != dataBlockCounter.end())
    {
        // search for vehicle name in the nested map
        auto it2 = (it1->second).find(wsm->getSender());
        // this vehicle already exists
        if(it2 != (it1->second).end())
        {
            // append the new point to the datablock
            fprintf(plotterPtr, "set print $data%d append \n", (it2->second).counter);
            fprintf(plotterPtr, "print \"%0.2f %0.2f %0.2f\" \n", wsm->getPos().x, wsm->getPos().y, wsm->getSpeed());
        }
        else
        {
            unsigned int size = 0;
            for(auto i : dataBlockCounter)
                for(auto j : i.second)
                    size++;

            auto firstElement = it1->second.begin();  // get the first element of the nested map
            HSV color = firstElement->second.color;   // get the color in this block
            dataBlockEntry_t entry = {size + 1, color};
            (it1->second).insert( std::make_pair(wsm->getSender(), entry) );

            // update color shades
            std::vector<double> shades = Color::generateColorShades(it1->second.size());
            int counter = 0;
            for(auto &c : it1->second)
            {
                c.second.color.saturation = shades[counter];
                counter++;
            }

            // creating a new datablock
            fprintf(plotterPtr, "$data%d << EOD \n", size + 1);
            fprintf(plotterPtr, "%0.2f %0.2f %0.2f \n", wsm->getPos().x, wsm->getPos().y, wsm->getSpeed());
            fprintf(plotterPtr, "EOD \n");
        }
    }
    else
    {
        unsigned int size = 0;
        for(auto i : dataBlockCounter)
            for(auto j : i.second)
                size++;

        HSV color = Color::getUniqueHSVColor();
        dataBlockEntry_t entry = {size + 1, color};
        dataBlockCounter[real_label].insert( std::make_pair(wsm->getSender(), entry) );

        // creating a new datablock
        fprintf(plotterPtr, "$data%d << EOD \n", size + 1);
        fprintf(plotterPtr, "%0.2f %0.2f %0.2f \n", wsm->getPos().x, wsm->getPos().y, wsm->getSpeed());
        fprintf(plotterPtr, "EOD \n");
    }

    // debugging
    if(false)
    {
        for(auto &i : dataBlockCounter)
        {
            std::cout << "label: " << i.first << std::endl;
            for(auto &j : i.second)
            {
                std::cout << "    Id: " << j.first
                        << ", blockNum: " << j.second.counter
                        << ", color: (" << j.second.color.hue << "," << j.second.color.saturation << "," << j.second.color.value << ")"
                        << std::endl;

                // prints datablock value
                // fprintf(plotterPtr, "set print \n");
                // fprintf(plotterPtr, "print $data%d \n", j.second.counter);
            }
        }

        std::cout << std::endl;
    }

    // merge all small datablocks
    fprintf(plotterPtr, "undefine $dataAll \n");
    fprintf(plotterPtr, "set print $dataAll \n");
    for(auto &i : dataBlockCounter)
    {
        for(auto &j : i.second)
        {
            fprintf(plotterPtr, "print $data%d \n", j.second.counter);
            fprintf(plotterPtr, "print \"\" \n");
        }
    }

    // make plot
    fprintf(plotterPtr, "splot ");

    int index = 0;
    for(auto &i : dataBlockCounter)
    {
        for(auto &j : i.second)
        {
            // get the color for this block
            HSV color = j.second.color;
            fprintf(plotterPtr, "'$dataAll' index %d using 1:2:3 with points pointtype 7 pointsize 1 linecolor rgbcolor hsv2rgb(%f,%f,%f) linewidth 1 title 'class %d',", index, color.hue/360, color.saturation/100, color.value/100, i.first);
            index++;
        }
    }

    // complete the splot command
    fprintf(plotterPtr, " \n");

    fflush(plotterPtr);
}

}
