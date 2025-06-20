TARGET = cleo
OBJS = main.o utils.o text.o libres.o armhook.o touch.o ui.o plugins.o pattern.o core.o mutex.o strutils.o memutils.o psplang.o
LIBS = -lstdc++ -lm

# Define to build this as a prx (instead of a static elf)
BUILD_PRX=1
# Define the name of our custom exports (minus the .exp extension)
PRX_EXPORTS=exports.exp

#USE_PSPSDK_LIBC = 1

INCDIR = 
CFLAGS = -Os -G0 -Wall -D PSP -mpreferred-stack-boundary=4
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)
LIBDIR =

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
