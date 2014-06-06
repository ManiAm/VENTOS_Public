#
# OMNeT++/OMNEST Makefile for libapplication
#
# This file was generated with the command:
#  opp_makemake -f --make-so -o application -O out -d msg -I/home/dylan/vgrid/VENTOS/eigen-3.2.1 -I/home/dylan/vgrid/VENTOS/rapidxml-1.13 -I../../mixim-sommer/src/modules/analogueModel -I../../mixim-sommer/src/modules/mac/ieee80211p -I../../mixim-sommer/src/modules/phy -I../../mixim-sommer/src/modules/messages -I../../mixim-sommer/src/base/utils -I../../mixim-sommer/src/base/phyLayer -I../../mixim-sommer/src/modules/utility -I../../mixim-sommer/src/modules -I../../mixim-sommer/src/base/modules -I../../mixim-sommer/src/base/messages -I../../mixim-sommer/src/base/connectionManager -I../../mixim-sommer/src/modules/obstacle -L../../mixim-sommer/out/$(CONFIGNAME)/src/modules -L../../mixim-sommer/out/$(CONFIGNAME)/src/base -L../../mixim-sommer/out/$(CONFIGNAME)/tests/testUtils -L../out/$(CONFIGNAME)/application/msg -lmsg -lmiximbase -lmiximmodules -lmiximmodules -lmiximbase -lmiximtestUtils -KMIXIM_SOMMER_PROJ=../../mixim-sommer
#

# Name of target to be created (-o option)
TARGET = libapplication$(SHARED_LIB_SUFFIX)

# C++ include paths (with -I)
INCLUDE_PATH = \
    -I../eigen-3.2.1 \
    -I../rapidxml-1.13 \
    -I../../mixim-sommer/src/modules/analogueModel \
    -I../../mixim-sommer/src/modules/mac/ieee80211p \
    -I../../mixim-sommer/src/modules/phy \
    -I../../mixim-sommer/src/modules/messages \
    -I../../mixim-sommer/src/base/utils \
    -I../../mixim-sommer/src/base/phyLayer \
    -I../../mixim-sommer/src/modules/utility \
    -I../../mixim-sommer/src/modules \
    -I../../mixim-sommer/src/base/modules \
    -I../../mixim-sommer/src/base/messages \
    -I../../mixim-sommer/src/base/connectionManager \
    -I../../mixim-sommer/src/modules/obstacle \
    -I.

# Additional object and library files to link with
EXTRA_OBJS =

# Additional libraries (-L, -l options)
LIBS = -L../../mixim-sommer/out/$(CONFIGNAME)/src/modules -L../../mixim-sommer/out/$(CONFIGNAME)/src/base -L../../mixim-sommer/out/$(CONFIGNAME)/tests/testUtils -L../out/$(CONFIGNAME)/application/msg  -lmsg -lmiximbase -lmiximmodules -lmiximmodules -lmiximbase -lmiximtestUtils
LIBS += -Wl,-rpath,`abspath ../../mixim-sommer/out/$(CONFIGNAME)/src/modules` -Wl,-rpath,`abspath ../../mixim-sommer/out/$(CONFIGNAME)/src/base` -Wl,-rpath,`abspath ../../mixim-sommer/out/$(CONFIGNAME)/tests/testUtils` -Wl,-rpath,`abspath ../out/$(CONFIGNAME)/application/msg`

# Output directory
PROJECT_OUTPUT_DIR = ../out
PROJECTRELATIVE_PATH = application
O = $(PROJECT_OUTPUT_DIR)/$(CONFIGNAME)/$(PROJECTRELATIVE_PATH)

# Object files for local .cc and .msg files
OBJS = \
    $O/Global_05_Warmup.o \
    $O/RSUMobility.o \
    $O/Global_07_Router.o \
    $O/ApplV_06_Manager.o \
    $O/Global_08_TrafficLight.o \
    $O/Mac1609_4_Mod.o \
    $O/ApplV_05_PlatoonMg.o \
    $O/ApplV_01_Base.o \
    $O/zApplVPlatoonFormation2.o \
    $O/ApplV_03_System.o \
    $O/ApplRSU.o \
    $O/Global_06_SpeedProfile.o \
    $O/NodeEdge.o \
    $O/Global_03_Statistics.o \
    $O/Global_04_VehicleAdd.o \
    $O/Global_02_TraCI_App.o \
    $O/TraCIColor.o \
    $O/zApplVPlatoonFormation.o \
    $O/ApplV_02_Beacon.o \
    $O/ApplV_04_AID.o \
    $O/ApplAdversary.o \
    $O/Global_01_TraCI_Extend.o

# Message files
MSGFILES =

