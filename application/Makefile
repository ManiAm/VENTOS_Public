#
# OMNeT++/OMNEST Makefile for libapplication
#
# This file was generated with the command:
#  opp_makemake -f --make-so -o application -O out -d msg -I/home/mani/Desktop/VENTOS/eigen3 -I../../veins/src/modules/messages -I../../veins/src/modules/obstacle -I../../veins/src/modules -I../../veins/src/base/modules -I../../veins/src/modules/utility -I../../veins/src/base/utils -I../../veins/src/base/messages -I../../veins/src/base/phyLayer -Imsg -I../../veins/src/modules/phy -I../../veins/src/modules/mac/ieee80211p -I../../veins/src/base/connectionManager -I../../veins/src/modules/analogueModel -L../../veins/out/$(CONFIGNAME)/src/modules -L../../veins/out/$(CONFIGNAME)/tests/testUtils -L../../veins/out/$(CONFIGNAME)/src/base -L../out/$(CONFIGNAME)/application/msg -lmsg -lmiximbase -lmiximmodules -lmiximmodules -lmiximtestUtils -lmiximbase -KVEINS_PROJ=../../veins
#

# Name of target to be created (-o option)
TARGET = libapplication$(SHARED_LIB_SUFFIX)

# C++ include paths (with -I)
INCLUDE_PATH = \
    -I../eigen3 \
    -I../../veins/src/modules/messages \
    -I../../veins/src/modules/obstacle \
    -I../../veins/src/modules \
    -I../../veins/src/base/modules \
    -I../../veins/src/modules/utility \
    -I../../veins/src/base/utils \
    -I../../veins/src/base/messages \
    -I../../veins/src/base/phyLayer \
    -Imsg \
    -I../../veins/src/modules/phy \
    -I../../veins/src/modules/mac/ieee80211p \
    -I../../veins/src/base/connectionManager \
    -I../../veins/src/modules/analogueModel \
    -I.

# Additional object and library files to link with
EXTRA_OBJS =

# Additional libraries (-L, -l options)
LIBS = -L../../veins/out/$(CONFIGNAME)/src/modules -L../../veins/out/$(CONFIGNAME)/tests/testUtils -L../../veins/out/$(CONFIGNAME)/src/base -L../out/$(CONFIGNAME)/application/msg  -lmsg -lmiximbase -lmiximmodules -lmiximmodules -lmiximtestUtils -lmiximbase
LIBS += -Wl,-rpath,`abspath ../../veins/out/$(CONFIGNAME)/src/modules` -Wl,-rpath,`abspath ../../veins/out/$(CONFIGNAME)/tests/testUtils` -Wl,-rpath,`abspath ../../veins/out/$(CONFIGNAME)/src/base` -Wl,-rpath,`abspath ../out/$(CONFIGNAME)/application/msg`

# Output directory
PROJECT_OUTPUT_DIR = ../out
PROJECTRELATIVE_PATH = application
O = $(PROJECT_OUTPUT_DIR)/$(CONFIGNAME)/$(PROJECTRELATIVE_PATH)

# Object files for local .cc and .msg files
OBJS = \
    $O/16TraCI_App.o \
    $O/10ApplRSU.o \
    $O/zApplVPlatoonFormation2.o \
    $O/05ApplVPlatoonMg.o \
    $O/12TrafficLight.o \
    $O/08Warmup.o \
    $O/03ApplVBeacon.o \
    $O/04ApplV_AID.o \
    $O/zApplVPlatoonFormation.o \
    $O/Mac1609_4_Mod.o \
    $O/07VehicleAdd.o \
    $O/TraCIColor.o \
    $O/13ApplAdversary.o \
    $O/02ApplVBase.o \
    $O/09SpeedProfile.o \
    $O/06ApplVManager.o \
    $O/11RSUMobility.o \
    $O/14Statistics.o \
    $O/15TraCI_Extend.o

# Message files
MSGFILES =

# Other makefile variables (-K)
VEINS_PROJ=../../veins

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
$O/02ApplVBase.o: 02ApplVBase.cc \
	01ExtraClasses.h \
	02ApplVBase.h \
	15TraCI_Extend.h \
	msg/Messages_m.h \
	$(VEINS_PROJ)/src/base/connectionManager/BaseConnectionManager.h \
	$(VEINS_PROJ)/src/base/connectionManager/ChannelAccess.h \
	$(VEINS_PROJ)/src/base/connectionManager/NicEntry.h \
	$(VEINS_PROJ)/src/base/messages/MacPkt_m.h \
	$(VEINS_PROJ)/src/base/modules/BaseApplLayer.h \
	$(VEINS_PROJ)/src/base/modules/BaseBattery.h \
	$(VEINS_PROJ)/src/base/modules/BaseLayer.h \
	$(VEINS_PROJ)/src/base/modules/BaseMobility.h \
	$(VEINS_PROJ)/src/base/modules/BaseModule.h \
	$(VEINS_PROJ)/src/base/modules/BaseWorldUtility.h \
	$(VEINS_PROJ)/src/base/modules/BatteryAccess.h \
	$(VEINS_PROJ)/src/base/utils/Coord.h \
	$(VEINS_PROJ)/src/base/utils/FWMath.h \
	$(VEINS_PROJ)/src/base/utils/FindModule.h \
	$(VEINS_PROJ)/src/base/utils/HostState.h \
	$(VEINS_PROJ)/src/base/utils/MiXiMDefs.h \
	$(VEINS_PROJ)/src/base/utils/Move.h \
	$(VEINS_PROJ)/src/base/utils/NetwToMacControlInfo.h \
	$(VEINS_PROJ)/src/base/utils/PassedMessage.h \
	$(VEINS_PROJ)/src/base/utils/SimpleAddress.h \
	$(VEINS_PROJ)/src/base/utils/miximkerneldefs.h \
	$(VEINS_PROJ)/src/modules/mac/ieee80211p/WaveAppToMac1609_4Interface.h \
	$(VEINS_PROJ)/src/modules/messages/Mac80211Pkt_m.h \
	$(VEINS_PROJ)/src/modules/mobility/traci/TraCIColor.h \
	$(VEINS_PROJ)/src/modules/mobility/traci/TraCIConstants.h \
	$(VEINS_PROJ)/src/modules/mobility/traci/TraCIMobility.h \
	$(VEINS_PROJ)/src/modules/mobility/traci/TraCIScenarioManager.h \
	$(VEINS_PROJ)/src/modules/obstacle/Obstacle.h \
	$(VEINS_PROJ)/src/modules/obstacle/ObstacleControl.h \
	$(VEINS_PROJ)/src/modules/utility/Consts80211p.h \
	$(VEINS_PROJ)/src/modules/world/annotations/AnnotationManager.h
