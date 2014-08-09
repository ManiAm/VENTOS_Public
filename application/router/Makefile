#
# OMNeT++/OMNEST Makefile for librouter
#
# This file was generated with the command:
#  opp_makemake -f --make-so -o router -O out -I/home/mani/Desktop/VENTOS/eigen-3.2.1 -I/home/mani/Desktop/VENTOS/rapidxml-1.13 -I../../../Veins/src/modules/utility -I../../../Veins/src/modules/phy -I../../../Veins/src/base/modules -I../../../Veins/src/modules/analogueModel -I../../../Veins/src/modules/mac/ieee80211p -I../core -I../../../Veins/src/base/messages -I../../../Veins/src/modules/messages -I../../../Veins/src/base/connectionManager -I../../../Veins/src/inet -I.. -I../../../Veins/src -I../../../Veins/src/base/utils -I../../../Veins/src/base/phyLayer -L../../../Veins/out/$$\(CONFIGNAME\)/src -L../../out/$$\(CONFIGNAME\)/application -L../../out/$$\(CONFIGNAME\)/application/msg -L../../out/$$\(CONFIGNAME\)/application/core -lboost_filesystem -lboost_system -lmsg -lVeins -KVEINS_PROJ=../../../Veins
#

# Name of target to be created (-o option)
TARGET = librouter$(SHARED_LIB_SUFFIX)

# C++ include paths (with -I)
INCLUDE_PATH = \
    -I../../eigen-3.2.1 \
    -I../../rapidxml-1.13 \
    -I../../../Veins/src/modules/utility \
    -I../../../Veins/src/modules/phy \
    -I../../../Veins/src/base/modules \
    -I../../../Veins/src/modules/analogueModel \
    -I../../../Veins/src/modules/mac/ieee80211p \
    -I../core \
    -I../../../Veins/src/base/messages \
    -I../../../Veins/src/modules/messages \
    -I../../../Veins/src/base/connectionManager \
    -I../../../Veins/src/inet \
    -I.. \
    -I../../../Veins/src \
    -I../../../Veins/src/base/utils \
    -I../../../Veins/src/base/phyLayer \
    -I.

# Additional object and library files to link with
EXTRA_OBJS =

# Additional libraries (-L, -l options)
LIBS = -L../../../Veins/out/$(CONFIGNAME)/src -L../../out/$(CONFIGNAME)/application -L../../out/$(CONFIGNAME)/application/msg -L../../out/$(CONFIGNAME)/application/core  -lboost_filesystem -lboost_system -lmsg -lVeins
LIBS += -Wl,-rpath,`abspath ../../../Veins/out/$(CONFIGNAME)/src` -Wl,-rpath,`abspath ../../out/$(CONFIGNAME)/application` -Wl,-rpath,`abspath ../../out/$(CONFIGNAME)/application/msg` -Wl,-rpath,`abspath ../../out/$(CONFIGNAME)/application/core`

# Output directory
PROJECT_OUTPUT_DIR = ../../out
PROJECTRELATIVE_PATH = application/router
O = $(PROJECT_OUTPUT_DIR)/$(CONFIGNAME)/$(PROJECTRELATIVE_PATH)

# Object files for local .cc and .msg files
OBJS = \
    $O/ApplVSystem.o \
    $O/TrafficLight.o \
    $O/Node.o \
    $O/Histogram.o \
    $O/Net.o \
    $O/Hypertree.o \
    $O/Edge.o \
    $O/Router.o

# Message files
MSGFILES =

