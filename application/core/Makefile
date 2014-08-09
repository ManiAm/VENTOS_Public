#
# OMNeT++/OMNEST Makefile for libcore
#
# This file was generated with the command:
#  opp_makemake -f --make-so -o core -O out -I/home/mani/Desktop/VENTOS/eigen-3.2.1 -I/home/mani/Desktop/VENTOS/rapidxml-1.13 -I../../../Veins/src/modules/utility -I../../../Veins/src/modules/phy -I../../../Veins/src/modules/analogueModel -I../../../Veins/src/base/modules -I../../../Veins/src/modules/mac/ieee80211p -I../../../Veins/src/base/messages -I../../../Veins/src/modules/messages -I../../../Veins/src/base/connectionManager -I../../../Veins/src/inet -I.. -I../../../Veins/src -I../../../Veins/src/base/utils -I../router -I../../../Veins/src/base/phyLayer -L../../../Veins/out/$$\(CONFIGNAME\)/src -L../../out/$$\(CONFIGNAME\)/application -L../../out/$$\(CONFIGNAME\)/application/msg -L../../out/$$\(CONFIGNAME\)/application/router -lboost_filesystem -lboost_system -lmsg -lVeins -KVEINS_PROJ=../../../Veins
#

# Name of target to be created (-o option)
TARGET = libcore$(SHARED_LIB_SUFFIX)

# C++ include paths (with -I)
INCLUDE_PATH = \
    -I../../eigen-3.2.1 \
    -I../../rapidxml-1.13 \
    -I../../../Veins/src/modules/utility \
    -I../../../Veins/src/modules/phy \
    -I../../../Veins/src/modules/analogueModel \
    -I../../../Veins/src/base/modules \
    -I../../../Veins/src/modules/mac/ieee80211p \
    -I../../../Veins/src/base/messages \
    -I../../../Veins/src/modules/messages \
    -I../../../Veins/src/base/connectionManager \
    -I../../../Veins/src/inet \
    -I.. \
    -I../../../Veins/src \
    -I../../../Veins/src/base/utils \
    -I../router \
    -I../../../Veins/src/base/phyLayer \
    -I.

# Additional object and library files to link with
EXTRA_OBJS =

# Additional libraries (-L, -l options)
LIBS = -L../../../Veins/out/$(CONFIGNAME)/src -L../../out/$(CONFIGNAME)/application -L../../out/$(CONFIGNAME)/application/msg -L../../out/$(CONFIGNAME)/application/router  -lboost_filesystem -lboost_system -lmsg -lVeins
LIBS += -Wl,-rpath,`abspath ../../../Veins/out/$(CONFIGNAME)/src` -Wl,-rpath,`abspath ../../out/$(CONFIGNAME)/application` -Wl,-rpath,`abspath ../../out/$(CONFIGNAME)/application/msg` -Wl,-rpath,`abspath ../../out/$(CONFIGNAME)/application/router`

# Output directory
PROJECT_OUTPUT_DIR = ../../out
PROJECTRELATIVE_PATH = application/core
O = $(PROJECT_OUTPUT_DIR)/$(CONFIGNAME)/$(PROJECTRELATIVE_PATH)

