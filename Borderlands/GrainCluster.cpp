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
//  GrainCluster.cpp
//  Borderlands
//
//  Created by Christopher Carlson on 11/23/11.
//

#include "Style.h"
#include "GrainCluster.h"

GrainCluster::~GrainCluster()
{
    if (_grains != NULL)
    {
        for (int i = 0; i < _grains->size(); i++)
            delete _grains->at(i);
        
        delete _grains;
    }
    
    if (_vis)
        delete _vis;
    
    if (_lock)
        delete _lock;
    
    if (_channelMults)
        delete[] _channelMults;
}

GrainCluster::GrainCluster(Style *style,
                           vector<AudioFile*> *soundSet,
                           float numVoices)
{
    _style = style;
    
    // initialize mutext
    _lock = new Mutex();
    
    // cluster id
    _id = ++clusterId;
    
    // playback bool to make sure we precisely time grain triggers
    _awaitingPlay = false;
    
    // number of voices
    _numVoices = numVoices;
    
    // initialize random number generator (for random motion)
    srand(time(NULL));
    
    // initialize interfacing flags
    _addFlag = false;
    _removeFlag = false;
    
    // keep pointer to the sound set
    _sounds = soundSet;
    
    // trigger idx
    _nextGrain = 0;
    
    // intialize timer
    _localTime = 0;

    // default duration (ms)
    _duration = 500.0;

    // default pitch 
    _pitch = 1.0f;
    
    // default window type
    _windowType = HANNING;
    
    // initialize pitch LFO
    _pitchLFOFreq = 0.01f;
    _pitchLFOAmount = 0.0f;
    
    // initialize channel multiplier array
    _channelMults = new SAMPLE[CHANNELS];
    for (int i = 0; i < CHANNELS; i++)
        _channelMults[i] = 0.999f;
    
    _currentAroundChan = 1;
    _stereoSide = 0; 
    _side = 1;
    
    _spatialMode = UNITY;
    _channelLocation = -1;
    
    _dirMode = RANDOM_DIR;
 
    // create grain voice vector
    _grains = new vector<GrainVoice *>;
    
    // populate grain cloud
    for (int i = 0; i < _numVoices; i++)
        _grains->push_back(new GrainVoice(_style,
                                          _sounds,
                                          _duration,
                                          _pitch));

    // set volume of cloud to unity
    setVolumeDb(0.0);
    
    // set overlap (default to full overlap)
    setOverlap(1.0f);
    
    // direction 
    // setDirection(RANDOM_DIR);
    
    // initialize trigger time (samples)
    _bangTime = _duration * SRATE * (double) 0.001 / _overlap;    

    // load grains
    for (int i = 0; i < _grains->size(); i++)
        _grains->at(i)->setDurationMs(_duration);
    
    // state - (user can remove cloud from "play" for editing)
    _isActive = true;
}

// register controller for communication with view
void
GrainCluster::registerVis(GrainClusterVis * vis)
{
    _vis = vis;
    _vis->setDuration(_duration);
}

// turn on/off
void
GrainCluster::toggleActive()
{
    _isActive = !_isActive;
}

bool
GrainCluster::getActiveState()
{
    return _isActive;
}

// set window type
void
GrainCluster::setWindowType(int winType)
{
    int numWins = Window::instance().numWindows();
    _windowType = winType % numWins;
    
    if (_windowType < 0)
        _windowType = Window::instance().numWindows()-1;
    
    if (_windowType == RANDOM_WIN)
    {
        for (int i = 0; i < _grains->size();i++)
            _grains->at(i)->
                setWindow((int)floor(randf()*Window::instance().numWindows()-1));
    }
    else
    {
        for (int i = 0; i < _grains->size();i++)
            // cout << "windowtype " << _windowType << endl;
            _grains->at(i)->setWindow(_windowType);
    }
}

int
GrainCluster::getWindowType()
{
    return _windowType;
}

void
GrainCluster::addGrain()
{
    _addFlag = true;
    _vis->addGrain();
}

void
GrainCluster::removeGrain()
{
    _removeFlag = true;
    _vis->removeGrain();
}

// return id for grain cluster
unsigned int
GrainCluster::getId()
{
    return _id;
}


// overlap (input on 0 to 1 scale)
void
GrainCluster::setOverlap(float target)
{
    if (target > 1.0f)
        target = 1.0f;
    else if (target < 0.0f)
        target = 0.0f;
    _overlapNorm = target;
    // oops wrong!//overlap = ((float)(_grains->size()))*0.25f*exp(log(2.0f)*target);
 
    float num = (float)_grains->size();

    _overlap = exp(log(num)*target);

    // cout<<"overlap set" << overlap << endl;
    updateBangTime();
}

