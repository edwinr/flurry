//-------------------------------------------------------------------
//  File:   Std.h
//  Created:  02/12/00 9:01:PM
//  Author:   Aaron Hilton
//  Comments: Standard header file to include all source files.
//            (Precompiled header)
//-------------------------------------------------------------------
#ifndef __STD_h_
#define __STD_h_

#ifdef __APPLE__
#include <OpenGL/glu.h>
#else
#ifdef _WIN32
#include <Windows.h>
#endif
#include <GL/glu.h>
#endif


#include "PTypes.h"
#include "Gl_saver.h"

float FastDistance2D(float x, float y);

#define RandFlt(min, max) (min + (max - min) * rand() / (float)RAND_MAX)

#define RandBell(scale) \
    (scale * (1.0f - (rand() + rand() + rand()) / ((float)RAND_MAX * 1.5f)))

#endif  // _STD_h_
