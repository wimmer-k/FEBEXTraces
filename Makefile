.EXPORT_ALL_VARIABLES:

.PHONY: all

BIN_DIR = $(BASEDIR)/bin
LIB_DIR = $(BASEDIR)/lib
COMMON_DIR = $(BASEDIR)/progs/common

GO4_DIR = /cvmfs/eel.gsi.de/debian10-x86_64/go4/602-00
ROOTCFLAGS  := $(shell root-config --cflags)
ROOTLIBS    := $(shell root-config --libs)
ROOTGLIBS   := $(shell root-config --glibs)
ROOTINC     := -I$(shell root-config --incdir)

CPP         = g++
CFLAGS	    = -Wall -Wno-long-long -g -O3 $(ROOTCFLAGS) -fPIC -D_FILE_OFFSET_BITS=64 -MMD

INCLUDES    = -I./inc -I$(GO4_DIR)/include/ -I$(COMMON_DIR)
BASELIBS    = -lm $(ROOTLIBS) $(ROOTGLIBS) -L$(LIB_DIR) -L$(GO4_DIR)/lib/ 
LIBS  	    =  $(BASELIBS) -lCommandLineInterface -lGo4AnalBase -lGo4Analysis -lGo4Base -lGo4TaskHandler -lFEBEX

LFLAGS	    = -g -fPIC -shared
CFLAGS 	    += -Wl,--no-as-needed $(SWITCH)
LFLAGS 	    += -Wl,--no-as-needed 
CFLAGS 	    += -Wno-unused-variable -Wno-unused-but-set-variable -Wno-write-strings -std=c++11 -fdiagnostics-color

ifndef GO4SYS
GO4SYS = $(shell go4-config --go4sys)
endif

USING_ROOT_6 = $(shell expr $(shell root-config --version | cut -f1 -d.) \>= 6)

ifeq ($(USING_ROOT_6),1)
	EXTRAS =  TraceDictionary_rdict.pcm
endif

O_FILES = build/Trace.o build/TraceDictionary.o

all: $(EXTRAS) $(LIB_DIR)/libFEBEX.so ShowTraces

ShowTraces: ShowTraces.cc $(LIB_DIR)/libFEBEX.so
	@echo "Compiling $@"
	@$(CPP) $(CFLAGS) $(INCLUDES) $< $(LIBS) -o $(BIN_DIR)/$@ 

WhiteRabbits: WhiteRabbits.cc 
	@echo "Compiling $@"
	@$(CPP) $(CFLAGS) $(INCLUDES) $< $(LIBS) -o $(BIN_DIR)/$@ 

$(LIB_DIR)/libFEBEX.so: $(O_FILES)
	@echo "Making $@"
	@$(CPP) $(LFLAGS) -o $@ $^ -lc


build/%.o: src/%.cc inc/%.hh
	@echo "Compiling $@"
	@mkdir -p $(dir $@)
	@$(CPP) $(CFLAGS) $(INCLUDES) -c $< -o $@ 

build/%Dictionary.o: build/%Dictionary.cc
	@echo "Compiling $@"
	@mkdir -p $(dir $@)
	@$(CPP) $(CFLAGS) $(INCLUDES) -fPIC -c $< -o $@

build/%Dictionary.cc: inc/%.hh inc/%LinkDef.h
	@echo "Building $@"
	@mkdir -p $(dir $@)
	@rootcint -f $@ -c $(INCLUDES) $(notdir $^)

build/%Dictionary_rdict.pcm: build/%Dictionary.cc
	@echo "Confirming $@"
	@touch $@

%Dictionary_rdict.pcm: build/%Dictionary_rdict.pcm 
	@echo "Placing $@"
	@cp build/$@ $(LIB_DIR)

clean:
	@echo "Cleaning up"
	@rm -rf build doc
	@rm -f inc/*~ src/*~ *~
	@rm -f scripts/*~  scripts/*_C.*
