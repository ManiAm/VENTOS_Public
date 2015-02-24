#
# OMNeT++/OMNEST Makefile for application
#
# This file was generated with the command:
#  opp_makemake -f -o application -O out -d pedestrian -d trafficLight -d msg -d rsu -d bike -d adversary -d router -d traci -d vehicle -d global -I/home/philip/Desktop/VENTOS/VENTOS/eigen-3.2.1 -I/home/philip/Desktop/VENTOS/VENTOS/rapidxml-1.13 -L/home/philip/Desktop/Veins/veins/out/$$\(CONFIGNAME\)/src -L../out/$$\(CONFIGNAME\)/application/pedestrian -L../out/$$\(CONFIGNAME\)/application/trafficLight -L../out/$$\(CONFIGNAME\)/application/msg -L../out/$$\(CONFIGNAME\)/application/rsu -L../out/$$\(CONFIGNAME\)/application/bike -L../out/$$\(CONFIGNAME\)/application/adversary -L../out/$$\(CONFIGNAME\)/application/router -L../out/$$\(CONFIGNAME\)/application/traci -L../out/$$\(CONFIGNAME\)/application/vehicle -L../out/$$\(CONFIGNAME\)/application/global -lveins -KVEINS_PROJ=/home/philip/Desktop/Veins/veins
#

# Name of target to be created (-o option)
TARGET = application$(EXE_SUFFIX)

# User interface (uncomment one) (-u option)
USERIF_LIBS = $(ALL_ENV_LIBS) # that is, $(TKENV_LIBS) $(CMDENV_LIBS)
#USERIF_LIBS = $(CMDENV_LIBS)
#USERIF_LIBS = $(TKENV_LIBS)

# C++ include paths (with -I)
INCLUDE_PATH = -I../eigen-3.2.1 -I../rapidxml-1.13 -I.

# Additional object and library files to link with
EXTRA_OBJS =

# Additional libraries (-L, -l options)
LIBS = -L$(VEINS_PROJ)/out/$(CONFIGNAME)/src -L../out/$(CONFIGNAME)/application/pedestrian -L../out/$(CONFIGNAME)/application/trafficLight -L../out/$(CONFIGNAME)/application/msg -L../out/$(CONFIGNAME)/application/rsu -L../out/$(CONFIGNAME)/application/bike -L../out/$(CONFIGNAME)/application/adversary -L../out/$(CONFIGNAME)/application/router -L../out/$(CONFIGNAME)/application/traci -L../out/$(CONFIGNAME)/application/vehicle -L../out/$(CONFIGNAME)/application/global  -lveins
LIBS += -Wl,-rpath,`abspath $(VEINS_PROJ)/out/$(CONFIGNAME)/src` -Wl,-rpath,`abspath ../out/$(CONFIGNAME)/application/pedestrian` -Wl,-rpath,`abspath ../out/$(CONFIGNAME)/application/trafficLight` -Wl,-rpath,`abspath ../out/$(CONFIGNAME)/application/msg` -Wl,-rpath,`abspath ../out/$(CONFIGNAME)/application/rsu` -Wl,-rpath,`abspath ../out/$(CONFIGNAME)/application/bike` -Wl,-rpath,`abspath ../out/$(CONFIGNAME)/application/adversary` -Wl,-rpath,`abspath ../out/$(CONFIGNAME)/application/router` -Wl,-rpath,`abspath ../out/$(CONFIGNAME)/application/traci` -Wl,-rpath,`abspath ../out/$(CONFIGNAME)/application/vehicle` -Wl,-rpath,`abspath ../out/$(CONFIGNAME)/application/global`

# Output directory
PROJECT_OUTPUT_DIR = ../out
PROJECTRELATIVE_PATH = application
O = $(PROJECT_OUTPUT_DIR)/$(CONFIGNAME)/$(PROJECTRELATIVE_PATH)

# Object files for local .cc and .msg files
OBJS =

# Message files
MSGFILES =

# Other makefile variables (-K)
VEINS_PROJ=/home/philip/Desktop/Veins/veins

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
OMNETPP_LIBS = -L"$(OMNETPP_LIB_SUBDIR)" -L"$(OMNETPP_LIB_DIR)" -loppmain$D $(USERIF_LIBS) $(KERNEL_LIBS) $(SYS_LIBS)

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
# inserted from file 'makefrag':
submakedirs:  msg_dir traci_dir global_dir trafficLight_dir rsu_dir vehicle_dir router_dir pedestrian_dir bike_dir adversary_dir

# <<<
#------------------------------------------------------------------------------

# Main target
all: $O/$(TARGET)
	$(Q)$(LN) $O/$(TARGET) .