float
GrainCluster::getOverlap()
{
    return _overlapNorm;
}

// duration
void
GrainCluster::setDurationMs(float dur)
{
    if (dur >= 1.0f)
    {
        _duration = dur;
        for (int i = 0; i < _grains->size(); i++)
            _grains->at(i)->setDurationMs(_duration);
        
        updateBangTime();
        
        // notify visualization
        if (_vis)
            _vis->setDuration(_duration);
    }
}

// update internal grain trigger time
void
GrainCluster::updateBangTime()
{
    _bangTime = _duration * SRATE * (double) 0.001 / _overlap;
    // cout << "duration: " << duration << ", new bang time " << _bangTime << endl;
}


// pitch
void
GrainCluster::setPitch(float targetPitch)
{
    if (targetPitch < 0.0001)
        targetPitch = 0.0001;
    
    _pitch = targetPitch;
    for (int i = 0; i < _grains->size(); i++)
        _grains->at(i)->setPitch(targetPitch);
}

float
GrainCluster::getPitch()
{
    return _pitch;
}

//-----------------------------------------------------------------
// Cluster volume
//-----------------------------------------------------------------
void
GrainCluster::setVolumeDb(float volDb)
{
    // max = 6 db, min = -60 db
    if (volDb > 6.0)
        volDb = 6.0;
    
    if (volDb < -60.0)
        volDb = -60.0;
    
    _volumeDb = volDb;
    
    // convert to 0-1 representation
    _normedVol = pow( 10.0 , volDb * 0.05 );
    
    for (int i = 0; i < _grains->size(); i++)
        _grains->at(i)->setVolume(_normedVol);
}

float
GrainCluster::getVolumeDb()
{
    return _volumeDb;
}

// direction mode
void
GrainCluster::setDirection(int dirMode)
{
    _dirMode = dirMode % 3;
    if (_dirMode < 0)
        _dirMode = 2;
    
    // cout << "dirmode num" << _dirMode << endl;
    switch (_dirMode)
    {
        case FORWARD:
            // cout << "set for" << endl;
            for (int i = 0; i < _grains->size(); i++)
                _grains->at(i)->setDirection(1.0);
            break;
            
        case BACKWARD:
            // cout << "set back" << endl;
            for (int i = 0; i < _grains->size(); i++)
                _grains->at(i)->setDirection(-1.0);
            break;
            
        case RANDOM_DIR:
            for (int i = 0; i < _grains->size(); i++)
            {
                if (randf() > 0.5)
                    _grains->at(i)->setDirection(1.0);
                else
                    _grains->at(i)->setDirection(-1.0);
            }
                
        default:
            break;
    }
}

// return grain direction int
// (see enum.  currently, 0 = forward, 1 = back, 2 = random)
int
GrainCluster::getDirection()
{
    return _dirMode;
}

// return duration in ms
float
GrainCluster::getDurationMs()
{
    return _duration;
}


// return number of voices in this cloud
unsigned int
GrainCluster::getNumVoices()
{
    return _grains->size();
}