# Other makefile variables (-K)
MIXIM_SOMMER_PROJ=../../mixim-sommer

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
	$(MIXIM_SOMMER_PROJ)/src/base/connectionManager/BaseConnectionManager.h \
	$(MIXIM_SOMMER_PROJ)/src/base/connectionManager/ChannelAccess.h \
	$(MIXIM_SOMMER_PROJ)/src/base/connectionManager/NicEntry.h \
	$(MIXIM_SOMMER_PROJ)/src/base/messages/MacPkt_m.h \
	$(MIXIM_SOMMER_PROJ)/src/base/modules/BaseApplLayer.h \
	$(MIXIM_SOMMER_PROJ)/src/base/modules/BaseBattery.h \
	$(MIXIM_SOMMER_PROJ)/src/base/modules/BaseLayer.h \
	$(MIXIM_SOMMER_PROJ)/src/base/modules/BaseMobility.h \
	$(MIXIM_SOMMER_PROJ)/src/base/modules/BaseModule.h \
	$(MIXIM_SOMMER_PROJ)/src/base/modules/BaseWorldUtility.h \
	$(MIXIM_SOMMER_PROJ)/src/base/modules/BatteryAccess.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/Coord.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/FWMath.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/FindModule.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/HostState.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/MiXiMDefs.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/Move.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/NetwToMacControlInfo.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/PassedMessage.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/SimpleAddress.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/miximkerneldefs.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/mac/ieee80211p/WaveAppToMac1609_4Interface.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/messages/Mac80211Pkt_m.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/mobility/traci/TraCIColor.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/mobility/traci/TraCIConstants.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/mobility/traci/TraCIMobility.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/mobility/traci/TraCIScenarioManager.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/obstacle/Obstacle.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/obstacle/ObstacleControl.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/utility/Consts80211p.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/world/annotations/AnnotationManager.h
$O/ApplRSU.o: ApplRSU.cc \
	Appl.h \
	ApplRSU.h \
	Global_01_TraCI_Extend.h \
	msg/Messages_m.h \
	$(MIXIM_SOMMER_PROJ)/src/base/connectionManager/BaseConnectionManager.h \
	$(MIXIM_SOMMER_PROJ)/src/base/connectionManager/ChannelAccess.h \
	$(MIXIM_SOMMER_PROJ)/src/base/connectionManager/NicEntry.h \
	$(MIXIM_SOMMER_PROJ)/src/base/messages/MacPkt_m.h \
	$(MIXIM_SOMMER_PROJ)/src/base/modules/BaseApplLayer.h \
	$(MIXIM_SOMMER_PROJ)/src/base/modules/BaseBattery.h \
	$(MIXIM_SOMMER_PROJ)/src/base/modules/BaseLayer.h \
	$(MIXIM_SOMMER_PROJ)/src/base/modules/BaseMobility.h \
	$(MIXIM_SOMMER_PROJ)/src/base/modules/BaseModule.h \
	$(MIXIM_SOMMER_PROJ)/src/base/modules/BaseWorldUtility.h \
	$(MIXIM_SOMMER_PROJ)/src/base/modules/BatteryAccess.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/Coord.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/FWMath.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/FindModule.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/HostState.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/MiXiMDefs.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/Move.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/NetwToMacControlInfo.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/PassedMessage.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/SimpleAddress.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/miximkerneldefs.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/mac/ieee80211p/WaveAppToMac1609_4Interface.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/messages/Mac80211Pkt_m.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/mobility/traci/TraCIColor.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/mobility/traci/TraCIConstants.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/mobility/traci/TraCIMobility.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/mobility/traci/TraCIScenarioManager.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/obstacle/Obstacle.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/obstacle/ObstacleControl.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/utility/Consts80211p.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/world/annotations/AnnotationManager.h
$O/ApplV_01_Base.o: ApplV_01_Base.cc \
	Appl.h \
	ApplV_01_Base.h \
	Global_01_TraCI_Extend.h \
	msg/Messages_m.h \
	$(MIXIM_SOMMER_PROJ)/src/base/connectionManager/BaseConnectionManager.h \
	$(MIXIM_SOMMER_PROJ)/src/base/connectionManager/ChannelAccess.h \
	$(MIXIM_SOMMER_PROJ)/src/base/connectionManager/NicEntry.h \
	$(MIXIM_SOMMER_PROJ)/src/base/messages/MacPkt_m.h \
	$(MIXIM_SOMMER_PROJ)/src/base/modules/BaseApplLayer.h \
	$(MIXIM_SOMMER_PROJ)/src/base/modules/BaseBattery.h \
	$(MIXIM_SOMMER_PROJ)/src/base/modules/BaseLayer.h \
	$(MIXIM_SOMMER_PROJ)/src/base/modules/BaseMobility.h \
	$(MIXIM_SOMMER_PROJ)/src/base/modules/BaseModule.h \
	$(MIXIM_SOMMER_PROJ)/src/base/modules/BaseWorldUtility.h \
	$(MIXIM_SOMMER_PROJ)/src/base/modules/BatteryAccess.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/Coord.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/FWMath.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/FindModule.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/HostState.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/MiXiMDefs.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/Move.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/NetwToMacControlInfo.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/PassedMessage.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/SimpleAddress.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/miximkerneldefs.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/mac/ieee80211p/WaveAppToMac1609_4Interface.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/messages/Mac80211Pkt_m.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/mobility/traci/TraCIColor.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/mobility/traci/TraCIConstants.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/mobility/traci/TraCIMobility.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/mobility/traci/TraCIScenarioManager.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/obstacle/Obstacle.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/obstacle/ObstacleControl.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/utility/Consts80211p.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/world/annotations/AnnotationManager.h