$O/03ApplVBeacon.o: 03ApplVBeacon.cc \
	01ExtraClasses.h \
	02ApplVBase.h \
	03ApplVBeacon.h \
	15TraCI_Extend.h \
	msg/Messages_m.h \
	$(VEINS_PROJ)/src/base/connectionManager/BaseConnectionManager.h \
	$(VEINS_PROJ)/src/base/connectionManager/ChannelAccess.h \
	$(VEINS_PROJ)/src/base/connectionManager/NicEntry.h \
	$(VEINS_PROJ)/src/base/messages/MacPkt_m.h \
	$(VEINS_PROJ)/src/base/modules/BaseApplLayer.h \
	$(VEINS_PROJ)/src/base/modules/BaseBattery.h \
	$(VEINS_PROJ)/src/base/modules/BaseLayer.h \
	$(VEINS_PROJ)/src/base/modules/BaseMobility.h \
	$(VEINS_PROJ)/src/base/modules/BaseModule.h \
	$(VEINS_PROJ)/src/base/modules/BaseWorldUtility.h \
	$(VEINS_PROJ)/src/base/modules/BatteryAccess.h \
	$(VEINS_PROJ)/src/base/utils/Coord.h \
	$(VEINS_PROJ)/src/base/utils/FWMath.h \
	$(VEINS_PROJ)/src/base/utils/FindModule.h \
	$(VEINS_PROJ)/src/base/utils/HostState.h \
	$(VEINS_PROJ)/src/base/utils/MiXiMDefs.h \
	$(VEINS_PROJ)/src/base/utils/Move.h \
	$(VEINS_PROJ)/src/base/utils/NetwToMacControlInfo.h \
	$(VEINS_PROJ)/src/base/utils/PassedMessage.h \
	$(VEINS_PROJ)/src/base/utils/SimpleAddress.h \
	$(VEINS_PROJ)/src/base/utils/miximkerneldefs.h \
	$(VEINS_PROJ)/src/modules/mac/ieee80211p/WaveAppToMac1609_4Interface.h \
	$(VEINS_PROJ)/src/modules/messages/Mac80211Pkt_m.h \
	$(VEINS_PROJ)/src/modules/mobility/traci/TraCIColor.h \
	$(VEINS_PROJ)/src/modules/mobility/traci/TraCIConstants.h \
	$(VEINS_PROJ)/src/modules/mobility/traci/TraCIMobility.h \
	$(VEINS_PROJ)/src/modules/mobility/traci/TraCIScenarioManager.h \
	$(VEINS_PROJ)/src/modules/obstacle/Obstacle.h \
	$(VEINS_PROJ)/src/modules/obstacle/ObstacleControl.h \
	$(VEINS_PROJ)/src/modules/utility/Consts80211p.h \
	$(VEINS_PROJ)/src/modules/world/annotations/AnnotationManager.h
$O/04ApplV_AID.o: 04ApplV_AID.cc \
	01ExtraClasses.h \
	02ApplVBase.h \
	03ApplVBeacon.h \
	04ApplV_AID.h \
	15TraCI_Extend.h \
	msg/Messages_m.h \
	$(VEINS_PROJ)/src/base/connectionManager/BaseConnectionManager.h \
	$(VEINS_PROJ)/src/base/connectionManager/ChannelAccess.h \
	$(VEINS_PROJ)/src/base/connectionManager/NicEntry.h \
	$(VEINS_PROJ)/src/base/messages/MacPkt_m.h \
	$(VEINS_PROJ)/src/base/modules/BaseApplLayer.h \
	$(VEINS_PROJ)/src/base/modules/BaseBattery.h \
	$(VEINS_PROJ)/src/base/modules/BaseLayer.h \
	$(VEINS_PROJ)/src/base/modules/BaseMobility.h \
	$(VEINS_PROJ)/src/base/modules/BaseModule.h \
	$(VEINS_PROJ)/src/base/modules/BaseWorldUtility.h \
	$(VEINS_PROJ)/src/base/modules/BatteryAccess.h \
	$(VEINS_PROJ)/src/base/utils/Coord.h \
	$(VEINS_PROJ)/src/base/utils/FWMath.h \
	$(VEINS_PROJ)/src/base/utils/FindModule.h \
	$(VEINS_PROJ)/src/base/utils/HostState.h \
	$(VEINS_PROJ)/src/base/utils/MiXiMDefs.h \
	$(VEINS_PROJ)/src/base/utils/Move.h \
	$(VEINS_PROJ)/src/base/utils/NetwToMacControlInfo.h \
	$(VEINS_PROJ)/src/base/utils/PassedMessage.h \
	$(VEINS_PROJ)/src/base/utils/SimpleAddress.h \
	$(VEINS_PROJ)/src/base/utils/miximkerneldefs.h \
	$(VEINS_PROJ)/src/modules/mac/ieee80211p/WaveAppToMac1609_4Interface.h \
	$(VEINS_PROJ)/src/modules/messages/Mac80211Pkt_m.h \
	$(VEINS_PROJ)/src/modules/mobility/traci/TraCIColor.h \
	$(VEINS_PROJ)/src/modules/mobility/traci/TraCIConstants.h \
	$(VEINS_PROJ)/src/modules/mobility/traci/TraCIMobility.h \
	$(VEINS_PROJ)/src/modules/mobility/traci/TraCIScenarioManager.h \
	$(VEINS_PROJ)/src/modules/obstacle/Obstacle.h \
	$(VEINS_PROJ)/src/modules/obstacle/ObstacleControl.h \
	$(VEINS_PROJ)/src/modules/utility/Consts80211p.h \
	$(VEINS_PROJ)/src/modules/world/annotations/AnnotationManager.h
