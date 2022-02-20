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
//  GrainCluster.h
//  Borderlands
//
//  Created by Christopher Carlson on 11/15/11.
//

#ifndef GRAIN_CLUSTER_H
#define GRAIN_CLUSTER_H

#include <map>
#include <vector>
#include <iostream>
#include <string>
#include <cstdlib>
#include <time.h>
#include <ctime>
#include <Stk.h>

#include "GrainVoice.h"
#include "globals.h"
#include "Window.h"
#include "Thread.h"
#include "SoundRect.h"

// direction modes
enum
{
    FORWARD,
    BACKWARD,
    RANDOM_DIR
};

// spatialization modes
enum
{
    UNITY,
    STEREO,
    AROUND
}; // eventually include channel list specification and VBAP?

using namespace std;

// forward declarations
class GrainCluster;
class GrainClusterVis;
class Style;

// ids
static unsigned int clusterId = 0;

//class interface
class GrainCluster
{    
public:
    virtual ~GrainCluster();
    
    GrainCluster(Style *style,
                 vector<AudioFile *> *soundSet, float numVoices);
    
    // compute next buffer of audio (accumulate from grains)
    void nextBuffer(double * accumBuff, unsigned int numFrames);
    
    // CLUSTER PARAMETER accessors/mutators
    // set duration for all grains
    void setDurationMs(float dur);
    float getDurationMs();
    
    // overlap
    void setOverlap(float targetOverlap);
    float getOverlap();
    
    // pitch
    void setPitch(float targetPitch);
    float getPitch();
    
    // pitch lfo methods
    void setPitchLFOFreq(float pfreq);
    float getPitchLFOFreq();
    void setPitchLFOAmount(float lfoamt);
    float getPitchLFOAmount();
    
    // direction
    void setDirection(int dirMode);
    int getDirection();
    
    // add/remove grain voice
    void addGrain();
    void removeGrain();
    
    // set window type
    void setWindowType(int windowType);
    int getWindowType();
    
    // spatialization methods
    // (see enum for mode.  channel number is optional and has default arg); 
    void setSpatialMode(int mode, int channelNumber);
    int getSpatialMode();
    int getSpatialChannel();
    
    // volume
    void setVolumeDb(float volDB);
    float getVolumeDb();

    // get unique id of grain cluster    
    unsigned int getId();
    
    // register visualization 
    void registerVis(GrainClusterVis *vis);
    
    // turn on/off
    void toggleActive();
    bool getActiveState();

    
    // return number of voices
    unsigned int getNumVoices();
    
    
protected:
    // update internal trigger point
    void updateBangTime();
    
    // spatialization
    // get new channel multiplier buffer to pass to grain voice instance
    void updateSpatialization();
    
private:
    unsigned int _id; // unique id
    
    bool _isActive; // on/off state
    bool _awaitingPlay; // triggered but not ready to play?
    bool _addFlag;
    bool _removeFlag; // add/remove requests submitted?
    unsigned long _localTime; // internal clock
    double _startTime; // instantiation time
    double _bangTime; // trigger time for next grain
    unsigned int _nextGrain; // grain voice index
    
    // spatialization vars
    int _currentAroundChan;
    int _stereoSide; 
    int _side;

   
    // thread safety
    Mutex *_lock; 
    
    // registered visualization
    GrainClusterVis *_vis; 
    
    // spatialization
    double *_channelMults;
    int _spatialMode;
    int _channelLocation;

    // volume
    float _volumeDb;
    float _normedVol;
    
    // vector of grains
    vector<GrainVoice *> *_grains;
    
    // number of grains in this cluster
    unsigned int _numVoices;

    // cluster params
    float _overlap;
    float _overlapNorm;
    float _pitch;
    float _duration;
    float _pitchLFOFreq;
    float _pitchLFOAmount;
    
    int _dirMode;
    int _windowType;
    
    // audio files
    vector<AudioFile *> *_sounds;

    Style *_style;
};




// VISUALIZATION/CONTROLLER
class GrainClusterVis
{
public:
    ~GrainClusterVis();
    
    // takes center position (x,y), number of voices, sound rectangles
    GrainClusterVis(Style *_style,
                    unsigned int winWidth,
                    unsigned int winHeight,
                    float x, float y,
                    unsigned int numVoices,
                    vector<SoundRect*>*rects);

    void updateWinWidthHeight(unsigned int newWinWidth,
                              unsigned int newWinHeight);
        
    // render
    void draw();
    
    // get playback position in registered rectangles and return to grain cloud
    void getTriggerPos(unsigned int idx,
                       double *playPos,
                       double *playVols,
                       float dur);

    // move grains
    void updateCloudPosition(float x, float y);
    void updateGrainPosition(int idx, float x, float y);
    void setState(int idx, bool on);
    
    // add grain
    void addGrain();
    
    // remove grain element from visualization
    void removeGrain();
    
    // set selection (highlight)
    void setSelectState(bool state);
    
    // determine if mouse click is in selection range
    bool select(float x, float y);
    
    // get my x coordinate
    float getX();
    
    // get my y coordinate
    float getY();
    
    // randomness params for grain positions
    float getXRandExtent();
    float getYRandExtent();
    void setXRandExtent(float mouseX);
    void setYRandExtent(float mouseY);
    void setRandExtent(float mouseX, float mouseY);
    
    // set the pulse duration (which determines the frequency of the pulse)
    void setDuration(float dur);
    
private:
    bool _isOn;
    bool _isSelected;
    bool _addFlag;
    bool _removeFlag;
    double _startTime;
    unsigned int _winWidth;
    unsigned int _winHeight;
    
    float _xRandExtent;
    float _yRandExtent;
    
    float _freq;
    float _gcX;
    float _gcY;
    
    float _selRad;
    float _lambda;
    
    float _maxSelRad;
    float _minSelRad;
    float _targetRad;
    
    unsigned int _numGrains;
    
    // grain voice visualizations
    vector<GrainVis*> * _grains;
    
    // registered sound rectangles 
    vector<SoundRect*> * _landscape;

    Style *_style;
};

#endif