$O/ApplV_02_Beacon.o: ApplV_02_Beacon.cc \
	Appl.h \
	ApplV_01_Base.h \
	ApplV_02_Beacon.h \
	Global_01_TraCI_Extend.h \
	msg/Messages_m.h \
	$(MIXIM_SOMMER_PROJ)/src/base/connectionManager/BaseConnectionManager.h \
	$(MIXIM_SOMMER_PROJ)/src/base/connectionManager/ChannelAccess.h \
	$(MIXIM_SOMMER_PROJ)/src/base/connectionManager/NicEntry.h \
	$(MIXIM_SOMMER_PROJ)/src/base/messages/MacPkt_m.h \
	$(MIXIM_SOMMER_PROJ)/src/base/modules/BaseApplLayer.h \
	$(MIXIM_SOMMER_PROJ)/src/base/modules/BaseBattery.h \
	$(MIXIM_SOMMER_PROJ)/src/base/modules/BaseLayer.h \
	$(MIXIM_SOMMER_PROJ)/src/base/modules/BaseMobility.h \
	$(MIXIM_SOMMER_PROJ)/src/base/modules/BaseModule.h \
	$(MIXIM_SOMMER_PROJ)/src/base/modules/BaseWorldUtility.h \
	$(MIXIM_SOMMER_PROJ)/src/base/modules/BatteryAccess.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/Coord.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/FWMath.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/FindModule.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/HostState.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/MiXiMDefs.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/Move.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/NetwToMacControlInfo.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/PassedMessage.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/SimpleAddress.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/miximkerneldefs.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/mac/ieee80211p/WaveAppToMac1609_4Interface.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/messages/Mac80211Pkt_m.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/mobility/traci/TraCIColor.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/mobility/traci/TraCIConstants.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/mobility/traci/TraCIMobility.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/mobility/traci/TraCIScenarioManager.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/obstacle/Obstacle.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/obstacle/ObstacleControl.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/utility/Consts80211p.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/world/annotations/AnnotationManager.h
$O/ApplV_03_System.o: ApplV_03_System.cc \
	Appl.h \
	ApplV_01_Base.h \
	ApplV_02_Beacon.h \
	ApplV_03_System.h \
	Global_01_TraCI_Extend.h \
	msg/Messages_m.h \
	$(MIXIM_SOMMER_PROJ)/src/base/connectionManager/BaseConnectionManager.h \
	$(MIXIM_SOMMER_PROJ)/src/base/connectionManager/ChannelAccess.h \
	$(MIXIM_SOMMER_PROJ)/src/base/connectionManager/NicEntry.h \
	$(MIXIM_SOMMER_PROJ)/src/base/messages/MacPkt_m.h \
	$(MIXIM_SOMMER_PROJ)/src/base/modules/BaseApplLayer.h \
	$(MIXIM_SOMMER_PROJ)/src/base/modules/BaseBattery.h \
	$(MIXIM_SOMMER_PROJ)/src/base/modules/BaseLayer.h \
	$(MIXIM_SOMMER_PROJ)/src/base/modules/BaseMobility.h \
	$(MIXIM_SOMMER_PROJ)/src/base/modules/BaseModule.h \
	$(MIXIM_SOMMER_PROJ)/src/base/modules/BaseWorldUtility.h \
	$(MIXIM_SOMMER_PROJ)/src/base/modules/BatteryAccess.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/Coord.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/FWMath.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/FindModule.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/HostState.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/MiXiMDefs.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/Move.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/NetwToMacControlInfo.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/PassedMessage.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/SimpleAddress.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/miximkerneldefs.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/mac/ieee80211p/WaveAppToMac1609_4Interface.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/messages/Mac80211Pkt_m.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/mobility/traci/TraCIColor.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/mobility/traci/TraCIConstants.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/mobility/traci/TraCIMobility.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/mobility/traci/TraCIScenarioManager.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/obstacle/Obstacle.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/obstacle/ObstacleControl.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/utility/Consts80211p.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/world/annotations/AnnotationManager.h
$O/ApplV_04_AID.o: ApplV_04_AID.cc \
	Appl.h \
	ApplV_01_Base.h \
	ApplV_02_Beacon.h \
	ApplV_03_System.h \
	ApplV_04_AID.h \
	Global_01_TraCI_Extend.h \
	msg/Messages_m.h \
	$(MIXIM_SOMMER_PROJ)/src/base/connectionManager/BaseConnectionManager.h \
	$(MIXIM_SOMMER_PROJ)/src/base/connectionManager/ChannelAccess.h \
	$(MIXIM_SOMMER_PROJ)/src/base/connectionManager/NicEntry.h \
	$(MIXIM_SOMMER_PROJ)/src/base/messages/MacPkt_m.h \
	$(MIXIM_SOMMER_PROJ)/src/base/modules/BaseApplLayer.h \
	$(MIXIM_SOMMER_PROJ)/src/base/modules/BaseBattery.h \
	$(MIXIM_SOMMER_PROJ)/src/base/modules/BaseLayer.h \
	$(MIXIM_SOMMER_PROJ)/src/base/modules/BaseMobility.h \
	$(MIXIM_SOMMER_PROJ)/src/base/modules/BaseModule.h \
	$(MIXIM_SOMMER_PROJ)/src/base/modules/BaseWorldUtility.h \
	$(MIXIM_SOMMER_PROJ)/src/base/modules/BatteryAccess.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/Coord.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/FWMath.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/FindModule.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/HostState.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/MiXiMDefs.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/Move.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/NetwToMacControlInfo.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/PassedMessage.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/SimpleAddress.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/miximkerneldefs.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/mac/ieee80211p/WaveAppToMac1609_4Interface.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/messages/Mac80211Pkt_m.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/mobility/traci/TraCIColor.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/mobility/traci/TraCIConstants.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/mobility/traci/TraCIMobility.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/mobility/traci/TraCIScenarioManager.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/obstacle/Obstacle.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/obstacle/ObstacleControl.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/utility/Consts80211p.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/world/annotations/AnnotationManager.h
