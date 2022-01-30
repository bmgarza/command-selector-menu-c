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

# NOTE: BMG (Jan. 04, 2022) We handle the log.c submodule differently than we handle microjson. We are always going to
#  check if we need to copy over the files from log.c to our source to make sure that we always have the latest from the
#  log.c repository.
LOGCDIR = lib/log.c/src

OBJDIR = obj

BINDIR = bin

# TODO: BMG (Nov. 25, 2021) Make sure you change this to be cpp specific
CPPSRC = \
	$(SRCDIR)/main.cpp \
	$(SRCDIR)/csm-commandline-arguments.cpp \
	$(SRCDIR)/csm-utilities.cpp

CCSRC = \
	$(SRCDIR)/color-print-lib.c \
	$(SRCDIR)/csm-print-dialogs.c \
	$(SRCDIR)/json-process-utils.c \
	$(SRCDIR)/log.c

COPY_FILES = \
	$(SRCDIR)/csm-test.json

BINDIR_COPY_FILES = $(COPY_FILES:$(SRCDIR)/%=$(BINDIR)/%)

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
DELOBJ = $(BLDOBJ)
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
$(APPNAME): $(BINDIR_COPY_FILES) $(BINDIR)/$(APPNAME)

$(BINDIR)/$(APPNAME): $(BLDOBJ)
	@echo "$(CPP) Building Application: $@"
	@$(CPP) $(CPPFLAGS) -o $@ $^ $(LDFLAGS)

# Creates the dependecy rules
%.d: ../$(SRCDIR)/%.cpp
	@$(CPP) $(CPPFLAGS) $(INCFLAGS) $< -MM -MT $(@:%.d=%.o) >$@

%.d: ../$(SRCDIR)/%.c
	@$(CC) $(CCFLAGS) $(INCFLAGS) $< -MM -MT $(@:%.d=%.o) >$@

# NOTE: BMG (Nov. 25, 2021) Make sure that the microjson library that we are using is built whenever we are building the
#  application.
$(MJSONDIR)/mjson.o: $(MJSONDIR)/mjson.c $(MJSONDIR)/mjson.h
	$(MAKE) -C $(MJSONDIR) mjson.o

# NOTE: BMG (Jan. 04, 2022) Copy over any non-source files over to the bin directory
$(BINDIR)/%: $(SRCDIR)/%
	@echo "Copying file: $<"
	@cp -r $< $@

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

$(OBJDIR)/log.o: $(SRCDIR)/log.c
	@echo "$(CC) Compiling: $<"
	@$(CC) $(CCFLAGS) -DLOG_USE_COLOR $(INCFLAGS) -o $@ -c $<

################### Cleaning rules for Unix-based OS ###################
# Cleans complete project
.PHONY: clean
clean:
	$(RM) $(DELOBJ) $(DEP) $(BINDIR)/$(APPNAME) $(BINDIR_COPY_FILES)

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