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
//  Window.cpp
//  Borderlands
//
//  Created by Christopher Carlson on 11/13/11.
//

#include "Window.h"

Window::~Window()
{
    delete []_hanningWin;
    delete []_triWin;
    //delete []_trapWin;
    delete []_sincWin;
    delete []_rexpDecWin;
    delete []_expDecWin;
}

Window::Window(unsigned long length)
{
    _hanningWin = new SAMPLE[length];
    _triWin = new SAMPLE[length];
    //_trapWin = new SAMPLE[length];
    _expDecWin = new SAMPLE[length];
    _rexpDecWin = new SAMPLE[length];
    _sincWin = new SAMPLE[length];
    
    // create
    generateWindows(length);
}

int
Window::numWindows()
{
    return 6;
}

Window &
Window::instance()
{
    static Window *_window = NULL;
    if (_window == NULL)
        _window = new Window(WINDOW_LEN);
        
    return *_window;
}

//
////resize windows
//void Window::resizeWindows(unsigned long length){
//    
//    if (hanningWin != NULL){
//        delete [] hanningWin;
//        hanningWin = new SAMPLE[length];
//    }
//    
//    if (triWin != NULL){
//        delete [] triWin;       
//        triWin = new SAMPLE[length];
//    }
//    
//    
//    if (trapWin != NULL){
//        delete [] trapWin;
//        trapWin = new SAMPLE[length];
//        
//        
//    }
//    if (sincWin != NULL){
//        delete [] sincWin;
//        sincWin = new SAMPLE[length];
//        
//    }
//    
//    if (rexpDecWin != NULL){
//        delete [] rexpDecWin;
//        rexpDecWin = new SAMPLE[length];
//        
//    }
//    if (expDecWin != NULL){
//        delete [] expDecWin;
//        expDecWin = new SAMPLE[length];
//    }
//    
//    
//    
//}

// create all windows
void
Window::generateWindows(unsigned long length)
{
    // clear all windows
    for (int i = 0; i < length; i++)
    {
        _hanningWin[i] = (SAMPLE)0.0;
        _triWin[i] = (SAMPLE)0.0;
        // _trapWin[i] = (double)0.0;
        _sincWin[i] = (SAMPLE)0.0;
        _expDecWin[i] = (SAMPLE)0.0;
        _rexpDecWin[i] = (SAMPLE)0.0;
    }
    
    hanning(_hanningWin, length);
    triangle(_triWin, length);
    //trapezoid(_trapWin, length);
    sinc(_sincWin, length, 8);
    expdec(_expDecWin, _rexpDecWin, length);
}

// window generation functions

//-------------------------------------------------------------------------------
// hanning / raised cosine window
//-------------------------------------------------------------------------------
void
Window::hanning(SAMPLE *window, unsigned long length)
{
    assert(length > 0);
    
    unsigned long i;
    double pi, phase = 0, delta;
    
    delta = 2 * PI / (double) length;
    
    for(i = 0; i < length; i++)
    {
        window[i] = (SAMPLE)(0.5 * (1.0 - cos(phase)));
        phase += delta;
    }
}

//-------------------------------------------------------------------------------
// 
//-------------------------------------------------------------------------------
//void Window::trapezoid( SAMPLE *window, unsigned long length )
//{
//    assert(length > 0);
//    
//}


//-------------------------------------------------------------------------------
// triangle window
//-------------------------------------------------------------------------------
void
Window::triangle(SAMPLE *window, unsigned long length)
{
    assert(length > 0);
    
    double norm = (2.0 / ((double)length - 1));
    double invnorm = 1.0/norm;
    
    for (int i = 0; i < length; i++)
        window[i] = norm * (invnorm - fabs(i - invnorm));
}

//-------------------------------------------------------------------------------
// 
//-------------------------------------------------------------------------------
void
Window::expdec(SAMPLE *forWin, SAMPLE *revWin, unsigned long length)
{
    assert(length > 0);
    
    //create decaying exponential (forward and reverse)
    
    //exp time constant (in samples)
    double tau = 512.0;
    double tauInv = 1.0/tau;
    double eps = 0.0005;
    unsigned long transition = length - 48;
    
    for (int i = 0; i < length; i++)
    {
        forWin[i] = exp(-((double) i)*tauInv);
        if (revWin)
            revWin[length - i - 1] = forWin[i]; 
    }
}

//-------------------------------------------------------------------------------
// SINC window with flexible number of zero crossings
//-------------------------------------------------------------------------------
void
Window::sinc(SAMPLE *window, unsigned long length, int numZeroCross)
{
    //note - numZeroCross should be even number, otherwise window will shift.
    //note also - numZeroCross = 1 is Lanczos window - main lobe
    double inc = numZeroCross/(double)length;
    double x = -(numZeroCross)/2.0;
   // std::cout << numZeroCross << endl;
    
    for (int i = 0; i < length; i++)
    {
        if (x != 0)
            _sincWin[i] = fabs(sin( PI * x)/(PI * x));
        else
            _sincWin[i] = (double) 1.0;

        x += inc;
    }
}

//return pointer to required window
SAMPLE *
Window::getWindow(unsigned int windowType)
{
    switch (windowType)
    {
        case HANNING:
            return _hanningWin;
            break;
            
        case TRIANGLE:
           // cout << "triangle-selected"<<endl;
            return _triWin;
            break;
            
            //        case TRAPEZOID:
            //            return trapWin;
            //            break;
            
        case EXPDEC:
            return _expDecWin;
            break;
            
        case REXPDEC:
            return _rexpDecWin;
            break;
            
        case SINC:
            return _sincWin;
            break;
            
        default:
            return _hanningWin;
            break;
    }
}