$O/ApplV_05_PlatoonMg.o: ApplV_05_PlatoonMg.cc \
	Appl.h \
	ApplV_01_Base.h \
	ApplV_02_Beacon.h \
	ApplV_03_System.h \
	ApplV_04_AID.h \
	ApplV_05_PlatoonMg.h \
	Global_01_TraCI_Extend.h \
	msg/Messages_m.h \
	$(MIXIM_SOMMER_PROJ)/src/base/connectionManager/BaseConnectionManager.h \
	$(MIXIM_SOMMER_PROJ)/src/base/connectionManager/ChannelAccess.h \
	$(MIXIM_SOMMER_PROJ)/src/base/connectionManager/NicEntry.h \
	$(MIXIM_SOMMER_PROJ)/src/base/messages/MacPkt_m.h \
	$(MIXIM_SOMMER_PROJ)/src/base/modules/BaseApplLayer.h \
	$(MIXIM_SOMMER_PROJ)/src/base/modules/BaseBattery.h \
	$(MIXIM_SOMMER_PROJ)/src/base/modules/BaseLayer.h \
	$(MIXIM_SOMMER_PROJ)/src/base/modules/BaseMobility.h \
	$(MIXIM_SOMMER_PROJ)/src/base/modules/BaseModule.h \
	$(MIXIM_SOMMER_PROJ)/src/base/modules/BaseWorldUtility.h \
	$(MIXIM_SOMMER_PROJ)/src/base/modules/BatteryAccess.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/Coord.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/FWMath.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/FindModule.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/HostState.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/MiXiMDefs.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/Move.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/NetwToMacControlInfo.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/PassedMessage.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/SimpleAddress.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/miximkerneldefs.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/mac/ieee80211p/WaveAppToMac1609_4Interface.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/messages/Mac80211Pkt_m.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/mobility/traci/TraCIColor.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/mobility/traci/TraCIConstants.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/mobility/traci/TraCIMobility.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/mobility/traci/TraCIScenarioManager.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/obstacle/Obstacle.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/obstacle/ObstacleControl.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/utility/Consts80211p.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/world/annotations/AnnotationManager.h
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
	$(MIXIM_SOMMER_PROJ)/src/base/connectionManager/BaseConnectionManager.h \
	$(MIXIM_SOMMER_PROJ)/src/base/connectionManager/ChannelAccess.h \
	$(MIXIM_SOMMER_PROJ)/src/base/connectionManager/NicEntry.h \
	$(MIXIM_SOMMER_PROJ)/src/base/messages/MacPkt_m.h \
	$(MIXIM_SOMMER_PROJ)/src/base/modules/BaseApplLayer.h \
	$(MIXIM_SOMMER_PROJ)/src/base/modules/BaseBattery.h \
	$(MIXIM_SOMMER_PROJ)/src/base/modules/BaseLayer.h \
	$(MIXIM_SOMMER_PROJ)/src/base/modules/BaseMobility.h \
	$(MIXIM_SOMMER_PROJ)/src/base/modules/BaseModule.h \
	$(MIXIM_SOMMER_PROJ)/src/base/modules/BaseWorldUtility.h \
	$(MIXIM_SOMMER_PROJ)/src/base/modules/BatteryAccess.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/Coord.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/FWMath.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/FindModule.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/HostState.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/MiXiMDefs.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/Move.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/NetwToMacControlInfo.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/PassedMessage.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/SimpleAddress.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/miximkerneldefs.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/mac/ieee80211p/WaveAppToMac1609_4Interface.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/messages/Mac80211Pkt_m.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/mobility/traci/TraCIColor.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/mobility/traci/TraCIConstants.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/mobility/traci/TraCIMobility.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/mobility/traci/TraCIScenarioManager.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/obstacle/Obstacle.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/obstacle/ObstacleControl.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/utility/Consts80211p.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/world/annotations/AnnotationManager.h
$O/Global_01_TraCI_Extend.o: Global_01_TraCI_Extend.cc \
	Appl.h \
	Global_01_TraCI_Extend.h \
	$(MIXIM_SOMMER_PROJ)/src/base/connectionManager/BaseConnectionManager.h \
	$(MIXIM_SOMMER_PROJ)/src/base/connectionManager/NicEntry.h \
	$(MIXIM_SOMMER_PROJ)/src/base/modules/BaseBattery.h \
	$(MIXIM_SOMMER_PROJ)/src/base/modules/BaseMobility.h \
	$(MIXIM_SOMMER_PROJ)/src/base/modules/BaseModule.h \
	$(MIXIM_SOMMER_PROJ)/src/base/modules/BaseWorldUtility.h \
	$(MIXIM_SOMMER_PROJ)/src/base/modules/BatteryAccess.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/Coord.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/FWMath.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/FindModule.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/HostState.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/MiXiMDefs.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/Move.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/miximkerneldefs.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/mobility/traci/TraCIColor.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/mobility/traci/TraCIConstants.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/mobility/traci/TraCIMobility.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/mobility/traci/TraCIScenarioManager.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/obstacle/Obstacle.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/obstacle/ObstacleControl.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/world/annotations/AnnotationManager.h
