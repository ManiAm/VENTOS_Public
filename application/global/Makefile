#
# OMNeT++/OMNEST Makefile for libglobal
#
# This file was generated with the command:
#  opp_makemake -f --make-so -o global -O out -I/home/mani/Desktop/VENTOS/eigen-3.2.1 -I/home/mani/Desktop/VENTOS/rapidxml-1.13 -I.. -I../../../veins/src/veins/base/connectionManager -I../router -I../../../veins/src -I../../../veins/src/veins/base/utils -I../traci -I../../../veins/src/veins/base/modules -I../../../veins/src/veins/modules/mac/ieee80211p -I../../../veins/src/veins -I../vehicle -I../rsu -L../../../veins/out/$$\(CONFIGNAME\)/src -L../../out/$$\(CONFIGNAME\)/application/msg -L../../out/$$\(CONFIGNAME\)/application/router -L../../out/$$\(CONFIGNAME\)/application/pedestrian -L../../out/$$\(CONFIGNAME\)/application/bike -L../../out/$$\(CONFIGNAME\)/application/traci -L../../out/$$\(CONFIGNAME\)/application/adversary -L../../out/$$\(CONFIGNAME\)/application/vehicle -L../../out/$$\(CONFIGNAME\)/application/trafficLight -L../../out/$$\(CONFIGNAME\)/application/rsu -lboost_filesystem -lboost_system -lmsg -ltraci -lveins -KVEINS_PROJ=../../../veins
#

# Name of target to be created (-o option)
TARGET = libglobal$(SHARED_LIB_SUFFIX)

# C++ include paths (with -I)
INCLUDE_PATH = \
    -I../../eigen-3.2.1 \
    -I../../rapidxml-1.13 \
    -I.. \
    -I../../../veins/src/veins/base/connectionManager \
    -I../router \
    -I../../../veins/src \
    -I../../../veins/src/veins/base/utils \
    -I../traci \
    -I../../../veins/src/veins/base/modules \
    -I../../../veins/src/veins/modules/mac/ieee80211p \
    -I../../../veins/src/veins \
    -I../vehicle \
    -I../rsu \
    -I.

# Additional object and library files to link with
EXTRA_OBJS =

# Additional libraries (-L, -l options)
LIBS = -L../../../veins/out/$(CONFIGNAME)/src -L../../out/$(CONFIGNAME)/application/msg -L../../out/$(CONFIGNAME)/application/router -L../../out/$(CONFIGNAME)/application/pedestrian -L../../out/$(CONFIGNAME)/application/bike -L../../out/$(CONFIGNAME)/application/traci -L../../out/$(CONFIGNAME)/application/adversary -L../../out/$(CONFIGNAME)/application/vehicle -L../../out/$(CONFIGNAME)/application/trafficLight -L../../out/$(CONFIGNAME)/application/rsu  -lboost_filesystem -lboost_system -lmsg -ltraci -lveins
LIBS += -Wl,-rpath,`abspath ../../../veins/out/$(CONFIGNAME)/src` -Wl,-rpath,`abspath ../../out/$(CONFIGNAME)/application/msg` -Wl,-rpath,`abspath ../../out/$(CONFIGNAME)/application/router` -Wl,-rpath,`abspath ../../out/$(CONFIGNAME)/application/pedestrian` -Wl,-rpath,`abspath ../../out/$(CONFIGNAME)/application/bike` -Wl,-rpath,`abspath ../../out/$(CONFIGNAME)/application/traci` -Wl,-rpath,`abspath ../../out/$(CONFIGNAME)/application/adversary` -Wl,-rpath,`abspath ../../out/$(CONFIGNAME)/application/vehicle` -Wl,-rpath,`abspath ../../out/$(CONFIGNAME)/application/trafficLight` -Wl,-rpath,`abspath ../../out/$(CONFIGNAME)/application/rsu`

# Output directory
PROJECT_OUTPUT_DIR = ../../out
PROJECTRELATIVE_PATH = application/global
O = $(PROJECT_OUTPUT_DIR)/$(CONFIGNAME)/$(PROJECTRELATIVE_PATH)

