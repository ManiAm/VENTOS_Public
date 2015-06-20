#
# OMNeT++/OMNEST Makefile for librouter
#
# This file was generated with the command:
#  opp_makemake -f --make-so -o router -O out -I/home/mani/Desktop/VENTOS/eigen-3.2.1 -I/home/mani/Desktop/VENTOS/rapidxml-1.13 -I../../../veins/src/veins/base/connectionManager -I../../../veins/src -I../../../veins/src/veins/base/utils -I../traci -I../vehicle -I../../../veins/src/veins -I../global -I.. -I../../../veins/src/veins/base/modules -I../../../veins/src/veins/modules/mac/ieee80211p -I../trafficLight -I../rsu -L../../../veins/out/$$\(CONFIGNAME\)/src -L../../out/$$\(CONFIGNAME\)/application/msg -L../../out/$$\(CONFIGNAME\)/application/pedestrian -L../../out/$$\(CONFIGNAME\)/application/bike -L../../out/$$\(CONFIGNAME\)/application/traci -L../../out/$$\(CONFIGNAME\)/application/adversary -L../../out/$$\(CONFIGNAME\)/application/vehicle -L../../out/$$\(CONFIGNAME\)/application/trafficLight -L../../out/$$\(CONFIGNAME\)/application/rsu -L../../out/$$\(CONFIGNAME\)/application/global -lboost_filesystem -lboost_system -lmsg -ltraci -ltrafficLight -lveins -KVEINS_PROJ=../../../veins
#

# Name of target to be created (-o option)
TARGET = librouter$(SHARED_LIB_SUFFIX)

# C++ include paths (with -I)
INCLUDE_PATH = \
    -I../../eigen-3.2.1 \
    -I../../rapidxml-1.13 \
    -I../../../veins/src/veins/base/connectionManager \
    -I../../../veins/src \
    -I../../../veins/src/veins/base/utils \
    -I../traci \
    -I../vehicle \
    -I../../../veins/src/veins \
    -I../global \
    -I.. \
    -I../../../veins/src/veins/base/modules \
    -I../../../veins/src/veins/modules/mac/ieee80211p \
    -I../trafficLight \
    -I../rsu \
    -I.

# Additional object and library files to link with
EXTRA_OBJS =

# Additional libraries (-L, -l options)
LIBS = -L../../../veins/out/$(CONFIGNAME)/src -L../../out/$(CONFIGNAME)/application/msg -L../../out/$(CONFIGNAME)/application/pedestrian -L../../out/$(CONFIGNAME)/application/bike -L../../out/$(CONFIGNAME)/application/traci -L../../out/$(CONFIGNAME)/application/adversary -L../../out/$(CONFIGNAME)/application/vehicle -L../../out/$(CONFIGNAME)/application/trafficLight -L../../out/$(CONFIGNAME)/application/rsu -L../../out/$(CONFIGNAME)/application/global  -lboost_filesystem -lboost_system -lmsg -ltraci -ltrafficLight -lveins
LIBS += -Wl,-rpath,`abspath ../../../veins/out/$(CONFIGNAME)/src` -Wl,-rpath,`abspath ../../out/$(CONFIGNAME)/application/msg` -Wl,-rpath,`abspath ../../out/$(CONFIGNAME)/application/pedestrian` -Wl,-rpath,`abspath ../../out/$(CONFIGNAME)/application/bike` -Wl,-rpath,`abspath ../../out/$(CONFIGNAME)/application/traci` -Wl,-rpath,`abspath ../../out/$(CONFIGNAME)/application/adversary` -Wl,-rpath,`abspath ../../out/$(CONFIGNAME)/application/vehicle` -Wl,-rpath,`abspath ../../out/$(CONFIGNAME)/application/trafficLight` -Wl,-rpath,`abspath ../../out/$(CONFIGNAME)/application/rsu` -Wl,-rpath,`abspath ../../out/$(CONFIGNAME)/application/global`

# Output directory
PROJECT_OUTPUT_DIR = ../../out
PROJECTRELATIVE_PATH = application/router
O = $(PROJECT_OUTPUT_DIR)/$(CONFIGNAME)/$(PROJECTRELATIVE_PATH)

# Object files for local .cc and .msg files
OBJS = \
    $O/RouterGlobals.o \
    $O/Node.o \
    $O/Net.o \
    $O/Hypertree.o \
    $O/Vehicle.o \
    $O/Edge.o \
    $O/EdgeCosts.o \
    $O/Router.o