$O/Global_02_TraCI_App.o: Global_02_TraCI_App.cc \
	Appl.h \
	ApplV_01_Base.h \
	ApplV_02_Beacon.h \
	ApplV_03_System.h \
	ApplV_04_AID.h \
	ApplV_05_PlatoonMg.h \
	ApplV_06_Manager.h \
	Global_01_TraCI_Extend.h \
	Global_02_TraCI_App.h \
	Global_04_VehicleAdd.h \
	Global_05_Warmup.h \
	Global_06_SpeedProfile.h \
	Global_08_TrafficLight.h \
	msg/Messages_m.h \
	$(MIXIM_SOMMER_PROJ)/src/base/connectionManager/BaseConnectionManager.h \
	$(MIXIM_SOMMER_PROJ)/src/base/connectionManager/ChannelAccess.h \
	$(MIXIM_SOMMER_PROJ)/src/base/connectionManager/NicEntry.h \
	$(MIXIM_SOMMER_PROJ)/src/base/messages/MacPkt_m.h \
	$(MIXIM_SOMMER_PROJ)/src/base/modules/BaseApplLayer.h \
	$(MIXIM_SOMMER_PROJ)/src/base/modules/BaseBattery.h \
	$(MIXIM_SOMMER_PROJ)/src/base/modules/BaseLayer.h \
	$(MIXIM_SOMMER_PROJ)/src/base/modules/BaseMobility.h \
	$(MIXIM_SOMMER_PROJ)/src/base/modules/BaseModule.h \
	$(MIXIM_SOMMER_PROJ)/src/base/modules/BaseWorldUtility.h \
	$(MIXIM_SOMMER_PROJ)/src/base/modules/BatteryAccess.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/Coord.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/FWMath.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/FindModule.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/HostState.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/MiXiMDefs.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/Move.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/NetwToMacControlInfo.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/PassedMessage.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/SimpleAddress.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/miximkerneldefs.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/mac/ieee80211p/WaveAppToMac1609_4Interface.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/messages/Mac80211Pkt_m.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/mobility/traci/TraCIColor.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/mobility/traci/TraCIConstants.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/mobility/traci/TraCIMobility.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/mobility/traci/TraCIScenarioManager.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/mobility/traci/TraCIScenarioManagerLaunchd.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/obstacle/Obstacle.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/obstacle/ObstacleControl.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/utility/Consts80211p.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/world/annotations/AnnotationManager.h
$O/Global_03_Statistics.o: Global_03_Statistics.cc \
	Appl.h \
	ApplRSU.h \
	Global_01_TraCI_Extend.h \
	Global_03_Statistics.h \
	msg/Messages_m.h \
	$(MIXIM_SOMMER_PROJ)/src/base/connectionManager/BaseConnectionManager.h \
	$(MIXIM_SOMMER_PROJ)/src/base/connectionManager/ChannelAccess.h \
	$(MIXIM_SOMMER_PROJ)/src/base/connectionManager/NicEntry.h \
	$(MIXIM_SOMMER_PROJ)/src/base/messages/MacPkt_m.h \
	$(MIXIM_SOMMER_PROJ)/src/base/modules/BaseApplLayer.h \
	$(MIXIM_SOMMER_PROJ)/src/base/modules/BaseBattery.h \
	$(MIXIM_SOMMER_PROJ)/src/base/modules/BaseLayer.h \
	$(MIXIM_SOMMER_PROJ)/src/base/modules/BaseMobility.h \
	$(MIXIM_SOMMER_PROJ)/src/base/modules/BaseModule.h \
	$(MIXIM_SOMMER_PROJ)/src/base/modules/BaseWorldUtility.h \
	$(MIXIM_SOMMER_PROJ)/src/base/modules/BatteryAccess.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/Coord.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/FWMath.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/FindModule.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/HostState.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/MiXiMDefs.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/Move.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/NetwControlInfo.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/NetwToMacControlInfo.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/PassedMessage.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/SimpleAddress.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/miximkerneldefs.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/mac/ieee80211p/WaveAppToMac1609_4Interface.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/messages/Mac80211Pkt_m.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/mobility/traci/TraCIColor.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/mobility/traci/TraCIConstants.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/mobility/traci/TraCIMobility.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/mobility/traci/TraCIScenarioManager.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/obstacle/Obstacle.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/obstacle/ObstacleControl.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/utility/Consts80211p.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/world/annotations/AnnotationManager.h
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
	$(MIXIM_SOMMER_PROJ)/src/base/connectionManager/BaseConnectionManager.h \
	$(MIXIM_SOMMER_PROJ)/src/base/connectionManager/ChannelAccess.h \
	$(MIXIM_SOMMER_PROJ)/src/base/connectionManager/NicEntry.h \
	$(MIXIM_SOMMER_PROJ)/src/base/messages/MacPkt_m.h \
	$(MIXIM_SOMMER_PROJ)/src/base/modules/BaseApplLayer.h \
	$(MIXIM_SOMMER_PROJ)/src/base/modules/BaseBattery.h \
	$(MIXIM_SOMMER_PROJ)/src/base/modules/BaseLayer.h \
	$(MIXIM_SOMMER_PROJ)/src/base/modules/BaseMobility.h \
	$(MIXIM_SOMMER_PROJ)/src/base/modules/BaseModule.h \
	$(MIXIM_SOMMER_PROJ)/src/base/modules/BaseWorldUtility.h \
	$(MIXIM_SOMMER_PROJ)/src/base/modules/BatteryAccess.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/Coord.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/FWMath.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/FindModule.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/HostState.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/MiXiMDefs.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/Move.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/NetwToMacControlInfo.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/PassedMessage.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/SimpleAddress.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/miximkerneldefs.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/mac/ieee80211p/WaveAppToMac1609_4Interface.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/messages/Mac80211Pkt_m.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/mobility/traci/TraCIColor.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/mobility/traci/TraCIConstants.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/mobility/traci/TraCIMobility.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/mobility/traci/TraCIScenarioManager.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/mobility/traci/TraCIScenarioManagerLaunchd.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/obstacle/Obstacle.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/obstacle/ObstacleControl.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/utility/Consts80211p.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/world/annotations/AnnotationManager.h
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
	$(MIXIM_SOMMER_PROJ)/src/base/connectionManager/BaseConnectionManager.h \
	$(MIXIM_SOMMER_PROJ)/src/base/connectionManager/ChannelAccess.h \
	$(MIXIM_SOMMER_PROJ)/src/base/connectionManager/NicEntry.h \
	$(MIXIM_SOMMER_PROJ)/src/base/messages/MacPkt_m.h \
	$(MIXIM_SOMMER_PROJ)/src/base/modules/BaseApplLayer.h \
	$(MIXIM_SOMMER_PROJ)/src/base/modules/BaseBattery.h \
	$(MIXIM_SOMMER_PROJ)/src/base/modules/BaseLayer.h \
	$(MIXIM_SOMMER_PROJ)/src/base/modules/BaseMobility.h \
	$(MIXIM_SOMMER_PROJ)/src/base/modules/BaseModule.h \
	$(MIXIM_SOMMER_PROJ)/src/base/modules/BaseWorldUtility.h \
	$(MIXIM_SOMMER_PROJ)/src/base/modules/BatteryAccess.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/Coord.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/FWMath.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/FindModule.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/HostState.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/MiXiMDefs.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/Move.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/NetwToMacControlInfo.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/PassedMessage.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/SimpleAddress.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/miximkerneldefs.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/mac/ieee80211p/WaveAppToMac1609_4Interface.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/messages/Mac80211Pkt_m.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/mobility/traci/TraCIColor.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/mobility/traci/TraCIConstants.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/mobility/traci/TraCIMobility.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/mobility/traci/TraCIScenarioManager.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/mobility/traci/TraCIScenarioManagerLaunchd.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/obstacle/Obstacle.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/obstacle/ObstacleControl.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/utility/Consts80211p.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/world/annotations/AnnotationManager.h