# Other makefile variables (-K)
VEINS_PROJ=../../../Veins

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
$O/ApplVSystem.o: ApplVSystem.cc \
	../core/Appl.h \
	../core/ApplV_01_Base.h \
	../core/ApplV_02_Beacon.h \
	../core/Global_01_TraCI_Extend.h \
	../msg/Messages_m.h \
	ApplVSystem.h \
	Histogram.h \
	Hypertree.h \
	$(VEINS_PROJ)/src/base/connectionManager/BaseConnectionManager.h \
	$(VEINS_PROJ)/src/base/connectionManager/ChannelAccess.h \
	$(VEINS_PROJ)/src/base/connectionManager/NicEntry.h \
	$(VEINS_PROJ)/src/base/modules/BaseApplLayer.h \
	$(VEINS_PROJ)/src/base/modules/BaseBattery.h \
	$(VEINS_PROJ)/src/base/modules/BaseLayer.h \
	$(VEINS_PROJ)/src/base/modules/BaseMobility.h \
	$(VEINS_PROJ)/src/base/modules/BaseModule.h \
	$(VEINS_PROJ)/src/base/modules/BaseWorldUtility.h \
	$(VEINS_PROJ)/src/base/modules/BatteryAccess.h \
	$(VEINS_PROJ)/src/base/utils/FindModule.h \
	$(VEINS_PROJ)/src/base/utils/HostState.h \
	$(VEINS_PROJ)/src/base/utils/MiXiMDefs.h \
	$(VEINS_PROJ)/src/base/utils/Move.h \
	$(VEINS_PROJ)/src/base/utils/NetwToMacControlInfo.h \
	$(VEINS_PROJ)/src/base/utils/PassedMessage.h \
	$(VEINS_PROJ)/src/base/utils/SimpleAddress.h \
	$(VEINS_PROJ)/src/base/utils/miximkerneldefs.h \
	$(VEINS_PROJ)/src/inet/Coord.h \
	$(VEINS_PROJ)/src/inet/FWMath.h \
	$(VEINS_PROJ)/src/modules/mac/ieee80211p/WaveAppToMac1609_4Interface.h \
	$(VEINS_PROJ)/src/modules/messages/WaveShortMessage_m.h \
	$(VEINS_PROJ)/src/modules/mobility/traci/TraCIBuffer.h \
	$(VEINS_PROJ)/src/modules/mobility/traci/TraCIColor.h \
	$(VEINS_PROJ)/src/modules/mobility/traci/TraCICommandInterface.h \
	$(VEINS_PROJ)/src/modules/mobility/traci/TraCIConnection.h \
	$(VEINS_PROJ)/src/modules/mobility/traci/TraCIConstants.h \
	$(VEINS_PROJ)/src/modules/mobility/traci/TraCICoord.h \
	$(VEINS_PROJ)/src/modules/mobility/traci/TraCIMobility.h \
	$(VEINS_PROJ)/src/modules/mobility/traci/TraCIScenarioManager.h \
	$(VEINS_PROJ)/src/modules/obstacle/Obstacle.h \
	$(VEINS_PROJ)/src/modules/obstacle/ObstacleControl.h \
	$(VEINS_PROJ)/src/modules/utility/Consts80211p.h \
	$(VEINS_PROJ)/src/modules/world/annotations/AnnotationManager.h
$O/Edge.o: Edge.cc \
	Edge.h \
	Histogram.h \
	Node.h
$O/Histogram.o: Histogram.cc \
	Histogram.h
$O/Hypertree.o: Hypertree.cc \
	Hypertree.h
$O/Net.o: Net.cc \
	../core/Appl.h \
	../core/ApplV_01_Base.h \
	../core/ApplV_02_Beacon.h \
	../core/Global_01_TraCI_Extend.h \
	../msg/Messages_m.h \
	Edge.h \
	Histogram.h \
	Hypertree.h \
	Net.h \
	Node.h \
	PathingData.h \
	Router.h \
	TrafficLight.h \
	$(VEINS_PROJ)/src/base/connectionManager/BaseConnectionManager.h \
	$(VEINS_PROJ)/src/base/connectionManager/ChannelAccess.h \
	$(VEINS_PROJ)/src/base/connectionManager/NicEntry.h \
	$(VEINS_PROJ)/src/base/modules/BaseApplLayer.h \
	$(VEINS_PROJ)/src/base/modules/BaseBattery.h \
	$(VEINS_PROJ)/src/base/modules/BaseLayer.h \
	$(VEINS_PROJ)/src/base/modules/BaseMobility.h \
	$(VEINS_PROJ)/src/base/modules/BaseModule.h \
	$(VEINS_PROJ)/src/base/modules/BaseWorldUtility.h \
	$(VEINS_PROJ)/src/base/modules/BatteryAccess.h \
	$(VEINS_PROJ)/src/base/utils/FindModule.h \
	$(VEINS_PROJ)/src/base/utils/HostState.h \
	$(VEINS_PROJ)/src/base/utils/MiXiMDefs.h \
	$(VEINS_PROJ)/src/base/utils/Move.h \
	$(VEINS_PROJ)/src/base/utils/NetwToMacControlInfo.h \
	$(VEINS_PROJ)/src/base/utils/PassedMessage.h \
	$(VEINS_PROJ)/src/base/utils/SimpleAddress.h \
	$(VEINS_PROJ)/src/base/utils/miximkerneldefs.h \
	$(VEINS_PROJ)/src/inet/Coord.h \
	$(VEINS_PROJ)/src/inet/FWMath.h \
	$(VEINS_PROJ)/src/modules/mac/ieee80211p/WaveAppToMac1609_4Interface.h \
	$(VEINS_PROJ)/src/modules/messages/WaveShortMessage_m.h \
	$(VEINS_PROJ)/src/modules/mobility/traci/TraCIBuffer.h \
	$(VEINS_PROJ)/src/modules/mobility/traci/TraCIColor.h \
	$(VEINS_PROJ)/src/modules/mobility/traci/TraCICommandInterface.h \
	$(VEINS_PROJ)/src/modules/mobility/traci/TraCIConnection.h \
	$(VEINS_PROJ)/src/modules/mobility/traci/TraCIConstants.h \
	$(VEINS_PROJ)/src/modules/mobility/traci/TraCICoord.h \
	$(VEINS_PROJ)/src/modules/mobility/traci/TraCIMobility.h \
	$(VEINS_PROJ)/src/modules/mobility/traci/TraCIScenarioManager.h \
	$(VEINS_PROJ)/src/modules/obstacle/Obstacle.h \
	$(VEINS_PROJ)/src/modules/obstacle/ObstacleControl.h \
	$(VEINS_PROJ)/src/modules/utility/Consts80211p.h \
	$(VEINS_PROJ)/src/modules/world/annotations/AnnotationManager.h