$O/05ApplVPlatoonMg.o: 05ApplVPlatoonMg.cc \
	01ExtraClasses.h \
	02ApplVBase.h \
	03ApplVBeacon.h \
	04ApplV_AID.h \
	05ApplVPlatoonMg.h \
	15TraCI_Extend.h \
	msg/Messages_m.h \
	$(VEINS_PROJ)/src/base/connectionManager/BaseConnectionManager.h \
	$(VEINS_PROJ)/src/base/connectionManager/ChannelAccess.h \
	$(VEINS_PROJ)/src/base/connectionManager/NicEntry.h \
	$(VEINS_PROJ)/src/base/messages/MacPkt_m.h \
	$(VEINS_PROJ)/src/base/modules/BaseApplLayer.h \
	$(VEINS_PROJ)/src/base/modules/BaseBattery.h \
	$(VEINS_PROJ)/src/base/modules/BaseLayer.h \
	$(VEINS_PROJ)/src/base/modules/BaseMobility.h \
	$(VEINS_PROJ)/src/base/modules/BaseModule.h \
	$(VEINS_PROJ)/src/base/modules/BaseWorldUtility.h \
	$(VEINS_PROJ)/src/base/modules/BatteryAccess.h \
	$(VEINS_PROJ)/src/base/utils/Coord.h \
	$(VEINS_PROJ)/src/base/utils/FWMath.h \
	$(VEINS_PROJ)/src/base/utils/FindModule.h \
	$(VEINS_PROJ)/src/base/utils/HostState.h \
	$(VEINS_PROJ)/src/base/utils/MiXiMDefs.h \
	$(VEINS_PROJ)/src/base/utils/Move.h \
	$(VEINS_PROJ)/src/base/utils/NetwToMacControlInfo.h \
	$(VEINS_PROJ)/src/base/utils/PassedMessage.h \
	$(VEINS_PROJ)/src/base/utils/SimpleAddress.h \
	$(VEINS_PROJ)/src/base/utils/miximkerneldefs.h \
	$(VEINS_PROJ)/src/modules/mac/ieee80211p/WaveAppToMac1609_4Interface.h \
	$(VEINS_PROJ)/src/modules/messages/Mac80211Pkt_m.h \
	$(VEINS_PROJ)/src/modules/mobility/traci/TraCIColor.h \
	$(VEINS_PROJ)/src/modules/mobility/traci/TraCIConstants.h \
	$(VEINS_PROJ)/src/modules/mobility/traci/TraCIMobility.h \
	$(VEINS_PROJ)/src/modules/mobility/traci/TraCIScenarioManager.h \
	$(VEINS_PROJ)/src/modules/obstacle/Obstacle.h \
	$(VEINS_PROJ)/src/modules/obstacle/ObstacleControl.h \
	$(VEINS_PROJ)/src/modules/utility/Consts80211p.h \
	$(VEINS_PROJ)/src/modules/world/annotations/AnnotationManager.h
$O/06ApplVManager.o: 06ApplVManager.cc \
	01ExtraClasses.h \
	02ApplVBase.h \
	03ApplVBeacon.h \
	04ApplV_AID.h \
	05ApplVPlatoonMg.h \
	06ApplVManager.h \
	15TraCI_Extend.h \
	msg/Messages_m.h \
	$(VEINS_PROJ)/src/base/connectionManager/BaseConnectionManager.h \
	$(VEINS_PROJ)/src/base/connectionManager/ChannelAccess.h \
	$(VEINS_PROJ)/src/base/connectionManager/NicEntry.h \
	$(VEINS_PROJ)/src/base/messages/MacPkt_m.h \
	$(VEINS_PROJ)/src/base/modules/BaseApplLayer.h \
	$(VEINS_PROJ)/src/base/modules/BaseBattery.h \
	$(VEINS_PROJ)/src/base/modules/BaseLayer.h \
	$(VEINS_PROJ)/src/base/modules/BaseMobility.h \
	$(VEINS_PROJ)/src/base/modules/BaseModule.h \
	$(VEINS_PROJ)/src/base/modules/BaseWorldUtility.h \
	$(VEINS_PROJ)/src/base/modules/BatteryAccess.h \
	$(VEINS_PROJ)/src/base/utils/Coord.h \
	$(VEINS_PROJ)/src/base/utils/FWMath.h \
	$(VEINS_PROJ)/src/base/utils/FindModule.h \
	$(VEINS_PROJ)/src/base/utils/HostState.h \
	$(VEINS_PROJ)/src/base/utils/MiXiMDefs.h \
	$(VEINS_PROJ)/src/base/utils/Move.h \
	$(VEINS_PROJ)/src/base/utils/NetwToMacControlInfo.h \
	$(VEINS_PROJ)/src/base/utils/PassedMessage.h \
	$(VEINS_PROJ)/src/base/utils/SimpleAddress.h \
	$(VEINS_PROJ)/src/base/utils/miximkerneldefs.h \
	$(VEINS_PROJ)/src/modules/mac/ieee80211p/WaveAppToMac1609_4Interface.h \
	$(VEINS_PROJ)/src/modules/messages/Mac80211Pkt_m.h \
	$(VEINS_PROJ)/src/modules/mobility/traci/TraCIColor.h \
	$(VEINS_PROJ)/src/modules/mobility/traci/TraCIConstants.h \
	$(VEINS_PROJ)/src/modules/mobility/traci/TraCIMobility.h \
	$(VEINS_PROJ)/src/modules/mobility/traci/TraCIScenarioManager.h \
	$(VEINS_PROJ)/src/modules/obstacle/Obstacle.h \
	$(VEINS_PROJ)/src/modules/obstacle/ObstacleControl.h \
	$(VEINS_PROJ)/src/modules/utility/Consts80211p.h \
	$(VEINS_PROJ)/src/modules/world/annotations/AnnotationManager.h
$O/07VehicleAdd.o: 07VehicleAdd.cc \
	01ExtraClasses.h \
	02ApplVBase.h \
	03ApplVBeacon.h \
	04ApplV_AID.h \
	05ApplVPlatoonMg.h \
	06ApplVManager.h \
	07VehicleAdd.h \
	15TraCI_Extend.h \
	msg/Messages_m.h \
	$(VEINS_PROJ)/src/base/connectionManager/BaseConnectionManager.h \
	$(VEINS_PROJ)/src/base/connectionManager/ChannelAccess.h \
	$(VEINS_PROJ)/src/base/connectionManager/NicEntry.h \
	$(VEINS_PROJ)/src/base/messages/MacPkt_m.h \
	$(VEINS_PROJ)/src/base/modules/BaseApplLayer.h \
	$(VEINS_PROJ)/src/base/modules/BaseBattery.h \
	$(VEINS_PROJ)/src/base/modules/BaseLayer.h \
	$(VEINS_PROJ)/src/base/modules/BaseMobility.h \
	$(VEINS_PROJ)/src/base/modules/BaseModule.h \
	$(VEINS_PROJ)/src/base/modules/BaseWorldUtility.h \
	$(VEINS_PROJ)/src/base/modules/BatteryAccess.h \
	$(VEINS_PROJ)/src/base/utils/Coord.h \
	$(VEINS_PROJ)/src/base/utils/FWMath.h \
	$(VEINS_PROJ)/src/base/utils/FindModule.h \
	$(VEINS_PROJ)/src/base/utils/HostState.h \
	$(VEINS_PROJ)/src/base/utils/MiXiMDefs.h \
	$(VEINS_PROJ)/src/base/utils/Move.h \
	$(VEINS_PROJ)/src/base/utils/NetwToMacControlInfo.h \
	$(VEINS_PROJ)/src/base/utils/PassedMessage.h \
	$(VEINS_PROJ)/src/base/utils/SimpleAddress.h \
	$(VEINS_PROJ)/src/base/utils/miximkerneldefs.h \
	$(VEINS_PROJ)/src/modules/mac/ieee80211p/WaveAppToMac1609_4Interface.h \
	$(VEINS_PROJ)/src/modules/messages/Mac80211Pkt_m.h \
	$(VEINS_PROJ)/src/modules/mobility/traci/TraCIColor.h \
	$(VEINS_PROJ)/src/modules/mobility/traci/TraCIConstants.h \
	$(VEINS_PROJ)/src/modules/mobility/traci/TraCIMobility.h \
	$(VEINS_PROJ)/src/modules/mobility/traci/TraCIScenarioManager.h \
	$(VEINS_PROJ)/src/modules/mobility/traci/TraCIScenarioManagerLaunchd.h \
	$(VEINS_PROJ)/src/modules/obstacle/Obstacle.h \
	$(VEINS_PROJ)/src/modules/obstacle/ObstacleControl.h \
	$(VEINS_PROJ)/src/modules/utility/Consts80211p.h \
	$(VEINS_PROJ)/src/modules/world/annotations/AnnotationManager.h