$O/Global_06_SpeedProfile.o: Global_06_SpeedProfile.cc \
	Appl.h \
	Global_01_TraCI_Extend.h \
	Global_06_SpeedProfile.h \
	$(MIXIM_SOMMER_PROJ)/src/base/connectionManager/BaseConnectionManager.h \
	$(MIXIM_SOMMER_PROJ)/src/base/connectionManager/NicEntry.h \
	$(MIXIM_SOMMER_PROJ)/src/base/modules/BaseBattery.h \
	$(MIXIM_SOMMER_PROJ)/src/base/modules/BaseMobility.h \
	$(MIXIM_SOMMER_PROJ)/src/base/modules/BaseModule.h \
	$(MIXIM_SOMMER_PROJ)/src/base/modules/BaseWorldUtility.h \
	$(MIXIM_SOMMER_PROJ)/src/base/modules/BatteryAccess.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/Coord.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/FWMath.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/FindModule.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/HostState.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/MiXiMDefs.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/Move.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/miximkerneldefs.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/mobility/traci/TraCIColor.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/mobility/traci/TraCIConstants.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/mobility/traci/TraCIMobility.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/mobility/traci/TraCIScenarioManager.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/obstacle/Obstacle.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/obstacle/ObstacleControl.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/world/annotations/AnnotationManager.h
