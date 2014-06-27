#
# OMNeT++/OMNEST Makefile for libapplication
#
# This file was generated with the command:
#  opp_makemake -f --make-so -o application -O out -d msg -I/home/mani/Desktop/VENTOS/eigen-3.2.1 -I/home/mani/Desktop/VENTOS/rapidxml-1.13 -I../../veinsGit/src/base/messages -I../../veinsGit/src/modules/utility -I../../veinsGit/src/modules -I../../veinsGit/src/modules/analogueModel -I../../veinsGit/src/modules/phy -I../../veinsGit/src/modules/mac/ieee80211p -I../../veinsGit/src/modules/messages -I../../veinsGit/src -I../../veinsGit/src/inet -I../../veinsGit/src/base/connectionManager -I../../veinsGit/src/base/phyLayer -I../../veinsGit/src/base/modules -I../../veinsGit/src/base/utils -L../../veinsGit/out/$(CONFIGNAME)/src -L../out/$(CONFIGNAME)/application/msg -lboost_filesystem -lboost_system -lmsg -lveinsGit -KVEINSGIT_PROJ=../../veinsGit
#

# Name of target to be created (-o option)
TARGET = libapplication$(SHARED_LIB_SUFFIX)

# C++ include paths (with -I)
INCLUDE_PATH = \
    -I../eigen-3.2.1 \
    -I../rapidxml-1.13 \
    -I../../veinsGit/src/base/messages \
    -I../../veinsGit/src/modules/utility \
    -I../../veinsGit/src/modules \
    -I../../veinsGit/src/modules/analogueModel \
    -I../../veinsGit/src/modules/phy \
    -I../../veinsGit/src/modules/mac/ieee80211p \
    -I../../veinsGit/src/modules/messages \
    -I../../veinsGit/src \
    -I../../veinsGit/src/inet \
    -I../../veinsGit/src/base/connectionManager \
    -I../../veinsGit/src/base/phyLayer \
    -I../../veinsGit/src/base/modules \
    -I../../veinsGit/src/base/utils \
    -I.

# Additional object and library files to link with
EXTRA_OBJS =

# Additional libraries (-L, -l options)
LIBS = -L../../veinsGit/out/$(CONFIGNAME)/src -L../out/$(CONFIGNAME)/application/msg  -lboost_filesystem -lboost_system -lmsg -lveinsGit
LIBS += -Wl,-rpath,`abspath ../../veinsGit/out/$(CONFIGNAME)/src` -Wl,-rpath,`abspath ../out/$(CONFIGNAME)/application/msg`

# Output directory
PROJECT_OUTPUT_DIR = ../out
PROJECTRELATIVE_PATH = application
O = $(PROJECT_OUTPUT_DIR)/$(CONFIGNAME)/$(PROJECTRELATIVE_PATH)

# Object files for local .cc and .msg files
OBJS = \
    $O/ApplV_05e_LeaderLeave.o \
    $O/Global_03_Statistics.o \
    $O/zApplVPlatoonFormation2.o \
    $O/NodeEdge.o \
    $O/RSUMobility.o \
    $O/ApplV_05b_Merge.o \
    $O/ApplV_06_Manager.o \
    $O/zApplVPlatoonFormation.o \
    $O/Global_02_TraCI_App.o \
    $O/Mac1609_4_Mod.o \
    $O/ApplV_05a_Entry.o \
    $O/Global_07_Router.o \
    $O/ApplRSU_01_Base.o \
    $O/Global_04_VehicleAdd.o \
    $O/ApplV_05c_Split.o \
    $O/ApplV_03_System.o \
    $O/ApplV_02_Beacon.o \
    $O/ApplV_04_AID.o \
    $O/Global_01_TraCI_Extend.o \
    $O/ApplV_05d_FollowerLeave.o \
    $O/ApplAdversary.o \
    $O/ApplV_05_PlatoonMg.o \
    $O/ApplRSU_03_Manager.o \
    $O/Global_08_TrafficLight.o \
    $O/ApplV_01_Base.o \
    $O/ApplRSU_02_AID.o \
    $O/Global_05_Warmup.o \
    $O/Global_06_SpeedProfile.o

# Message files
MSGFILES =

# Other makefile variables (-K)
VEINSGIT_PROJ=../../veinsGit

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

$O/$(TARGET): $(OBJS) submakedirs $(wildcard $(EXTRA_OBJS)) Makefile
	@$(MKPATH) $O
	@echo Creating shared library: $@
	$(Q)$(SHLIB_LD) -o $O/$(TARGET)  $(OBJS) $(EXTRA_OBJS) $(AS_NEEDED_OFF) $(WHOLE_ARCHIVE_ON) $(LIBS) $(WHOLE_ARCHIVE_OFF) $(OMNETPP_LIBS) $(LDFLAGS)
	$(Q)$(SHLIB_POSTPROCESS) $O/$(TARGET)

submakedirs:  msg_dir

.PHONY: all clean cleanall depend msgheaders  msg
msg: msg_dir

msg_dir:
	cd msg && $(MAKE) all

.SUFFIXES: .cc

$O/%.o: %.cc $(COPTS_FILE)
	@$(MKPATH) $(dir $@)
	$(qecho) "$<"
	$(Q)$(CXX) -c $(COPTS) -o $@ $<

%_m.cc %_m.h: %.msg
	$(qecho) MSGC: $<
	$(Q)$(MSGC) -s _m.cc $(MSGCOPTS) $?

msgheaders: $(MSGFILES:.msg=_m.h)
	$(Q)cd msg && $(MAKE) msgheaders

