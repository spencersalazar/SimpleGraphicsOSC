#
# File			Makefile
# Title			Makes the demo
# Author		PowerVR
#
# Copyright		Copyright 2003-2004 by Imagination Technologies Limited.
#

.PHONY: clean

#SDKDIR  = ../../../../..
SDKDIR = ~/advanced/GFX/GFX_Linux_SDK/OGLES2/SDKPackage
VPATH = ../..

#PLATFORM = LinuxOMAP3

#include $(SDKDIR)/Builds/OGLES2/$(PLATFORM)/make_platform.mak
include make_platform.mak
#include oscpack/Makefile

SRCNAME = SimpleGraphics
OUTNAME = SimpleGraphics

OSC_DIR=oscpack
OSC_SRCS=osc/OscTypes.cpp osc/OscReceivedElements.cpp osc/OscPrintReceivedElements.cpp \
ip/posix/NetworkingUtils.cpp ip/posix/UdpSocket.cpp
OSC_SRCS:=$(addprefix $(OSC_DIR)/,$(OSC_SRCS))
OSC_OBJECTS=$(addsuffix .o,$(basename $(OSC_SRCS)))

SG_OBJECTS = $(addsuffix .o,$(SRCNAME))

OBJECTS=$(OSC_OBJECTS) $(SG_OBJECTS)

COMMON_INCLUDES = $(addprefix -I, $(PLAT_INC)) -Ilibst/include -I$(OSC_DIR)
LINK += -Llibst/lib -lst -lfreetype -lpng -ljpeg

$(OUTNAME) : $(OBJECTS) 
	$(PLAT_CPP) -o $(OUTNAME) $(OBJECTS) $(LINK) $(PLAT_LINK)

$(SG_OBJECTS): %.o: %.cpp
	$(PLAT_CC) $(PLAT_CFLAGS) -c $(COMMON_INCLUDES) $(INCLUDES) $^ -o $@

$(OSC_OBJECTS): %.o: %.cpp
	make -C oscpack $(@:oscpack/%.o=%.o)

clean:
	-rm -rf *.o $(OBJECTS) $(OUTNAME)