$O/Global_07_Router.o: Global_07_Router.cc \
	Appl.h \
	ApplV_01_Base.h \
	ApplV_02_Beacon.h \
	Global_01_TraCI_Extend.h \
	Global_07_Router.h \
	NodeEdge.h \
	msg/Messages_m.h \
	$(MIXIM_SOMMER_PROJ)/src/base/connectionManager/BaseConnectionManager.h \
	$(MIXIM_SOMMER_PROJ)/src/base/connectionManager/ChannelAccess.h \
	$(MIXIM_SOMMER_PROJ)/src/base/connectionManager/NicEntry.h \
	$(MIXIM_SOMMER_PROJ)/src/base/messages/MacPkt_m.h \
	$(MIXIM_SOMMER_PROJ)/src/base/modules/BaseApplLayer.h \
	$(MIXIM_SOMMER_PROJ)/src/base/modules/BaseBattery.h \
	$(MIXIM_SOMMER_PROJ)/src/base/modules/BaseLayer.h \
	$(MIXIM_SOMMER_PROJ)/src/base/modules/BaseMobility.h \
	$(MIXIM_SOMMER_PROJ)/src/base/modules/BaseModule.h \
	$(MIXIM_SOMMER_PROJ)/src/base/modules/BaseWorldUtility.h \
	$(MIXIM_SOMMER_PROJ)/src/base/modules/BatteryAccess.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/Coord.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/FWMath.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/FindModule.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/HostState.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/MiXiMDefs.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/Move.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/NetwToMacControlInfo.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/PassedMessage.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/SimpleAddress.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/miximkerneldefs.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/mac/ieee80211p/WaveAppToMac1609_4Interface.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/messages/Mac80211Pkt_m.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/mobility/traci/TraCIColor.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/mobility/traci/TraCIConstants.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/mobility/traci/TraCIMobility.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/mobility/traci/TraCIScenarioManager.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/obstacle/Obstacle.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/obstacle/ObstacleControl.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/utility/Consts80211p.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/world/annotations/AnnotationManager.h
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
	$(MIXIM_SOMMER_PROJ)/src/base/connectionManager/BaseConnectionManager.h \
	$(MIXIM_SOMMER_PROJ)/src/base/connectionManager/ChannelAccess.h \
	$(MIXIM_SOMMER_PROJ)/src/base/connectionManager/NicEntry.h \
	$(MIXIM_SOMMER_PROJ)/src/base/messages/MacPkt_m.h \
	$(MIXIM_SOMMER_PROJ)/src/base/modules/BaseApplLayer.h \
	$(MIXIM_SOMMER_PROJ)/src/base/modules/BaseBattery.h \
	$(MIXIM_SOMMER_PROJ)/src/base/modules/BaseLayer.h \
	$(MIXIM_SOMMER_PROJ)/src/base/modules/BaseMobility.h \
	$(MIXIM_SOMMER_PROJ)/src/base/modules/BaseModule.h \
	$(MIXIM_SOMMER_PROJ)/src/base/modules/BaseWorldUtility.h \
	$(MIXIM_SOMMER_PROJ)/src/base/modules/BatteryAccess.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/Coord.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/FWMath.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/FindModule.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/HostState.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/MiXiMDefs.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/Move.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/NetwToMacControlInfo.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/PassedMessage.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/SimpleAddress.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/miximkerneldefs.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/mac/ieee80211p/WaveAppToMac1609_4Interface.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/messages/Mac80211Pkt_m.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/mobility/traci/TraCIColor.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/mobility/traci/TraCIConstants.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/mobility/traci/TraCIMobility.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/mobility/traci/TraCIScenarioManager.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/mobility/traci/TraCIScenarioManagerLaunchd.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/obstacle/Obstacle.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/obstacle/ObstacleControl.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/utility/Consts80211p.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/world/annotations/AnnotationManager.h
$O/Mac1609_4_Mod.o: Mac1609_4_Mod.cc \
	Appl.h \
	Mac1609_4_Mod.h \
	$(MIXIM_SOMMER_PROJ)/src/base/connectionManager/BaseConnectionManager.h \
	$(MIXIM_SOMMER_PROJ)/src/base/connectionManager/ChannelAccess.h \
	$(MIXIM_SOMMER_PROJ)/src/base/connectionManager/NicEntry.h \
	$(MIXIM_SOMMER_PROJ)/src/base/messages/AirFrame_m.h \
	$(MIXIM_SOMMER_PROJ)/src/base/messages/ChannelSenseRequest_m.h \
	$(MIXIM_SOMMER_PROJ)/src/base/messages/MacPkt_m.h \
	$(MIXIM_SOMMER_PROJ)/src/base/modules/BaseBattery.h \
	$(MIXIM_SOMMER_PROJ)/src/base/modules/BaseLayer.h \
	$(MIXIM_SOMMER_PROJ)/src/base/modules/BaseMacLayer.h \
	$(MIXIM_SOMMER_PROJ)/src/base/modules/BaseMobility.h \
	$(MIXIM_SOMMER_PROJ)/src/base/modules/BaseModule.h \
	$(MIXIM_SOMMER_PROJ)/src/base/modules/BaseWorldUtility.h \
	$(MIXIM_SOMMER_PROJ)/src/base/modules/BatteryAccess.h \
	$(MIXIM_SOMMER_PROJ)/src/base/phyLayer/AnalogueModel.h \
	$(MIXIM_SOMMER_PROJ)/src/base/phyLayer/BaseDecider.h \
	$(MIXIM_SOMMER_PROJ)/src/base/phyLayer/BasePhyLayer.h \
	$(MIXIM_SOMMER_PROJ)/src/base/phyLayer/ChannelInfo.h \
	$(MIXIM_SOMMER_PROJ)/src/base/phyLayer/ChannelState.h \
	$(MIXIM_SOMMER_PROJ)/src/base/phyLayer/Decider.h \
	$(MIXIM_SOMMER_PROJ)/src/base/phyLayer/DeciderToPhyInterface.h \
	$(MIXIM_SOMMER_PROJ)/src/base/phyLayer/Interpolation.h \
	$(MIXIM_SOMMER_PROJ)/src/base/phyLayer/MacToPhyControlInfo.h \
	$(MIXIM_SOMMER_PROJ)/src/base/phyLayer/MacToPhyInterface.h \
	$(MIXIM_SOMMER_PROJ)/src/base/phyLayer/Mapping.h \
	$(MIXIM_SOMMER_PROJ)/src/base/phyLayer/MappingBase.h \
	$(MIXIM_SOMMER_PROJ)/src/base/phyLayer/MappingUtils.h \
	$(MIXIM_SOMMER_PROJ)/src/base/phyLayer/PhyUtils.h \
	$(MIXIM_SOMMER_PROJ)/src/base/phyLayer/Signal_.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/Coord.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/FWMath.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/FindModule.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/HostState.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/MiXiMDefs.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/Move.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/NetwToMacControlInfo.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/PassedMessage.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/SimpleAddress.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/miximkerneldefs.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/analogueModel/JakesFading.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/analogueModel/LogNormalShadowing.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/analogueModel/SimplePathlossModel.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/mac/ieee80211p/Mac1609_4.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/mac/ieee80211p/Mac80211pToPhy11pInterface.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/mac/ieee80211p/WaveAppToMac1609_4Interface.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/messages/Mac80211Pkt_m.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/messages/WaveShortMessage_m.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/phy/Decider80211p.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/phy/Decider80211pToPhy80211pInterface.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/phy/PhyLayer.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/phy/PhyLayer80211p.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/phy/SNRThresholdDecider.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/utility/Consts80211p.h