# Message files
MSGFILES =

# Other makefile variables (-K)
VEINS_PROJ=../../../veins

#------------------------------------------------------------------------------

# Pull in OMNeT++ configuration (Makefile.inc or configuser.vc)

ifneq ("$(OMNETPP_CONFIGFILE)","")
CONFIGFILE = $(OMNETPP_CONFIGFILE)
else
ifneq ("$(OMNETPP_ROOT)","")
CONFIGFILE = $(OMNETPP_ROOT)/Makefile.inc
else
CONFIGFILE = $(shell opp_configfilepath)
endif
endif

ifeq ("$(wildcard $(CONFIGFILE))","")
$(error Config file '$(CONFIGFILE)' does not exist -- add the OMNeT++ bin directory to the path so that opp_configfilepath can be found, or set the OMNETPP_CONFIGFILE variable to point to Makefile.inc)
endif

include $(CONFIGFILE)

# Simulation kernel and user interface libraries
OMNETPP_LIB_SUBDIR = $(OMNETPP_LIB_DIR)/$(TOOLCHAIN_NAME)
OMNETPP_LIBS = -L"$(OMNETPP_LIB_SUBDIR)" -L"$(OMNETPP_LIB_DIR)" -loppenvir$D $(KERNEL_LIBS) $(SYS_LIBS)

COPTS = $(CFLAGS)  $(INCLUDE_PATH) -I$(OMNETPP_INCL_DIR)
MSGCOPTS = $(INCLUDE_PATH)

# we want to recompile everything if COPTS changes,
# so we store COPTS into $COPTS_FILE and have object
# files depend on it (except when "make depend" was called)
COPTS_FILE = $O/.last-copts
ifneq ($(MAKECMDGOALS),depend)
ifneq ("$(COPTS)","$(shell cat $(COPTS_FILE) 2>/dev/null || echo '')")
$(shell $(MKPATH) "$O" && echo "$(COPTS)" >$(COPTS_FILE))
endif
endif

#------------------------------------------------------------------------------
# User-supplied makefile fragment(s)
# >>>
# <<<
#------------------------------------------------------------------------------

# Main target
all: $O/$(TARGET)
	$(Q)$(LN) $O/$(TARGET) .

$O/$(TARGET): $(OBJS)  $(wildcard $(EXTRA_OBJS)) Makefile
	@$(MKPATH) $O
	@echo Creating shared library: $@
	$(Q)$(SHLIB_LD) -o $O/$(TARGET)  $(OBJS) $(EXTRA_OBJS) $(AS_NEEDED_OFF) $(WHOLE_ARCHIVE_ON) $(LIBS) $(WHOLE_ARCHIVE_OFF) $(OMNETPP_LIBS) $(LDFLAGS)
	$(Q)$(SHLIB_POSTPROCESS) $O/$(TARGET)

.PHONY: all clean cleanall depend msgheaders

.SUFFIXES: .cc

$O/%.o: %.cc $(COPTS_FILE)
	@$(MKPATH) $(dir $@)
	$(qecho) "$<"
	$(Q)$(CXX) -c $(CXXFLAGS) $(COPTS) -o $@ $<

%_m.cc %_m.h: %.msg
	$(qecho) MSGC: $<
	$(Q)$(MSGC) -s _m.cc $(MSGCOPTS) $?

msgheaders: $(MSGFILES:.msg=_m.h)

