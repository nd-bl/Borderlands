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
//  Window.h
//  Borderlands
//
//  Contains elements of chuck_fft by Ge Wang and Perry Cook
//  Created by Christopher Carlson on 11/13/11.
//


#ifndef WINDOW_H
#define WINDOW_H

#include <cmath>
#include "math.h"
#include "assert.h"
#include "globals.h"
#include <iostream>
#include <Stk.h>

using namespace std;

enum
{
    HANNING,
    TRIANGLE,
    EXPDEC,
    REXPDEC,
    SINC,
    RANDOM_WIN
};

class Window
{
public:
    static Window &instance();
    
    // return window
    SAMPLE *getWindow(unsigned int windowType);
    
    // resize windows - future possibility, but probably not needed
    // void resizeWindows(unsigned long length);

    int numWindows();
    
protected:
    // generate windows
    void generateWindows(unsigned long length);
    
    // window function prototypes
    void hanning(SAMPLE *window, unsigned long length);
    
    // void trapezoid(SAMPLE *window, unsigned long length);
    void triangle(SAMPLE *window, unsigned long length);
    void expdec(SAMPLE *forWin, SAMPLE *revWin, unsigned long length);
    void sinc(SAMPLE *window, unsigned long length, int numZeroCross = 6);

private:
    ~Window();
    Window(unsigned long length = 2048);
    
    SAMPLE *_hanningWin;
    //SAMPLE * trapWin;
    SAMPLE *_triWin;
    SAMPLE *_expDecWin;
    SAMPLE *_rexpDecWin;
    SAMPLE *_sincWin;
};

#endif