$O/08Warmup.o: 08Warmup.cc \
	01ExtraClasses.h \
	02ApplVBase.h \
	03ApplVBeacon.h \
	04ApplV_AID.h \
	05ApplVPlatoonMg.h \
	06ApplVManager.h \
	07VehicleAdd.h \
	08Warmup.h \
	09SpeedProfile.h \
	15TraCI_Extend.h \
	msg/Messages_m.h \
	$(VEINS_PROJ)/src/base/connectionManager/BaseConnectionManager.h \
	$(VEINS_PROJ)/src/base/connectionManager/ChannelAccess.h \
	$(VEINS_PROJ)/src/base/connectionManager/NicEntry.h \
	$(VEINS_PROJ)/src/base/messages/MacPkt_m.h \
	$(VEINS_PROJ)/src/base/modules/BaseApplLayer.h \
	$(VEINS_PROJ)/src/base/modules/BaseBattery.h \
	$(VEINS_PROJ)/src/base/modules/BaseLayer.h \
	$(VEINS_PROJ)/src/base/modules/BaseMobility.h \
	$(VEINS_PROJ)/src/base/modules/BaseModule.h \
	$(VEINS_PROJ)/src/base/modules/BaseWorldUtility.h \
	$(VEINS_PROJ)/src/base/modules/BatteryAccess.h \
	$(VEINS_PROJ)/src/base/utils/Coord.h \
	$(VEINS_PROJ)/src/base/utils/FWMath.h \
	$(VEINS_PROJ)/src/base/utils/FindModule.h \
	$(VEINS_PROJ)/src/base/utils/HostState.h \
	$(VEINS_PROJ)/src/base/utils/MiXiMDefs.h \
	$(VEINS_PROJ)/src/base/utils/Move.h \
	$(VEINS_PROJ)/src/base/utils/NetwToMacControlInfo.h \
	$(VEINS_PROJ)/src/base/utils/PassedMessage.h \
	$(VEINS_PROJ)/src/base/utils/SimpleAddress.h \
	$(VEINS_PROJ)/src/base/utils/miximkerneldefs.h \
	$(VEINS_PROJ)/src/modules/mac/ieee80211p/WaveAppToMac1609_4Interface.h \
	$(VEINS_PROJ)/src/modules/messages/Mac80211Pkt_m.h \
	$(VEINS_PROJ)/src/modules/mobility/traci/TraCIColor.h \
	$(VEINS_PROJ)/src/modules/mobility/traci/TraCIConstants.h \
	$(VEINS_PROJ)/src/modules/mobility/traci/TraCIMobility.h \
	$(VEINS_PROJ)/src/modules/mobility/traci/TraCIScenarioManager.h \
	$(VEINS_PROJ)/src/modules/mobility/traci/TraCIScenarioManagerLaunchd.h \
	$(VEINS_PROJ)/src/modules/obstacle/Obstacle.h \
	$(VEINS_PROJ)/src/modules/obstacle/ObstacleControl.h \
	$(VEINS_PROJ)/src/modules/utility/Consts80211p.h \
	$(VEINS_PROJ)/src/modules/world/annotations/AnnotationManager.h
$O/09SpeedProfile.o: 09SpeedProfile.cc \
	01ExtraClasses.h \
	09SpeedProfile.h \
	15TraCI_Extend.h \
	$(VEINS_PROJ)/src/base/connectionManager/BaseConnectionManager.h \
	$(VEINS_PROJ)/src/base/connectionManager/NicEntry.h \
	$(VEINS_PROJ)/src/base/modules/BaseBattery.h \
	$(VEINS_PROJ)/src/base/modules/BaseMobility.h \
	$(VEINS_PROJ)/src/base/modules/BaseModule.h \
	$(VEINS_PROJ)/src/base/modules/BaseWorldUtility.h \
	$(VEINS_PROJ)/src/base/modules/BatteryAccess.h \
	$(VEINS_PROJ)/src/base/utils/Coord.h \
	$(VEINS_PROJ)/src/base/utils/FWMath.h \
	$(VEINS_PROJ)/src/base/utils/FindModule.h \
	$(VEINS_PROJ)/src/base/utils/HostState.h \
	$(VEINS_PROJ)/src/base/utils/MiXiMDefs.h \
	$(VEINS_PROJ)/src/base/utils/Move.h \
	$(VEINS_PROJ)/src/base/utils/miximkerneldefs.h \
	$(VEINS_PROJ)/src/modules/mobility/traci/TraCIColor.h \
	$(VEINS_PROJ)/src/modules/mobility/traci/TraCIConstants.h \
	$(VEINS_PROJ)/src/modules/mobility/traci/TraCIMobility.h \
	$(VEINS_PROJ)/src/modules/mobility/traci/TraCIScenarioManager.h \
	$(VEINS_PROJ)/src/modules/obstacle/Obstacle.h \
	$(VEINS_PROJ)/src/modules/obstacle/ObstacleControl.h \
	$(VEINS_PROJ)/src/modules/world/annotations/AnnotationManager.h
