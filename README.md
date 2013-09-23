C3PO
====

Open this project in OMNET++

You need 'veins framework' which can be downloaded from here:
http://veins.car2x.org/

To be able to run C3PO project, you should make these modification in veins:

1) go to TraCIScenarioManager.h and find void executeOneTimestep()
change it to virtual void executeOneTimestep();

2) go to TraCIScenarioManager.cc and find method init_traci()
add (apiVersion == 6) to the if list!