$O/$(TARGET): $(OBJS) submakedirs $(wildcard $(EXTRA_OBJS)) Makefile
	@$(MKPATH) $O
	@echo Creating executable: $@
	$(Q)$(CXX) $(LDFLAGS) -o $O/$(TARGET)  $(OBJS) $(EXTRA_OBJS) $(AS_NEEDED_OFF) $(WHOLE_ARCHIVE_ON) $(LIBS) $(WHOLE_ARCHIVE_OFF) $(OMNETPP_LIBS)

submakedirs:  pedestrian_dir trafficLight_dir msg_dir rsu_dir bike_dir adversary_dir router_dir traci_dir vehicle_dir global_dir

.PHONY: all clean cleanall depend msgheaders  pedestrian trafficLight msg rsu bike adversary router traci vehicle global
pedestrian: pedestrian_dir
trafficLight: trafficLight_dir
msg: msg_dir
rsu: rsu_dir
bike: bike_dir
adversary: adversary_dir
router: router_dir
traci: traci_dir
vehicle: vehicle_dir
global: global_dir

pedestrian_dir:
	cd pedestrian && $(MAKE) all

trafficLight_dir:
	cd trafficLight && $(MAKE) all

msg_dir:
	cd msg && $(MAKE) all

rsu_dir:
	cd rsu && $(MAKE) all

bike_dir:
	cd bike && $(MAKE) all

adversary_dir:
	cd adversary && $(MAKE) all

router_dir:
	cd router && $(MAKE) all

traci_dir:
	cd traci && $(MAKE) all

vehicle_dir:
	cd vehicle && $(MAKE) all

global_dir:
	cd global && $(MAKE) all

.SUFFIXES: .cc

$O/%.o: %.cc $(COPTS_FILE)
	@$(MKPATH) $(dir $@)
	$(qecho) "$<"
	$(Q)$(CXX) -c $(CXXFLAGS) $(COPTS) -o $@ $<

%_m.cc %_m.h: %.msg
	$(qecho) MSGC: $<
	$(Q)$(MSGC) -s _m.cc $(MSGCOPTS) $?

msgheaders: $(MSGFILES:.msg=_m.h)
	$(Q)cd pedestrian && $(MAKE) msgheaders
	$(Q)cd trafficLight && $(MAKE) msgheaders
	$(Q)cd msg && $(MAKE) msgheaders
	$(Q)cd rsu && $(MAKE) msgheaders
	$(Q)cd bike && $(MAKE) msgheaders
	$(Q)cd adversary && $(MAKE) msgheaders
	$(Q)cd router && $(MAKE) msgheaders
	$(Q)cd traci && $(MAKE) msgheaders
	$(Q)cd vehicle && $(MAKE) msgheaders
	$(Q)cd global && $(MAKE) msgheaders

clean:
	$(qecho) Cleaning...
	$(Q)-rm -rf $O
	$(Q)-rm -f application application.exe libapplication.so libapplication.a libapplication.dll libapplication.dylib
	$(Q)-rm -f ./*_m.cc ./*_m.h

	-$(Q)cd pedestrian && $(MAKE) clean
	-$(Q)cd trafficLight && $(MAKE) clean
	-$(Q)cd msg && $(MAKE) clean
	-$(Q)cd rsu && $(MAKE) clean
	-$(Q)cd bike && $(MAKE) clean
	-$(Q)cd adversary && $(MAKE) clean
	-$(Q)cd router && $(MAKE) clean
	-$(Q)cd traci && $(MAKE) clean
	-$(Q)cd vehicle && $(MAKE) clean
	-$(Q)cd global && $(MAKE) clean

cleanall: clean
	$(Q)-rm -rf $(PROJECT_OUTPUT_DIR)

depend:
	$(qecho) Creating dependencies...
	$(Q)$(MAKEDEPEND) $(INCLUDE_PATH) -f Makefile -P\$$O/ -- $(MSG_CC_FILES)  ./*.cc
	$(Q)-cd pedestrian && if [ -f Makefile ]; then $(MAKE) depend; fi
	$(Q)-cd trafficLight && if [ -f Makefile ]; then $(MAKE) depend; fi
	$(Q)-cd msg && if [ -f Makefile ]; then $(MAKE) depend; fi
	$(Q)-cd rsu && if [ -f Makefile ]; then $(MAKE) depend; fi
	$(Q)-cd bike && if [ -f Makefile ]; then $(MAKE) depend; fi
	$(Q)-cd adversary && if [ -f Makefile ]; then $(MAKE) depend; fi
	$(Q)-cd router && if [ -f Makefile ]; then $(MAKE) depend; fi
	$(Q)-cd traci && if [ -f Makefile ]; then $(MAKE) depend; fi
	$(Q)-cd vehicle && if [ -f Makefile ]; then $(MAKE) depend; fi
	$(Q)-cd global && if [ -f Makefile ]; then $(MAKE) depend; fi

# DO NOT DELETE THIS LINE -- make depend depends on it.