$O/10ApplRSU.o: 10ApplRSU.cc \
	01ExtraClasses.h \
	10ApplRSU.h \
	15TraCI_Extend.h \
	msg/Messages_m.h \
	$(VEINS_PROJ)/src/base/connectionManager/BaseConnectionManager.h \
	$(VEINS_PROJ)/src/base/connectionManager/ChannelAccess.h \
	$(VEINS_PROJ)/src/base/connectionManager/NicEntry.h \
	$(VEINS_PROJ)/src/base/messages/MacPkt_m.h \
	$(VEINS_PROJ)/src/base/modules/BaseApplLayer.h \
	$(VEINS_PROJ)/src/base/modules/BaseBattery.h \
	$(VEINS_PROJ)/src/base/modules/BaseLayer.h \
	$(VEINS_PROJ)/src/base/modules/BaseMobility.h \
	$(VEINS_PROJ)/src/base/modules/BaseModule.h \
	$(VEINS_PROJ)/src/base/modules/BaseWorldUtility.h \
	$(VEINS_PROJ)/src/base/modules/BatteryAccess.h \
	$(VEINS_PROJ)/src/base/utils/Coord.h \
	$(VEINS_PROJ)/src/base/utils/FWMath.h \
	$(VEINS_PROJ)/src/base/utils/FindModule.h \
	$(VEINS_PROJ)/src/base/utils/HostState.h \
	$(VEINS_PROJ)/src/base/utils/MiXiMDefs.h \
	$(VEINS_PROJ)/src/base/utils/Move.h \
	$(VEINS_PROJ)/src/base/utils/NetwToMacControlInfo.h \
	$(VEINS_PROJ)/src/base/utils/PassedMessage.h \
	$(VEINS_PROJ)/src/base/utils/SimpleAddress.h \
	$(VEINS_PROJ)/src/base/utils/miximkerneldefs.h \
	$(VEINS_PROJ)/src/modules/mac/ieee80211p/WaveAppToMac1609_4Interface.h \
	$(VEINS_PROJ)/src/modules/messages/Mac80211Pkt_m.h \
	$(VEINS_PROJ)/src/modules/mobility/traci/TraCIColor.h \
	$(VEINS_PROJ)/src/modules/mobility/traci/TraCIConstants.h \
	$(VEINS_PROJ)/src/modules/mobility/traci/TraCIMobility.h \
	$(VEINS_PROJ)/src/modules/mobility/traci/TraCIScenarioManager.h \
	$(VEINS_PROJ)/src/modules/obstacle/Obstacle.h \
	$(VEINS_PROJ)/src/modules/obstacle/ObstacleControl.h \
	$(VEINS_PROJ)/src/modules/utility/Consts80211p.h \
	$(VEINS_PROJ)/src/modules/world/annotations/AnnotationManager.h
$O/11RSUMobility.o: 11RSUMobility.cc \
	01ExtraClasses.h \
	11RSUMobility.h \
	15TraCI_Extend.h \
	$(VEINS_PROJ)/src/base/connectionManager/BaseConnectionManager.h \
	$(VEINS_PROJ)/src/base/connectionManager/NicEntry.h \
	$(VEINS_PROJ)/src/base/modules/BaseBattery.h \
	$(VEINS_PROJ)/src/base/modules/BaseMobility.h \
	$(VEINS_PROJ)/src/base/modules/BaseModule.h \
	$(VEINS_PROJ)/src/base/modules/BaseWorldUtility.h \
	$(VEINS_PROJ)/src/base/modules/BatteryAccess.h \
	$(VEINS_PROJ)/src/base/utils/Coord.h \
	$(VEINS_PROJ)/src/base/utils/FWMath.h \
	$(VEINS_PROJ)/src/base/utils/FindModule.h \
	$(VEINS_PROJ)/src/base/utils/HostState.h \
	$(VEINS_PROJ)/src/base/utils/MiXiMDefs.h \
	$(VEINS_PROJ)/src/base/utils/Move.h \
	$(VEINS_PROJ)/src/base/utils/miximkerneldefs.h \
	$(VEINS_PROJ)/src/modules/mobility/traci/TraCIColor.h \
	$(VEINS_PROJ)/src/modules/mobility/traci/TraCIConstants.h \
	$(VEINS_PROJ)/src/modules/mobility/traci/TraCIMobility.h \
	$(VEINS_PROJ)/src/modules/mobility/traci/TraCIScenarioManager.h \
	$(VEINS_PROJ)/src/modules/obstacle/Obstacle.h \
	$(VEINS_PROJ)/src/modules/obstacle/ObstacleControl.h \
	$(VEINS_PROJ)/src/modules/world/annotations/AnnotationManager.h
$O/12TrafficLight.o: 12TrafficLight.cc \
	01ExtraClasses.h \
	02ApplVBase.h \
	03ApplVBeacon.h \
	04ApplV_AID.h \
	05ApplVPlatoonMg.h \
	06ApplVManager.h \
	12TrafficLight.h \
	15TraCI_Extend.h \
	msg/Messages_m.h \
	$(VEINS_PROJ)/src/base/connectionManager/BaseConnectionManager.h \
	$(VEINS_PROJ)/src/base/connectionManager/ChannelAccess.h \
	$(VEINS_PROJ)/src/base/connectionManager/NicEntry.h \
	$(VEINS_PROJ)/src/base/messages/MacPkt_m.h \
	$(VEINS_PROJ)/src/base/modules/BaseApplLayer.h \
	$(VEINS_PROJ)/src/base/modules/BaseBattery.h \
	$(VEINS_PROJ)/src/base/modules/BaseLayer.h \
	$(VEINS_PROJ)/src/base/modules/BaseMobility.h \
	$(VEINS_PROJ)/src/base/modules/BaseModule.h \
	$(VEINS_PROJ)/src/base/modules/BaseWorldUtility.h \
	$(VEINS_PROJ)/src/base/modules/BatteryAccess.h \
	$(VEINS_PROJ)/src/base/utils/Coord.h \
	$(VEINS_PROJ)/src/base/utils/FWMath.h \
	$(VEINS_PROJ)/src/base/utils/FindModule.h \
	$(VEINS_PROJ)/src/base/utils/HostState.h \
	$(VEINS_PROJ)/src/base/utils/MiXiMDefs.h \
	$(VEINS_PROJ)/src/base/utils/Move.h \
	$(VEINS_PROJ)/src/base/utils/NetwToMacControlInfo.h \
	$(VEINS_PROJ)/src/base/utils/PassedMessage.h \
	$(VEINS_PROJ)/src/base/utils/SimpleAddress.h \
	$(VEINS_PROJ)/src/base/utils/miximkerneldefs.h \
	$(VEINS_PROJ)/src/modules/mac/ieee80211p/WaveAppToMac1609_4Interface.h \
	$(VEINS_PROJ)/src/modules/messages/Mac80211Pkt_m.h \
	$(VEINS_PROJ)/src/modules/mobility/traci/TraCIColor.h \
	$(VEINS_PROJ)/src/modules/mobility/traci/TraCIConstants.h \
	$(VEINS_PROJ)/src/modules/mobility/traci/TraCIMobility.h \
	$(VEINS_PROJ)/src/modules/mobility/traci/TraCIScenarioManager.h \
	$(VEINS_PROJ)/src/modules/mobility/traci/TraCIScenarioManagerLaunchd.h \
	$(VEINS_PROJ)/src/modules/obstacle/Obstacle.h \
	$(VEINS_PROJ)/src/modules/obstacle/ObstacleControl.h \
	$(VEINS_PROJ)/src/modules/utility/Consts80211p.h \
	$(VEINS_PROJ)/src/modules/world/annotations/AnnotationManager.h
