# File Name       :Makefile
# Author          :Karthik Rao
# Version         :1.3
#
# To build executables : make
# To delete obj & executables : make clean
#


SRCROOT         = $(CURDIR)/src
BUILDROOT       = $(CURDIR)/build
OBJDIR          = $(BUILDROOT)/obj

# Defaults
AR              = ar
CC              = gcc
CXX             = g++
LD              = mpicxx
CXX             = mpicxx
STRIP           = strip

debug           ?= 0


# Paths
VPATH           += $(CURDIR)

# Targets
TARGETS         += $(BUILDROOT)/game
TARGETS         += $(BUILDROOT)/sequential

# Includes
INCLUDES        += -I$(CURDIR)

# game
GAME_OBJS       +=  $(OBJDIR)/game.o
SEQ_OBJS        +=  $(OBJDIR)/sequential.o

# Libraries
LIBS            += -lpthread
LIBS            += -lm

LINKFLAGS       += -L$(CURDIR)

# Flags
ifeq ($(debug),1)
COMMONFLAGS     += -DDEBUG=1
CFLAGS          += -g
STRIP            =  echo
else
COMMONFLAGS     += -DDEBUG=0
endif

COMMON_WARNINGS += -W
COMMON_WARNINGS += -Wall
COMMON_WARNINGS += -Werror
COMMON_WARNINGS += -Wextra
COMMON_WARNINGS += -Wformat
COMMON_WARNINGS += -Wmissing-braces
COMMON_WARNINGS += -Wno-cast-align
COMMON_WARNINGS += -Wparentheses
COMMON_WARNINGS += -Wshadow
COMMON_WARNINGS += -Wsign-compare
COMMON_WARNINGS += -Wswitch
COMMON_WARNINGS += -Wuninitialized
COMMON_WARNINGS += -Wunknown-pragmas
COMMON_WARNINGS += -Wunused-function
COMMON_WARNINGS += -Wunused-label
COMMON_WARNINGS += -Wunused-parameter
COMMON_WARNINGS += -Wno-unused-result
COMMON_WARNINGS += -Wunused-value
COMMON_WARNINGS += -Wunused-variable
CWARNS          += $(COMMON_WARNINGS)
#not valid for c++
#CWARNS          += -Wmissing-prototypes

CXXFLAGS        += -O3
CXXFLAGS        += -std=c++11

CFLAGS          += $(INCLUDES) $(COMMONFLAGS) $(CWARNS) $(CXXFLAGS) -Os
#CFLAGS          += -std=gnu99

# Rules
.PHONY : all
all: $(TARGETS)


$(BUILDROOT)/game: $(GAME_OBJS)
	@echo "Building $(notdir $@)"
	@$(CXX) $(CFLAGS) $(LIBS)  $(LINKFLAGS) -o $@ $^  $(LIBS)
	@$(STRIP) $@
	@echo "=== BUILD COMPLETE: $(notdir $@)"
	cp $(BUILDROOT)/game $(CURDIR)

$(BUILDROOT)/sequential: $(SEQ_OBJS)
	@echo "Building $(notdir $@)"
	@$(CXX) $(CFLAGS) $(LIBS)  $(LINKFLAGS) -o $@ $^  $(LIBS)
	@$(STRIP) $@
	@echo "=== BUILD COMPLETE: $(notdir $@)"
	cp $(BUILDROOT)/sequential $(CURDIR)

.PHONY : clean
clean:
	-rm -rf $(BUILDROOT)
	-rm $(CURDIR)/game
	-rm $(CURDIR)/sequential

$(SEQ_OBJS):			| $(BUILDROOT)/obj
$(GAME_OBJS):			| $(BUILDROOT)/obj

$(BUILDROOT)/obj:
	@mkdir -p $@

# General rules
$(OBJDIR)/%.o: %.c
	@echo "Compiling $(notdir $<)"
	@$(CXX) -c $(CFLAGS) -o $@ $<

$(OBJDIR)/%.o: %.cpp
	@echo "Compiling $(notdir $<)"
	@$(CXX) -c $(CFLAGS) -o $@ $<

