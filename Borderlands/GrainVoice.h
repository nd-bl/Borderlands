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
//  GrainVoice.h
//  Borderlands
//
//  Created by Christopher Carlson on 11/13/11.
//

#ifndef GRAINVOICE_H
#define GRAINVOICE_H

#include "globals.h"
#include "AudioFileSet.h"
#include "Window.h"

#include <vector>
#include <math.h>
#include <time.h>
#include <ctime>
#include <Stk.h>

#ifdef __MACOSX_CORE__
#include <GLUT/glut.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#endif

// forward declarations
class GrainVoice;
class GrainVis;
class Style;

// AUDIO CLASS
class GrainVoice
{ 
public:
    virtual ~GrainVoice();
    
    GrainVoice(Style *style,
               vector<AudioFile *> *soundSet,
               float durationMs,
               float thePitch);
    
    // dump samples into next buffer
    void nextBuffer(double *accumBuff,
                    unsigned int numFrames,
                    unsigned int bufferPos,
                    int name);
    
    // set on
    bool playMe(double *startPositions,
                double *startVols);

    // report state
    bool isPlaying();
    
    // queue up params for next grain
    void setDurationMs(float dur);
    
    // set/get playback rate
    void setPitch(float newPitch);
    
    // get playback rate
    float getPitch();    
    
    // volume
    void setVolume(float theVolNormed);
    float getVolume();
    
    // set spatialization
    void setChannelMultipliers(double *multipliers);
    
    // set playback direction
    void setDirection(float thedir);
    
    // change window type
    void setWindow(unsigned int windowType);
    
protected:
    // makes temp params permanent
    void updateParams();
    
private:
    // pointer to all audio file buffers
    vector <AudioFile *> *_sounds;
    
    // status
    bool _playingState;
    
    // param update required flag
    bool _newParam;
    
    // numsounds
    unsigned int _numSounds;
    
    // grain parameters
    float _duration;
    float _queuedDuration;
    double _winDurationSamps;
    double _pitch;
    float _queuedPitch;
    double _direction;
    float _queuedDirection; 
    double _playInc;
    
    // local volume (set by user)
    float _localAtten;
    float _queuedLocalAtten;
    
    // panning values
    double *_chanMults;
    double *_queuedChanMults;
    
    // audio files being sampled
    vector<int> *_activeSounds;
    
    // window type
    unsigned int _windowType;
    unsigned int _queuedWindowType;
    
    // window reading params
    double _winInc;
    double _winReader;
    
    // pointer to audio window (hanning, triangle, etc.)
    double *_window;
    
    //array of position values (in frames, not samples)
    //-1 means not in current soundfile
    double *_playPositions;
    double *_playVols;

    Style *_style;
};

// GRAPHICS CLASS
// handle display, picking.  
// register selection listener
class GrainVis
{
public:
    ~GrainVis();

    GrainVis(Style *style,
             unsigned int winWidth,
             unsigned int winHeight,
             float x, float y);

    void updateWinWidthHeight(unsigned int newWinWidth,
                              unsigned int newWinHeight);
        
    void draw();
    
    // position update function
    void moveTo(float x, float y);
    float getX();
    float getY();
    void trigger(float theDur);
    
private:
    bool _isOn;
    bool _firstTrigger;
    
    double _startTime;
    double _triggerTime;
    
    float _gX;
    float _gY;
    float _colR;
    float _colG;
    float _colB;
    float _colA;

    // float defG, defB;
    float _defR;
    float _defG;
    float _defB;
    float _defA;
    
    float _size;
    float _defSize;
    float _onSize; //GL point size
    float _durSec;

    unsigned int _winWidth;
    unsigned int _winHeight;

    Style *_style;
};




#endif