$O/13ApplAdversary.o: 13ApplAdversary.cc \
	01ExtraClasses.h \
	13ApplAdversary.h \
	15TraCI_Extend.h \
	msg/Messages_m.h \
	$(VEINS_PROJ)/src/base/connectionManager/BaseConnectionManager.h \
	$(VEINS_PROJ)/src/base/connectionManager/ChannelAccess.h \
	$(VEINS_PROJ)/src/base/connectionManager/NicEntry.h \
	$(VEINS_PROJ)/src/base/messages/MacPkt_m.h \
	$(VEINS_PROJ)/src/base/modules/BaseApplLayer.h \
	$(VEINS_PROJ)/src/base/modules/BaseBattery.h \
	$(VEINS_PROJ)/src/base/modules/BaseLayer.h \
	$(VEINS_PROJ)/src/base/modules/BaseMobility.h \
	$(VEINS_PROJ)/src/base/modules/BaseModule.h \
	$(VEINS_PROJ)/src/base/modules/BaseWorldUtility.h \
	$(VEINS_PROJ)/src/base/modules/BatteryAccess.h \
	$(VEINS_PROJ)/src/base/utils/Coord.h \
	$(VEINS_PROJ)/src/base/utils/FWMath.h \
	$(VEINS_PROJ)/src/base/utils/FindModule.h \
	$(VEINS_PROJ)/src/base/utils/HostState.h \
	$(VEINS_PROJ)/src/base/utils/MiXiMDefs.h \
	$(VEINS_PROJ)/src/base/utils/Move.h \
	$(VEINS_PROJ)/src/base/utils/NetwToMacControlInfo.h \
	$(VEINS_PROJ)/src/base/utils/PassedMessage.h \
	$(VEINS_PROJ)/src/base/utils/SimpleAddress.h \
	$(VEINS_PROJ)/src/base/utils/miximkerneldefs.h \
	$(VEINS_PROJ)/src/modules/mac/ieee80211p/WaveAppToMac1609_4Interface.h \
	$(VEINS_PROJ)/src/modules/messages/Mac80211Pkt_m.h \
	$(VEINS_PROJ)/src/modules/mobility/traci/TraCIColor.h \
	$(VEINS_PROJ)/src/modules/mobility/traci/TraCIConstants.h \
	$(VEINS_PROJ)/src/modules/mobility/traci/TraCIMobility.h \
	$(VEINS_PROJ)/src/modules/mobility/traci/TraCIScenarioManager.h \
	$(VEINS_PROJ)/src/modules/obstacle/Obstacle.h \
	$(VEINS_PROJ)/src/modules/obstacle/ObstacleControl.h \
	$(VEINS_PROJ)/src/modules/utility/Consts80211p.h \
	$(VEINS_PROJ)/src/modules/world/annotations/AnnotationManager.h
$O/14Statistics.o: 14Statistics.cc \
	01ExtraClasses.h \
	14Statistics.h \
	15TraCI_Extend.h \
	$(VEINS_PROJ)/src/base/connectionManager/BaseConnectionManager.h \
	$(VEINS_PROJ)/src/base/connectionManager/NicEntry.h \
	$(VEINS_PROJ)/src/base/modules/BaseBattery.h \
	$(VEINS_PROJ)/src/base/modules/BaseMobility.h \
	$(VEINS_PROJ)/src/base/modules/BaseModule.h \
	$(VEINS_PROJ)/src/base/modules/BaseWorldUtility.h \
	$(VEINS_PROJ)/src/base/modules/BatteryAccess.h \
	$(VEINS_PROJ)/src/base/utils/Coord.h \
	$(VEINS_PROJ)/src/base/utils/FWMath.h \
	$(VEINS_PROJ)/src/base/utils/FindModule.h \
	$(VEINS_PROJ)/src/base/utils/HostState.h \
	$(VEINS_PROJ)/src/base/utils/MiXiMDefs.h \
	$(VEINS_PROJ)/src/base/utils/Move.h \
	$(VEINS_PROJ)/src/base/utils/NetwControlInfo.h \
	$(VEINS_PROJ)/src/base/utils/SimpleAddress.h \
	$(VEINS_PROJ)/src/base/utils/miximkerneldefs.h \
	$(VEINS_PROJ)/src/modules/mobility/traci/TraCIColor.h \
	$(VEINS_PROJ)/src/modules/mobility/traci/TraCIConstants.h \
	$(VEINS_PROJ)/src/modules/mobility/traci/TraCIMobility.h \
	$(VEINS_PROJ)/src/modules/mobility/traci/TraCIScenarioManager.h \
	$(VEINS_PROJ)/src/modules/obstacle/Obstacle.h \
	$(VEINS_PROJ)/src/modules/obstacle/ObstacleControl.h \
	$(VEINS_PROJ)/src/modules/world/annotations/AnnotationManager.h