clean:
	$(qecho) Cleaning...
	$(Q)-rm -rf $O
	$(Q)-rm -f application application.exe libapplication.so libapplication.a libapplication.dll libapplication.dylib
	$(Q)-rm -f ./*_m.cc ./*_m.h

	-$(Q)cd msg && $(MAKE) clean

cleanall: clean
	$(Q)-rm -rf $(PROJECT_OUTPUT_DIR)

depend:
	$(qecho) Creating dependencies...
	$(Q)$(MAKEDEPEND) $(INCLUDE_PATH) -f Makefile -P\$$O/ -- $(MSG_CC_FILES)  ./*.cc
	$(Q)-cd msg && if [ -f Makefile ]; then $(MAKE) depend; fi

# DO NOT DELETE THIS LINE -- make depend depends on it.
$O/ApplAdversary.o: ApplAdversary.cc \
	Appl.h \
	ApplAdversary.h \
	Global_01_TraCI_Extend.h \
	msg/Messages_m.h \
	$(VEINSGIT_PROJ)/src/base/connectionManager/BaseConnectionManager.h \
	$(VEINSGIT_PROJ)/src/base/connectionManager/ChannelAccess.h \
	$(VEINSGIT_PROJ)/src/base/connectionManager/NicEntry.h \
	$(VEINSGIT_PROJ)/src/base/messages/MacPkt_m.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseApplLayer.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseBattery.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseLayer.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseMobility.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseModule.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseWorldUtility.h \
	$(VEINSGIT_PROJ)/src/base/modules/BatteryAccess.h \
	$(VEINSGIT_PROJ)/src/base/utils/FindModule.h \
	$(VEINSGIT_PROJ)/src/base/utils/HostState.h \
	$(VEINSGIT_PROJ)/src/base/utils/MiXiMDefs.h \
	$(VEINSGIT_PROJ)/src/base/utils/Move.h \
	$(VEINSGIT_PROJ)/src/base/utils/NetwToMacControlInfo.h \
	$(VEINSGIT_PROJ)/src/base/utils/PassedMessage.h \
	$(VEINSGIT_PROJ)/src/base/utils/SimpleAddress.h \
	$(VEINSGIT_PROJ)/src/base/utils/miximkerneldefs.h \
	$(VEINSGIT_PROJ)/src/inet/Coord.h \
	$(VEINSGIT_PROJ)/src/inet/FWMath.h \
	$(VEINSGIT_PROJ)/src/modules/mac/ieee80211p/WaveAppToMac1609_4Interface.h \
	$(VEINSGIT_PROJ)/src/modules/messages/Mac80211Pkt_m.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIBuffer.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIColor.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCICommandInterface.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIConnection.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIConstants.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCICoord.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIMobility.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIScenarioManager.h \
	$(VEINSGIT_PROJ)/src/modules/obstacle/Obstacle.h \
	$(VEINSGIT_PROJ)/src/modules/obstacle/ObstacleControl.h \
	$(VEINSGIT_PROJ)/src/modules/utility/Consts80211p.h \
	$(VEINSGIT_PROJ)/src/modules/world/annotations/AnnotationManager.h
$O/ApplRSU_01_Base.o: ApplRSU_01_Base.cc \
	Appl.h \
	ApplRSU_01_Base.h \
	Global_01_TraCI_Extend.h \
	msg/Messages_m.h \
	$(VEINSGIT_PROJ)/src/base/connectionManager/BaseConnectionManager.h \
	$(VEINSGIT_PROJ)/src/base/connectionManager/ChannelAccess.h \
	$(VEINSGIT_PROJ)/src/base/connectionManager/NicEntry.h \
	$(VEINSGIT_PROJ)/src/base/messages/MacPkt_m.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseApplLayer.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseBattery.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseLayer.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseMobility.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseModule.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseWorldUtility.h \
	$(VEINSGIT_PROJ)/src/base/modules/BatteryAccess.h \
	$(VEINSGIT_PROJ)/src/base/utils/FindModule.h \
	$(VEINSGIT_PROJ)/src/base/utils/HostState.h \
	$(VEINSGIT_PROJ)/src/base/utils/MiXiMDefs.h \
	$(VEINSGIT_PROJ)/src/base/utils/Move.h \
	$(VEINSGIT_PROJ)/src/base/utils/NetwToMacControlInfo.h \
	$(VEINSGIT_PROJ)/src/base/utils/PassedMessage.h \
	$(VEINSGIT_PROJ)/src/base/utils/SimpleAddress.h \
	$(VEINSGIT_PROJ)/src/base/utils/miximkerneldefs.h \
	$(VEINSGIT_PROJ)/src/inet/Coord.h \
	$(VEINSGIT_PROJ)/src/inet/FWMath.h \
	$(VEINSGIT_PROJ)/src/modules/mac/ieee80211p/WaveAppToMac1609_4Interface.h \
	$(VEINSGIT_PROJ)/src/modules/messages/Mac80211Pkt_m.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIBuffer.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIColor.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCICommandInterface.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIConnection.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIConstants.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCICoord.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIMobility.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIScenarioManager.h \
	$(VEINSGIT_PROJ)/src/modules/obstacle/Obstacle.h \
	$(VEINSGIT_PROJ)/src/modules/obstacle/ObstacleControl.h \
	$(VEINSGIT_PROJ)/src/modules/utility/Consts80211p.h \
	$(VEINSGIT_PROJ)/src/modules/world/annotations/AnnotationManager.h
$O/ApplRSU_02_AID.o: ApplRSU_02_AID.cc \
	Appl.h \
	ApplRSU_01_Base.h \
	ApplRSU_02_AID.h \
	Global_01_TraCI_Extend.h \
	msg/Messages_m.h \
	$(VEINSGIT_PROJ)/src/base/connectionManager/BaseConnectionManager.h \
	$(VEINSGIT_PROJ)/src/base/connectionManager/ChannelAccess.h \
	$(VEINSGIT_PROJ)/src/base/connectionManager/NicEntry.h \
	$(VEINSGIT_PROJ)/src/base/messages/MacPkt_m.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseApplLayer.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseBattery.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseLayer.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseMobility.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseModule.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseWorldUtility.h \
	$(VEINSGIT_PROJ)/src/base/modules/BatteryAccess.h \
	$(VEINSGIT_PROJ)/src/base/utils/FindModule.h \
	$(VEINSGIT_PROJ)/src/base/utils/HostState.h \
	$(VEINSGIT_PROJ)/src/base/utils/MiXiMDefs.h \
	$(VEINSGIT_PROJ)/src/base/utils/Move.h \
	$(VEINSGIT_PROJ)/src/base/utils/NetwToMacControlInfo.h \
	$(VEINSGIT_PROJ)/src/base/utils/PassedMessage.h \
	$(VEINSGIT_PROJ)/src/base/utils/SimpleAddress.h \
	$(VEINSGIT_PROJ)/src/base/utils/miximkerneldefs.h \
	$(VEINSGIT_PROJ)/src/inet/Coord.h \
	$(VEINSGIT_PROJ)/src/inet/FWMath.h \
	$(VEINSGIT_PROJ)/src/modules/mac/ieee80211p/WaveAppToMac1609_4Interface.h \
	$(VEINSGIT_PROJ)/src/modules/messages/Mac80211Pkt_m.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIBuffer.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIColor.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCICommandInterface.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIConnection.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIConstants.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCICoord.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIMobility.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIScenarioManager.h \
	$(VEINSGIT_PROJ)/src/modules/obstacle/Obstacle.h \
	$(VEINSGIT_PROJ)/src/modules/obstacle/ObstacleControl.h \
	$(VEINSGIT_PROJ)/src/modules/utility/Consts80211p.h \
	$(VEINSGIT_PROJ)/src/modules/world/annotations/AnnotationManager.h
$O/ApplRSU_03_Manager.o: ApplRSU_03_Manager.cc \
	Appl.h \
	ApplRSU_01_Base.h \
	ApplRSU_02_AID.h \
	ApplRSU_03_Manager.h \
	Global_01_TraCI_Extend.h \
	msg/Messages_m.h \
	$(VEINSGIT_PROJ)/src/base/connectionManager/BaseConnectionManager.h \
	$(VEINSGIT_PROJ)/src/base/connectionManager/ChannelAccess.h \
	$(VEINSGIT_PROJ)/src/base/connectionManager/NicEntry.h \
	$(VEINSGIT_PROJ)/src/base/messages/MacPkt_m.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseApplLayer.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseBattery.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseLayer.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseMobility.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseModule.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseWorldUtility.h \
	$(VEINSGIT_PROJ)/src/base/modules/BatteryAccess.h \
	$(VEINSGIT_PROJ)/src/base/utils/FindModule.h \
	$(VEINSGIT_PROJ)/src/base/utils/HostState.h \
	$(VEINSGIT_PROJ)/src/base/utils/MiXiMDefs.h \
	$(VEINSGIT_PROJ)/src/base/utils/Move.h \
	$(VEINSGIT_PROJ)/src/base/utils/NetwToMacControlInfo.h \
	$(VEINSGIT_PROJ)/src/base/utils/PassedMessage.h \
	$(VEINSGIT_PROJ)/src/base/utils/SimpleAddress.h \
	$(VEINSGIT_PROJ)/src/base/utils/miximkerneldefs.h \
	$(VEINSGIT_PROJ)/src/inet/Coord.h \
	$(VEINSGIT_PROJ)/src/inet/FWMath.h \
	$(VEINSGIT_PROJ)/src/modules/mac/ieee80211p/WaveAppToMac1609_4Interface.h \
	$(VEINSGIT_PROJ)/src/modules/messages/Mac80211Pkt_m.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIBuffer.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIColor.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCICommandInterface.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIConnection.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIConstants.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCICoord.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIMobility.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIScenarioManager.h \
	$(VEINSGIT_PROJ)/src/modules/obstacle/Obstacle.h \
	$(VEINSGIT_PROJ)/src/modules/obstacle/ObstacleControl.h \
	$(VEINSGIT_PROJ)/src/modules/utility/Consts80211p.h \
	$(VEINSGIT_PROJ)/src/modules/world/annotations/AnnotationManager.h
$O/ApplV_01_Base.o: ApplV_01_Base.cc \
	Appl.h \
	ApplV_01_Base.h \
	Global_01_TraCI_Extend.h \
	msg/Messages_m.h \
	$(VEINSGIT_PROJ)/src/base/connectionManager/BaseConnectionManager.h \
	$(VEINSGIT_PROJ)/src/base/connectionManager/ChannelAccess.h \
	$(VEINSGIT_PROJ)/src/base/connectionManager/NicEntry.h \
	$(VEINSGIT_PROJ)/src/base/messages/MacPkt_m.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseApplLayer.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseBattery.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseLayer.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseMobility.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseModule.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseWorldUtility.h \
	$(VEINSGIT_PROJ)/src/base/modules/BatteryAccess.h \
	$(VEINSGIT_PROJ)/src/base/utils/FindModule.h \
	$(VEINSGIT_PROJ)/src/base/utils/HostState.h \
	$(VEINSGIT_PROJ)/src/base/utils/MiXiMDefs.h \
	$(VEINSGIT_PROJ)/src/base/utils/Move.h \
	$(VEINSGIT_PROJ)/src/base/utils/NetwToMacControlInfo.h \
	$(VEINSGIT_PROJ)/src/base/utils/PassedMessage.h \
	$(VEINSGIT_PROJ)/src/base/utils/SimpleAddress.h \
	$(VEINSGIT_PROJ)/src/base/utils/miximkerneldefs.h \
	$(VEINSGIT_PROJ)/src/inet/Coord.h \
	$(VEINSGIT_PROJ)/src/inet/FWMath.h \
	$(VEINSGIT_PROJ)/src/modules/mac/ieee80211p/WaveAppToMac1609_4Interface.h \
	$(VEINSGIT_PROJ)/src/modules/messages/Mac80211Pkt_m.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIBuffer.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIColor.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCICommandInterface.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIConnection.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIConstants.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCICoord.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIMobility.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIScenarioManager.h \
	$(VEINSGIT_PROJ)/src/modules/obstacle/Obstacle.h \
	$(VEINSGIT_PROJ)/src/modules/obstacle/ObstacleControl.h \
	$(VEINSGIT_PROJ)/src/modules/utility/Consts80211p.h \
	$(VEINSGIT_PROJ)/src/modules/world/annotations/AnnotationManager.h
$O/ApplV_02_Beacon.o: ApplV_02_Beacon.cc \
	Appl.h \
	ApplV_01_Base.h \
	ApplV_02_Beacon.h \
	Global_01_TraCI_Extend.h \
	msg/Messages_m.h \
	$(VEINSGIT_PROJ)/src/base/connectionManager/BaseConnectionManager.h \
	$(VEINSGIT_PROJ)/src/base/connectionManager/ChannelAccess.h \
	$(VEINSGIT_PROJ)/src/base/connectionManager/NicEntry.h \
	$(VEINSGIT_PROJ)/src/base/messages/MacPkt_m.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseApplLayer.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseBattery.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseLayer.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseMobility.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseModule.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseWorldUtility.h \
	$(VEINSGIT_PROJ)/src/base/modules/BatteryAccess.h \
	$(VEINSGIT_PROJ)/src/base/utils/FindModule.h \
	$(VEINSGIT_PROJ)/src/base/utils/HostState.h \
	$(VEINSGIT_PROJ)/src/base/utils/MiXiMDefs.h \
	$(VEINSGIT_PROJ)/src/base/utils/Move.h \
	$(VEINSGIT_PROJ)/src/base/utils/NetwToMacControlInfo.h \
	$(VEINSGIT_PROJ)/src/base/utils/PassedMessage.h \
	$(VEINSGIT_PROJ)/src/base/utils/SimpleAddress.h \
	$(VEINSGIT_PROJ)/src/base/utils/miximkerneldefs.h \
	$(VEINSGIT_PROJ)/src/inet/Coord.h \
	$(VEINSGIT_PROJ)/src/inet/FWMath.h \
	$(VEINSGIT_PROJ)/src/modules/mac/ieee80211p/WaveAppToMac1609_4Interface.h \
	$(VEINSGIT_PROJ)/src/modules/messages/Mac80211Pkt_m.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIBuffer.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIColor.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCICommandInterface.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIConnection.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIConstants.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCICoord.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIMobility.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIScenarioManager.h \
	$(VEINSGIT_PROJ)/src/modules/obstacle/Obstacle.h \
	$(VEINSGIT_PROJ)/src/modules/obstacle/ObstacleControl.h \
	$(VEINSGIT_PROJ)/src/modules/utility/Consts80211p.h \
	$(VEINSGIT_PROJ)/src/modules/world/annotations/AnnotationManager.h
$O/ApplV_03_System.o: ApplV_03_System.cc \
	Appl.h \
	ApplV_01_Base.h \
	ApplV_02_Beacon.h \
	ApplV_03_System.h \
	Global_01_TraCI_Extend.h \
	msg/Messages_m.h \
	$(VEINSGIT_PROJ)/src/base/connectionManager/BaseConnectionManager.h \
	$(VEINSGIT_PROJ)/src/base/connectionManager/ChannelAccess.h \
	$(VEINSGIT_PROJ)/src/base/connectionManager/NicEntry.h \
	$(VEINSGIT_PROJ)/src/base/messages/MacPkt_m.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseApplLayer.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseBattery.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseLayer.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseMobility.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseModule.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseWorldUtility.h \
	$(VEINSGIT_PROJ)/src/base/modules/BatteryAccess.h \
	$(VEINSGIT_PROJ)/src/base/utils/FindModule.h \
	$(VEINSGIT_PROJ)/src/base/utils/HostState.h \
	$(VEINSGIT_PROJ)/src/base/utils/MiXiMDefs.h \
	$(VEINSGIT_PROJ)/src/base/utils/Move.h \
	$(VEINSGIT_PROJ)/src/base/utils/NetwToMacControlInfo.h \
	$(VEINSGIT_PROJ)/src/base/utils/PassedMessage.h \
	$(VEINSGIT_PROJ)/src/base/utils/SimpleAddress.h \
	$(VEINSGIT_PROJ)/src/base/utils/miximkerneldefs.h \
	$(VEINSGIT_PROJ)/src/inet/Coord.h \
	$(VEINSGIT_PROJ)/src/inet/FWMath.h \
	$(VEINSGIT_PROJ)/src/modules/mac/ieee80211p/WaveAppToMac1609_4Interface.h \
	$(VEINSGIT_PROJ)/src/modules/messages/Mac80211Pkt_m.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIBuffer.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIColor.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCICommandInterface.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIConnection.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIConstants.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCICoord.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIMobility.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIScenarioManager.h \
	$(VEINSGIT_PROJ)/src/modules/obstacle/Obstacle.h \
	$(VEINSGIT_PROJ)/src/modules/obstacle/ObstacleControl.h \
	$(VEINSGIT_PROJ)/src/modules/utility/Consts80211p.h \
	$(VEINSGIT_PROJ)/src/modules/world/annotations/AnnotationManager.h
$O/ApplV_04_AID.o: ApplV_04_AID.cc \
	Appl.h \
	ApplV_01_Base.h \
	ApplV_02_Beacon.h \
	ApplV_03_System.h \
	ApplV_04_AID.h \
	Global_01_TraCI_Extend.h \
	msg/Messages_m.h \
	$(VEINSGIT_PROJ)/src/base/connectionManager/BaseConnectionManager.h \
	$(VEINSGIT_PROJ)/src/base/connectionManager/ChannelAccess.h \
	$(VEINSGIT_PROJ)/src/base/connectionManager/NicEntry.h \
	$(VEINSGIT_PROJ)/src/base/messages/MacPkt_m.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseApplLayer.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseBattery.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseLayer.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseMobility.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseModule.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseWorldUtility.h \
	$(VEINSGIT_PROJ)/src/base/modules/BatteryAccess.h \
	$(VEINSGIT_PROJ)/src/base/utils/FindModule.h \
	$(VEINSGIT_PROJ)/src/base/utils/HostState.h \
	$(VEINSGIT_PROJ)/src/base/utils/MiXiMDefs.h \
	$(VEINSGIT_PROJ)/src/base/utils/Move.h \
	$(VEINSGIT_PROJ)/src/base/utils/NetwToMacControlInfo.h \
	$(VEINSGIT_PROJ)/src/base/utils/PassedMessage.h \
	$(VEINSGIT_PROJ)/src/base/utils/SimpleAddress.h \
	$(VEINSGIT_PROJ)/src/base/utils/miximkerneldefs.h \
	$(VEINSGIT_PROJ)/src/inet/Coord.h \
	$(VEINSGIT_PROJ)/src/inet/FWMath.h \
	$(VEINSGIT_PROJ)/src/modules/mac/ieee80211p/WaveAppToMac1609_4Interface.h \
	$(VEINSGIT_PROJ)/src/modules/messages/Mac80211Pkt_m.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIBuffer.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIColor.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCICommandInterface.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIConnection.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIConstants.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCICoord.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIMobility.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIScenarioManager.h \
	$(VEINSGIT_PROJ)/src/modules/obstacle/Obstacle.h \
	$(VEINSGIT_PROJ)/src/modules/obstacle/ObstacleControl.h \
	$(VEINSGIT_PROJ)/src/modules/utility/Consts80211p.h \
	$(VEINSGIT_PROJ)/src/modules/world/annotations/AnnotationManager.h
$O/ApplV_05_PlatoonMg.o: ApplV_05_PlatoonMg.cc \
	Appl.h \
	ApplV_01_Base.h \
	ApplV_02_Beacon.h \
	ApplV_03_System.h \
	ApplV_04_AID.h \
	ApplV_05_PlatoonMg.h \
	Global_01_TraCI_Extend.h \
	msg/Messages_m.h \
	$(VEINSGIT_PROJ)/src/base/connectionManager/BaseConnectionManager.h \
	$(VEINSGIT_PROJ)/src/base/connectionManager/ChannelAccess.h \
	$(VEINSGIT_PROJ)/src/base/connectionManager/NicEntry.h \
	$(VEINSGIT_PROJ)/src/base/messages/MacPkt_m.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseApplLayer.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseBattery.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseLayer.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseMobility.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseModule.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseWorldUtility.h \
	$(VEINSGIT_PROJ)/src/base/modules/BatteryAccess.h \
	$(VEINSGIT_PROJ)/src/base/utils/FindModule.h \
	$(VEINSGIT_PROJ)/src/base/utils/HostState.h \
	$(VEINSGIT_PROJ)/src/base/utils/MiXiMDefs.h \
	$(VEINSGIT_PROJ)/src/base/utils/Move.h \
	$(VEINSGIT_PROJ)/src/base/utils/NetwToMacControlInfo.h \
	$(VEINSGIT_PROJ)/src/base/utils/PassedMessage.h \
	$(VEINSGIT_PROJ)/src/base/utils/SimpleAddress.h \
	$(VEINSGIT_PROJ)/src/base/utils/miximkerneldefs.h \
	$(VEINSGIT_PROJ)/src/inet/Coord.h \
	$(VEINSGIT_PROJ)/src/inet/FWMath.h \
	$(VEINSGIT_PROJ)/src/modules/mac/ieee80211p/WaveAppToMac1609_4Interface.h \
	$(VEINSGIT_PROJ)/src/modules/messages/Mac80211Pkt_m.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIBuffer.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIColor.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCICommandInterface.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIConnection.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIConstants.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCICoord.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIMobility.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIScenarioManager.h \
	$(VEINSGIT_PROJ)/src/modules/obstacle/Obstacle.h \
	$(VEINSGIT_PROJ)/src/modules/obstacle/ObstacleControl.h \
	$(VEINSGIT_PROJ)/src/modules/utility/Consts80211p.h \
	$(VEINSGIT_PROJ)/src/modules/world/annotations/AnnotationManager.h
$O/ApplV_05a_Entry.o: ApplV_05a_Entry.cc \
	Appl.h \
	ApplV_01_Base.h \
	ApplV_02_Beacon.h \
	ApplV_03_System.h \
	ApplV_04_AID.h \
	ApplV_05_PlatoonMg.h \
	Global_01_TraCI_Extend.h \
	msg/Messages_m.h \
	$(VEINSGIT_PROJ)/src/base/connectionManager/BaseConnectionManager.h \
	$(VEINSGIT_PROJ)/src/base/connectionManager/ChannelAccess.h \
	$(VEINSGIT_PROJ)/src/base/connectionManager/NicEntry.h \
	$(VEINSGIT_PROJ)/src/base/messages/MacPkt_m.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseApplLayer.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseBattery.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseLayer.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseMobility.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseModule.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseWorldUtility.h \
	$(VEINSGIT_PROJ)/src/base/modules/BatteryAccess.h \
	$(VEINSGIT_PROJ)/src/base/utils/FindModule.h \
	$(VEINSGIT_PROJ)/src/base/utils/HostState.h \
	$(VEINSGIT_PROJ)/src/base/utils/MiXiMDefs.h \
	$(VEINSGIT_PROJ)/src/base/utils/Move.h \
	$(VEINSGIT_PROJ)/src/base/utils/NetwToMacControlInfo.h \
	$(VEINSGIT_PROJ)/src/base/utils/PassedMessage.h \
	$(VEINSGIT_PROJ)/src/base/utils/SimpleAddress.h \
	$(VEINSGIT_PROJ)/src/base/utils/miximkerneldefs.h \
	$(VEINSGIT_PROJ)/src/inet/Coord.h \
	$(VEINSGIT_PROJ)/src/inet/FWMath.h \
	$(VEINSGIT_PROJ)/src/modules/mac/ieee80211p/WaveAppToMac1609_4Interface.h \
	$(VEINSGIT_PROJ)/src/modules/messages/Mac80211Pkt_m.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIBuffer.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIColor.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCICommandInterface.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIConnection.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIConstants.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCICoord.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIMobility.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIScenarioManager.h \
	$(VEINSGIT_PROJ)/src/modules/obstacle/Obstacle.h \
	$(VEINSGIT_PROJ)/src/modules/obstacle/ObstacleControl.h \
	$(VEINSGIT_PROJ)/src/modules/utility/Consts80211p.h \
	$(VEINSGIT_PROJ)/src/modules/world/annotations/AnnotationManager.h
$O/ApplV_05b_Merge.o: ApplV_05b_Merge.cc \
	Appl.h \
	ApplV_01_Base.h \
	ApplV_02_Beacon.h \
	ApplV_03_System.h \
	ApplV_04_AID.h \
	ApplV_05_PlatoonMg.h \
	Global_01_TraCI_Extend.h \
	msg/Messages_m.h \
	$(VEINSGIT_PROJ)/src/base/connectionManager/BaseConnectionManager.h \
	$(VEINSGIT_PROJ)/src/base/connectionManager/ChannelAccess.h \
	$(VEINSGIT_PROJ)/src/base/connectionManager/NicEntry.h \
	$(VEINSGIT_PROJ)/src/base/messages/MacPkt_m.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseApplLayer.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseBattery.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseLayer.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseMobility.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseModule.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseWorldUtility.h \
	$(VEINSGIT_PROJ)/src/base/modules/BatteryAccess.h \
	$(VEINSGIT_PROJ)/src/base/utils/FindModule.h \
	$(VEINSGIT_PROJ)/src/base/utils/HostState.h \
	$(VEINSGIT_PROJ)/src/base/utils/MiXiMDefs.h \
	$(VEINSGIT_PROJ)/src/base/utils/Move.h \
	$(VEINSGIT_PROJ)/src/base/utils/NetwToMacControlInfo.h \
	$(VEINSGIT_PROJ)/src/base/utils/PassedMessage.h \
	$(VEINSGIT_PROJ)/src/base/utils/SimpleAddress.h \
	$(VEINSGIT_PROJ)/src/base/utils/miximkerneldefs.h \
	$(VEINSGIT_PROJ)/src/inet/Coord.h \
	$(VEINSGIT_PROJ)/src/inet/FWMath.h \
	$(VEINSGIT_PROJ)/src/modules/mac/ieee80211p/WaveAppToMac1609_4Interface.h \
	$(VEINSGIT_PROJ)/src/modules/messages/Mac80211Pkt_m.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIBuffer.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIColor.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCICommandInterface.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIConnection.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIConstants.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCICoord.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIMobility.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIScenarioManager.h \
	$(VEINSGIT_PROJ)/src/modules/obstacle/Obstacle.h \
	$(VEINSGIT_PROJ)/src/modules/obstacle/ObstacleControl.h \
	$(VEINSGIT_PROJ)/src/modules/utility/Consts80211p.h \
	$(VEINSGIT_PROJ)/src/modules/world/annotations/AnnotationManager.h
$O/ApplV_05c_Split.o: ApplV_05c_Split.cc \
	Appl.h \
	ApplV_01_Base.h \
	ApplV_02_Beacon.h \
	ApplV_03_System.h \
	ApplV_04_AID.h \
	ApplV_05_PlatoonMg.h \
	Global_01_TraCI_Extend.h \
	msg/Messages_m.h \
	$(VEINSGIT_PROJ)/src/base/connectionManager/BaseConnectionManager.h \
	$(VEINSGIT_PROJ)/src/base/connectionManager/ChannelAccess.h \
	$(VEINSGIT_PROJ)/src/base/connectionManager/NicEntry.h \
	$(VEINSGIT_PROJ)/src/base/messages/MacPkt_m.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseApplLayer.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseBattery.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseLayer.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseMobility.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseModule.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseWorldUtility.h \
	$(VEINSGIT_PROJ)/src/base/modules/BatteryAccess.h \
	$(VEINSGIT_PROJ)/src/base/utils/FindModule.h \
	$(VEINSGIT_PROJ)/src/base/utils/HostState.h \
	$(VEINSGIT_PROJ)/src/base/utils/MiXiMDefs.h \
	$(VEINSGIT_PROJ)/src/base/utils/Move.h \
	$(VEINSGIT_PROJ)/src/base/utils/NetwToMacControlInfo.h \
	$(VEINSGIT_PROJ)/src/base/utils/PassedMessage.h \
	$(VEINSGIT_PROJ)/src/base/utils/SimpleAddress.h \
	$(VEINSGIT_PROJ)/src/base/utils/miximkerneldefs.h \
	$(VEINSGIT_PROJ)/src/inet/Coord.h \
	$(VEINSGIT_PROJ)/src/inet/FWMath.h \
	$(VEINSGIT_PROJ)/src/modules/mac/ieee80211p/WaveAppToMac1609_4Interface.h \
	$(VEINSGIT_PROJ)/src/modules/messages/Mac80211Pkt_m.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIBuffer.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIColor.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCICommandInterface.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIConnection.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIConstants.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCICoord.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIMobility.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIScenarioManager.h \
	$(VEINSGIT_PROJ)/src/modules/obstacle/Obstacle.h \
	$(VEINSGIT_PROJ)/src/modules/obstacle/ObstacleControl.h \
	$(VEINSGIT_PROJ)/src/modules/utility/Consts80211p.h \
	$(VEINSGIT_PROJ)/src/modules/world/annotations/AnnotationManager.h
$O/ApplV_05d_FollowerLeave.o: ApplV_05d_FollowerLeave.cc \
	Appl.h \
	ApplV_01_Base.h \
	ApplV_02_Beacon.h \
	ApplV_03_System.h \
	ApplV_04_AID.h \
	ApplV_05_PlatoonMg.h \
	Global_01_TraCI_Extend.h \
	msg/Messages_m.h \
	$(VEINSGIT_PROJ)/src/base/connectionManager/BaseConnectionManager.h \
	$(VEINSGIT_PROJ)/src/base/connectionManager/ChannelAccess.h \
	$(VEINSGIT_PROJ)/src/base/connectionManager/NicEntry.h \
	$(VEINSGIT_PROJ)/src/base/messages/MacPkt_m.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseApplLayer.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseBattery.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseLayer.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseMobility.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseModule.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseWorldUtility.h \
	$(VEINSGIT_PROJ)/src/base/modules/BatteryAccess.h \
	$(VEINSGIT_PROJ)/src/base/utils/FindModule.h \
	$(VEINSGIT_PROJ)/src/base/utils/HostState.h \
	$(VEINSGIT_PROJ)/src/base/utils/MiXiMDefs.h \
	$(VEINSGIT_PROJ)/src/base/utils/Move.h \
	$(VEINSGIT_PROJ)/src/base/utils/NetwToMacControlInfo.h \
	$(VEINSGIT_PROJ)/src/base/utils/PassedMessage.h \
	$(VEINSGIT_PROJ)/src/base/utils/SimpleAddress.h \
	$(VEINSGIT_PROJ)/src/base/utils/miximkerneldefs.h \
	$(VEINSGIT_PROJ)/src/inet/Coord.h \
	$(VEINSGIT_PROJ)/src/inet/FWMath.h \
	$(VEINSGIT_PROJ)/src/modules/mac/ieee80211p/WaveAppToMac1609_4Interface.h \
	$(VEINSGIT_PROJ)/src/modules/messages/Mac80211Pkt_m.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIBuffer.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIColor.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCICommandInterface.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIConnection.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIConstants.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCICoord.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIMobility.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIScenarioManager.h \
	$(VEINSGIT_PROJ)/src/modules/obstacle/Obstacle.h \
	$(VEINSGIT_PROJ)/src/modules/obstacle/ObstacleControl.h \
	$(VEINSGIT_PROJ)/src/modules/utility/Consts80211p.h \
	$(VEINSGIT_PROJ)/src/modules/world/annotations/AnnotationManager.h
$O/ApplV_05e_LeaderLeave.o: ApplV_05e_LeaderLeave.cc \
	Appl.h \
	ApplV_01_Base.h \
	ApplV_02_Beacon.h \
	ApplV_03_System.h \
	ApplV_04_AID.h \
	ApplV_05_PlatoonMg.h \
	Global_01_TraCI_Extend.h \
	msg/Messages_m.h \
	$(VEINSGIT_PROJ)/src/base/connectionManager/BaseConnectionManager.h \
	$(VEINSGIT_PROJ)/src/base/connectionManager/ChannelAccess.h \
	$(VEINSGIT_PROJ)/src/base/connectionManager/NicEntry.h \
	$(VEINSGIT_PROJ)/src/base/messages/MacPkt_m.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseApplLayer.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseBattery.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseLayer.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseMobility.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseModule.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseWorldUtility.h \
	$(VEINSGIT_PROJ)/src/base/modules/BatteryAccess.h \
	$(VEINSGIT_PROJ)/src/base/utils/FindModule.h \
	$(VEINSGIT_PROJ)/src/base/utils/HostState.h \
	$(VEINSGIT_PROJ)/src/base/utils/MiXiMDefs.h \
	$(VEINSGIT_PROJ)/src/base/utils/Move.h \
	$(VEINSGIT_PROJ)/src/base/utils/NetwToMacControlInfo.h \
	$(VEINSGIT_PROJ)/src/base/utils/PassedMessage.h \
	$(VEINSGIT_PROJ)/src/base/utils/SimpleAddress.h \
	$(VEINSGIT_PROJ)/src/base/utils/miximkerneldefs.h \
	$(VEINSGIT_PROJ)/src/inet/Coord.h \
	$(VEINSGIT_PROJ)/src/inet/FWMath.h \
	$(VEINSGIT_PROJ)/src/modules/mac/ieee80211p/WaveAppToMac1609_4Interface.h \
	$(VEINSGIT_PROJ)/src/modules/messages/Mac80211Pkt_m.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIBuffer.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIColor.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCICommandInterface.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIConnection.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIConstants.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCICoord.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIMobility.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIScenarioManager.h \
	$(VEINSGIT_PROJ)/src/modules/obstacle/Obstacle.h \
	$(VEINSGIT_PROJ)/src/modules/obstacle/ObstacleControl.h \
	$(VEINSGIT_PROJ)/src/modules/utility/Consts80211p.h \
	$(VEINSGIT_PROJ)/src/modules/world/annotations/AnnotationManager.h
$O/ApplV_06_Manager.o: ApplV_06_Manager.cc \
	Appl.h \
	ApplV_01_Base.h \
	ApplV_02_Beacon.h \
	ApplV_03_System.h \
	ApplV_04_AID.h \
	ApplV_05_PlatoonMg.h \
	ApplV_06_Manager.h \
	Global_01_TraCI_Extend.h \
	msg/Messages_m.h \
	$(VEINSGIT_PROJ)/src/base/connectionManager/BaseConnectionManager.h \
	$(VEINSGIT_PROJ)/src/base/connectionManager/ChannelAccess.h \
	$(VEINSGIT_PROJ)/src/base/connectionManager/NicEntry.h \
	$(VEINSGIT_PROJ)/src/base/messages/MacPkt_m.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseApplLayer.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseBattery.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseLayer.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseMobility.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseModule.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseWorldUtility.h \
	$(VEINSGIT_PROJ)/src/base/modules/BatteryAccess.h \
	$(VEINSGIT_PROJ)/src/base/utils/FindModule.h \
	$(VEINSGIT_PROJ)/src/base/utils/HostState.h \
	$(VEINSGIT_PROJ)/src/base/utils/MiXiMDefs.h \
	$(VEINSGIT_PROJ)/src/base/utils/Move.h \
	$(VEINSGIT_PROJ)/src/base/utils/NetwToMacControlInfo.h \
	$(VEINSGIT_PROJ)/src/base/utils/PassedMessage.h \
	$(VEINSGIT_PROJ)/src/base/utils/SimpleAddress.h \
	$(VEINSGIT_PROJ)/src/base/utils/miximkerneldefs.h \
	$(VEINSGIT_PROJ)/src/inet/Coord.h \
	$(VEINSGIT_PROJ)/src/inet/FWMath.h \
	$(VEINSGIT_PROJ)/src/modules/mac/ieee80211p/WaveAppToMac1609_4Interface.h \
	$(VEINSGIT_PROJ)/src/modules/messages/Mac80211Pkt_m.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIBuffer.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIColor.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCICommandInterface.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIConnection.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIConstants.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCICoord.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIMobility.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIScenarioManager.h \
	$(VEINSGIT_PROJ)/src/modules/obstacle/Obstacle.h \
	$(VEINSGIT_PROJ)/src/modules/obstacle/ObstacleControl.h \
	$(VEINSGIT_PROJ)/src/modules/utility/Consts80211p.h \
	$(VEINSGIT_PROJ)/src/modules/world/annotations/AnnotationManager.h
$O/Global_01_TraCI_Extend.o: Global_01_TraCI_Extend.cc \
	Appl.h \
	Global_01_TraCI_Extend.h \
	$(VEINSGIT_PROJ)/src/base/connectionManager/BaseConnectionManager.h \
	$(VEINSGIT_PROJ)/src/base/connectionManager/NicEntry.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseBattery.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseMobility.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseModule.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseWorldUtility.h \
	$(VEINSGIT_PROJ)/src/base/modules/BatteryAccess.h \
	$(VEINSGIT_PROJ)/src/base/utils/FindModule.h \
	$(VEINSGIT_PROJ)/src/base/utils/HostState.h \
	$(VEINSGIT_PROJ)/src/base/utils/MiXiMDefs.h \
	$(VEINSGIT_PROJ)/src/base/utils/Move.h \
	$(VEINSGIT_PROJ)/src/base/utils/miximkerneldefs.h \
	$(VEINSGIT_PROJ)/src/inet/Coord.h \
	$(VEINSGIT_PROJ)/src/inet/FWMath.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIBuffer.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIColor.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCICommandInterface.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIConnection.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIConstants.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCICoord.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIMobility.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIScenarioManager.h \
	$(VEINSGIT_PROJ)/src/modules/obstacle/Obstacle.h \
	$(VEINSGIT_PROJ)/src/modules/obstacle/ObstacleControl.h \
	$(VEINSGIT_PROJ)/src/modules/world/annotations/AnnotationManager.h
$O/Global_02_TraCI_App.o: Global_02_TraCI_App.cc \
	Appl.h \
	ApplRSU_01_Base.h \
	ApplRSU_02_AID.h \
	ApplRSU_03_Manager.h \
	ApplV_01_Base.h \
	ApplV_02_Beacon.h \
	ApplV_03_System.h \
	ApplV_04_AID.h \
	ApplV_05_PlatoonMg.h \
	ApplV_06_Manager.h \
	Global_01_TraCI_Extend.h \
	Global_02_TraCI_App.h \
	Global_03_Statistics.h \
	Global_04_VehicleAdd.h \
	Global_05_Warmup.h \
	Global_06_SpeedProfile.h \
	Global_08_TrafficLight.h \
	msg/Messages_m.h \
	$(VEINSGIT_PROJ)/src/base/connectionManager/BaseConnectionManager.h \
	$(VEINSGIT_PROJ)/src/base/connectionManager/ChannelAccess.h \
	$(VEINSGIT_PROJ)/src/base/connectionManager/NicEntry.h \
	$(VEINSGIT_PROJ)/src/base/messages/MacPkt_m.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseApplLayer.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseBattery.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseLayer.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseMobility.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseModule.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseWorldUtility.h \
	$(VEINSGIT_PROJ)/src/base/modules/BatteryAccess.h \
	$(VEINSGIT_PROJ)/src/base/utils/FindModule.h \
	$(VEINSGIT_PROJ)/src/base/utils/HostState.h \
	$(VEINSGIT_PROJ)/src/base/utils/MiXiMDefs.h \
	$(VEINSGIT_PROJ)/src/base/utils/Move.h \
	$(VEINSGIT_PROJ)/src/base/utils/NetwToMacControlInfo.h \
	$(VEINSGIT_PROJ)/src/base/utils/PassedMessage.h \
	$(VEINSGIT_PROJ)/src/base/utils/SimpleAddress.h \
	$(VEINSGIT_PROJ)/src/base/utils/miximkerneldefs.h \
	$(VEINSGIT_PROJ)/src/inet/Coord.h \
	$(VEINSGIT_PROJ)/src/inet/FWMath.h \
	$(VEINSGIT_PROJ)/src/modules/mac/ieee80211p/WaveAppToMac1609_4Interface.h \
	$(VEINSGIT_PROJ)/src/modules/messages/Mac80211Pkt_m.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIBuffer.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIColor.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCICommandInterface.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIConnection.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIConstants.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCICoord.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIMobility.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIScenarioManager.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIScenarioManagerLaunchd.h \
	$(VEINSGIT_PROJ)/src/modules/obstacle/Obstacle.h \
	$(VEINSGIT_PROJ)/src/modules/obstacle/ObstacleControl.h \
	$(VEINSGIT_PROJ)/src/modules/utility/Consts80211p.h \
	$(VEINSGIT_PROJ)/src/modules/world/annotations/AnnotationManager.h
$O/Global_03_Statistics.o: Global_03_Statistics.cc \
	Appl.h \
	ApplRSU_01_Base.h \
	ApplRSU_02_AID.h \
	ApplRSU_03_Manager.h \
	Global_01_TraCI_Extend.h \
	Global_03_Statistics.h \
	msg/Messages_m.h \
	$(VEINSGIT_PROJ)/src/base/connectionManager/BaseConnectionManager.h \
	$(VEINSGIT_PROJ)/src/base/connectionManager/ChannelAccess.h \
	$(VEINSGIT_PROJ)/src/base/connectionManager/NicEntry.h \
	$(VEINSGIT_PROJ)/src/base/messages/MacPkt_m.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseApplLayer.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseBattery.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseLayer.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseMobility.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseModule.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseWorldUtility.h \
	$(VEINSGIT_PROJ)/src/base/modules/BatteryAccess.h \
	$(VEINSGIT_PROJ)/src/base/utils/FindModule.h \
	$(VEINSGIT_PROJ)/src/base/utils/HostState.h \
	$(VEINSGIT_PROJ)/src/base/utils/MiXiMDefs.h \
	$(VEINSGIT_PROJ)/src/base/utils/Move.h \
	$(VEINSGIT_PROJ)/src/base/utils/NetwToMacControlInfo.h \
	$(VEINSGIT_PROJ)/src/base/utils/PassedMessage.h \
	$(VEINSGIT_PROJ)/src/base/utils/SimpleAddress.h \
	$(VEINSGIT_PROJ)/src/base/utils/miximkerneldefs.h \
	$(VEINSGIT_PROJ)/src/inet/Coord.h \
	$(VEINSGIT_PROJ)/src/inet/FWMath.h \
	$(VEINSGIT_PROJ)/src/modules/mac/ieee80211p/WaveAppToMac1609_4Interface.h \
	$(VEINSGIT_PROJ)/src/modules/messages/Mac80211Pkt_m.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIBuffer.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIColor.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCICommandInterface.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIConnection.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIConstants.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCICoord.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIMobility.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIScenarioManager.h \
	$(VEINSGIT_PROJ)/src/modules/obstacle/Obstacle.h \
	$(VEINSGIT_PROJ)/src/modules/obstacle/ObstacleControl.h \
	$(VEINSGIT_PROJ)/src/modules/utility/Consts80211p.h \
	$(VEINSGIT_PROJ)/src/modules/world/annotations/AnnotationManager.h
$O/Global_04_VehicleAdd.o: Global_04_VehicleAdd.cc \
	Appl.h \
	ApplV_01_Base.h \
	ApplV_02_Beacon.h \
	ApplV_03_System.h \
	ApplV_04_AID.h \
	ApplV_05_PlatoonMg.h \
	ApplV_06_Manager.h \
	Global_01_TraCI_Extend.h \
	Global_04_VehicleAdd.h \
	msg/Messages_m.h \
	$(VEINSGIT_PROJ)/src/base/connectionManager/BaseConnectionManager.h \
	$(VEINSGIT_PROJ)/src/base/connectionManager/ChannelAccess.h \
	$(VEINSGIT_PROJ)/src/base/connectionManager/NicEntry.h \
	$(VEINSGIT_PROJ)/src/base/messages/MacPkt_m.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseApplLayer.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseBattery.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseLayer.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseMobility.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseModule.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseWorldUtility.h \
	$(VEINSGIT_PROJ)/src/base/modules/BatteryAccess.h \
	$(VEINSGIT_PROJ)/src/base/utils/FindModule.h \
	$(VEINSGIT_PROJ)/src/base/utils/HostState.h \
	$(VEINSGIT_PROJ)/src/base/utils/MiXiMDefs.h \
	$(VEINSGIT_PROJ)/src/base/utils/Move.h \
	$(VEINSGIT_PROJ)/src/base/utils/NetwToMacControlInfo.h \
	$(VEINSGIT_PROJ)/src/base/utils/PassedMessage.h \
	$(VEINSGIT_PROJ)/src/base/utils/SimpleAddress.h \
	$(VEINSGIT_PROJ)/src/base/utils/miximkerneldefs.h \
	$(VEINSGIT_PROJ)/src/inet/Coord.h \
	$(VEINSGIT_PROJ)/src/inet/FWMath.h \
	$(VEINSGIT_PROJ)/src/modules/mac/ieee80211p/WaveAppToMac1609_4Interface.h \
	$(VEINSGIT_PROJ)/src/modules/messages/Mac80211Pkt_m.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIBuffer.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIColor.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCICommandInterface.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIConnection.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIConstants.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCICoord.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIMobility.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIScenarioManager.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIScenarioManagerLaunchd.h \
	$(VEINSGIT_PROJ)/src/modules/obstacle/Obstacle.h \
	$(VEINSGIT_PROJ)/src/modules/obstacle/ObstacleControl.h \
	$(VEINSGIT_PROJ)/src/modules/utility/Consts80211p.h \
	$(VEINSGIT_PROJ)/src/modules/world/annotations/AnnotationManager.h
$O/Global_05_Warmup.o: Global_05_Warmup.cc \
	Appl.h \
	ApplV_01_Base.h \
	ApplV_02_Beacon.h \
	ApplV_03_System.h \
	ApplV_04_AID.h \
	ApplV_05_PlatoonMg.h \
	ApplV_06_Manager.h \
	Global_01_TraCI_Extend.h \
	Global_04_VehicleAdd.h \
	Global_05_Warmup.h \
	Global_06_SpeedProfile.h \
	msg/Messages_m.h \
	$(VEINSGIT_PROJ)/src/base/connectionManager/BaseConnectionManager.h \
	$(VEINSGIT_PROJ)/src/base/connectionManager/ChannelAccess.h \
	$(VEINSGIT_PROJ)/src/base/connectionManager/NicEntry.h \
	$(VEINSGIT_PROJ)/src/base/messages/MacPkt_m.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseApplLayer.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseBattery.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseLayer.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseMobility.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseModule.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseWorldUtility.h \
	$(VEINSGIT_PROJ)/src/base/modules/BatteryAccess.h \
	$(VEINSGIT_PROJ)/src/base/utils/FindModule.h \
	$(VEINSGIT_PROJ)/src/base/utils/HostState.h \
	$(VEINSGIT_PROJ)/src/base/utils/MiXiMDefs.h \
	$(VEINSGIT_PROJ)/src/base/utils/Move.h \
	$(VEINSGIT_PROJ)/src/base/utils/NetwToMacControlInfo.h \
	$(VEINSGIT_PROJ)/src/base/utils/PassedMessage.h \
	$(VEINSGIT_PROJ)/src/base/utils/SimpleAddress.h \
	$(VEINSGIT_PROJ)/src/base/utils/miximkerneldefs.h \
	$(VEINSGIT_PROJ)/src/inet/Coord.h \
	$(VEINSGIT_PROJ)/src/inet/FWMath.h \
	$(VEINSGIT_PROJ)/src/modules/mac/ieee80211p/WaveAppToMac1609_4Interface.h \
	$(VEINSGIT_PROJ)/src/modules/messages/Mac80211Pkt_m.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIBuffer.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIColor.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCICommandInterface.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIConnection.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIConstants.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCICoord.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIMobility.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIScenarioManager.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIScenarioManagerLaunchd.h \
	$(VEINSGIT_PROJ)/src/modules/obstacle/Obstacle.h \
	$(VEINSGIT_PROJ)/src/modules/obstacle/ObstacleControl.h \
	$(VEINSGIT_PROJ)/src/modules/utility/Consts80211p.h \
	$(VEINSGIT_PROJ)/src/modules/world/annotations/AnnotationManager.h
$O/Global_06_SpeedProfile.o: Global_06_SpeedProfile.cc \
	Appl.h \
	Global_01_TraCI_Extend.h \
	Global_06_SpeedProfile.h \
	$(VEINSGIT_PROJ)/src/base/connectionManager/BaseConnectionManager.h \
	$(VEINSGIT_PROJ)/src/base/connectionManager/NicEntry.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseBattery.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseMobility.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseModule.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseWorldUtility.h \
	$(VEINSGIT_PROJ)/src/base/modules/BatteryAccess.h \
	$(VEINSGIT_PROJ)/src/base/utils/FindModule.h \
	$(VEINSGIT_PROJ)/src/base/utils/HostState.h \
	$(VEINSGIT_PROJ)/src/base/utils/MiXiMDefs.h \
	$(VEINSGIT_PROJ)/src/base/utils/Move.h \
	$(VEINSGIT_PROJ)/src/base/utils/miximkerneldefs.h \
	$(VEINSGIT_PROJ)/src/inet/Coord.h \
	$(VEINSGIT_PROJ)/src/inet/FWMath.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIBuffer.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIColor.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCICommandInterface.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIConnection.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIConstants.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCICoord.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIMobility.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIScenarioManager.h \
	$(VEINSGIT_PROJ)/src/modules/obstacle/Obstacle.h \
	$(VEINSGIT_PROJ)/src/modules/obstacle/ObstacleControl.h \
	$(VEINSGIT_PROJ)/src/modules/world/annotations/AnnotationManager.h
$O/Global_07_Router.o: Global_07_Router.cc \
	Appl.h \
	ApplV_01_Base.h \
	ApplV_02_Beacon.h \
	Global_01_TraCI_Extend.h \
	Global_07_Router.h \
	NodeEdge.h \
	msg/Messages_m.h \
	$(VEINSGIT_PROJ)/src/base/connectionManager/BaseConnectionManager.h \
	$(VEINSGIT_PROJ)/src/base/connectionManager/ChannelAccess.h \
	$(VEINSGIT_PROJ)/src/base/connectionManager/NicEntry.h \
	$(VEINSGIT_PROJ)/src/base/messages/MacPkt_m.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseApplLayer.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseBattery.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseLayer.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseMobility.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseModule.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseWorldUtility.h \
	$(VEINSGIT_PROJ)/src/base/modules/BatteryAccess.h \
	$(VEINSGIT_PROJ)/src/base/utils/FindModule.h \
	$(VEINSGIT_PROJ)/src/base/utils/HostState.h \
	$(VEINSGIT_PROJ)/src/base/utils/MiXiMDefs.h \
	$(VEINSGIT_PROJ)/src/base/utils/Move.h \
	$(VEINSGIT_PROJ)/src/base/utils/NetwToMacControlInfo.h \
	$(VEINSGIT_PROJ)/src/base/utils/PassedMessage.h \
	$(VEINSGIT_PROJ)/src/base/utils/SimpleAddress.h \
	$(VEINSGIT_PROJ)/src/base/utils/miximkerneldefs.h \
	$(VEINSGIT_PROJ)/src/inet/Coord.h \
	$(VEINSGIT_PROJ)/src/inet/FWMath.h \
	$(VEINSGIT_PROJ)/src/modules/mac/ieee80211p/WaveAppToMac1609_4Interface.h \
	$(VEINSGIT_PROJ)/src/modules/messages/Mac80211Pkt_m.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIBuffer.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIColor.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCICommandInterface.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIConnection.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIConstants.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCICoord.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIMobility.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIScenarioManager.h \
	$(VEINSGIT_PROJ)/src/modules/obstacle/Obstacle.h \
	$(VEINSGIT_PROJ)/src/modules/obstacle/ObstacleControl.h \
	$(VEINSGIT_PROJ)/src/modules/utility/Consts80211p.h \
	$(VEINSGIT_PROJ)/src/modules/world/annotations/AnnotationManager.h
$O/Global_08_TrafficLight.o: Global_08_TrafficLight.cc \
	Appl.h \
	ApplV_01_Base.h \
	ApplV_02_Beacon.h \
	ApplV_03_System.h \
	ApplV_04_AID.h \
	ApplV_05_PlatoonMg.h \
	ApplV_06_Manager.h \
	Global_01_TraCI_Extend.h \
	Global_08_TrafficLight.h \
	msg/Messages_m.h \
	$(VEINSGIT_PROJ)/src/base/connectionManager/BaseConnectionManager.h \
	$(VEINSGIT_PROJ)/src/base/connectionManager/ChannelAccess.h \
	$(VEINSGIT_PROJ)/src/base/connectionManager/NicEntry.h \
	$(VEINSGIT_PROJ)/src/base/messages/MacPkt_m.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseApplLayer.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseBattery.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseLayer.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseMobility.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseModule.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseWorldUtility.h \
	$(VEINSGIT_PROJ)/src/base/modules/BatteryAccess.h \
	$(VEINSGIT_PROJ)/src/base/utils/FindModule.h \
	$(VEINSGIT_PROJ)/src/base/utils/HostState.h \
	$(VEINSGIT_PROJ)/src/base/utils/MiXiMDefs.h \
	$(VEINSGIT_PROJ)/src/base/utils/Move.h \
	$(VEINSGIT_PROJ)/src/base/utils/NetwToMacControlInfo.h \
	$(VEINSGIT_PROJ)/src/base/utils/PassedMessage.h \
	$(VEINSGIT_PROJ)/src/base/utils/SimpleAddress.h \
	$(VEINSGIT_PROJ)/src/base/utils/miximkerneldefs.h \
	$(VEINSGIT_PROJ)/src/inet/Coord.h \
	$(VEINSGIT_PROJ)/src/inet/FWMath.h \
	$(VEINSGIT_PROJ)/src/modules/mac/ieee80211p/WaveAppToMac1609_4Interface.h \
	$(VEINSGIT_PROJ)/src/modules/messages/Mac80211Pkt_m.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIBuffer.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIColor.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCICommandInterface.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIConnection.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIConstants.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCICoord.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIMobility.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIScenarioManager.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIScenarioManagerLaunchd.h \
	$(VEINSGIT_PROJ)/src/modules/obstacle/Obstacle.h \
	$(VEINSGIT_PROJ)/src/modules/obstacle/ObstacleControl.h \
	$(VEINSGIT_PROJ)/src/modules/utility/Consts80211p.h \
	$(VEINSGIT_PROJ)/src/modules/world/annotations/AnnotationManager.h
$O/Mac1609_4_Mod.o: Mac1609_4_Mod.cc \
	Appl.h \
	Mac1609_4_Mod.h \
	$(VEINSGIT_PROJ)/src/base/connectionManager/BaseConnectionManager.h \
	$(VEINSGIT_PROJ)/src/base/connectionManager/ChannelAccess.h \
	$(VEINSGIT_PROJ)/src/base/connectionManager/NicEntry.h \
	$(VEINSGIT_PROJ)/src/base/messages/AirFrame_m.h \
	$(VEINSGIT_PROJ)/src/base/messages/ChannelSenseRequest_m.h \
	$(VEINSGIT_PROJ)/src/base/messages/MacPkt_m.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseBattery.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseLayer.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseMacLayer.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseMobility.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseModule.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseWorldUtility.h \
	$(VEINSGIT_PROJ)/src/base/modules/BatteryAccess.h \
	$(VEINSGIT_PROJ)/src/base/phyLayer/AnalogueModel.h \
	$(VEINSGIT_PROJ)/src/base/phyLayer/BaseDecider.h \
	$(VEINSGIT_PROJ)/src/base/phyLayer/BasePhyLayer.h \
	$(VEINSGIT_PROJ)/src/base/phyLayer/ChannelInfo.h \
	$(VEINSGIT_PROJ)/src/base/phyLayer/ChannelState.h \
	$(VEINSGIT_PROJ)/src/base/phyLayer/Decider.h \
	$(VEINSGIT_PROJ)/src/base/phyLayer/DeciderToPhyInterface.h \
	$(VEINSGIT_PROJ)/src/base/phyLayer/Interpolation.h \
	$(VEINSGIT_PROJ)/src/base/phyLayer/MacToPhyControlInfo.h \
	$(VEINSGIT_PROJ)/src/base/phyLayer/MacToPhyInterface.h \
	$(VEINSGIT_PROJ)/src/base/phyLayer/Mapping.h \
	$(VEINSGIT_PROJ)/src/base/phyLayer/MappingBase.h \
	$(VEINSGIT_PROJ)/src/base/phyLayer/MappingUtils.h \
	$(VEINSGIT_PROJ)/src/base/phyLayer/PhyUtils.h \
	$(VEINSGIT_PROJ)/src/base/phyLayer/Signal_.h \
	$(VEINSGIT_PROJ)/src/base/utils/FindModule.h \
	$(VEINSGIT_PROJ)/src/base/utils/HostState.h \
	$(VEINSGIT_PROJ)/src/base/utils/MiXiMDefs.h \
	$(VEINSGIT_PROJ)/src/base/utils/Move.h \
	$(VEINSGIT_PROJ)/src/base/utils/NetwToMacControlInfo.h \
	$(VEINSGIT_PROJ)/src/base/utils/PassedMessage.h \
	$(VEINSGIT_PROJ)/src/base/utils/SimpleAddress.h \
	$(VEINSGIT_PROJ)/src/base/utils/miximkerneldefs.h \
	$(VEINSGIT_PROJ)/src/inet/Coord.h \
	$(VEINSGIT_PROJ)/src/inet/FWMath.h \
	$(VEINSGIT_PROJ)/src/modules/analogueModel/JakesFading.h \
	$(VEINSGIT_PROJ)/src/modules/analogueModel/LogNormalShadowing.h \
	$(VEINSGIT_PROJ)/src/modules/analogueModel/SimplePathlossModel.h \
	$(VEINSGIT_PROJ)/src/modules/mac/ieee80211p/Mac1609_4.h \
	$(VEINSGIT_PROJ)/src/modules/mac/ieee80211p/Mac80211pToPhy11pInterface.h \
	$(VEINSGIT_PROJ)/src/modules/mac/ieee80211p/WaveAppToMac1609_4Interface.h \
	$(VEINSGIT_PROJ)/src/modules/messages/Mac80211Pkt_m.h \
	$(VEINSGIT_PROJ)/src/modules/messages/WaveShortMessage_m.h \
	$(VEINSGIT_PROJ)/src/modules/phy/Decider80211p.h \
	$(VEINSGIT_PROJ)/src/modules/phy/Decider80211pToPhy80211pInterface.h \
	$(VEINSGIT_PROJ)/src/modules/phy/PhyLayer80211p.h \
	$(VEINSGIT_PROJ)/src/modules/phy/SNRThresholdDecider.h \
	$(VEINSGIT_PROJ)/src/modules/utility/Consts80211p.h
$O/NodeEdge.o: NodeEdge.cc \
	NodeEdge.h
$O/RSUMobility.o: RSUMobility.cc \
	Appl.h \
	Global_01_TraCI_Extend.h \
	RSUMobility.h \
	$(VEINSGIT_PROJ)/src/base/connectionManager/BaseConnectionManager.h \
	$(VEINSGIT_PROJ)/src/base/connectionManager/NicEntry.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseBattery.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseMobility.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseModule.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseWorldUtility.h \
	$(VEINSGIT_PROJ)/src/base/modules/BatteryAccess.h \
	$(VEINSGIT_PROJ)/src/base/utils/FindModule.h \
	$(VEINSGIT_PROJ)/src/base/utils/HostState.h \
	$(VEINSGIT_PROJ)/src/base/utils/MiXiMDefs.h \
	$(VEINSGIT_PROJ)/src/base/utils/Move.h \
	$(VEINSGIT_PROJ)/src/base/utils/miximkerneldefs.h \
	$(VEINSGIT_PROJ)/src/inet/Coord.h \
	$(VEINSGIT_PROJ)/src/inet/FWMath.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIBuffer.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIColor.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCICommandInterface.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIConnection.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIConstants.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCICoord.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIMobility.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIScenarioManager.h \
	$(VEINSGIT_PROJ)/src/modules/obstacle/Obstacle.h \
	$(VEINSGIT_PROJ)/src/modules/obstacle/ObstacleControl.h \
	$(VEINSGIT_PROJ)/src/modules/world/annotations/AnnotationManager.h
$O/zApplVPlatoonFormation.o: zApplVPlatoonFormation.cc \
	Appl.h \
	ApplV_01_Base.h \
	ApplV_02_Beacon.h \
	ApplV_03_System.h \
	ApplV_04_AID.h \
	ApplV_05_PlatoonMg.h \
	ApplV_06_Manager.h \
	Global_01_TraCI_Extend.h \
	msg/Messages_m.h \
	zApplVPlatoonFormation.h \
	$(VEINSGIT_PROJ)/src/base/connectionManager/BaseConnectionManager.h \
	$(VEINSGIT_PROJ)/src/base/connectionManager/ChannelAccess.h \
	$(VEINSGIT_PROJ)/src/base/connectionManager/NicEntry.h \
	$(VEINSGIT_PROJ)/src/base/messages/MacPkt_m.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseApplLayer.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseBattery.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseLayer.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseMobility.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseModule.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseWorldUtility.h \
	$(VEINSGIT_PROJ)/src/base/modules/BatteryAccess.h \
	$(VEINSGIT_PROJ)/src/base/utils/FindModule.h \
	$(VEINSGIT_PROJ)/src/base/utils/HostState.h \
	$(VEINSGIT_PROJ)/src/base/utils/MiXiMDefs.h \
	$(VEINSGIT_PROJ)/src/base/utils/Move.h \
	$(VEINSGIT_PROJ)/src/base/utils/NetwToMacControlInfo.h \
	$(VEINSGIT_PROJ)/src/base/utils/PassedMessage.h \
	$(VEINSGIT_PROJ)/src/base/utils/SimpleAddress.h \
	$(VEINSGIT_PROJ)/src/base/utils/miximkerneldefs.h \
	$(VEINSGIT_PROJ)/src/inet/Coord.h \
	$(VEINSGIT_PROJ)/src/inet/FWMath.h \
	$(VEINSGIT_PROJ)/src/modules/mac/ieee80211p/WaveAppToMac1609_4Interface.h \
	$(VEINSGIT_PROJ)/src/modules/messages/Mac80211Pkt_m.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIBuffer.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIColor.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCICommandInterface.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIConnection.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIConstants.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCICoord.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIMobility.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIScenarioManager.h \
	$(VEINSGIT_PROJ)/src/modules/obstacle/Obstacle.h \
	$(VEINSGIT_PROJ)/src/modules/obstacle/ObstacleControl.h \
	$(VEINSGIT_PROJ)/src/modules/utility/Consts80211p.h \
	$(VEINSGIT_PROJ)/src/modules/world/annotations/AnnotationManager.h
$O/zApplVPlatoonFormation2.o: zApplVPlatoonFormation2.cc \
	Appl.h \
	ApplV_01_Base.h \
	ApplV_02_Beacon.h \
	ApplV_03_System.h \
	ApplV_04_AID.h \
	ApplV_05_PlatoonMg.h \
	ApplV_06_Manager.h \
	Global_01_TraCI_Extend.h \
	msg/Messages_m.h \
	zApplVPlatoonFormation.h \
	zApplVPlatoonFormation2.h \
	$(VEINSGIT_PROJ)/src/base/connectionManager/BaseConnectionManager.h \
	$(VEINSGIT_PROJ)/src/base/connectionManager/ChannelAccess.h \
	$(VEINSGIT_PROJ)/src/base/connectionManager/NicEntry.h \
	$(VEINSGIT_PROJ)/src/base/messages/MacPkt_m.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseApplLayer.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseBattery.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseLayer.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseMobility.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseModule.h \
	$(VEINSGIT_PROJ)/src/base/modules/BaseWorldUtility.h \
	$(VEINSGIT_PROJ)/src/base/modules/BatteryAccess.h \
	$(VEINSGIT_PROJ)/src/base/utils/FindModule.h \
	$(VEINSGIT_PROJ)/src/base/utils/HostState.h \
	$(VEINSGIT_PROJ)/src/base/utils/MiXiMDefs.h \
	$(VEINSGIT_PROJ)/src/base/utils/Move.h \
	$(VEINSGIT_PROJ)/src/base/utils/NetwToMacControlInfo.h \
	$(VEINSGIT_PROJ)/src/base/utils/PassedMessage.h \
	$(VEINSGIT_PROJ)/src/base/utils/SimpleAddress.h \
	$(VEINSGIT_PROJ)/src/base/utils/miximkerneldefs.h \
	$(VEINSGIT_PROJ)/src/inet/Coord.h \
	$(VEINSGIT_PROJ)/src/inet/FWMath.h \
	$(VEINSGIT_PROJ)/src/modules/mac/ieee80211p/WaveAppToMac1609_4Interface.h \
	$(VEINSGIT_PROJ)/src/modules/messages/Mac80211Pkt_m.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIBuffer.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIColor.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCICommandInterface.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIConnection.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIConstants.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCICoord.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIMobility.h \
	$(VEINSGIT_PROJ)/src/modules/mobility/traci/TraCIScenarioManager.h \
	$(VEINSGIT_PROJ)/src/modules/obstacle/Obstacle.h \
	$(VEINSGIT_PROJ)/src/modules/obstacle/ObstacleControl.h \
	$(VEINSGIT_PROJ)/src/modules/utility/Consts80211p.h \
	$(VEINSGIT_PROJ)/src/modules/world/annotations/AnnotationManager.h