// compute audio
void
GrainCluster::nextBuffer(SAMPLE * accumBuff,unsigned int numFrames)
{
    if (_addFlag)
    {
        _addFlag = false;
        _grains->push_back(new GrainVoice(_style, _sounds,
                                          _duration, _pitch));
        int idx = _grains->size()-1;
        _grains->at(idx)->setWindow(_windowType);
        
        switch (_dirMode)
        {
            case FORWARD:
                _grains->at(idx)->setDirection(1.0);
                break;
                
            case BACKWARD:
                _grains->at(idx)->setDirection(-1.0);
                break;
                
            case RANDOM_DIR:
                if (randf()>0.5)
                    _grains->at(idx)->setDirection(1.0);
                else
                    _grains->at(idx)->setDirection(-1.0);
                break;

            default:
                break;
        }
        
        _grains->at(idx)->setVolume(_normedVol);
        _numVoices += 1;
        setOverlap(_overlapNorm);
    }
    
    if (_removeFlag)
    {
        _lock->lock();
        if (_grains->size() > 1)
        {
            if (_nextGrain >= _grains->size()-1)
                _nextGrain = 0;
            
            _grains->pop_back();
            setOverlap(_overlapNorm);
        }
        _removeFlag = false;
        _lock->unlock();
    }
    
    if (_isActive)
    {    
        // initialize play positions array
        double playPositions[_sounds->size()];
        double playVols[_sounds->size()];
        
        // buffer variables
        unsigned int nextFrame = 0;
        
        // compute sub_buffers for reduced function calls
        int frameSkip = numFrames/2;
        
        // fill buffer
        for (int j = 0; j < (numFrames/(frameSkip)); j++)
        {    
            // check for bang
            if ((_localTime > _bangTime) || (_awaitingPlay))
            {    
                // debug
                // cout << "bang " << _nextGrain << endl;
                // reset local
                if (!_awaitingPlay)
                {
                    _localTime = 0;
                    // clear play and volume buffs
                    for (int i = 0; i < _sounds->size(); i++)
                    {
                        playPositions[i] = (double)(-1.0);
                        playVols[i] = (double) 0.0;
                    }
                    
                    // TODO:  get position vector for grain with idx
                    // _nextGrain from controller
                    
                    // udate positions vector (currently randomized)q
                    if (_vis)
                        _vis->getTriggerPos(_nextGrain,
                                            playPositions,
                                            playVols,
                                            _duration);
                }
                
                // get next pitch (using LFO)
                // eventually generalize to an applyLFOs method
                // (if LFO control will be exerted over multiple params)
                if ((_pitchLFOAmount > 0.0f) && (_pitchLFOFreq > 0.0f))
                {
                    float nextPitch =
                        fabs(_pitch + _pitchLFOAmount *
                             sin(2*PI*_pitchLFOFreq*GTime::instance()._sec));
                    _grains->at(_nextGrain)->setPitch(nextPitch);
                }
                
                // update spatialization/get new channel multiplier set
                updateSpatialization();
                _grains->at(_nextGrain)->setChannelMultipliers(_channelMults);
                
                // trigger grain
                _awaitingPlay =
                    _grains->at(_nextGrain)->playMe(playPositions, playVols);
                
                // only advance if next grain is playable.
                // otherwise, cycle through again
                
                // to wait for playback
                if (!_awaitingPlay)
                {
                    // queue next grain for trigger
                    _nextGrain++;
                    
                    // wrap grain idx
                    if (_nextGrain >= _grains->size())
                        _nextGrain = 0;
                }
                else
                {
                    // debug
                    // cout << "playback delayed "<< endl;
                }
            }
            
            // advance time
            _localTime += frameSkip;
            
            // sample offset (1 sample at a time for now)
            nextFrame = j*frameSkip;
            
            // iterate over all grains
            for (int k = 0; k < _grains->size(); k++)
            {
                _grains->at(k)->nextBuffer(accumBuff, frameSkip, nextFrame, k);
            }
        }
    }
}

// pitch lfo methods
void
GrainCluster::setPitchLFOFreq(float pfreq)
{
    _pitchLFOFreq = fabs(pfreq);
}

void
GrainCluster::setPitchLFOAmount(float lfoamt)
{
    if (lfoamt < 0.0)
        lfoamt = 0.0f;
    
    _pitchLFOAmount = lfoamt;
}

float
GrainCluster::getPitchLFOFreq()
{
    return _pitchLFOFreq;
}

float
GrainCluster::getPitchLFOAmount()
{
    return _pitchLFOAmount;
}

// spatialization methods
void
GrainCluster::setSpatialMode(int mode,
                             int channelNumber = -1)
{
    _spatialMode = mode % 3;
    if (_spatialMode < 0)
        _spatialMode = 2;
    
    // for positioning in a single audio channel. - not used currently
    // eventually swap out for azimuth instead of single channel
    if (channelNumber >=0)
        _channelLocation = channelNumber;
}

int
GrainCluster::getSpatialMode()
{
    return _spatialMode;
}

int
GrainCluster::getSpatialChannel()
{
    return _channelLocation;
}

