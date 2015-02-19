VENTOS
======

VENTOS framework is developed in Ubuntu OS, and we have not tested it under Windows (yet!). This section shows you how to install VENTOS and its prerequisites step by step on Ubuntu 12.04 or 14.04 machine. 


Install OMNET++
---------------

**Step 1:** Download OMNET++ from "http://www.omnetpp.org/". Then extract the file to a location, say your desktop (make sure the folder that you extract OMNET++ file into, does not have space). You can find installation instructions in doc/InstallGuide.pdf file.

**Step 2:** OMNeT++ requires that its bin directory be in the PATH. You should add the following line to your .bashrc (note that you should change the path if you installed OMNET++ in a different directory):

        export PATH=$PATH:/home/mani/Desktop/omnetpp-4.5/bin

**Step 3:** Set the TCL_LIBRARY environment variable like following in .bashrc. Without this, Tkenv (the GUI runtime environment) may be unable to find the BLT library.

        export TCL_LIBRARY=/usr/share/tcltk/tcl8.5

In Ubuntu 14.04 use tcl8.6 instead.

**Step 4:** In the OMNeT++ directory, type the following command. The configure script detects installed software and configuration of your system. It writes the results into the `Makefile.inc` file, which will be read by the makefiles during the build process. Pay close attention to errors and warnings! (ignore warnings regarding Akaroa!).

        ./configure

**Step 5:** Now you can compile OMNET++ by using make. Compiling takes some time, so relax and grab a cup of coffee!

        make

**Step 6:** You can now verify that the sample simulations run correctly. For example, the dyna simulation is started by entering the following commands. First we go to the sample/dyna folder, and then we run it.

        cd samples/dyna
        ./dyna

**Step 7:** You can run the IDE by typing the following command in the terminal:

        omnetpp

The first time you run OMNET++, Eclipse IDE asks you to select a workspace. Select the folder that you will use to store all your project files (Veins, VENTOS, INET, etc). If you have intention to store all your projects on desktop, then change the workspace to desktop. Also check "Use this as the default and do not ask again".

"Introduction to OMNET++" page will appear. Click on "Workbench" to pop up the Eclipse IDE environment.

Then it asks you to install INET framework or import some examples into your workspace. Uncheck them both since we do not need them for the time being.

**Step 8:** You can also create a shortcut of OMNET++ on your desktop. To do this type the following command in OMNET++ directory.

        make install-desktop-icon

or you can create an application launcher:

        make install-menu-item


Import Veins
------------

Veins is on github, and you should clone it into your computer.

**Step 1:** install the git package:

         sudo apt-get install git

**Step 2:** Create a directory on your computer (say Veins on desktop). In terminal, navigate to this folder , and issue the following command:

         git clone https://github.com/sommer/veins

**Step 3:** Now you can import Veins project into the OMNET++ IDE. Choose "File->Import" from the menu. Choose "General->Existing Projects into Workspace" from the upcoming dialog and proceed with "Next". Choose "Select root directory" and select the Veins folder. "veins" should appear in the "Projects". Unselect "Copy project into workspace" if the Veins folder is already in your workspace. Click "Finish".

**Step 4:** Now make these modifications in Veins source code. In the IDE, open Veins project and go to 'src/veins/modules/mobility/traci/' folder.

- Open TraCIScenarioManager.h file and change `void init_traci()` to `virtual void init_traci()`, `void executeOneTimestep()` to `virtual void executeOneTimestep()`, and `void addModule()` to `virtual void addModule()`

- Open TraCICommandInterface.h file, and change access modifier private into public!

- [optional] Open TraCIConnection.cc file, and comment line "MYDEBUG << "Reading TraCI message of " << bufLength << " bytes" << endl;" and also "MYDEBUG << "Writing TraCI message of " << buf.length() << " bytes" << endl;"

**Step 5:** After these modifications, build the Veins project. To build the Veins project, right-click on the project and choose "Build Project". Wait for a while and then check the Console windows at the bottom of the Eclipse IDE to make sure no errors occurred. At the end you should see a message like this:

          00::04:42 Build Finished (took 2m:23s.177ms)


Import VENTOS
-------------

**Step 1:** install the git package (if you have not done it before):

         sudo apt-get install git

**Step 2:** Create a directory on your computer (say VENTOS on desktop). In terminal, navigate to this folder, and issue the following commnand:

         git clone https://github.com/ManiAm/VENTOS

It will ask your username/password, and then receives a copy of the project.

**Step 3:** Now you can import VENTOS project into the OMNET++ IDE. Choose "File->Import" from the menu. Choose "General->Existing Projects into Workspace" from the upcoming dialog and proceed with "Next". Choose "Select root directory" and select the VENTOS folder. "VENTOS" should appear in the "Projects". Unselect "Copy project into workspace" if the VENTOS folder is already in your workspace. Click "Finish".

**Step 4:** Right click on VENTOS project and click Properties. Go to 'Project References' and make sure Veins is selected.

**Step 5:** Make sure you have these packages installed:

Ubuntu 12.04 : `libboost-filesystem1.48-dev`, `libboost-system1.48-dev`, `libssl-dev`

Ubuntu 14.04 : `libboost-filesystem1.54-dev`, `libboost-system1.54-dev`, `libssl-dev`

`libcurl3`, `libxerces-c2-dev`, `libfox-1.6-dev`, `libproj-dev`

**Step 6:** Install Gnuplot 4.6:

    sudo add-apt-repository ppa:gladky-anton/precise-backports
    sudo apt-get update
    sudo apt-get install gnuplot
    sudo apt-get install gnuplot-x11


Running the Project
-------------------

**Step 1:** Before running the VENTOS project, you have to run the python script "sumo-launchd.py". Go to the Veins folder and look for sumo-launchd.py file. Run this python script using the following command:

         python sumo-launchd.py -vv -c sumoD

The bove command runs the command-line version of sumo. Thus you will not be able to see the sumo gui windows. If you need to see the sumo-gui windows while OMNET++ simulation is running, then you should use the following command:

         python sumo-launchd.py -vv -c sumo-guiD

After running the above script, DO NOT close the terminal window. Leave it open.

**Step 2:** Now you can run the VENTOS project. To do this, right click on the omnetpp.ini file in the IDE and choose: 

         "Run as" -> "Run configurations..."

Choose 'OMNET++ Simulation' and click on 'New launch configuration' button. Give this configuration a name like C3PO. In Executable, choose opp_run and in 'Config name' choose a configuration from drop down list like General. Leave the rest of the options to default. Click Apply and then click Run. OMNET++ tries to compile/build the project, and then run it.


