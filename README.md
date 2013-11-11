C3PO
====

Open this project in OMNET++

You need 'veins framework' which can be downloaded from here:
http://veins.car2x.org/

Pre-requisites
--------------

To be able to run C3PO project, you should make these modification in veins:

1) go to TraCIScenarioManager.h and find void executeOneTimestep()
change it to virtual void executeOneTimestep();

2) go to TraCIScenarioManager.cc and find method init_traci()
add (apiVersion == 6) to the if list!

3) [optional] comment the MYDEBUG line in TraCIScenarioManager::receiveTraCIMessage() and also TraCIScenarioManager::sendTraCIMessage to make the output console less messy :)

Running the project
-------------------

Go to the veins folder and look for a python script called sumo-launchd.py. You need to run this python script using the following command:

         python sumo-launchd.py -vv -c sumoD

The bove command runs the command-line version of sumo. Thus you will not be able to see the sumo gui windows. If you need to see the sumo-gui windows while omnet++ simulation is running, then you should use the following command:

         python sumo-launchd.py -vv -c sumo-guiD

After running the above script, run the C3PO project in omnet++. Simulation parameters are all in the omnet.ini file.