// spatialization logic
void
GrainCluster::updateSpatialization()
{
    
    // currently assumes orientation L: 0,2,4,...  R: 1,3,5, etc (interleaved) 
    switch (_spatialMode)
    {
        case UNITY:
            for (int i = 0; i < CHANNELS; i++)
                _channelMults[i] = 0.999f;
            break;
            
        case STEREO:
            if (_stereoSide == 0)
                // left
            {
                for (int i = 0; i < CHANNELS; i++)
                {
                    _channelMults[i]= 0.0f;
                    if ((i % 2) == 0)
                        _channelMults[i] = 0.999f;
                    else
                        _channelMults[i] = 0.0f;
                }
                _stereoSide = 1;
            }
            else
                // right
            {
                for (int i = 0; i < CHANNELS; i++)
                {
                    _channelMults[i]= 0.0f;
                    if ((i % 2) == 0)
                        _channelMults[i] = 0.0f;
                    else
                        _channelMults[i] = 0.999f;
                }
                _stereoSide = 0;
            }
            break;
            
        case AROUND:
            for (int i = 0; i < CHANNELS; i++)
                _channelMults[i] = 0;
           
            // 1 3 5 7 6 4 2 0
            
            _channelMults[_currentAroundChan] = 0.999;
            _currentAroundChan += _side*2;
            if ((_currentAroundChan > CHANNELS) ||
                (_currentAroundChan < 0))
            {
                _side = -1*_side;
                _currentAroundChan += _side*3;
            }
            // _currentAroundChan = _currentAroundChan % CHANNELS;
            break;
            
        default:
            break;
    }    
}

//-----------------------------------------------------------------------------
// GRAPHICS
//----------------------------------------------------------------------------
GrainClusterVis::~GrainClusterVis()
{
    if (_grains)
        delete _grains;
}

GrainClusterVis::GrainClusterVis(Style *style,
                                 unsigned int winWidth,
                                 unsigned int winHeight,
                                 float x, float y,
                                 unsigned int numVoices,
                                 vector<SoundRect*>*rects)
{
    _style = style;
    
    _winWidth = winWidth;
    _winHeight = winHeight;
    
    _startTime = GTime::instance()._sec;
    // cout << "cluster started at : " << _startTime << " sec " << endl;

    _gcX = x;
    _gcY = y;

    // cout << "cluster x" << _gcX << endl;
    // cout << "cluster y" << _gcY  << endl;

    // randomness params
    _xRandExtent = 3.0;
    _yRandExtent = 3.0;
    
    // init add and remove flags to false
    _addFlag = false;
    _removeFlag = false;
    
    // select on instantiation
    _isSelected = true;
    
    // pulse frequency
    _freq = 1.0f;
    
    // pointer to landscape visualization objects
    _landscape = rects;

    _grains = new vector<GrainVis *>;
    
    for (int i = 0; i < numVoices; i++)
    {
        _grains->push_back(new GrainVis(_style, _winWidth,_winHeight,_gcX,_gcY));
    }

    _numGrains = numVoices;
    
    //visualization stuff
    _minSelRad = 15.0f;
    _maxSelRad = 19.0f;
    _lambda = 0.997;
    _selRad = _minSelRad;
    _targetRad = _maxSelRad;
}

void
GrainClusterVis::updateWinDimensions(unsigned int newWinWidth,
                                     unsigned int newWinHeight)
{
    if ((_winWidth == newWinWidth) && 
        (_winHeight == newWinHeight))
        return;

    // update the rect pos and size if window size changed
    float wRatio = ((float)newWinWidth)/_winWidth;
    float hRatio = ((float)newWinHeight)/_winHeight;

    _gcX *= wRatio;
    _gcY *= hRatio;

    _xRandExtent *= wRatio;
    _yRandExtent *= hRatio;
        
    /*_minSelRad *= wRatio;
      _maxSelRad *= wRatio;
      _selRad = _minSelRad;
      _targetRad = _maxSelRad;
    */

    _winWidth = newWinWidth;
    _winHeight = newWinHeight;

    for (int i = 0; i < _grains->size(); i++)
        (*_grains)[i]->updateWinDimensions(newWinWidth, newWinHeight);
}

void
GrainClusterVis::setDuration(float dur)
{
    _freq = 1000.0/dur;
}

// return cluster x
float
GrainClusterVis::getX()
{
    return _gcX;
}

// return cluster y
float
GrainClusterVis::getY()
{
    return _gcY;
}

