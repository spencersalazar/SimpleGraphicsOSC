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

OBJECTS = $(addsuffix .o,$(SRCNAME)) \
../oscpack/osc/OscTypes.o ../oscpack/osc/OscReceivedElements.o \
../oscpack/osc/OscPrintReceivedElements.o \
../oscpack/ip/posix/NetworkingUtils.o ../oscpack/ip/posix/UdpSocket.o

COMMON_INCLUDES = $(addprefix -I, $(PLAT_INC)) -Ilibst/include
LINK += -Llibst/lib -lst -lfreetype -lpng -ljpeg

$(OUTNAME) : $(OBJECTS) 
	$(PLAT_CPP) -o $(OUTNAME) $(OBJECTS) $(LINK) $(PLAT_LINK)

%.o: %.cpp
	$(PLAT_CC) $(PLAT_CFLAGS) -c $(COMMON_INCLUDES) $(INCLUDES) $^ -o $@

clean:
	-rm -rf *.o $(OUTNAME)

