//------------------------------------------------------------------------------
// BORDERLANDS:  An interactive granular sampler.  
//------------------------------------------------------------------------------
// More information is available at 
//     http::/ccrma.stanford.edu/~carlsonc/256a/Borderlands/index.html
//
//
// Copyright (C) 2011  Christopher Carlson
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, version 3.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.


//
//  globals.h
//  Borderlands
//
//  Created by Christopher Carlson on 11/15/11.
//


#ifndef GLOBALS_H
#define GLOBALS_H

#include <stdlib.h>
#include <GTime.h>

#define USE_SAMPLE_FLOAT 1

// create sample datatype
#if USE_SAMPLES_DOUBLE
#define SAMPLE double
#else
#define SAMPLE float
#endif

// create rtaudio format
//#define FORMAT RTAUDIO_FLOAT64
#define FORMAT RTAUDIO_FLOAT32

// set the sample rate
#define SRATE 44100

// number of output channels
#define CHANNELS 2

// window length
#define WINDOW_LEN 2048

// graphics picking
#define NAMEINCREMENT 100

// function to produce a random float
#define randf() ( (float)rand()/RAND_MAX )

#define randd() ( (double)rand()/RAND_MAX )

// global attenuation to prevent clipping
static const double globalAtten = 0.5;

//osc local port
//#define LOCAL_PORT 10001
//#define OUTPUT_BUFFER_SIZE 1024

//#define FPS 25.0
//#define FPS 60.0
#define FPS -1.0 // for default

////-----------------------------------------------------------------------------
//// Defines a point in a 2D space (coords x, y)
////-----------------------------------------------------------------------------

//NOW A CLASS INSTEAD - see pt2d.h and pt2d.cpp
//struct pt2d
//{ 
//    pt2d( float _x, float _y ) : x(_x), y(_y) {};
//    
//    float x;
//    float y;
//};

#endif
