VENTOS
======

VENTOS is an integrated C++ simulator for studying traffic flows in Vehicular Ad-hoc Networks (VANETs). You can find more information [here](http://maniamoozadeh2.wix.com/ventos). VENTOS framework is developed in Ubuntu OS, and we have not tested it under other operating systems (yet!). Follow these instructions in order to install VENTOS (and its prerequisites) on your Ubuntu 12 or 14 machine. 


Install OMNET++
---------------

**Step 1:** Download the latest OMNET++ compressed file from [here](http://www.omnetpp.org/omnetpp) and extract it into a folder (folder name should not have space), and make sure you have write permission. 


**Step 2:** You should install a couple of packages, but first refresh the database of available packages:

    sudo apt-get update

Then

    sudo apt-get install build-essential gcc g++ bison flex perl tcl-dev tk-dev libxml2-dev zlib1g-dev default-jre doxygen graphviz libwebkitgtk-1.0-0 openmpi-bin libopenmpi-dev libpcap-dev


Note: Use `java -version` to check your current version of java, and make sure it is 1.7 or higher. If not, install it as follows:

    sudo add-apt-repository ppa:webupd8team/java
    sudo apt-get update
    sudo apt-get install oracle-java7-installer
    sudo apt-get install oracle-java7-set-default


**Step 3:** Navigate to OMNET++ folder, open the file 'configure.user' and make sure that USE_CXX11 is set to 'yes'. Also use `g++ -v` and check if the g++ version is 4.8 or higher. If not, install it as follows:

    sudo add-apt-repository ppa:ubuntu-toolchain-r/test
    sudo apt-get update
    sudo update-alternatives --remove-all gcc 
    sudo update-alternatives --remove-all g++
    sudo apt-get install gcc-4.8
    sudo apt-get install g++-4.8
    sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-4.8 20
    sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-4.8 20
    sudo update-alternatives --config gcc
    sudo update-alternatives --config g++

**Step 4:** OMNeT++ requires that its bin directory be in the PATH. You should add the following line to your .bashrc (note that you should change the path accordinglly):

    export PATH=$PATH:/home/mani/Desktop/omnetpp-4.6/bin

Note that You need to close and re-open the terminal for the changes to take effect.

**Step 5:** In the OMNeT++ directory, type the following command. The configure script detects installed software and configuration of your system. It writes the results into the `Makefile.inc` file, which will be read by the makefiles during the build process. Pay close attention to errors and warnings! (ignore warnings regarding Akaroa!).

    ./configure

**Step 6:** Now you can compile OMNET++ by using make. Compiling takes some time, so relax, grab a cup of coffee and watch [this](https://www.youtube.com/watch?v=A6XUVjK9W4o).

    make

**Step 7:** Installation of OMNET++ is done!, and you can verify that the sample simulations run correctly. For example, the dyna simulation is started by entering the following commands. First we go to the sample/dyna folder, and then we run it.

    cd samples/dyna
    ./dyna

**Step 8:** Good! You can run the Eclipse IDE by typing the following command in the terminal:

    omnetpp

The first time you run OMNET++, Eclipse IDE asks you to select a workspace. Select the folder that you will use to store all your project files (Veins, VENTOS, INET, etc). If you have intention to store all your projects on Desktop, then change the workspace to Desktop. Also check "Use this as the default and do not ask again".

"Introduction to OMNET++" page will appear. Click on "Workbench". Then it asks you to install INET framework or import some examples into your workspace. Uncheck them both since we do not need them for the time being.

**Step 9:** Running OMNET++ each time from command line might not be desirable for you. You can create a shortcut to OMNET++ on your desktop. To do this type the following command in OMNET++ directory.

    make install-desktop-icon

or you can create an application launcher:

    make install-menu-item


Import Veins
------------

Veins is on github, and you should clone it into your computer.

**Step 1:** install the git package:

    sudo apt-get install git

**Step 2:** Use the following command to clone the Veins repository on your machine. This command creates a folder called veins in the current directory (run the command in the same folder as Eclipse workspace as you specified before in step 8)

    git clone https://github.com/sommer/veins

**Step 3:** Now you can import the Veins project into the IDE. Choose "File->Import" from the menu. Choose "General->Existing Projects into Workspace" from the upcoming dialog and proceed with "Next". Choose "Select root directory" and select the Veins folder. "veins" should appear in the "Projects" section. Unselect "Copy project into workspace" if the veins folder is already in your workspace. Click "Finish".

**Step 4:** Now make these modifications in Veins source code. In the IDE, open veins project and go to 'src/veins/modules/mobility/traci/' folder.

- Open TraCIScenarioManager.h file and change `void executeOneTimestep()` to `virtual void executeOneTimestep()`, and `void addModule()` to `virtual void addModule()`

- Open TraCICommandInterface.h file, and change access modifier private into public!

- [optional] Open TraCIConnection.cc file, and comment line "MYDEBUG << "Reading TraCI message of " << bufLength << " bytes" << endl;" and also "MYDEBUG << "Writing TraCI message of " << buf.length() << " bytes" << endl;"

**Step 5:** After these modifications, build the veins project. To build the veins project, right-click on the project name and choose "Build Project". Wait for a while and then check the Console windows at the bottom of the Eclipse IDE to make sure no errors occurred. At the end you should see a message like this:

    00::04:42 Build Finished (took 2m:23s.177ms)


Import VENTOS
-------------

**Step 1:** Make sure you have these packages installed:

Ubuntu 12.04:

    sudo apt-get install libboost-system1.48-dev libboost-filesystem1.48-dev libssl-dev libcurl4-gnutls-dev libxerces-c2-dev libfox-1.6-dev libproj-dev gnuplot gnuplot-x11
    
Ubuntu 14.04:

    sudo apt-get install libboost-system1.54-dev libboost-filesystem1.54-dev libssl-dev libcurl4-gnutls-dev libxerces-c2-dev libfox-1.6-dev libproj-dev gnuplot gnuplot-x11 


**Step 2:** install the git package (if you have not done it before):

    sudo apt-get install git

**Step 3:** Clone the VENTOS repository in the current directory (run the command in the same folder as Eclipse workspace as you specified before). You also need to provide your username and password of github account.

    git clone https://github.com/ManiAm/VENTOS

**Step 4:** Now you can import VENTOS project into the OMNET++ IDE. Choose "File->Import" from the menu. Choose "General->Existing Projects into Workspace" from the upcoming dialog and proceed with "Next". Choose "Select root directory" and select the VENTOS folder. "VENTOS" should appear in the "Projects" section. Unselect "Copy project into workspace" if the VENTOS folder is already in your workspace. Click "Finish".

Note: Wait for a while until "C/C++ Indexer" finishes its job.

**Step 5:** Right click on VENTOS project name and click Properties. Go to 'Project References' and make sure veins is selected.

**Step 6:** Now you build the VENTOS project. Right-click on the project name and choose "Build Project". Wait for a while and then check the Console windows at the bottom of the Eclipse IDE to make sure no errors occurred.
    

Running the VENTOS Project
--------------------------

**Step 1:** Before running the VENTOS project, you have to run the python script "sumo-launchd.py" which is in the veins folder. The general format of this script is:

    python sumo-launchd.py --vv -c /path/to/sumo/binary

sumoD and sumo-guiD are two SUMO binaries that can be used with the above command. They both will be downloaded automatically in the VENTOS/sumoBinary folder upon the first run of the VENTOS project. sumo-guiD is the GUI version that provides a graphical interface to SUMO, and is visible while the OMNET++ simulation is running (and is very good for debugging purposes). On the other hands, sumoD is the command-line version which is faster.

Thus you should run either of the following commands in the veins folder (with this assumption that VENTOS folder is stored on Desktop):

SUMO in graphical mode

    python sumo-launchd.py -vv -c /home/mani/Desktop/VENTOS/sumoBinary/sumo-guiD

SUMO in command-line mode

    python sumo-launchd.py -vv -c /home/mani/Desktop/VENTOS/sumoBinary/sumoD


**Step 2:** To make the life easier, you can create an alias command to make a shortcut to the above long commands. Open .bashrc and add these lines at the end.

    export veinsPATH=/home/mani/Desktop/veins
    export sumoBinaryPATH=/home/mani/Desktop/VENTOS/sumoBinary
    alias sumo-cmd='python $veinsPATH/sumo-launchd.py -vv -c $sumoBinaryPATH/sumoD'
    alias sumo-gui='python $veinsPATH/sumo-launchd.py -vv -c $sumoBinaryPATH/sumo-guiD'

Now, you can use sumo-cmd and sumo-gui commands instead! Note that after running either of these commands, DO NOT close the terminal window. Leave it open.


**Step 3:** Now you can run the VENTOS project. To do this, right click on the omnetpp.ini file in the IDE and choose: 

    "Run as" -> "Run configurations..."

Choose 'OMNET++ Simulation' and click on 'New launch configuration' button at the top left. Give this configuration a name like myConfig. In Executable, choose opp_run and in 'Config name' choose a configuration from drop down list like CACCVehicleStream1. Leave the rest of the options to default. Click Apply and then click Run.

Upon the first execution, VENTOS tries to download sumoD and sumo-guiD. Type y to proceed.

