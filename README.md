# VENTOS #

VENTOS is an open-source integrated VANET C++ simulator for studying vehicular traffic flows, collaborative driving, and interactions between vehicles and infrastructure through DSRC-enabled wireless communication capability. VENTOS is useful for researchers in transportation engineering, control theory and vehicular networking fields. VENTOS is being developed in Rubinet Lab, UC Davis since 2013 and is the main tool in the [C3PO](https://www.ece.ucdavis.edu/~chuah/rubinet/projects/c3po.html) project. VENTOS is being used over the years by different researchers to test their applications and protocols. 

+ develope platoon management protocol [(paper)](http://www.sciencedirect.com/science/article/pii/S2214209615000145).
+ study the security vulnerabilities of connected vehicles [(paper)](http://ieeexplore.ieee.org/abstract/document/7120028/).
+ designing new actuated traffic signal controls [(paper)](soon).
+ develope dynamic vehicular traffic routing [(paper)](https://trid.trb.org/view.aspx?id=1393674).

## Mailing List ##

You can subscribe to the VENTOS mailing list to stay informed of new releases and to get the latest news about VENTOS. Send an email to [maniam](mailto:maniam@ucdavis.edu) with subject 'VENTOS mailing list' in order to be addedd into the VENTOS mailing list.

## Getting started ##

See the official [website](http://maniam.github.io/VENTOS/) for documentation, installation instructions and tutorial.

## Issues ##

You can send your questions to [Mani Amoozadeh](mailto:maniam@ucdavis.edu). Besides the official documentation and the examples, you can post your VENTOS-related questions on stackoverflow website with the 'VENTOS' tag. You can also open an issue in the VENTOS issue tracker [here](https://github.com/ManiAm/VENTOS_Public/issues).

## Changelog ##

### Git ###

**bug fixes**
+ VENTOS can now be installed from the downloaded zip file
+ vehicleGetColor is now working correctly
+ fix various memory leakes
+ measure sim start/end/duration more accuratly
+ fix simulation end time recording
+ fix getRSUposition when calling from initialize
+ clean up is now done in destructor not finish!

**enhancements**
+ using a shorter relative path for sumoConfig
+ gnuplot has its own class now
+ vehicle equilibrium is now working for emulated cars and preserve color
+ enable parallel build
+ can now set the vglog window title with loggingWindowTitle
+ adding glogActive parameters to turn glog window off/on
+ recording PHY frames that are sent but not received in statistics
+ TraCI logging is now working more efficiently
+ adding vehicleExist TraCI command
+ adding vehicleSlowDown TraCI command
+ adding vehicleGetWaitingTime TraCI command
+ adding new commands to get XML elements of addNode.xml
+ can now run omnet++ in cmd mode without forc-running sumo in cmd mode
+ removing quit-on-end to facilitate debugging
+ runme script checks eigen installation more accuratly
+ get rid of depricated method 'getHostByName'
+ updating the manual
+ various small improvements

### VENTOS 1.0 (July 4, 2017) ###

+ improving the user interface
+ adding traffic signal controls
+ introducing the VENTOS manual

### VENTOS 0.5 (Oct 21, 2015) ###

+ first public release
+ first implementation of our platoon management protocol
+ first implementation of our ACC/CACC car-following model
+ adversary module to simulate security attacks