clean:
	$(qecho) Cleaning...
	$(Q)-rm -rf $O
	$(Q)-rm -f router router.exe librouter.so librouter.a librouter.dll librouter.dylib
	$(Q)-rm -f ./*_m.cc ./*_m.h

cleanall: clean
	$(Q)-rm -rf $(PROJECT_OUTPUT_DIR)

depend:
	$(qecho) Creating dependencies...
	$(Q)$(MAKEDEPEND) $(INCLUDE_PATH) -f Makefile -P\$$O/ -- $(MSG_CC_FILES)  ./*.cc

# DO NOT DELETE THIS LINE -- make depend depends on it.
$O/Edge.o: Edge.cc \
	Edge.h \
	EdgeCosts.h \
	Node.h \
	RouterGlobals.h
$O/EdgeCosts.o: EdgeCosts.cc \
	EdgeCosts.h \
	RouterGlobals.h
$O/Hypertree.o: Hypertree.cc \
	Hypertree.h
$O/Net.o: Net.cc \
	../global/Appl.h \
	../msg/Messages_m.h \
	Edge.h \
	EdgeCosts.h \
	Hypertree.h \
	Net.h \
	Node.h \
	Router.h \
	RouterGlobals.h \
	Vehicle.h \
	../rsu/ApplRSU_01_Base.h \
	../rsu/ApplRSU_02_AID.h \
	../rsu/ApplRSU_03_TL_VANET.h \
	../traci/TraCI_Extend.h \
	../trafficLight/01_TL_Base.h \
	../trafficLight/02_LoopDetectors.h \
	../trafficLight/03_TL_Fixed.h \
	../trafficLight/04_TL_Adaptive_RingAndBarrier.h \
	../trafficLight/05_TL_Adaptive_Queue.h \
	../trafficLight/06_TL_Adaptive_Webster.h \
	../trafficLight/07_TL_VANET.h \
	../trafficLight/08_TL_Router.h \
	../vehicle/ApplV_01_Base.h \
	../vehicle/ApplV_02_Beacon.h \
	$(VEINS_PROJ)/src/veins/base/connectionManager/BaseConnectionManager.h \
	$(VEINS_PROJ)/src/veins/base/connectionManager/ChannelAccess.h \
	$(VEINS_PROJ)/src/veins/base/connectionManager/NicEntry.h \
	$(VEINS_PROJ)/src/veins/base/modules/BaseApplLayer.h \
	$(VEINS_PROJ)/src/veins/base/modules/BaseBattery.h \
	$(VEINS_PROJ)/src/veins/base/modules/BaseLayer.h \
	$(VEINS_PROJ)/src/veins/base/modules/BaseMobility.h \
	$(VEINS_PROJ)/src/veins/base/modules/BaseModule.h \
	$(VEINS_PROJ)/src/veins/base/modules/BaseWorldUtility.h \
	$(VEINS_PROJ)/src/veins/base/modules/BatteryAccess.h \
	$(VEINS_PROJ)/src/veins/base/utils/Coord.h \
	$(VEINS_PROJ)/src/veins/base/utils/FWMath.h \
	$(VEINS_PROJ)/src/veins/base/utils/FindModule.h \
	$(VEINS_PROJ)/src/veins/base/utils/HostState.h \
	$(VEINS_PROJ)/src/veins/base/utils/MiXiMDefs.h \
	$(VEINS_PROJ)/src/veins/base/utils/Move.h \
	$(VEINS_PROJ)/src/veins/base/utils/NetwToMacControlInfo.h \
	$(VEINS_PROJ)/src/veins/base/utils/PassedMessage.h \
	$(VEINS_PROJ)/src/veins/base/utils/SimpleAddress.h \
	$(VEINS_PROJ)/src/veins/base/utils/miximkerneldefs.h \
	$(VEINS_PROJ)/src/veins/modules/mac/ieee80211p/WaveAppToMac1609_4Interface.h \
	$(VEINS_PROJ)/src/veins/modules/messages/WaveShortMessage_m.h \
	$(VEINS_PROJ)/src/veins/modules/mobility/traci/TraCIBuffer.h \
	$(VEINS_PROJ)/src/veins/modules/mobility/traci/TraCIColor.h \
	$(VEINS_PROJ)/src/veins/modules/mobility/traci/TraCICommandInterface.h \
	$(VEINS_PROJ)/src/veins/modules/mobility/traci/TraCIConnection.h \
	$(VEINS_PROJ)/src/veins/modules/mobility/traci/TraCIConstants.h \
	$(VEINS_PROJ)/src/veins/modules/mobility/traci/TraCICoord.h \
	$(VEINS_PROJ)/src/veins/modules/mobility/traci/TraCIMobility.h \
	$(VEINS_PROJ)/src/veins/modules/mobility/traci/TraCIScenarioManager.h \
	$(VEINS_PROJ)/src/veins/modules/obstacle/Obstacle.h \
	$(VEINS_PROJ)/src/veins/modules/obstacle/ObstacleControl.h \
	$(VEINS_PROJ)/src/veins/modules/utility/Consts80211p.h \
	$(VEINS_PROJ)/src/veins/modules/world/annotations/AnnotationManager.h
$O/Node.o: Node.cc \
	Node.h
$O/Router.o: Router.cc \
	../global/Appl.h \
	../msg/Messages_m.h \
	Edge.h \
	EdgeCosts.h \
	Hypertree.h \
	Net.h \
	Node.h \
	Router.h \
	RouterGlobals.h \
	Vehicle.h \
	../rsu/ApplRSU_01_Base.h \
	../rsu/ApplRSU_02_AID.h \
	../rsu/ApplRSU_03_TL_VANET.h \
	../traci/TraCI_Extend.h \
	../trafficLight/01_TL_Base.h \
	../trafficLight/02_LoopDetectors.h \
	../trafficLight/03_TL_Fixed.h \
	../trafficLight/04_TL_Adaptive_RingAndBarrier.h \
	../trafficLight/05_TL_Adaptive_Queue.h \
	../trafficLight/06_TL_Adaptive_Webster.h \
	../trafficLight/07_TL_VANET.h \
	../trafficLight/08_TL_Router.h \
	../vehicle/ApplV_01_Base.h \
	../vehicle/ApplV_02_Beacon.h \
	$(VEINS_PROJ)/src/veins/base/connectionManager/BaseConnectionManager.h \
	$(VEINS_PROJ)/src/veins/base/connectionManager/ChannelAccess.h \
	$(VEINS_PROJ)/src/veins/base/connectionManager/NicEntry.h \
	$(VEINS_PROJ)/src/veins/base/modules/BaseApplLayer.h \
	$(VEINS_PROJ)/src/veins/base/modules/BaseBattery.h \
	$(VEINS_PROJ)/src/veins/base/modules/BaseLayer.h \
	$(VEINS_PROJ)/src/veins/base/modules/BaseMobility.h \
	$(VEINS_PROJ)/src/veins/base/modules/BaseModule.h \
	$(VEINS_PROJ)/src/veins/base/modules/BaseWorldUtility.h \
	$(VEINS_PROJ)/src/veins/base/modules/BatteryAccess.h \
	$(VEINS_PROJ)/src/veins/base/utils/Coord.h \
	$(VEINS_PROJ)/src/veins/base/utils/FWMath.h \
	$(VEINS_PROJ)/src/veins/base/utils/FindModule.h \
	$(VEINS_PROJ)/src/veins/base/utils/HostState.h \
	$(VEINS_PROJ)/src/veins/base/utils/MiXiMDefs.h \
	$(VEINS_PROJ)/src/veins/base/utils/Move.h \
	$(VEINS_PROJ)/src/veins/base/utils/NetwToMacControlInfo.h \
	$(VEINS_PROJ)/src/veins/base/utils/PassedMessage.h \
	$(VEINS_PROJ)/src/veins/base/utils/SimpleAddress.h \
	$(VEINS_PROJ)/src/veins/base/utils/miximkerneldefs.h \
	$(VEINS_PROJ)/src/veins/modules/mac/ieee80211p/WaveAppToMac1609_4Interface.h \
	$(VEINS_PROJ)/src/veins/modules/messages/WaveShortMessage_m.h \
	$(VEINS_PROJ)/src/veins/modules/mobility/traci/TraCIBuffer.h \
	$(VEINS_PROJ)/src/veins/modules/mobility/traci/TraCIColor.h \
	$(VEINS_PROJ)/src/veins/modules/mobility/traci/TraCICommandInterface.h \
	$(VEINS_PROJ)/src/veins/modules/mobility/traci/TraCIConnection.h \
	$(VEINS_PROJ)/src/veins/modules/mobility/traci/TraCIConstants.h \
	$(VEINS_PROJ)/src/veins/modules/mobility/traci/TraCICoord.h \
	$(VEINS_PROJ)/src/veins/modules/mobility/traci/TraCIMobility.h \
	$(VEINS_PROJ)/src/veins/modules/mobility/traci/TraCIScenarioManager.h \
	$(VEINS_PROJ)/src/veins/modules/obstacle/Obstacle.h \
	$(VEINS_PROJ)/src/veins/modules/obstacle/ObstacleControl.h \
	$(VEINS_PROJ)/src/veins/modules/utility/Consts80211p.h \
	$(VEINS_PROJ)/src/veins/modules/world/annotations/AnnotationManager.h
$O/RouterGlobals.o: RouterGlobals.cc \
	RouterGlobals.h
$O/Vehicle.o: Vehicle.cc \
	Vehicle.h

