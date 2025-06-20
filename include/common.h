#pragma once

#ifdef ANDROID
#include <jni.h>
#else
#ifdef __INTELLISENSE__
#define __STDC__
#endif
#include <pspsdk.h>
#include <pspstdio.h>
#include <pspkernel.h>
#include <pspctrl.h>
#include <pspdisplay.h>
#endif

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

