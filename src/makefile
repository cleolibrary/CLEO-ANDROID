TARGET = ../data/memstick/PSP/PLUGINS/cleo/cleo
OBJS = main.o utils.o text.o libres.o armhook.o touch.o ui.o plugins.o pattern.o core.o mutex.o strutils.o memutils.o psplang.o 
LIBS = -lstdc++ -lm

BUILD_PRX=1
PRX_EXPORTS=exports.exp

INCDIR = 
CFLAGS = -Os -G0 -Wall -D PSP -mpreferred-stack-boundary=4 -mabi=eabi
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)
LIBDIR =

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
