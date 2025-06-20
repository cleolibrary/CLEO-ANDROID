#pragma once

#ifdef ANDROID
#include <jni.h>
#include <dlfcn.h>
#include <pthread.h>
#include <sys/mman.h>
#include <android/log.h>
#include <stl/_vector.h> // if ide can't resolve standard vector include
//#include <list>
//#include <iterator>
//#include <limits.h>
#include <ctype.h>
//#include <sys/time.h>
#else
#ifdef __INTELLISENSE__
#define __STDC__
#endif
#include <pspsdk.h>
#include <pspstdio.h>
#include <pspkernel.h>
#include <pspctrl.h>
#include <pspdebug.h>
#include <pspdisplay.h>
#endif

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <dirent.h>
#include <math.h>
#include <algorithm>
#include <string>
#include <vector>
#include <map>

#define cast reinterpret_cast

typedef char *LPSTR;
typedef const char *LPCSTR;
typedef uint8_t *ptr;

#define __STATIC_ASSERT(expr) typedef char __static_assert[expr ? 1 : -1];

#define STRUCT_TYPE(x) x
#define STRUCT_SIZE(struc, size) __STATIC_ASSERT(sizeof(STRUCT_TYPE(struc)) == size)

typedef std::basic_string<uint16_t, std::char_traits<uint16_t>, std::allocator<uint16_t> > wide_string;

#pragma GCC diagnostic ignored "-Wswitch"
#pragma GCC diagnostic ignored "-Wsign-compare"
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wreturn-type"

#ifdef ANDROID

#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"

#pragma GCC diagnostic ignored "-Wpragmas"
#pragma GCC diagnostic ignored "-Wunused-local-typedefs"

#else

#pragma GCC diagnostic ignored "-Wuninitialized"

#endif
