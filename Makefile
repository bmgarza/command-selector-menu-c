########################################################################
####################### Makefile Template ##############################
########################################################################

# Compiler settings - Can be customized.
CPP = g++
CPPFLAGS = -std=c++11 -Wall -O2
LDFLAGS = 

CC = gcc
CCFLAGS = -Wall -O2

# Makefile settings - Can be customized.
APPNAME = csm

SRCDIR = src

INCDIR = $(SRCDIR)/include
MJSONDIR = lib/microjson
INCFLAGS = -I$(INCDIR) -I$(MJSONDIR)

OBJDIR = obj

BINDIR = bin

############## Do not change anything from here downwards! #############
# TODO: BMG (Nov. 25, 2021) Make sure you change this to be cpp specific
CPPSRC = \
	$(SRCDIR)/main.cpp \
	$(SRCDIR)/csm-commandline-arguments.cpp \
	$(SRCDIR)/csm-print-dialogs.cpp \
	$(SRCDIR)/csm-utilities.cpp

CCSRC = \
	$(SRCDIR)/color-print-lib.c

CPPSRCOBJ = \
	$(CPPSRC:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)
CCSRCOBJ = \
	$(CCSRC:$(SRCDIR)/%.c=$(OBJDIR)/%.o)
SRCOBJ = \
	$(CPPSRCOBJ) \
	$(CCSRCOBJ)
BLDOBJ = \
	$(SRCOBJ) \
	$(MJSONDIR)/mjson.o

DEP = $(SRCOBJ:$(OBJDIR)/%.o=$(OBJDIR)/%.d)
# UNIX-based OS variables & settings
RM = rm
DELOBJ = $(CPPSRCOBJ)
# Windows OS variables & settings
DEL = del
EXE = .exe
WDELOBJ = $(CPPSRC:$(SRCDIR)/%.cpp=$(OBJDIR)\\%.o)

########################################################################
####################### Targets beginning here #########################
########################################################################

all: makedirs $(APPNAME)

.PHONY: makedirs
makedirs: 
	@mkdir -p $(OBJDIR)
	@mkdir -p $(BINDIR)

# Builds the app
$(APPNAME): $(BLDOBJ)
	@echo "$(CPP) Building Application: $@"
	@$(CPP) $(CPPFLAGS) -o $(BINDIR)/$@ $^ $(LDFLAGS)

# Creates the dependecy rules
%.d: ../$(SRCDIR)/%.cpp
	@$(CPP) $(CPPFLAGS) $(INCFLAGS) $< -MM -MT $(@:%.d=%.o) >$@

%.d: ../$(SRCDIR)/%.c
	@$(CC) $(CCFLAGS) $(INCFLAGS) $< -MM -MT $(@:%.d=%.o) >$@

# NOTE: BMG (Nov. 25, 2021) Make sure that the microjson library that we are using is built whenever we are building the
#  application.
$(MJSONDIR)/mjson.o: $(MJSONDIR)/mjson.c $(MJSONDIR)/mjson.h
	$(MAKE) -C $(MJSONDIR) mjson.o

# Includes all .h files
# NOTE: BMG (Nov. 23, 2021) This line is basically pulling in all the .d files in the obj directory to make sure that we
#  are tracking the correct set of dependencies
-include $(DEP)

# Building rule for .o files and its .c/.cpp in combination with all .h
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	@echo "$(CPP) Compiling: $<"
	@$(CPP) $(CPPFLAGS) $(INCFLAGS) -o $@ -c $<

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@echo "$(CC) Compiling: $<"
	@$(CC) $(CCFLAGS) $(INCFLAGS) -o $@ -c $<

################### Cleaning rules for Unix-based OS ###################
# Cleans complete project
.PHONY: clean
clean:
	$(RM) $(DELOBJ) $(DEP) $(BINDIR)/$(APPNAME)

# Cleans only all files with the extension .d
.PHONY: cleandep
cleandep:
	$(RM) $(DEP)

#################### Cleaning rules for Windows OS #####################
# Cleans complete project
.PHONY: cleanw
cleanw:
	$(DEL) $(WDELOBJ) $(DEP) $(APPNAME)$(EXE)

# Cleans only all files with the extension .d
.PHONY: cleandepw
cleandepw:
	$(DEL) $(DEP)