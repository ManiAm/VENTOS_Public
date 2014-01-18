VENTOS
======

VENTOS (**VE**hicular **N**e**T**work **O**pen **S**imulator) is a novel integrated open-source simulator for studying traffic flows in vehicular ad-hoc networks. VENTOS is the main tool in C3PO research. More information here: "http://www.ece.ucdavis.edu/rubinet/projects/c3po.html". This project uses the following softwares and packages:

OMNET++: OMNET++ is a network simulator.

Veins framework: We used Veins to connect SUMO to OMNET++ using TraCI interface, as well as using IEEE 802.11p protocol in data-link layer.

SUMO: SUMO is an open source, microscopic, continuous-space, discrete-time road traffic simulator, developed at German Aerospace Center. We implemeted multiple car following models in SUMO.


Install OMNET++
---------------

**Step 1:** Download OMNET++ from "http://www.omnetpp.org/". Then extract the file to a location, say your desktop (make sure the folder that you extract OMNET++ file into, does not have space). You can find installation instructions in doc/InstallGuide.pdf file.

**Step 2:** Install these packages in Ubuntu

        sudo apt-get install build-essential gcc g++ bison flex perl \
        tcl-dev tk-dev blt libxml2-dev zlib1g-dev default-jre \
        doxygen graphviz libwebkitgtk-1.0-0 openmpi-bin libopenmpi-dev libpcap-dev

**Step 3:** The default tooltip background color in Ubuntu is black, which causes certain tooltips in the OMNeT++ IDE to become unreadable (black-on-black). This annoyance can be resolved by changing the tooltip colors in Ubuntu. Install gnome-color-chooser:

        $ sudo apt-get install gnome-color-chooser

Run it:
 
        $ gnome-color-chooser

Find the Tooltips group on the Specific tab, and change the settings to black foreground over pale yellow background. Click Apply.

**Step 4:** OMNeT++ requires that its bin directory should be in the PATH. You should add a line something like this to your .bashrc (note that you should change the path if you installed omnet++ in a different directoy):

        export PATH=$PATH:/home/mani/Desktop/omnetpp-4.3.1/bin

**Step 5:** Set the TCL_LIBRARY environment variable like following in .bashrc. Without this, Tkenv (the GUI runtime environment) may be unable to find the BLT library.

        export TCL_LIBRARY=/usr/share/tcltk/tcl8.5

**Step 6:** In the OMNeT++ directory, type the following command. The configure script detects installed software and configuration of your system. It writes the results into the Makefile.inc file, which will be read by the makefiles during the build process. Pay close attention to warnings! (ignore warnings regarding Akaroa!).

        $ ./configure

**Step 7:** Now you can compile OMNET++ by using make. Compiling takes some time, so relax and grab a cup of coffe!

        make

**Step 8:** You can now verify that the sample simulations run correctly. For example, the dyna simulation is started by entering the following commands. First we go to the sample/dyna folder, and then we run it.

        cd samples/dyna
        ./dyna

**Step 9:** You can run the IDE by typing the following command in the terminal:

        $ omnetpp

The first time you run OMNET++, eclipse IDE asks you to select a workspace. Select the folder that you will use to store all your project files (Veins, VENTOS, INET, etc). If you have intention to store all your projects on desktop, then change the workspace to dekstop. Also check "Use this as the default and do not ask again".

"Introduction to OMNET++" page will apear. Click on "Workbench" to pop up the eclipse IDE environment.

Then it ask you to install INET framework or import some examples into your workspace. Uncheck them both since we do not need them for the time being.

**Step 10:** You can also create a shortcut of OMNET++ on your desktop. To do this type the following command in OMNET++ directory.

        make install-desktop-icon

or you can create an application launcher:

        make install-menu-item


Install Veins
-------------

**Step 1:** Download Veins from "http://veins.car2x.org" to a location like desktop (without extracting it).

**Step 2:** In OMNET++, choose "File->Import" from the menu. Choose "General->Existing Projects into Workspace" from the upcoming dialog and proceed with "Next". Choose "Select archive file" and select the download veins file. "mixim (veins-2.1)" should appear in the "Projects". At last click "Finish". Now you can delete the downloaded veins file.

To build the veins project, right-click on the project and choose "Build Project". Wait for a while and then check the Console windows at the bottom of the eclipse IDE to make sure no errors occured. At the end you should see a message like this:

          00::04:42 Build Finished (took 2m:23s.177ms)

**Step 3:** To be able to run C3PO project, you should make these modifications in Veins:

1) In the IDE, open veins project and go to src/modules/mobility/traci/TraCIScenarioManager.h and find void executeOneTimestep(), and then change it to virtual void executeOneTimestep();

2) go to src/modules/mobility/traci/TraCIScenarioManager.cc and find method init_traci(). In the body of the method, find the if statement and change it from 

		if ((apiVersion == 3) || (apiVersion == 5)) {
			MYDEBUG << "TraCI server \"" << serverVersion << "\" reports API version " << apiVersion << endl;
		}

to

		if ((apiVersion == 3) || (apiVersion == 5) || (apiVersion == 6) || (apiVersion == 7) ) {
			MYDEBUG << "TraCI server \"" << serverVersion << "\" reports API version " << apiVersion << endl;
		}

3) [optional] comment the MYDEBUG line in TraCIScenarioManager::receiveTraCIMessage() and also TraCIScenarioManager::sendTraCIMessage to make the output console less messy :)

After these modifications, build the veins project again. Build process is fast this time!


Import VENTOS
-------------

VENTOS project is on github, and you should clone it into your computer.

**Step 1:** install the git package:

         sudo apt-get install git

**Step 2:** Create a directory on your computer (say VENTOS on desktop). In terminal, navigate to this folder , and issue the following commnand:

         git clone https://github.com/ManiAm/VENTOS

It will ask your username/password, and then receives a copy of the project.

**Step 3:** Now you can import VENTOS project into the OMNET++ IDE. Choose "File->Import" from the menu. Choose "General->Existing Projects into Workspace" from the upcoming dialog and proceed with "Next". Choose "Select root directory" and select the VENTOS folder. "C3PO" should appear in the "Projects". Unselect "Copy project into workspace" if the VENTOS folder is already in your workspace. Click "Finish".

**Step 4:** Right click on C3PO project and click Properties. Go to 'Project References' and make sure mixim is selected.


Install SUMO
------------

**Step 1:** Download the SUMO from "http://sumo-sim.org/wiki/Main_Page".









Running the Project
-------------------

**Step 1:** Before running the VENTOS project, you have to run the python script "sumo-launchd.py". Go to the veins folder and look for sumo-launchd.py file. Run this python script using the following command:

         python sumo-launchd.py -vv -c sumoD

The bove command runs the command-line version of sumo. Thus you will not be able to see the sumo gui windows. If you need to see the sumo-gui windows while OMNET++ simulation is running, then you should use the following command:

         python sumo-launchd.py -vv -c sumo-guiD

After running the above script, DO NOT close the terminal window. Leave it open.

**Step 2:** Now you can run the VENTOS project. To do this, right click on the omnetpp.ini file in the IDE and choose: 

         "Run as" -> "Run configurations..."

Choose 'OMNET++ Simulation' and click on 'New launch configuration' button. Give this configuration a name like C3PO. In Executable, choose opp_run and in 'Config name' choose a configuration from drop down list like General. Leave the rest of options to default. Click Apply and then click Run. OMNET++ tries to compile/build the project, and then run it.


Committing your changes
-----------------------