$O/15TraCI_Extend.o: 15TraCI_Extend.cc \
	01ExtraClasses.h \
	15TraCI_Extend.h \
	$(VEINS_PROJ)/src/base/connectionManager/BaseConnectionManager.h \
	$(VEINS_PROJ)/src/base/connectionManager/NicEntry.h \
	$(VEINS_PROJ)/src/base/modules/BaseBattery.h \
	$(VEINS_PROJ)/src/base/modules/BaseMobility.h \
	$(VEINS_PROJ)/src/base/modules/BaseModule.h \
	$(VEINS_PROJ)/src/base/modules/BaseWorldUtility.h \
	$(VEINS_PROJ)/src/base/modules/BatteryAccess.h \
	$(VEINS_PROJ)/src/base/utils/Coord.h \
	$(VEINS_PROJ)/src/base/utils/FWMath.h \
	$(VEINS_PROJ)/src/base/utils/FindModule.h \
	$(VEINS_PROJ)/src/base/utils/HostState.h \
	$(VEINS_PROJ)/src/base/utils/MiXiMDefs.h \
	$(VEINS_PROJ)/src/base/utils/Move.h \
	$(VEINS_PROJ)/src/base/utils/miximkerneldefs.h \
	$(VEINS_PROJ)/src/modules/mobility/traci/TraCIColor.h \
	$(VEINS_PROJ)/src/modules/mobility/traci/TraCIConstants.h \
	$(VEINS_PROJ)/src/modules/mobility/traci/TraCIMobility.h \
	$(VEINS_PROJ)/src/modules/mobility/traci/TraCIScenarioManager.h \
	$(VEINS_PROJ)/src/modules/obstacle/Obstacle.h \
	$(VEINS_PROJ)/src/modules/obstacle/ObstacleControl.h \
	$(VEINS_PROJ)/src/modules/world/annotations/AnnotationManager.h
$O/16TraCI_App.o: 16TraCI_App.cc \
	01ExtraClasses.h \
	02ApplVBase.h \
	03ApplVBeacon.h \
	04ApplV_AID.h \
	05ApplVPlatoonMg.h \
	06ApplVManager.h \
	07VehicleAdd.h \
	08Warmup.h \
	09SpeedProfile.h \
	12TrafficLight.h \
	15TraCI_Extend.h \
	16TraCI_App.h \
	msg/Messages_m.h \
	$(VEINS_PROJ)/src/base/connectionManager/BaseConnectionManager.h \
	$(VEINS_PROJ)/src/base/connectionManager/ChannelAccess.h \
	$(VEINS_PROJ)/src/base/connectionManager/NicEntry.h \
	$(VEINS_PROJ)/src/base/messages/MacPkt_m.h \
	$(VEINS_PROJ)/src/base/modules/BaseApplLayer.h \
	$(VEINS_PROJ)/src/base/modules/BaseBattery.h \
	$(VEINS_PROJ)/src/base/modules/BaseLayer.h \
	$(VEINS_PROJ)/src/base/modules/BaseMobility.h \
	$(VEINS_PROJ)/src/base/modules/BaseModule.h \
	$(VEINS_PROJ)/src/base/modules/BaseWorldUtility.h \
	$(VEINS_PROJ)/src/base/modules/BatteryAccess.h \
	$(VEINS_PROJ)/src/base/utils/Coord.h \
	$(VEINS_PROJ)/src/base/utils/FWMath.h \
	$(VEINS_PROJ)/src/base/utils/FindModule.h \
	$(VEINS_PROJ)/src/base/utils/HostState.h \
	$(VEINS_PROJ)/src/base/utils/MiXiMDefs.h \
	$(VEINS_PROJ)/src/base/utils/Move.h \
	$(VEINS_PROJ)/src/base/utils/NetwToMacControlInfo.h \
	$(VEINS_PROJ)/src/base/utils/PassedMessage.h \
	$(VEINS_PROJ)/src/base/utils/SimpleAddress.h \
	$(VEINS_PROJ)/src/base/utils/miximkerneldefs.h \
	$(VEINS_PROJ)/src/modules/mac/ieee80211p/WaveAppToMac1609_4Interface.h \
	$(VEINS_PROJ)/src/modules/messages/Mac80211Pkt_m.h \
	$(VEINS_PROJ)/src/modules/mobility/traci/TraCIColor.h \
	$(VEINS_PROJ)/src/modules/mobility/traci/TraCIConstants.h \
	$(VEINS_PROJ)/src/modules/mobility/traci/TraCIMobility.h \
	$(VEINS_PROJ)/src/modules/mobility/traci/TraCIScenarioManager.h \
	$(VEINS_PROJ)/src/modules/mobility/traci/TraCIScenarioManagerLaunchd.h \
	$(VEINS_PROJ)/src/modules/obstacle/Obstacle.h \
	$(VEINS_PROJ)/src/modules/obstacle/ObstacleControl.h \
	$(VEINS_PROJ)/src/modules/utility/Consts80211p.h \
	$(VEINS_PROJ)/src/modules/world/annotations/AnnotationManager.h
$O/Mac1609_4_Mod.o: Mac1609_4_Mod.cc \
	01ExtraClasses.h \
	Mac1609_4_Mod.h \
	$(VEINS_PROJ)/src/base/connectionManager/BaseConnectionManager.h \
	$(VEINS_PROJ)/src/base/connectionManager/ChannelAccess.h \
	$(VEINS_PROJ)/src/base/connectionManager/NicEntry.h \
	$(VEINS_PROJ)/src/base/messages/AirFrame_m.h \
	$(VEINS_PROJ)/src/base/messages/ChannelSenseRequest_m.h \
	$(VEINS_PROJ)/src/base/messages/MacPkt_m.h \
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
	$(VEINS_PROJ)/src/base/utils/Coord.h \
	$(VEINS_PROJ)/src/base/utils/FWMath.h \
	$(VEINS_PROJ)/src/base/utils/FindModule.h \
	$(VEINS_PROJ)/src/base/utils/HostState.h \
	$(VEINS_PROJ)/src/base/utils/MiXiMDefs.h \
	$(VEINS_PROJ)/src/base/utils/Move.h \
	$(VEINS_PROJ)/src/base/utils/NetwToMacControlInfo.h \
	$(VEINS_PROJ)/src/base/utils/PassedMessage.h \
	$(VEINS_PROJ)/src/base/utils/SimpleAddress.h \
	$(VEINS_PROJ)/src/base/utils/miximkerneldefs.h \
	$(VEINS_PROJ)/src/modules/analogueModel/JakesFading.h \
	$(VEINS_PROJ)/src/modules/analogueModel/LogNormalShadowing.h \
	$(VEINS_PROJ)/src/modules/analogueModel/SimplePathlossModel.h \
	$(VEINS_PROJ)/src/modules/mac/ieee80211p/Mac1609_4.h \
	$(VEINS_PROJ)/src/modules/mac/ieee80211p/Mac80211pToPhy11pInterface.h \
	$(VEINS_PROJ)/src/modules/mac/ieee80211p/WaveAppToMac1609_4Interface.h \
	$(VEINS_PROJ)/src/modules/messages/Mac80211Pkt_m.h \
	$(VEINS_PROJ)/src/modules/messages/WaveShortMessage_m.h \
	$(VEINS_PROJ)/src/modules/phy/Decider80211p.h \
	$(VEINS_PROJ)/src/modules/phy/Decider80211pToPhy80211pInterface.h \
	$(VEINS_PROJ)/src/modules/phy/PhyLayer.h \
	$(VEINS_PROJ)/src/modules/phy/PhyLayer80211p.h \
	$(VEINS_PROJ)/src/modules/phy/SNRThresholdDecider.h \
	$(VEINS_PROJ)/src/modules/utility/Consts80211p.h
