VENTOS
======

VENTOS is an integrated C++ simulator for studying traffic flows in Vehicular Ad-hoc Networks (VANETs). You can find more information [here](http://rubinet.ece.ucdavis.edu/projects/ventos). VENTOS framework is developed in Ubuntu OS, and we have not tested it under other operating systems (yet!). Follow these instructions in order to install VENTOS (and its prerequisites) on your Ubuntu 14 machine.


Install OMNET++
---------------

**Step 1:** Download the latest OMNET++ compressed file from [here](http://www.omnetpp.org/omnetpp) and extract it into a folder (folder name should not have space), and make sure you have write permission in that folder.


**Step 2:** You should install a couple of packages, but first refresh the database of available packages:

    sudo apt-get update

Then

    sudo apt-get install build-essential gcc g++ bison flex perl tcl-dev tk-dev libxml2-dev zlib1g-dev default-jre doxygen graphviz libwebkitgtk-1.0-0 openmpi-bin libopenmpi-dev libpcap-dev


**Step 3:** Use `java -version` to check your current version of java, and make sure it is 1.7 or higher. If not, install it as follows:

    sudo add-apt-repository ppa:webupd8team/java
    sudo apt-get update
    sudo apt-get install oracle-java7-installer
    sudo apt-get install oracle-java7-set-default

**Step 4:** OMNeT++ requires that its bin directory be in the PATH. You should add the following line to your .bashrc (note that you should change the path accordinglly):

    export PATH=$PATH:/home/mani/Desktop/omnetpp-4.6/bin

Save the file and then close/re-open the terminal for the changes to take effect.

**Step 5:** In the OMNeT++ directory, type the following command. Pay close attention to errors and warnings, and ignore warnings regarding Akaroa!.

    ./configure

For geeks: In the OMNET++ folder you can see the `configure.in` file that is used by autoconf to generate the `configure` script. Additional input parameters for configure script are defined in `configure.user` file. The configure script detects installed software and configuration of your system, and generates three outputs: `config.log`, `config.status`, and `Makefile.inc`. The first two files are used for debugging purposes, and the last file will be read later by the makefiles during the build process.

**Step 6:** Starting from version 4.6, OMNET++ uses C++11 standard (formerly known as C++0x) by default. This means that OMNET++ source code as well as all the imported projects will be compiled with this flag. Look for this line in the configure output, and make sure that it is 'yes'.

    checking whether g++ supports -std=c++11... yes

If not, first navigate to OMNET++ folder and open the 'configure.user' file. Find USE_CXX11 flag and set it to 'yes'. If USE_CXX11 is already set to 'yes', then your g++ compiler does not support C++11 standard. Follow these instructions to install g++ 4.8:

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

Run ./configure again and re-check.

**Step 7:** Now you can compile OMNET++ by using make. Compiling takes some time, so relax, grab a cup of coffee <img src="https://github.com/ManiAm/VENTOS/blob/master/coffee.png" width="25" height="25" /> and watch [this](https://www.youtube.com/watch?v=A6XUVjK9W4o).

    make

**Step 8:** OMNET++ installation is done and you can verify this by running a sample simulation. For example, the dyna simulation is started by entering the following commands. First go to the sample/dyna folder, and then run it.

    cd samples/dyna
    ./dyna

**Step 9:** Good! You can run the Eclipse IDE by typing the following command in the terminal:

    omnetpp

The first time you run OMNET++, Eclipse IDE asks you to select a workspace. Select the folder that you will use to store all your project files (Veins, VENTOS, INET, etc). If you have intention to store all your projects on Desktop, then change the workspace to Desktop. Also check "Use this as the default and do not ask again".

"Introduction to OMNET++" page will appear. Click on "Workbench". Then it asks you to install INET framework or import some examples into your workspace. Uncheck them both since we do not need them for the time being.

**Step 10:** Running OMNET++ each time from command line might not be desirable for you. You can create a shortcut to OMNET++ on your desktop. To do this type the following command in OMNET++ directory.

    make install-desktop-icon

or you can create an application launcher:

    make install-menu-item


Import VENTOS
-------------

**Step 1:** install the git package:

    sudo apt-get install git

**Step 2:** Make sure you have these packages installed on your Ubuntu 14 machine:

    sudo apt-get install libboost-all-dev libssl-dev libcurl4-gnutls-dev libxerces-c2-dev libfox-1.6-dev libproj-dev libpcap-dev libusb-1.0-0-dev gnuplot gnuplot-x11 
    
**Step 3:** Download snmp++ API from [here](http://www.agentpp.com/download/snmp++-3.3.5.tar.gz). Extract it and then build the library:

    ./configure
    make
    sudo make install

libsnmp++ library will be copied into /usr/local/lib and the header files will be copied into /usr/local/include.

**Step 4:** Clone the shark repository into your computer:

    git clone https://github.com/Shark-ML/Shark/

Install all the required packages:

    sudo apt-get install cmake cmake-curses-gui libatlas-base-dev libboost-all-dev

Go to the Shark folder and make the library:

    mkdir build
    cd build
    cmake ../
    
In the build folder, open the CMakeCache.txt file and make the following changes:

- Change BUILD_EXAMPLES to OFF
- Change BUILD_TESTING to OFF
- Change BUILD_SHARED_LIBS to ON
- Change CBLAS_INCLUDES:FILEPATH to /usr/include/atlas/cblas.h

And then

    make
    sudo make install

libshark library will be copied into /usr/local/lib and the header files will be copied into /usr/local/include.

**Step 5:** Finally after installing the above shared libraries run the following command.

    sudo ldconfig -v

ldconfig is a program that is used to maintain the shared library cache in linux. This cache is typically stored in the file /etc/ld.so.cache and is used by the system to map a shared library name to the location of the corresponding shared library file.

**Step 6:** Download Eigen library from [here](http://eigen.tuxfamily.org/index.php?title=Main_Page). Extract it and then install it on your system.

    mkdir build
    cd build
    cmake ../
    sudo make install

Eigen consists only of header files, hence there is nothing to compile before you can use it. The header files will be copied into /usr/local/include.

**Step 7:** Clone the VENTOS repository in the current directory (run the command in the same folder as Eclipse workspace that you specified before). You also need to provide your github username and password.

    git clone https://github.com/ManiAm/VENTOS

**Step 8:** Now you can import VENTOS project into the OMNET++ IDE. Choose "File->Import" from the menu. Choose "General->Existing Projects into Workspace" from the upcoming dialog and proceed with "Next". Choose "Select root directory" and select the VENTOS folder. "VENTOS" should appear in the "Projects" section. Unselect "Copy project into workspace" if the VENTOS folder is already in your workspace. Click "Finish".

Note: Wait for a while until "C/C++ Indexer" finishes its job.

**Step 9:** Now you can build the VENTOS project. Use Ctrl+B or right-click on the project name and choose "Build Project". Wait for a while and then check the console windows at the bottom of the Eclipse IDE to make sure no errors occurred.
    

Running the VENTOS Project
--------------------------

Before running the VENTOS project, latest SUMO binaries (`sumo-guiD` and `sumoD`) should be obtained. These binaries will be downloaded automatically in the VENTOS/sumoBinary folder upon the first run of the project. sumo-guiD is the GUI version that provides a graphical interface to SUMO, and is visible while the OMNET++ simulation is running (and is very useful for debugging purposes). On the other hands, sumoD is the command-line version which is faster. 

Note: you can not use the official SUMO binaries since we have extended the TraCI commands, car-following models, etc. Using the official SUMO binaries will probably give you run-time error.

**Step 1:** To save you the trouble of manually running SUMO prior to every OMNeT++ simulation, the Veins module framework comes with a small python script (sumo-launchd.py) to do that for you. This script will proxy TCP connections between OMNeT++ and SUMO, starting a new copy of the SUMO simulation for every OMNeT++ simulation connecting. From the veins folder, run either of the following commands (we assumed that VENTOS folder is on Desktop):

SUMO in graphical mode:

    python sumo-launchd.py -vv -c /home/mani/Desktop/VENTOS/sumoBinary/sumo-guiD

SUMO in command-line mode:

    python sumo-launchd.py -vv -c /home/mani/Desktop/VENTOS/sumoBinary/sumoD

The terminal should look like the following picture. The script will print `Listening on port 9999` and wait for the simulation to start. Note that after running either of these commands, DO NOT close the terminal window. Leave it open.

<img src="https://github.com/ManiAm/VENTOS/blob/master/launchd.png" />

**Step 2:** To make the life easier, you can create an alias command to make a shortcut to the above long commands. Open .bashrc and add these lines at the end (you should change the PATHs accordingly).

    export veinsPATH=/home/mani/Desktop/veins
    export sumoBinaryPATH=/home/mani/Desktop/VENTOS/sumoBinary
    alias sumo-cmd='python $veinsPATH/sumo-launchd.py -vv -c $sumoBinaryPATH/sumoD'
    alias sumo-gui='python $veinsPATH/sumo-launchd.py -vv -c $sumoBinaryPATH/sumo-guiD'

Now you can use sumo-cmd and sumo-gui commands instead!

**Step 3:** libpcap library is used to sniff packets from network interfaces. We need to give permission to opp_run to be able to access the network interfaces (check [here](http://packetlife.net/blog/2010/mar/19/sniffing-wireshark-non-root-user/)). Moreover, libusb needs permission to open a USB device for read/write. This is done through setting [capabilities](http://linux.die.net/man/7/capabilities) using the following command:

    sudo setcap cap_dac_override,cap_net_raw,cap_net_admin=eip /home/mani/Desktop/omnetpp-4.6/bin/opp_run

Note that you need to change the path to opp_run accordingly.

**Step 4:** Run the VENTOS project by right clicking on the project name in the IDE and choose: 

    "Run as" -> "Run configurations..."

Choose 'OMNET++ Simulation' and click on 'New launch configuration' button at the top left. Give this configuration a name like myConfig. In Executable, choose opp_run and in 'Ini file(s)' choose one of the ini files in the project like params2_CACC.ini. From 'Config name' choose a configuration from the drop down list like CACCVehicleStream1. Leave the rest of the options to default. Click Apply and then click Run.

Upon the first execution, VENTOS tries to download sumoD and sumo-guiD. Type y to proceed. Try to play with other configuration scenarios to feel confortable with the environment and change different parameters to see the effect on the simulation output.


Running VENTOS from command-line
--------------------------------

The Eclipse IDE launcher helps you start the simulation easily by building the command line and the environment for your
program automatically. Sometimes it would be great to start the simulation from outside the IDE. To get the command line used for running the simulation, run the simulation from the IDE. Go to 'debug view' and right click on your program and select 'properties'. The IDE shows the currently used command line such as:

    opp_run -r 0 -u Cmdenv -c CACCVehiclePlatoonManag1 -n .:../veins/examples/veins:../veins/src/veins --tkenv-image-path=../veins/images -l application/router/router -l application/pedestrian/pedestrian -l application/sniffing/sniffing -l application/traci/traci -l application/vehicle/vehicle -l application/global/global -l application/msg/msg -l application/trafficLightCobalt/trafficLightCobalt -l application/bike/bike -l application/adversary/adversary -l application/trafficLight/trafficLight -l application/rsu/rsu -l ../veins/src/veins params3_Platoon.ini

- `opp_run` allows starting simulation models that are linked as shared libraries
- `-r <runnumber>` allows you to select runs
- `-u Cmdenv` tells omnetpp to run under Cmdenv (command-line environment)
- `-c <configname>` option is used to select a configuration
- `-n` option is used to specify the NED path
- `-l` option is used to load additional shared libraries


Running VENTOS on a Server
--------------------------

You can run the project on a remote server (Ubuntu Server preferably). 

**Step 1:** You have to make sure that a ssh server is running on the remote server. To check if a ssh server is running on the server, type the following command in the server:

    ssh localhost

If you receive the following message then the ssh server is not installed.

    ssh: connect to host localhost port 22: Connection refused

Install openssh-server using the following command:

    apt-get install openssh-server

Now if you type ssh localhost, you should get the following message (type no):

    The authenticity of host 'localhost (127.0.0.1)' can't be established.
    ECDSA key fingerprint is fc:7e:9a:c7:33:86:47:4b:35:00:22:b7:be:5c:d1:c7.
    Are you sure you want to continue connecting (yes/no)? 

**Step 2:** SSH to the remote server as following. `mars.ece.ucdavis.edu` is the name of the server (you can use the server's IP address as well) and rubinet is the username.

    ssh -X rubinet@mars.ece.ucdavis.edu

Note 1: The -X flag enables X11 forwarding and forwards the program that uses a GUI through your SSH connection. Whenever you launch a program that uses a GUI, it will pop up as if you were physically sitting at that computer (this method is preferrable to using VNC which forwards the entire desktop). 

Note 2: The `X server` should be running on **your** machine. Use the following command to check if X server is running on your machine:

    xset q

If X server is not installed on your machine, then install it:

    sudo apt-get install xorg
    sudo apt-get install openbox

**Step 3:** Follow the same instructions in previous sections to install OMNET++, Veins and VENTOS on the remote server.

**Step 4:** While using OMNET++, the SSH session should be open. Closing the terminal windows or restarting your machine terminates the SSH session and the running simulation will be closed. To keep the SSH session open, you need to use `screen` or `tmux`. We will use `tmux` since it is more powerfull. 

Start tmux by typing `tmux` into the shell. When you do that, it just looks like your screen clears and you’re back at the same terminal prompt. You will notice that the terminal now has a green bar along the bottom. Type `omnetpp` to start the IDE and then type `sumo-cmd &` to run the python script in the background.

**Step 5:** Detach the tmux session by pressing `Ctrl+B` and then typing `D`. You can now safely logoff from the remote machine, your process will keep running inside tmux. When you come back again and want to check the status of your process you can use `tmux attach` to attach to your tmux session.

In order to exchange files between your computer and remote server, you can use filezilla.