$O/NodeEdge.o: NodeEdge.cc \
	NodeEdge.h
$O/RSUMobility.o: RSUMobility.cc \
	Appl.h \
	Global_01_TraCI_Extend.h \
	RSUMobility.h \
	$(MIXIM_SOMMER_PROJ)/src/base/connectionManager/BaseConnectionManager.h \
	$(MIXIM_SOMMER_PROJ)/src/base/connectionManager/NicEntry.h \
	$(MIXIM_SOMMER_PROJ)/src/base/modules/BaseBattery.h \
	$(MIXIM_SOMMER_PROJ)/src/base/modules/BaseMobility.h \
	$(MIXIM_SOMMER_PROJ)/src/base/modules/BaseModule.h \
	$(MIXIM_SOMMER_PROJ)/src/base/modules/BaseWorldUtility.h \
	$(MIXIM_SOMMER_PROJ)/src/base/modules/BatteryAccess.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/Coord.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/FWMath.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/FindModule.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/HostState.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/MiXiMDefs.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/Move.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/miximkerneldefs.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/mobility/traci/TraCIColor.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/mobility/traci/TraCIConstants.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/mobility/traci/TraCIMobility.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/mobility/traci/TraCIScenarioManager.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/obstacle/Obstacle.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/obstacle/ObstacleControl.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/world/annotations/AnnotationManager.h
$O/TraCIColor.o: TraCIColor.cc \
	$(MIXIM_SOMMER_PROJ)/src/modules/mobility/traci/TraCIColor.h
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
	$(MIXIM_SOMMER_PROJ)/src/base/connectionManager/BaseConnectionManager.h \
	$(MIXIM_SOMMER_PROJ)/src/base/connectionManager/ChannelAccess.h \
	$(MIXIM_SOMMER_PROJ)/src/base/connectionManager/NicEntry.h \
	$(MIXIM_SOMMER_PROJ)/src/base/messages/MacPkt_m.h \
	$(MIXIM_SOMMER_PROJ)/src/base/modules/BaseApplLayer.h \
	$(MIXIM_SOMMER_PROJ)/src/base/modules/BaseBattery.h \
	$(MIXIM_SOMMER_PROJ)/src/base/modules/BaseLayer.h \
	$(MIXIM_SOMMER_PROJ)/src/base/modules/BaseMobility.h \
	$(MIXIM_SOMMER_PROJ)/src/base/modules/BaseModule.h \
	$(MIXIM_SOMMER_PROJ)/src/base/modules/BaseWorldUtility.h \
	$(MIXIM_SOMMER_PROJ)/src/base/modules/BatteryAccess.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/Coord.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/FWMath.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/FindModule.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/HostState.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/MiXiMDefs.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/Move.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/NetwToMacControlInfo.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/PassedMessage.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/SimpleAddress.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/miximkerneldefs.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/mac/ieee80211p/WaveAppToMac1609_4Interface.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/messages/Mac80211Pkt_m.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/mobility/traci/TraCIColor.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/mobility/traci/TraCIConstants.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/mobility/traci/TraCIMobility.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/mobility/traci/TraCIScenarioManager.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/obstacle/Obstacle.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/obstacle/ObstacleControl.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/utility/Consts80211p.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/world/annotations/AnnotationManager.h
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
	$(MIXIM_SOMMER_PROJ)/src/base/connectionManager/BaseConnectionManager.h \
	$(MIXIM_SOMMER_PROJ)/src/base/connectionManager/ChannelAccess.h \
	$(MIXIM_SOMMER_PROJ)/src/base/connectionManager/NicEntry.h \
	$(MIXIM_SOMMER_PROJ)/src/base/messages/MacPkt_m.h \
	$(MIXIM_SOMMER_PROJ)/src/base/modules/BaseApplLayer.h \
	$(MIXIM_SOMMER_PROJ)/src/base/modules/BaseBattery.h \
	$(MIXIM_SOMMER_PROJ)/src/base/modules/BaseLayer.h \
	$(MIXIM_SOMMER_PROJ)/src/base/modules/BaseMobility.h \
	$(MIXIM_SOMMER_PROJ)/src/base/modules/BaseModule.h \
	$(MIXIM_SOMMER_PROJ)/src/base/modules/BaseWorldUtility.h \
	$(MIXIM_SOMMER_PROJ)/src/base/modules/BatteryAccess.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/Coord.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/FWMath.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/FindModule.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/HostState.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/MiXiMDefs.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/Move.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/NetwToMacControlInfo.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/PassedMessage.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/SimpleAddress.h \
	$(MIXIM_SOMMER_PROJ)/src/base/utils/miximkerneldefs.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/mac/ieee80211p/WaveAppToMac1609_4Interface.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/messages/Mac80211Pkt_m.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/mobility/traci/TraCIColor.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/mobility/traci/TraCIConstants.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/mobility/traci/TraCIMobility.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/mobility/traci/TraCIScenarioManager.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/obstacle/Obstacle.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/obstacle/ObstacleControl.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/utility/Consts80211p.h \
	$(MIXIM_SOMMER_PROJ)/src/modules/world/annotations/AnnotationManager.h

