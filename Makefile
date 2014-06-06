#
# OMNeT++/OMNEST Makefile for VENTOS
#
# This file was generated with the command:
#  opp_makemake -f -O out -d application -X. -I/home/dylan/vgrid/VENTOS/eigen-3.2.1 -I/home/dylan/vgrid/VENTOS/rapidxml-1.13 -L../mixim-sommer/out/$(CONFIGNAME)/src/modules -L../mixim-sommer/out/$(CONFIGNAME)/src/base -L../mixim-sommer/out/$(CONFIGNAME)/tests/testUtils -L./out/$(CONFIGNAME)/application -L./out/$(CONFIGNAME)/application/msg -lmiximmodules -lmiximbase -lmiximtestUtils -KMIXIM_SOMMER_PROJ=../mixim-sommer
#

# Name of target to be created (-o option)
TARGET = VENTOS$(EXE_SUFFIX)

# User interface (uncomment one) (-u option)
USERIF_LIBS = $(ALL_ENV_LIBS) # that is, $(TKENV_LIBS) $(CMDENV_LIBS)
#USERIF_LIBS = $(CMDENV_LIBS)
#USERIF_LIBS = $(TKENV_LIBS)

# Additional object and library files to link with
EXTRA_OBJS =

# Additional libraries (-L, -l options)
LIBS = -L../mixim-sommer/out/$(CONFIGNAME)/src/modules -L../mixim-sommer/out/$(CONFIGNAME)/src/base -L../mixim-sommer/out/$(CONFIGNAME)/tests/testUtils -Lout/$(CONFIGNAME)/application -Lout/$(CONFIGNAME)/application/msg  -lmiximmodules -lmiximbase -lmiximtestUtils
LIBS += -Wl,-rpath,`abspath ../mixim-sommer/out/$(CONFIGNAME)/src/modules` -Wl,-rpath,`abspath ../mixim-sommer/out/$(CONFIGNAME)/src/base` -Wl,-rpath,`abspath ../mixim-sommer/out/$(CONFIGNAME)/tests/testUtils` -Wl,-rpath,`abspath out/$(CONFIGNAME)/application` -Wl,-rpath,`abspath out/$(CONFIGNAME)/application/msg`

# Output directory
PROJECT_OUTPUT_DIR = out
PROJECTRELATIVE_PATH =
O = $(PROJECT_OUTPUT_DIR)/$(CONFIGNAME)/$(PROJECTRELATIVE_PATH)

# Other makefile variables (-K)
MIXIM_SOMMER_PROJ=../mixim-sommer

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

$O/$(TARGET):  submakedirs $(wildcard $(EXTRA_OBJS)) Makefile
	@$(MKPATH) $O
	@echo Creating executable: $@
	$(Q)$(CXX) $(LDFLAGS) -o $O/$(TARGET)   $(EXTRA_OBJS) $(AS_NEEDED_OFF) $(WHOLE_ARCHIVE_ON) $(LIBS) $(WHOLE_ARCHIVE_OFF) $(OMNETPP_LIBS)

submakedirs:  application_dir

.PHONY: all clean cleanall depend msgheaders  application
application: application_dir

application_dir:
	cd application && $(MAKE) all

msgheaders:
	$(Q)cd application && $(MAKE) msgheaders

clean:
	$(qecho) Cleaning...
	$(Q)-rm -rf $O
	$(Q)-rm -f VENTOS VENTOS.exe libVENTOS.so libVENTOS.a libVENTOS.dll libVENTOS.dylib

	-$(Q)cd application && $(MAKE) clean

cleanall: clean
	$(Q)-rm -rf $(PROJECT_OUTPUT_DIR)

depend:
	$(qecho) Creating dependencies...
	$(Q)-cd application && if [ -f Makefile ]; then $(MAKE) depend; fi