$O/TraCIColor.o: TraCIColor.cc \
	$(VEINS_PROJ)/src/modules/mobility/traci/TraCIColor.h
$O/zApplVPlatoonFormation.o: zApplVPlatoonFormation.cc \
	01ExtraClasses.h \
	02ApplVBase.h \
	03ApplVBeacon.h \
	04ApplV_AID.h \
	05ApplVPlatoonMg.h \
	06ApplVManager.h \
	15TraCI_Extend.h \
	msg/Messages_m.h \
	zApplVPlatoonFormation.h \
	$(VEINS_PROJ)/src/base/connectionManager/BaseConnectionManager.h \
	$(VEINS_PROJ)/src/base/connectionManager/ChannelAccess.h \
	$(VEINS_PROJ)/src/base/connectionManager/NicEntry.h \
	$(VEINS_PROJ)/src/base/messages/MacPkt_m.h \
	$(VEINS_PROJ)/src/base/modules/BaseApplLayer.h \
	$(VEINS_PROJ)/src/base/modules/BaseBattery.h \
	$(VEINS_PROJ)/src/base/modules/BaseLayer.h \
	$(VEINS_PROJ)/src/base/modules/BaseMobility.h \
	$(VEINS_PROJ)/src/base/modules/BaseModule.h \
	$(VEINS_PROJ)/src/base/modules/BaseWorldUtility.h \
	$(VEINS_PROJ)/src/base/modules/BatteryAccess.h \
	$(VEINS_PROJ)/src/base/utils/Coord.h \
	$(VEINS_PROJ)/src/base/utils/FWMath.h \
	$(VEINS_PROJ)/src/base/utils/FindModule.h \
	$(VEINS_PROJ)/src/base/utils/HostState.h \
	$(VEINS_PROJ)/src/base/utils/MiXiMDefs.h \
	$(VEINS_PROJ)/src/base/utils/Move.h \
	$(VEINS_PROJ)/src/base/utils/NetwToMacControlInfo.h \
	$(VEINS_PROJ)/src/base/utils/PassedMessage.h \
	$(VEINS_PROJ)/src/base/utils/SimpleAddress.h \
	$(VEINS_PROJ)/src/base/utils/miximkerneldefs.h \
	$(VEINS_PROJ)/src/modules/mac/ieee80211p/WaveAppToMac1609_4Interface.h \
	$(VEINS_PROJ)/src/modules/messages/Mac80211Pkt_m.h \
	$(VEINS_PROJ)/src/modules/mobility/traci/TraCIColor.h \
	$(VEINS_PROJ)/src/modules/mobility/traci/TraCIConstants.h \
	$(VEINS_PROJ)/src/modules/mobility/traci/TraCIMobility.h \
	$(VEINS_PROJ)/src/modules/mobility/traci/TraCIScenarioManager.h \
	$(VEINS_PROJ)/src/modules/obstacle/Obstacle.h \
	$(VEINS_PROJ)/src/modules/obstacle/ObstacleControl.h \
	$(VEINS_PROJ)/src/modules/utility/Consts80211p.h \
	$(VEINS_PROJ)/src/modules/world/annotations/AnnotationManager.h
$O/zApplVPlatoonFormation2.o: zApplVPlatoonFormation2.cc \
	01ExtraClasses.h \
	02ApplVBase.h \
	03ApplVBeacon.h \
	04ApplV_AID.h \
	05ApplVPlatoonMg.h \
	06ApplVManager.h \
	15TraCI_Extend.h \
	msg/Messages_m.h \
	zApplVPlatoonFormation.h \
	zApplVPlatoonFormation2.h \
	$(VEINS_PROJ)/src/base/connectionManager/BaseConnectionManager.h \
	$(VEINS_PROJ)/src/base/connectionManager/ChannelAccess.h \
	$(VEINS_PROJ)/src/base/connectionManager/NicEntry.h \
	$(VEINS_PROJ)/src/base/messages/MacPkt_m.h \
	$(VEINS_PROJ)/src/base/modules/BaseApplLayer.h \
	$(VEINS_PROJ)/src/base/modules/BaseBattery.h \
	$(VEINS_PROJ)/src/base/modules/BaseLayer.h \
	$(VEINS_PROJ)/src/base/modules/BaseMobility.h \
	$(VEINS_PROJ)/src/base/modules/BaseModule.h \
	$(VEINS_PROJ)/src/base/modules/BaseWorldUtility.h \
	$(VEINS_PROJ)/src/base/modules/BatteryAccess.h \
	$(VEINS_PROJ)/src/base/utils/Coord.h \
	$(VEINS_PROJ)/src/base/utils/FWMath.h \
	$(VEINS_PROJ)/src/base/utils/FindModule.h \
	$(VEINS_PROJ)/src/base/utils/HostState.h \
	$(VEINS_PROJ)/src/base/utils/MiXiMDefs.h \
	$(VEINS_PROJ)/src/base/utils/Move.h \
	$(VEINS_PROJ)/src/base/utils/NetwToMacControlInfo.h \
	$(VEINS_PROJ)/src/base/utils/PassedMessage.h \
	$(VEINS_PROJ)/src/base/utils/SimpleAddress.h \
	$(VEINS_PROJ)/src/base/utils/miximkerneldefs.h \
	$(VEINS_PROJ)/src/modules/mac/ieee80211p/WaveAppToMac1609_4Interface.h \
	$(VEINS_PROJ)/src/modules/messages/Mac80211Pkt_m.h \
	$(VEINS_PROJ)/src/modules/mobility/traci/TraCIColor.h \
	$(VEINS_PROJ)/src/modules/mobility/traci/TraCIConstants.h \
	$(VEINS_PROJ)/src/modules/mobility/traci/TraCIMobility.h \
	$(VEINS_PROJ)/src/modules/mobility/traci/TraCIScenarioManager.h \
	$(VEINS_PROJ)/src/modules/obstacle/Obstacle.h \
	$(VEINS_PROJ)/src/modules/obstacle/ObstacleControl.h \
	$(VEINS_PROJ)/src/modules/utility/Consts80211p.h \
	$(VEINS_PROJ)/src/modules/world/annotations/AnnotationManager.h