void GrainClusterVis::draw()
{
    double t_sec = GTime::instance()._sec  - _startTime ;
    // cout << t_sec << endl;
    
    // if ((g_time -last_gtime) > 50){
    glPushMatrix();
    glTranslatef((GLfloat)_gcX,(GLfloat)_gcY,0.0);
    
    // Grain cluster representation
    if (_isSelected)
    {
        float grainColorSelected[4];
        _style->getGrainColorSelected(grainColorSelected);
        glColor4f(grainColorSelected[0], grainColorSelected[1],
                  grainColorSelected[2], grainColorSelected[3]);
    }
    else
    {
        float grainColor[4];
        _style->getGrainColor(grainColor);
        glColor4f(grainColor[0], grainColor[1],
                  grainColor[2], grainColor[3]);
    }
    
    _selRad = _minSelRad +
        0.5*(_maxSelRad-_minSelRad)*sin(2*PI*(_freq*t_sec + 0.125));
    gluDisk(gluNewQuadric(), _selRad, _selRad+5.0, 128, 2);
    glPopMatrix();

    // update grain motion;
    // Individual voices
    
    // disc version
    // (lower quality, but works on graphics cards that don't support GL_POINT_SMOOTH)
    //    
    //    for (int i = 0; i < _numGrains; i++){
    //        glPushMatrix();
    //        _grains->at(i)->draw(mode);
    //        glPopMatrix();
    //    }

    // end disc version
    
    // point version (preferred)
    glPushMatrix();

    // update grain motion;
    // Individual voices
     for (int i = 0; i < _numGrains; i++)
       _grains->at(i)->draw();
     
    glPopMatrix();
    
    // end point version
}

// get trigger position/volume relative to sound rects for single grain voice
void
GrainClusterVis::getTriggerPos(unsigned int idx,
                               double *playPos,
                               double *playVol,
                               float dur)
{
    bool trigger = false;
    SoundRect *rect = NULL;
    if (idx < _grains->size())
    {
        GrainVis *grain = _grains->at(idx);
        // TODO: motion models
        // updateGrainPosition(idx,_gcX + randf()*50.0 + randf()*(-50.0),_gcY + randf()*50.0 + randf()*(-50.0));
        updateGrainPosition(idx,
                            _gcX + (randf()*_xRandExtent - randf()*_xRandExtent),
                            _gcY + (randf()*_yRandExtent - randf()*_yRandExtent));
        
        for (int i = 0; i < _landscape->size(); i++)
        {
            rect = _landscape->at(i);
            bool tempTrig = false;
            tempTrig =
                rect->getNormedPosition(playPos,
                                        playVol,
                                        grain->getX(),
                                        grain->getY(),
                                        i);
            if (tempTrig)
                trigger = true;
            // cout << "playvol: " << *playPos << ", playpos: " << *playVol << endl;
            
        }
        
        if (trigger){
            grain->trigger(dur);
        }
    }
}


// rand cluster size
void
GrainClusterVis::setXRandExtent(float mouseX)
{
    _xRandExtent = fabs(mouseX - _gcX);
    if (_xRandExtent < 2.0f)
        _xRandExtent = 0.0f;
}

void
GrainClusterVis::setYRandExtent(float mouseY)
{
    _yRandExtent = fabs(mouseY - _gcY);
    if (_yRandExtent < 2.0f)
        _yRandExtent = 0.0f;
}
void
GrainClusterVis::setRandExtent(float mouseX,
                               float mouseY)
{
    setXRandExtent(mouseX);
    setYRandExtent(mouseY);
}

float
GrainClusterVis::getXRandExtent()
{
    return _xRandExtent;
}

float GrainClusterVis::getYRandExtent()
{
    return _yRandExtent;
}

void
GrainClusterVis::updateCloudPosition(float x,
                                     float y)
{
    float xDiff = x - _gcX;
    float yDiff = y - _gcY;
    _gcX = x;
    _gcY = y;
    for (int i = 0; i < _grains->size(); i++)
    {
        float newGrainX = _grains->at(i)->getX() + xDiff;
        float newGrainY = _grains->at(i)->getY() + yDiff;
        _grains->at(i)->moveTo(newGrainX,
                                newGrainY);
    }
}

void
GrainClusterVis::updateGrainPosition(int idx, float x, float y)
{
    if (idx < _numGrains)
        _grains->at(idx)->moveTo(x,y);
}

// check mouse selection
bool
GrainClusterVis::select(float x, float y)
{
    float xdiff = x - _gcX;
    float ydiff = y - _gcY;
    
    if (sqrt(xdiff*xdiff + ydiff*ydiff) < _maxSelRad)
        return true;
    else
        return false;
}

void
GrainClusterVis::setSelectState(bool selectState)
{
    _isSelected = selectState;
}

void
GrainClusterVis::addGrain()
{
    // _addFlag = true;
    _grains->push_back(new GrainVis(_style, _winWidth, _winHeight, _gcX, _gcY));
    _numGrains= _grains->size();
}

// remove a grain from the cloud (visualization only)
void
GrainClusterVis::removeGrain()
{
    // _removeFlag = true;
    if (_numGrains > 1)
    {
        // delete object
        _grains->pop_back();
        _numGrains = _grains->size();
    }
}