$O/Node.o: Node.cc \
	Node.h
$O/Router.o: Router.cc \
	../core/Appl.h \
	../core/ApplV_01_Base.h \
	../core/ApplV_02_Beacon.h \
	../core/Global_01_TraCI_Extend.h \
	../msg/Messages_m.h \
	Edge.h \
	Histogram.h \
	Hypertree.h \
	Net.h \
	Node.h \
	PathingData.h \
	Router.h \
	TrafficLight.h \
	$(VEINS_PROJ)/src/base/connectionManager/BaseConnectionManager.h \
	$(VEINS_PROJ)/src/base/connectionManager/ChannelAccess.h \
	$(VEINS_PROJ)/src/base/connectionManager/NicEntry.h \
	$(VEINS_PROJ)/src/base/modules/BaseApplLayer.h \
	$(VEINS_PROJ)/src/base/modules/BaseBattery.h \
	$(VEINS_PROJ)/src/base/modules/BaseLayer.h \
	$(VEINS_PROJ)/src/base/modules/BaseMobility.h \
	$(VEINS_PROJ)/src/base/modules/BaseModule.h \
	$(VEINS_PROJ)/src/base/modules/BaseWorldUtility.h \
	$(VEINS_PROJ)/src/base/modules/BatteryAccess.h \
	$(VEINS_PROJ)/src/base/utils/FindModule.h \
	$(VEINS_PROJ)/src/base/utils/HostState.h \
	$(VEINS_PROJ)/src/base/utils/MiXiMDefs.h \
	$(VEINS_PROJ)/src/base/utils/Move.h \
	$(VEINS_PROJ)/src/base/utils/NetwToMacControlInfo.h \
	$(VEINS_PROJ)/src/base/utils/PassedMessage.h \
	$(VEINS_PROJ)/src/base/utils/SimpleAddress.h \
	$(VEINS_PROJ)/src/base/utils/miximkerneldefs.h \
	$(VEINS_PROJ)/src/inet/Coord.h \
	$(VEINS_PROJ)/src/inet/FWMath.h \
	$(VEINS_PROJ)/src/modules/mac/ieee80211p/WaveAppToMac1609_4Interface.h \
	$(VEINS_PROJ)/src/modules/messages/WaveShortMessage_m.h \
	$(VEINS_PROJ)/src/modules/mobility/traci/TraCIBuffer.h \
	$(VEINS_PROJ)/src/modules/mobility/traci/TraCIColor.h \
	$(VEINS_PROJ)/src/modules/mobility/traci/TraCICommandInterface.h \
	$(VEINS_PROJ)/src/modules/mobility/traci/TraCIConnection.h \
	$(VEINS_PROJ)/src/modules/mobility/traci/TraCIConstants.h \
	$(VEINS_PROJ)/src/modules/mobility/traci/TraCICoord.h \
	$(VEINS_PROJ)/src/modules/mobility/traci/TraCIMobility.h \
	$(VEINS_PROJ)/src/modules/mobility/traci/TraCIScenarioManager.h \
	$(VEINS_PROJ)/src/modules/obstacle/Obstacle.h \
	$(VEINS_PROJ)/src/modules/obstacle/ObstacleControl.h \
	$(VEINS_PROJ)/src/modules/utility/Consts80211p.h \
	$(VEINS_PROJ)/src/modules/world/annotations/AnnotationManager.h
$O/TrafficLight.o: TrafficLight.cc \
	TrafficLight.h