# Object files for local .cc and .msg files
OBJS = $O/Statistics.o $O/Mac1609_4_Mod.o $O/Tracking.o $O/Plotter.o $O/ConstSpeedMobility.o

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
	$(Q)-rm -f global global.exe libglobal.so libglobal.a libglobal.dll libglobal.dylib
	$(Q)-rm -f ./*_m.cc ./*_m.h

cleanall: clean
	$(Q)-rm -rf $(PROJECT_OUTPUT_DIR)

depend:
	$(qecho) Creating dependencies...
	$(Q)$(MAKEDEPEND) $(INCLUDE_PATH) -f Makefile -P\$$O/ -- $(MSG_CC_FILES)  ./*.cc

# DO NOT DELETE THIS LINE -- make depend depends on it.
$O/ConstSpeedMobility.o: ConstSpeedMobility.cc \
	ConstSpeedMobility.h \
	$(VEINS_PROJ)/src/veins/base/modules/BaseBattery.h \
	$(VEINS_PROJ)/src/veins/base/modules/BaseMobility.h \
	$(VEINS_PROJ)/src/veins/base/modules/BaseModule.h \
	$(VEINS_PROJ)/src/veins/base/modules/BaseWorldUtility.h \
	$(VEINS_PROJ)/src/veins/base/modules/BatteryAccess.h \
	$(VEINS_PROJ)/src/veins/base/utils/Coord.h \
	$(VEINS_PROJ)/src/veins/base/utils/FWMath.h \
	$(VEINS_PROJ)/src/veins/base/utils/HostState.h \
	$(VEINS_PROJ)/src/veins/base/utils/MiXiMDefs.h \
	$(VEINS_PROJ)/src/veins/base/utils/Move.h \
	$(VEINS_PROJ)/src/veins/base/utils/miximkerneldefs.h
$O/Mac1609_4_Mod.o: Mac1609_4_Mod.cc \
	Appl.h \
	Mac1609_4_Mod.h \
	Statistics.h \
	../msg/Messages_m.h \
	../traci/TraCI_Extend.h \
	$(VEINS_PROJ)/src/veins/base/connectionManager/BaseConnectionManager.h \
	$(VEINS_PROJ)/src/veins/base/connectionManager/ChannelAccess.h \
	$(VEINS_PROJ)/src/veins/base/connectionManager/NicEntry.h \
	$(VEINS_PROJ)/src/veins/base/messages/AirFrame_m.h \
	$(VEINS_PROJ)/src/veins/base/messages/ChannelSenseRequest_m.h \
	$(VEINS_PROJ)/src/veins/base/messages/MacPkt_m.h \
	$(VEINS_PROJ)/src/veins/base/modules/BaseApplLayer.h \
	$(VEINS_PROJ)/src/veins/base/modules/BaseBattery.h \
	$(VEINS_PROJ)/src/veins/base/modules/BaseLayer.h \
	$(VEINS_PROJ)/src/veins/base/modules/BaseMacLayer.h \
	$(VEINS_PROJ)/src/veins/base/modules/BaseMobility.h \
	$(VEINS_PROJ)/src/veins/base/modules/BaseModule.h \
	$(VEINS_PROJ)/src/veins/base/modules/BaseWorldUtility.h \
	$(VEINS_PROJ)/src/veins/base/modules/BatteryAccess.h \
	$(VEINS_PROJ)/src/veins/base/phyLayer/AnalogueModel.h \
	$(VEINS_PROJ)/src/veins/base/phyLayer/BaseDecider.h \
	$(VEINS_PROJ)/src/veins/base/phyLayer/BasePhyLayer.h \
	$(VEINS_PROJ)/src/veins/base/phyLayer/ChannelInfo.h \
	$(VEINS_PROJ)/src/veins/base/phyLayer/ChannelState.h \
	$(VEINS_PROJ)/src/veins/base/phyLayer/Decider.h \
	$(VEINS_PROJ)/src/veins/base/phyLayer/DeciderToPhyInterface.h \
	$(VEINS_PROJ)/src/veins/base/phyLayer/Interpolation.h \
	$(VEINS_PROJ)/src/veins/base/phyLayer/MacToPhyControlInfo.h \
	$(VEINS_PROJ)/src/veins/base/phyLayer/MacToPhyInterface.h \
	$(VEINS_PROJ)/src/veins/base/phyLayer/Mapping.h \
	$(VEINS_PROJ)/src/veins/base/phyLayer/MappingBase.h \
	$(VEINS_PROJ)/src/veins/base/phyLayer/MappingUtils.h \
	$(VEINS_PROJ)/src/veins/base/phyLayer/PhyUtils.h \
	$(VEINS_PROJ)/src/veins/base/phyLayer/Signal_.h \
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
	$(VEINS_PROJ)/src/veins/modules/analogueModel/JakesFading.h \
	$(VEINS_PROJ)/src/veins/modules/analogueModel/LogNormalShadowing.h \
	$(VEINS_PROJ)/src/veins/modules/analogueModel/SimplePathlossModel.h \
	$(VEINS_PROJ)/src/veins/modules/mac/ieee80211p/Mac1609_4.h \
	$(VEINS_PROJ)/src/veins/modules/mac/ieee80211p/Mac80211pToPhy11pInterface.h \
	$(VEINS_PROJ)/src/veins/modules/mac/ieee80211p/WaveAppToMac1609_4Interface.h \
	$(VEINS_PROJ)/src/veins/modules/messages/Mac80211Pkt_m.h \
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
	$(VEINS_PROJ)/src/veins/modules/phy/Decider80211p.h \
	$(VEINS_PROJ)/src/veins/modules/phy/Decider80211pToPhy80211pInterface.h \
	$(VEINS_PROJ)/src/veins/modules/phy/PhyLayer80211p.h \
	$(VEINS_PROJ)/src/veins/modules/phy/SNRThresholdDecider.h \
	$(VEINS_PROJ)/src/veins/modules/utility/Consts80211p.h \
	$(VEINS_PROJ)/src/veins/modules/utility/ConstsPhy.h \
	$(VEINS_PROJ)/src/veins/modules/world/annotations/AnnotationManager.h
$O/Plotter.o: Plotter.cc \
	Appl.h \
	Plotter.h \
	../msg/Messages_m.h \
	../traci/TraCI_Extend.h \
	$(VEINS_PROJ)/src/veins/base/connectionManager/BaseConnectionManager.h \
	$(VEINS_PROJ)/src/veins/base/connectionManager/NicEntry.h \
	$(VEINS_PROJ)/src/veins/base/modules/BaseModule.h \
	$(VEINS_PROJ)/src/veins/base/modules/BaseWorldUtility.h \
	$(VEINS_PROJ)/src/veins/base/utils/Coord.h \
	$(VEINS_PROJ)/src/veins/base/utils/FWMath.h \
	$(VEINS_PROJ)/src/veins/base/utils/FindModule.h \
	$(VEINS_PROJ)/src/veins/base/utils/HostState.h \
	$(VEINS_PROJ)/src/veins/base/utils/MiXiMDefs.h \
	$(VEINS_PROJ)/src/veins/base/utils/miximkerneldefs.h \
	$(VEINS_PROJ)/src/veins/modules/messages/WaveShortMessage_m.h \
	$(VEINS_PROJ)/src/veins/modules/mobility/traci/TraCIBuffer.h \
	$(VEINS_PROJ)/src/veins/modules/mobility/traci/TraCIColor.h \
	$(VEINS_PROJ)/src/veins/modules/mobility/traci/TraCICommandInterface.h \
	$(VEINS_PROJ)/src/veins/modules/mobility/traci/TraCIConnection.h \
	$(VEINS_PROJ)/src/veins/modules/mobility/traci/TraCIConstants.h \
	$(VEINS_PROJ)/src/veins/modules/mobility/traci/TraCICoord.h \
	$(VEINS_PROJ)/src/veins/modules/mobility/traci/TraCIScenarioManager.h \
	$(VEINS_PROJ)/src/veins/modules/obstacle/Obstacle.h \
	$(VEINS_PROJ)/src/veins/modules/obstacle/ObstacleControl.h \
	$(VEINS_PROJ)/src/veins/modules/world/annotations/AnnotationManager.h
$O/Statistics.o: Statistics.cc \
	Appl.h \
	Statistics.h \
	../msg/Messages_m.h \
	../rsu/ApplRSU_01_Base.h \
	../rsu/ApplRSU_02_AID.h \
	../rsu/ApplRSU_03_Manager.h \
	../traci/TraCI_Extend.h \
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
$O/Tracking.o: Tracking.cc \
	Appl.h \
	Tracking.h \
	../msg/Messages_m.h \
	../traci/TraCI_Extend.h \
	$(VEINS_PROJ)/src/veins/base/connectionManager/BaseConnectionManager.h \
	$(VEINS_PROJ)/src/veins/base/connectionManager/NicEntry.h \
	$(VEINS_PROJ)/src/veins/base/modules/BaseModule.h \
	$(VEINS_PROJ)/src/veins/base/modules/BaseWorldUtility.h \
	$(VEINS_PROJ)/src/veins/base/utils/Coord.h \
	$(VEINS_PROJ)/src/veins/base/utils/FWMath.h \
	$(VEINS_PROJ)/src/veins/base/utils/FindModule.h \
	$(VEINS_PROJ)/src/veins/base/utils/HostState.h \
	$(VEINS_PROJ)/src/veins/base/utils/MiXiMDefs.h \
	$(VEINS_PROJ)/src/veins/base/utils/miximkerneldefs.h \
	$(VEINS_PROJ)/src/veins/modules/messages/WaveShortMessage_m.h \
	$(VEINS_PROJ)/src/veins/modules/mobility/traci/TraCIBuffer.h \
	$(VEINS_PROJ)/src/veins/modules/mobility/traci/TraCIColor.h \
	$(VEINS_PROJ)/src/veins/modules/mobility/traci/TraCICommandInterface.h \
	$(VEINS_PROJ)/src/veins/modules/mobility/traci/TraCIConnection.h \
	$(VEINS_PROJ)/src/veins/modules/mobility/traci/TraCIConstants.h \
	$(VEINS_PROJ)/src/veins/modules/mobility/traci/TraCICoord.h \
	$(VEINS_PROJ)/src/veins/modules/mobility/traci/TraCIScenarioManager.h \
	$(VEINS_PROJ)/src/veins/modules/obstacle/Obstacle.h \
	$(VEINS_PROJ)/src/veins/modules/obstacle/ObstacleControl.h \
	$(VEINS_PROJ)/src/veins/modules/world/annotations/AnnotationManager.h