# Object files for local .cc and .msg files
OBJS = \
    $O/ApplV_06a_Merge.o \
    $O/Global_06_VehicleAdd.o \
    $O/Global_08_Warmup.o \
    $O/RSUMobility.o \
    $O/Global_05_Plotter.o \
    $O/ApplV_06d_Entry.o \
    $O/ApplV_06_PlatoonMg.o \
    $O/ApplV_07_Manager.o \
    $O/ApplV_06b_Split.o \
    $O/ApplV_06e_LeaderLeave.o \
    $O/Global_07_RSUAdd.o \
    $O/Global_02_TraCI_App.o \
    $O/ApplV_05_PlatoonFormed.o \
    $O/Mac1609_4_Mod.o \
    $O/ApplV_06f_FollowerLeave.o \
    $O/ApplRSU_01_Base.o \
    $O/ApplV_02_Beacon.o \
    $O/ApplV_04_AID.o \
    $O/Global_01_TraCI_Extend.o \
    $O/Global_04_Statistics.o \
    $O/ApplAdversary.o \
    $O/Global_09_SpeedProfile.o \
    $O/Global_03_Tracking.o \
    $O/ApplRSU_03_Manager.o \
    $O/ApplV_01_Base.o \
    $O/ApplRSU_02_AID.o \
    $O/ApplV_06c_Common.o \
    $O/ConstSpeedMobility.o

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
	$(Q)-rm -f core core.exe libcore.so libcore.a libcore.dll libcore.dylib
	$(Q)-rm -f ./*_m.cc ./*_m.h

cleanall: clean
	$(Q)-rm -rf $(PROJECT_OUTPUT_DIR)

depend:
	$(qecho) Creating dependencies...
	$(Q)$(MAKEDEPEND) $(INCLUDE_PATH) -f Makefile -P\$$O/ -- $(MSG_CC_FILES)  ./*.cc

# DO NOT DELETE THIS LINE -- make depend depends on it.
$O/ApplAdversary.o: ApplAdversary.cc \
	Appl.h \
	ApplAdversary.h \
	Global_01_TraCI_Extend.h \
	../msg/Messages_m.h \
	../router/Histogram.h \
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
	$(VEINS_PROJ)/src/modules/mobility/traci/TraCIScenarioManager.h \
	$(VEINS_PROJ)/src/modules/obstacle/Obstacle.h \
	$(VEINS_PROJ)/src/modules/obstacle/ObstacleControl.h \
	$(VEINS_PROJ)/src/modules/utility/Consts80211p.h \
	$(VEINS_PROJ)/src/modules/world/annotations/AnnotationManager.h
$O/ApplRSU_01_Base.o: ApplRSU_01_Base.cc \
	Appl.h \
	ApplRSU_01_Base.h \
	ApplV_01_Base.h \
	ApplV_02_Beacon.h \
	ApplV_04_AID.h \
	ApplV_05_PlatoonFormed.h \
	ApplV_06_PlatoonMg.h \
	ApplV_07_Manager.h \
	Global_01_TraCI_Extend.h \
	Global_07_RSUAdd.h \
	../msg/Messages_m.h \
	../router/ApplVSystem.h \
	../router/Histogram.h \
	../router/Hypertree.h \
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
$O/ApplRSU_02_AID.o: ApplRSU_02_AID.cc \
	Appl.h \
	ApplRSU_01_Base.h \
	ApplRSU_02_AID.h \
	ApplV_01_Base.h \
	ApplV_02_Beacon.h \
	ApplV_04_AID.h \
	ApplV_05_PlatoonFormed.h \
	ApplV_06_PlatoonMg.h \
	ApplV_07_Manager.h \
	Global_01_TraCI_Extend.h \
	Global_07_RSUAdd.h \
	../msg/Messages_m.h \
	../router/ApplVSystem.h \
	../router/Histogram.h \
	../router/Hypertree.h \
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
$O/ApplRSU_03_Manager.o: ApplRSU_03_Manager.cc \
	Appl.h \
	ApplRSU_01_Base.h \
	ApplRSU_02_AID.h \
	ApplRSU_03_Manager.h \
	ApplV_01_Base.h \
	ApplV_02_Beacon.h \
	ApplV_04_AID.h \
	ApplV_05_PlatoonFormed.h \
	ApplV_06_PlatoonMg.h \
	ApplV_07_Manager.h \
	Global_01_TraCI_Extend.h \
	Global_07_RSUAdd.h \
	../msg/Messages_m.h \
	../router/ApplVSystem.h \
	../router/Histogram.h \
	../router/Hypertree.h \
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
$O/ApplV_01_Base.o: ApplV_01_Base.cc \
	Appl.h \
	ApplV_01_Base.h \
	Global_01_TraCI_Extend.h \
	../msg/Messages_m.h \
	../router/Histogram.h \
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
$O/ApplV_02_Beacon.o: ApplV_02_Beacon.cc \
	Appl.h \
	ApplV_01_Base.h \
	ApplV_02_Beacon.h \
	Global_01_TraCI_Extend.h \
	../msg/Messages_m.h \
	../router/Histogram.h \
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
$O/ApplV_04_AID.o: ApplV_04_AID.cc \
	Appl.h \
	ApplV_01_Base.h \
	ApplV_02_Beacon.h \
	ApplV_04_AID.h \
	Global_01_TraCI_Extend.h \
	../msg/Messages_m.h \
	../router/ApplVSystem.h \
	../router/Histogram.h \
	../router/Hypertree.h \
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
$O/ApplV_05_PlatoonFormed.o: ApplV_05_PlatoonFormed.cc \
	Appl.h \
	ApplV_01_Base.h \
	ApplV_02_Beacon.h \
	ApplV_04_AID.h \
	ApplV_05_PlatoonFormed.h \
	Global_01_TraCI_Extend.h \
	../msg/Messages_m.h \
	../router/ApplVSystem.h \
	../router/Histogram.h \
	../router/Hypertree.h \
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
$O/ApplV_06_PlatoonMg.o: ApplV_06_PlatoonMg.cc \
	Appl.h \
	ApplV_01_Base.h \
	ApplV_02_Beacon.h \
	ApplV_04_AID.h \
	ApplV_05_PlatoonFormed.h \
	ApplV_06_PlatoonMg.h \
	Global_01_TraCI_Extend.h \
	../msg/Messages_m.h \
	../router/ApplVSystem.h \
	../router/Histogram.h \
	../router/Hypertree.h \
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
$O/ApplV_06a_Merge.o: ApplV_06a_Merge.cc \
	Appl.h \
	ApplV_01_Base.h \
	ApplV_02_Beacon.h \
	ApplV_04_AID.h \
	ApplV_05_PlatoonFormed.h \
	ApplV_06_PlatoonMg.h \
	Global_01_TraCI_Extend.h \
	../msg/Messages_m.h \
	../router/ApplVSystem.h \
	../router/Histogram.h \
	../router/Hypertree.h \
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
$O/ApplV_06b_Split.o: ApplV_06b_Split.cc \
	Appl.h \
	ApplV_01_Base.h \
	ApplV_02_Beacon.h \
	ApplV_04_AID.h \
	ApplV_05_PlatoonFormed.h \
	ApplV_06_PlatoonMg.h \
	Global_01_TraCI_Extend.h \
	../msg/Messages_m.h \
	../router/ApplVSystem.h \
	../router/Histogram.h \
	../router/Hypertree.h \
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
$O/ApplV_06c_Common.o: ApplV_06c_Common.cc \
	Appl.h \
	ApplV_01_Base.h \
	ApplV_02_Beacon.h \
	ApplV_04_AID.h \
	ApplV_05_PlatoonFormed.h \
	ApplV_06_PlatoonMg.h \
	Global_01_TraCI_Extend.h \
	../msg/Messages_m.h \
	../router/ApplVSystem.h \
	../router/Histogram.h \
	../router/Hypertree.h \
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
$O/ApplV_06d_Entry.o: ApplV_06d_Entry.cc \
	Appl.h \
	ApplV_01_Base.h \
	ApplV_02_Beacon.h \
	ApplV_04_AID.h \
	ApplV_05_PlatoonFormed.h \
	ApplV_06_PlatoonMg.h \
	Global_01_TraCI_Extend.h \
	../msg/Messages_m.h \
	../router/ApplVSystem.h \
	../router/Histogram.h \
	../router/Hypertree.h \
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
$O/ApplV_06e_LeaderLeave.o: ApplV_06e_LeaderLeave.cc \
	Appl.h \
	ApplV_01_Base.h \
	ApplV_02_Beacon.h \
	ApplV_04_AID.h \
	ApplV_05_PlatoonFormed.h \
	ApplV_06_PlatoonMg.h \
	Global_01_TraCI_Extend.h \
	../msg/Messages_m.h \
	../router/ApplVSystem.h \
	../router/Histogram.h \
	../router/Hypertree.h \
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
$O/ApplV_06f_FollowerLeave.o: ApplV_06f_FollowerLeave.cc \
	Appl.h \
	ApplV_01_Base.h \
	ApplV_02_Beacon.h \
	ApplV_04_AID.h \
	ApplV_05_PlatoonFormed.h \
	ApplV_06_PlatoonMg.h \
	Global_01_TraCI_Extend.h \
	../msg/Messages_m.h \
	../router/ApplVSystem.h \
	../router/Histogram.h \
	../router/Hypertree.h \
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
$O/ApplV_07_Manager.o: ApplV_07_Manager.cc \
	Appl.h \
	ApplV_01_Base.h \
	ApplV_02_Beacon.h \
	ApplV_04_AID.h \
	ApplV_05_PlatoonFormed.h \
	ApplV_06_PlatoonMg.h \
	ApplV_07_Manager.h \
	Global_01_TraCI_Extend.h \
	../msg/Messages_m.h \
	../router/ApplVSystem.h \
	../router/Histogram.h \
	../router/Hypertree.h \
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
$O/ConstSpeedMobility.o: ConstSpeedMobility.cc \
	ConstSpeedMobility.h \
	$(VEINS_PROJ)/src/base/modules/BaseBattery.h \
	$(VEINS_PROJ)/src/base/modules/BaseMobility.h \
	$(VEINS_PROJ)/src/base/modules/BaseModule.h \
	$(VEINS_PROJ)/src/base/modules/BaseWorldUtility.h \
	$(VEINS_PROJ)/src/base/modules/BatteryAccess.h \
	$(VEINS_PROJ)/src/base/utils/HostState.h \
	$(VEINS_PROJ)/src/base/utils/MiXiMDefs.h \
	$(VEINS_PROJ)/src/base/utils/Move.h \
	$(VEINS_PROJ)/src/base/utils/miximkerneldefs.h \
	$(VEINS_PROJ)/src/inet/Coord.h \
	$(VEINS_PROJ)/src/inet/FWMath.h
$O/Global_01_TraCI_Extend.o: Global_01_TraCI_Extend.cc \
	Appl.h \
	Global_01_TraCI_Extend.h \
	../msg/Messages_m.h \
	../router/Histogram.h \
	$(VEINS_PROJ)/src/base/connectionManager/BaseConnectionManager.h \
	$(VEINS_PROJ)/src/base/connectionManager/NicEntry.h \
	$(VEINS_PROJ)/src/base/modules/BaseWorldUtility.h \
	$(VEINS_PROJ)/src/base/utils/FindModule.h \
	$(VEINS_PROJ)/src/base/utils/MiXiMDefs.h \
	$(VEINS_PROJ)/src/base/utils/miximkerneldefs.h \
	$(VEINS_PROJ)/src/inet/Coord.h \
	$(VEINS_PROJ)/src/inet/FWMath.h \
	$(VEINS_PROJ)/src/modules/messages/WaveShortMessage_m.h \
	$(VEINS_PROJ)/src/modules/mobility/traci/TraCIBuffer.h \
	$(VEINS_PROJ)/src/modules/mobility/traci/TraCIColor.h \
	$(VEINS_PROJ)/src/modules/mobility/traci/TraCICommandInterface.h \
	$(VEINS_PROJ)/src/modules/mobility/traci/TraCIConnection.h \
	$(VEINS_PROJ)/src/modules/mobility/traci/TraCIConstants.h \
	$(VEINS_PROJ)/src/modules/mobility/traci/TraCICoord.h \
	$(VEINS_PROJ)/src/modules/mobility/traci/TraCIScenarioManager.h \
	$(VEINS_PROJ)/src/modules/obstacle/Obstacle.h \
	$(VEINS_PROJ)/src/modules/obstacle/ObstacleControl.h \
	$(VEINS_PROJ)/src/modules/world/annotations/AnnotationManager.h
$O/Global_02_TraCI_App.o: Global_02_TraCI_App.cc \
	Appl.h \
	ApplRSU_01_Base.h \
	ApplRSU_02_AID.h \
	ApplRSU_03_Manager.h \
	ApplV_01_Base.h \
	ApplV_02_Beacon.h \
	ApplV_04_AID.h \
	ApplV_05_PlatoonFormed.h \
	ApplV_06_PlatoonMg.h \
	ApplV_07_Manager.h \
	Global_01_TraCI_Extend.h \
	Global_02_TraCI_App.h \
	Global_03_Tracking.h \
	Global_04_Statistics.h \
	Global_06_VehicleAdd.h \
	Global_07_RSUAdd.h \
	Global_08_Warmup.h \
	Global_09_SpeedProfile.h \
	../msg/Messages_m.h \
	../router/ApplVSystem.h \
	../router/Histogram.h \
	../router/Hypertree.h \
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
$O/Global_03_Tracking.o: Global_03_Tracking.cc \
	Appl.h \
	Global_01_TraCI_Extend.h \
	Global_03_Tracking.h \
	../msg/Messages_m.h \
	../router/Histogram.h \
	$(VEINS_PROJ)/src/base/connectionManager/BaseConnectionManager.h \
	$(VEINS_PROJ)/src/base/connectionManager/NicEntry.h \
	$(VEINS_PROJ)/src/base/modules/BaseModule.h \
	$(VEINS_PROJ)/src/base/modules/BaseWorldUtility.h \
	$(VEINS_PROJ)/src/base/utils/FindModule.h \
	$(VEINS_PROJ)/src/base/utils/HostState.h \
	$(VEINS_PROJ)/src/base/utils/MiXiMDefs.h \
	$(VEINS_PROJ)/src/base/utils/miximkerneldefs.h \
	$(VEINS_PROJ)/src/inet/Coord.h \
	$(VEINS_PROJ)/src/inet/FWMath.h \
	$(VEINS_PROJ)/src/modules/messages/WaveShortMessage_m.h \
	$(VEINS_PROJ)/src/modules/mobility/traci/TraCIBuffer.h \
	$(VEINS_PROJ)/src/modules/mobility/traci/TraCIColor.h \
	$(VEINS_PROJ)/src/modules/mobility/traci/TraCICommandInterface.h \
	$(VEINS_PROJ)/src/modules/mobility/traci/TraCIConnection.h \
	$(VEINS_PROJ)/src/modules/mobility/traci/TraCIConstants.h \
	$(VEINS_PROJ)/src/modules/mobility/traci/TraCICoord.h \
	$(VEINS_PROJ)/src/modules/mobility/traci/TraCIScenarioManager.h \
	$(VEINS_PROJ)/src/modules/obstacle/Obstacle.h \
	$(VEINS_PROJ)/src/modules/obstacle/ObstacleControl.h \
	$(VEINS_PROJ)/src/modules/world/annotations/AnnotationManager.h
$O/Global_04_Statistics.o: Global_04_Statistics.cc \
	Appl.h \
	ApplRSU_01_Base.h \
	ApplRSU_02_AID.h \
	ApplRSU_03_Manager.h \
	ApplV_01_Base.h \
	ApplV_02_Beacon.h \
	ApplV_04_AID.h \
	ApplV_05_PlatoonFormed.h \
	ApplV_06_PlatoonMg.h \
	ApplV_07_Manager.h \
	Global_01_TraCI_Extend.h \
	Global_04_Statistics.h \
	Global_07_RSUAdd.h \
	../msg/Messages_m.h \
	../router/ApplVSystem.h \
	../router/Histogram.h \
	../router/Hypertree.h \
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
$O/Global_05_Plotter.o: Global_05_Plotter.cc \
	Appl.h \
	Global_01_TraCI_Extend.h \
	Global_05_Plotter.h \
	../msg/Messages_m.h \
	../router/Histogram.h \
	$(VEINS_PROJ)/src/base/connectionManager/BaseConnectionManager.h \
	$(VEINS_PROJ)/src/base/connectionManager/NicEntry.h \
	$(VEINS_PROJ)/src/base/modules/BaseModule.h \
	$(VEINS_PROJ)/src/base/modules/BaseWorldUtility.h \
	$(VEINS_PROJ)/src/base/utils/FindModule.h \
	$(VEINS_PROJ)/src/base/utils/HostState.h \
	$(VEINS_PROJ)/src/base/utils/MiXiMDefs.h \
	$(VEINS_PROJ)/src/base/utils/miximkerneldefs.h \
	$(VEINS_PROJ)/src/inet/Coord.h \
	$(VEINS_PROJ)/src/inet/FWMath.h \
	$(VEINS_PROJ)/src/modules/messages/WaveShortMessage_m.h \
	$(VEINS_PROJ)/src/modules/mobility/traci/TraCIBuffer.h \
	$(VEINS_PROJ)/src/modules/mobility/traci/TraCIColor.h \
	$(VEINS_PROJ)/src/modules/mobility/traci/TraCICommandInterface.h \
	$(VEINS_PROJ)/src/modules/mobility/traci/TraCIConnection.h \
	$(VEINS_PROJ)/src/modules/mobility/traci/TraCIConstants.h \
	$(VEINS_PROJ)/src/modules/mobility/traci/TraCICoord.h \
	$(VEINS_PROJ)/src/modules/mobility/traci/TraCIScenarioManager.h \
	$(VEINS_PROJ)/src/modules/obstacle/Obstacle.h \
	$(VEINS_PROJ)/src/modules/obstacle/ObstacleControl.h \
	$(VEINS_PROJ)/src/modules/world/annotations/AnnotationManager.h
$O/Global_06_VehicleAdd.o: Global_06_VehicleAdd.cc \
	Appl.h \
	ApplV_01_Base.h \
	ApplV_02_Beacon.h \
	ApplV_04_AID.h \
	ApplV_05_PlatoonFormed.h \
	ApplV_06_PlatoonMg.h \
	ApplV_07_Manager.h \
	Global_01_TraCI_Extend.h \
	Global_06_VehicleAdd.h \
	../msg/Messages_m.h \
	../router/ApplVSystem.h \
	../router/Histogram.h \
	../router/Hypertree.h \
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
$O/Global_07_RSUAdd.o: Global_07_RSUAdd.cc \
	Appl.h \
	ApplV_01_Base.h \
	ApplV_02_Beacon.h \
	ApplV_04_AID.h \
	ApplV_05_PlatoonFormed.h \
	ApplV_06_PlatoonMg.h \
	ApplV_07_Manager.h \
	Global_01_TraCI_Extend.h \
	Global_07_RSUAdd.h \
	../msg/Messages_m.h \
	../router/ApplVSystem.h \
	../router/Histogram.h \
	../router/Hypertree.h \
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
$O/Global_08_Warmup.o: Global_08_Warmup.cc \
	Appl.h \
	ApplV_01_Base.h \
	ApplV_02_Beacon.h \
	ApplV_04_AID.h \
	ApplV_05_PlatoonFormed.h \
	ApplV_06_PlatoonMg.h \
	ApplV_07_Manager.h \
	Global_01_TraCI_Extend.h \
	Global_06_VehicleAdd.h \
	Global_08_Warmup.h \
	Global_09_SpeedProfile.h \
	../msg/Messages_m.h \
	../router/ApplVSystem.h \
	../router/Histogram.h \
	../router/Hypertree.h \
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
$O/Global_09_SpeedProfile.o: Global_09_SpeedProfile.cc \
	Appl.h \
	Global_01_TraCI_Extend.h \
	Global_09_SpeedProfile.h \
	../msg/Messages_m.h \
	../router/Histogram.h \
	$(VEINS_PROJ)/src/base/connectionManager/BaseConnectionManager.h \
	$(VEINS_PROJ)/src/base/connectionManager/NicEntry.h \
	$(VEINS_PROJ)/src/base/modules/BaseModule.h \
	$(VEINS_PROJ)/src/base/modules/BaseWorldUtility.h \
	$(VEINS_PROJ)/src/base/utils/FindModule.h \
	$(VEINS_PROJ)/src/base/utils/HostState.h \
	$(VEINS_PROJ)/src/base/utils/MiXiMDefs.h \
	$(VEINS_PROJ)/src/base/utils/miximkerneldefs.h \
	$(VEINS_PROJ)/src/inet/Coord.h \
	$(VEINS_PROJ)/src/inet/FWMath.h \
	$(VEINS_PROJ)/src/modules/messages/WaveShortMessage_m.h \
	$(VEINS_PROJ)/src/modules/mobility/traci/TraCIBuffer.h \
	$(VEINS_PROJ)/src/modules/mobility/traci/TraCIColor.h \
	$(VEINS_PROJ)/src/modules/mobility/traci/TraCICommandInterface.h \
	$(VEINS_PROJ)/src/modules/mobility/traci/TraCIConnection.h \
	$(VEINS_PROJ)/src/modules/mobility/traci/TraCIConstants.h \
	$(VEINS_PROJ)/src/modules/mobility/traci/TraCICoord.h \
	$(VEINS_PROJ)/src/modules/mobility/traci/TraCIScenarioManager.h \
	$(VEINS_PROJ)/src/modules/obstacle/Obstacle.h \
	$(VEINS_PROJ)/src/modules/obstacle/ObstacleControl.h \
	$(VEINS_PROJ)/src/modules/world/annotations/AnnotationManager.h
$O/Mac1609_4_Mod.o: Mac1609_4_Mod.cc \
	Appl.h \
	ApplRSU_01_Base.h \
	ApplRSU_02_AID.h \
	ApplRSU_03_Manager.h \
	ApplV_01_Base.h \
	ApplV_02_Beacon.h \
	ApplV_04_AID.h \
	ApplV_05_PlatoonFormed.h \
	ApplV_06_PlatoonMg.h \
	ApplV_07_Manager.h \
	Global_01_TraCI_Extend.h \
	Global_04_Statistics.h \
	Global_07_RSUAdd.h \
	Mac1609_4_Mod.h \
	../msg/Messages_m.h \
	../router/ApplVSystem.h \
	../router/Histogram.h \
	../router/Hypertree.h \
	$(VEINS_PROJ)/src/base/connectionManager/BaseConnectionManager.h \
	$(VEINS_PROJ)/src/base/connectionManager/ChannelAccess.h \
	$(VEINS_PROJ)/src/base/connectionManager/NicEntry.h \
	$(VEINS_PROJ)/src/base/messages/AirFrame_m.h \
	$(VEINS_PROJ)/src/base/messages/ChannelSenseRequest_m.h \
	$(VEINS_PROJ)/src/base/messages/MacPkt_m.h \
	$(VEINS_PROJ)/src/base/modules/BaseApplLayer.h \
	$(VEINS_PROJ)/src/base/modules/BaseBattery.h \
	$(VEINS_PROJ)/src/base/modules/BaseLayer.h \
	$(VEINS_PROJ)/src/base/modules/BaseMacLayer.h \
	$(VEINS_PROJ)/src/base/modules/BaseMobility.h \
	$(VEINS_PROJ)/src/base/modules/BaseModule.h \
	$(VEINS_PROJ)/src/base/modules/BaseWorldUtility.h \
	$(VEINS_PROJ)/src/base/modules/BatteryAccess.h \
	$(VEINS_PROJ)/src/base/phyLayer/AnalogueModel.h \
	$(VEINS_PROJ)/src/base/phyLayer/BaseDecider.h \
	$(VEINS_PROJ)/src/base/phyLayer/BasePhyLayer.h \
	$(VEINS_PROJ)/src/base/phyLayer/ChannelInfo.h \
	$(VEINS_PROJ)/src/base/phyLayer/ChannelState.h \
	$(VEINS_PROJ)/src/base/phyLayer/Decider.h \
	$(VEINS_PROJ)/src/base/phyLayer/DeciderToPhyInterface.h \
	$(VEINS_PROJ)/src/base/phyLayer/Interpolation.h \
	$(VEINS_PROJ)/src/base/phyLayer/MacToPhyControlInfo.h \
	$(VEINS_PROJ)/src/base/phyLayer/MacToPhyInterface.h \
	$(VEINS_PROJ)/src/base/phyLayer/Mapping.h \
	$(VEINS_PROJ)/src/base/phyLayer/MappingBase.h \
	$(VEINS_PROJ)/src/base/phyLayer/MappingUtils.h \
	$(VEINS_PROJ)/src/base/phyLayer/PhyUtils.h \
	$(VEINS_PROJ)/src/base/phyLayer/Signal_.h \
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
	$(VEINS_PROJ)/src/modules/analogueModel/JakesFading.h \
	$(VEINS_PROJ)/src/modules/analogueModel/LogNormalShadowing.h \
	$(VEINS_PROJ)/src/modules/analogueModel/SimplePathlossModel.h \
	$(VEINS_PROJ)/src/modules/mac/ieee80211p/Mac1609_4.h \
	$(VEINS_PROJ)/src/modules/mac/ieee80211p/Mac80211pToPhy11pInterface.h \
	$(VEINS_PROJ)/src/modules/mac/ieee80211p/WaveAppToMac1609_4Interface.h \
	$(VEINS_PROJ)/src/modules/messages/Mac80211Pkt_m.h \
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
	$(VEINS_PROJ)/src/modules/phy/Decider80211p.h \
	$(VEINS_PROJ)/src/modules/phy/Decider80211pToPhy80211pInterface.h \
	$(VEINS_PROJ)/src/modules/phy/PhyLayer80211p.h \
	$(VEINS_PROJ)/src/modules/phy/SNRThresholdDecider.h \
	$(VEINS_PROJ)/src/modules/utility/Consts80211p.h \
	$(VEINS_PROJ)/src/modules/world/annotations/AnnotationManager.h
$O/RSUMobility.o: RSUMobility.cc \
	Appl.h \
	ApplV_01_Base.h \
	ApplV_02_Beacon.h \
	ApplV_04_AID.h \
	ApplV_05_PlatoonFormed.h \
	ApplV_06_PlatoonMg.h \
	ApplV_07_Manager.h \
	Global_01_TraCI_Extend.h \
	Global_07_RSUAdd.h \
	RSUMobility.h \
	../msg/Messages_m.h \
	../router/ApplVSystem.h \
	../router/Histogram.h \
	../router/Hypertree.h \
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

