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
//  GrainVoice.cpp
//  Borderlands
//
//  Created by Christopher Carlson on 11/20/11.
//

#include "Style.h"
#include "GrainVoice.h"

//-------------------AUDIO----------------------------------------------------//

GrainVoice::~GrainVoice()
{    
    if (_sounds != NULL)
        delete _sounds;
    
    if (_playPositions != NULL)
        delete [] _playPositions;
    
    if (_playVols != NULL)
        delete [] _playVols;
    
    if (_window != NULL)
        delete[]_window;
    
    if (_activeSounds!= NULL)
        delete _activeSounds;
  
    if (_chanMults)
        delete[] _chanMults;
    
    if (_queuedChanMults)
        delete[]_queuedChanMults;
}

GrainVoice::GrainVoice(Style *style,
                       vector<AudioFile *> * soundSet,
                       float durationMs,
                       float pitch)
{
    _style = style;
    
    // store pointer to external vector of sound files 
    _sounds = soundSet; 
    
    // get number of loaded sounds
    _numSounds = (unsigned int)soundSet->size();
    
    // no active sounds on instantiation
    _activeSounds = NULL;
    
    // set play positions to -1 for all
    // note - will have to handle files being added at runtime
    // later if it becomes a feature
    if (_numSounds > 0)
    {
        _playPositions = new double[_numSounds];
        _playVols = new double[_numSounds];
        
        // initialize - (-1 signifies that sound should not be played)
        for (int i = 0; i < soundSet->size(); i++)
        {
            _playPositions[i] = -1.0;
            _playVols[i] = 0.0;
        }
    }
    else
        _playPositions = NULL;
    
    // playing status init
    _playingState = false;
    
    // direction
    if (randf() < 0.5)
        _direction = 1.0;
    else
        _direction = -1.0;
    _queuedDirection = _direction;
    
    // set default windowType
    _windowType = HANNING;
    _queuedWindowType = _windowType;
    
    // window type is hanning
    _window = Window::instance().getWindow(_windowType);

    // grain volume
    _localAtten = 1.0;
    _queuedLocalAtten = _localAtten;
    
    // grain duration (ms)
    _duration = durationMs;
    _queuedDuration = durationMs; 
    
    // grain playback rate
    _pitch = pitch;
    _queuedPitch = _pitch;
    
    // spatialization
    _queuedChanMults = new double[CHANNELS];
    _chanMults = new double[CHANNELS];
    // set panning values - all channels active by default
    for (int i = 0; i < CHANNELS; i++)
    {
        _chanMults[i] = 1.0;
        _queuedChanMults[i] = 1.0;
    }
        
    // new input flag (no new inputs on instantiation)
    _newParam = false;
    
    // set playhead increment
    _playInc = _pitch*_direction;
    
    // initialize window reading params
    _winReader = 0.0; //0 idx
    
    // get duration in samples (fractional)
    _winDurationSamps = ceil(_duration * SRATE * (double) 0.001);
    _winInc = (double)WINDOW_LEN / _winDurationSamps; 
}

//-----------------------------------------------------------------------------
// Turn on grain.  
//input args = position and volume vectors in sound rect space
// returns whether or not grain is awaiting play. 
// parent cloud will wait to play this voice if the voice is still
//this should not be an issue unless the overlap value is erroneous 
//-----------------------------------------------------------------------------
bool
GrainVoice::playMe(double * startPositions,
                   double * startVols)
{
    if (!_playingState)
    { 
        // next buffer call will play
        _playingState = true;
        
        // grab queued params if changed
        if (_newParam)
            updateParams();
        
        // convert relative start positions to sample locations
        if (_activeSounds != NULL)
            delete _activeSounds;
        
        _activeSounds = new vector<int>;
        
        for (int i = 0; i < _numSounds; i++)
        {
            if (startPositions[i] != -1)
            {
                _activeSounds->push_back(i);
                _playPositions[i] =
                    floor( startPositions[i] * (_sounds->at(i)->_frames - 1));
                _playVols[i] = startVols[i];
            }
        }
        
        // initialize window reader index
        _winReader = 0;
        
        return false;    
    }
    else
    {
        // debug
        // cout << "Grain triggered too soon..." << endl;
        return true;
    }
}

//-----------------------------------------------------------------------------
// Find out if grain is currently on
//-----------------------------------------------------------------------------
bool
GrainVoice::isPlaying()
{
    return _playingState;
}     

//-----------------------------------------------------------------------------
// Set channel multipliers
//-----------------------------------------------------------------------------
void
GrainVoice::setChannelMultipliers(double *multipliers)
{
    for (int i = 0; i < CHANNELS; i++)
        _queuedChanMults[i] = multipliers[i];
    
    _newParam = true;
}

//-----------------------------------------------------------------------------
// Set channel multipliers
//-----------------------------------------------------------------------------
void
GrainVoice::setVolume(float volNormed)
{
    float pVol = fabs(volNormed);
    _queuedLocalAtten = pVol;
}

//-----------------------------------------------------------------------------
// Get channel multipliers
//-----------------------------------------------------------------------------
float
GrainVoice::getVolume()
{
    return _localAtten;
}

//-----------------------------------------------------------------------------
// Set duration (effective on next trigger)
//-----------------------------------------------------------------------------
void
GrainVoice::setDurationMs(float dur)
{
    // get absolute value
    _queuedDuration = fabs(dur);
    if (_queuedDuration != _duration)
        _newParam = true;
}     

//-----------------------------------------------------------------------------
// Set pitch (effective on next trigger)
//-----------------------------------------------------------------------------
void
GrainVoice::setPitch(float newPitch)
{
    // get absolute value
    _queuedPitch = newPitch;
    if (_queuedPitch != _pitch)
        _newParam = true;
}     

float
GrainVoice::getPitch()
{
    if (_queuedPitch != _pitch)
        return _queuedPitch;
    else
        return _pitch;
}

//-----------------------------------------------------------------------------
// Update params
//-----------------------------------------------------------------------------
void
GrainVoice::updateParams()
{
    // update parameter set
    _localAtten = _queuedLocalAtten;
    
    // playback rate
    _pitch = _queuedPitch;
    
    // grain duration in ms
    _duration = _queuedDuration;
    
    // direction of playback
    _direction = _queuedDirection;
    
    // playhead increment
    _playInc = _direction*_pitch;
    
    // window
    _windowType = _queuedWindowType;
    
    // switch window
    _window = Window::instance().getWindow(_windowType);

    // double value, but eliminate fractional component - 
    _winDurationSamps = ceil(_duration * SRATE * (double) 0.001);
    
    // how far should we advance through windowing function each sample
    _winInc = (double)WINDOW_LEN / _winDurationSamps; 
    
    // spatialization - get new channel multipliers
    for (int i = 0; i < CHANNELS; i++)
        _chanMults[i] = _queuedChanMults[i];
    
    // all params have been updated
    _newParam = false;
}     

//-----------------------------------------------------------------------------
// Set window type (effective on next trigger)
//-----------------------------------------------------------------------------
void
GrainVoice::setWindow(unsigned int type)
{
    _queuedWindowType = type;
    if (_queuedWindowType != _windowType)
        _newParam = true;
}

//-----------------------------------------------------------------------------
// Set direction (effective on next trigger)
//-----------------------------------------------------------------------------
void
GrainVoice::setDirection(float dir)
{
    _queuedDirection = dir;
    if (_queuedDirection != _direction)
        _newParam = true;
}

//-----------------------------------------------------------------------------
// Compute next sub buffer of audio
//-----------------------------------------------------------------------------
void
GrainVoice::nextBuffer(double *accumBuff,
                       unsigned int numFrames,
                       unsigned int bufferOffset,
                       int name)
{
    // fill stereo accumulation buffer.
    // note, buffer output must be interlaced ch1,ch2,ch1,ch2, etc...
    // and playPositions are in frames, NOT SAMPLES.
    
    // only go through this ordeal if grain is active
    if (_playingState)
    { 
        // initialize local vars
        
        // linear interp coeff
        double nu = 0.0;
        
        // next window value
        double nextMult = 0.0;
        
        // ref idx for left bound of interp
        double flooredIdx = 0; 
        
        // next file index
        int nextSound = -1;
        
        // waveform params
        double * wave = NULL;
        int channels = 0;
        unsigned int frames = 0;
        
        // file reader position
        double pos = -1.0;
        
        // attenuation value
        double atten = 0.0;
        
        // output values 
        double nextAmp = 0.0;
        double monoWaveVal = 0.0;
        double stereoLeftVal = 0.0;
        double stereoRightVal = 0.0;
        
        // iterate over requested number of samples
        for (int i = 0; i < numFrames; i++)
        { 
            // Window multiplier
            // Get next val from window and check to see if we've reached the end
            if (_winReader > (WINDOW_LEN - 1))
            {
                nextMult = (double) 0.0;
                _winReader = 0;
                _playingState = false;

                return;
            }
            else
            {
                // interpolated read from window buffer
                flooredIdx = floor(_winReader);
                nu = _winReader - flooredIdx; // interp coeff
                // interpolated read (lin)
                nextMult =
                    ((double) 1.0 - nu)*_window[(unsigned long)flooredIdx] +
                    nu * _window[(unsigned long)flooredIdx + 1];
                // increment reader
                _winReader += _winInc;
            }
            
            // reinit sound accumulators for mono and stereo files
            // to prepare for this frame
            monoWaveVal = 0.0;
            stereoLeftVal = 0.0;
            stereoRightVal = 0.0;
            
            // Get next audio frame data (accumulate from each sound under grain) 
            // -- REMEMBER - playPositions are in frames, not samples
            for (int j = 0; j < _activeSounds->size(); j++)
            {    
                nextSound = _activeSounds->at(j);
                pos = _playPositions[nextSound]; // get start position
                atten = _playVols[nextSound]; // get volume relative to rect
                
                // if sound is in play,sample it
                if (pos > 0)
                {    
                    // sound vars
                    wave = _sounds->at(nextSound)->_wave;
                    channels = _sounds->at(nextSound)->_channels;
                    frames = _sounds->at(nextSound)->_frames;
                    
                    // get info for interpolation based on frame location
                    flooredIdx =  floor(pos);
                    nu =pos - flooredIdx;
                    
                    // handle mono and stereo files separately.
                    switch (channels)
                    {
                        case 1:           
                            // get next linearly interpolated sample
                            // val and make sure we are still inside
                            if ((flooredIdx >=0) && ((flooredIdx + 1) <
                                                     (frames - 1)))
                            {
                                nextAmp = (((double) 1.0 - nu)*
                                           wave[(unsigned long)flooredIdx] +
                                           nu * wave[(unsigned long)flooredIdx + 1])*
                                    nextMult * atten;
                               
                                // accumulate mono frame
                                monoWaveVal += nextAmp;
                                
                                /*//old
                                 accumBuff[(bufferOffset + i)*CHANNELS] += nextAmp;
                                //copy to other channels for mono
                                for (int k = 1; k < CHANNELS; k++){
                                    accumBuff[(bufferOffset + i) * CHANNELS + k] += nextAmp;
                                }
                                 */
                                
                                // advance after each stereo frame (do calc twice for mono)
                                _playPositions[nextSound] += _playInc;                                    
                            }
                            else
                            {
                                // not playing anymore
                                _playPositions[nextSound] = -1.0;
                            }
                            break;
                            
                        case 2: // stereo
                            // make sure we are still in sound
                            if ((flooredIdx >=0) && ((flooredIdx + 1) <
                                                     (frames - 1)))
                            {    
                                // left channel
                                stereoLeftVal +=
                                    (((double) 1.0 - nu)*
                                     wave[(unsigned long)flooredIdx*2] +
                                     nu * wave[(unsigned long)(flooredIdx + 1)*2])*
                                    nextMult*atten;
                                // right channel
                                stereoRightVal +=
                                    (((double) 1.0 - nu)*
                                     wave[(unsigned long)flooredIdx*2 + 1] + nu *
                                     wave[(unsigned long)(flooredIdx + 1)*2 + 1])*
                                    nextMult*atten;
                              
                                /*//old
                                //left channel
                                accumBuff[(bufferOffset + i)*CHANNELS] += (((double) 1.0 - nu)*wave[(unsigned long)flooredIdx*2] + nu * wave[(unsigned long)(flooredIdx + 1)*2])*nextMult*atten;
                                
                                //right channel
                                accumBuff[(bufferOffset + i) * CHANNELS + 1] += (((double) 1.0 - nu)*wave[(unsigned long)flooredIdx*2 + 1] + nu * wave[(unsigned long)(flooredIdx + 1)*2 + 1])*nextMult*atten;
                                 */
                                
                                // advance after each stereo frame (do calc twice for mono)
                                _playPositions[nextSound] += _playInc;
                            }
                            else
                            {
                                // not playing anymore
                                _playPositions[nextSound] = -1.0;
                            }
                            break;
                            
                            // don't handle numbers of channels > 2
                        default:
                            break;
                    } // end switch channels
                } // end position check
            } // end accumulation for current frame
            
            // spatialize output
            for (int k = 0; k < CHANNELS; k++)
            {
                // preserve stereo waveform L/R for now and just
                // sample alternate channels in "AROUND" case
                // (see GrainCluster.cpp updateSpatialization routine)
                if ((k % 2) == 0)
                {
                    accumBuff[(bufferOffset + i)*CHANNELS + k] +=
                        (stereoLeftVal + monoWaveVal) * _chanMults[k] * _localAtten;
                }
                else
                {
                    accumBuff[(bufferOffset + i)*CHANNELS + k] +=
                        (stereoRightVal + monoWaveVal) * _chanMults[k] * _localAtten;
                }
                
                // clip if needed
                if (accumBuff[(bufferOffset + i)*CHANNELS + k] > 1.0)
                {
                    accumBuff[(bufferOffset + i)*CHANNELS + k] = 1.0;
                }
                else
                {
                    if (accumBuff[(bufferOffset + i)*CHANNELS + k] < -1.0)
                    {
                        accumBuff[(bufferOffset + i)*CHANNELS + k] = -1.0;
                    }
                }
            }
            
        }
    }
    else
    {
        // cout << "return - not playing " << endl;
        return;
    }
}

// GRAPHICS-------------------------------------------------------------------//

GrainVis::~GrainVis() {}

GrainVis::GrainVis(Style *style,
                   unsigned int winWidth,
                   unsigned int winHeight,
                   float x, float y)
{
    _style = style;
    
    _winWidth = winWidth;
    _winHeight = winHeight;
    
    _gX = x;
    _gY = y;
    
    float grainVisColor[4];
    _style->getGrainVisColor(grainVisColor);
    _colR = grainVisColor[0];
    _colG = grainVisColor[1];
    _colB = grainVisColor[2];
    _colA = grainVisColor[3];

    float grainVisDefColor[4];
    _style->getGrainVisDefColor(grainVisDefColor);
    _defR = grainVisDefColor[0];
    _defG = grainVisDefColor[1];
    _defB = grainVisDefColor[2];
    _defA = grainVisDefColor[3];
    
    _defSize = 10.0f;
    _size = _defSize;
    _onSize = 30.0f;
    _isOn = false;
    _firstTrigger = false;
    _startTime = GTime::instance()._sec;
    _triggerTime = 0.0;
    
    //TODO:  colors
}

void
GrainVis::updateWinDimensions(unsigned int newWinWidth,
                              unsigned int newWinHeight)
{
    if ((_winWidth == newWinWidth) && 
        (_winHeight == newWinHeight))
        return;

    // update the rect pos and size if window size changed
    float wRatio = ((float)newWinWidth)/_winWidth;
    float hRatio = ((float)newWinHeight)/_winHeight;

    float sizeRatio = sqrt(newWinWidth*newWinWidth + newWinHeight*newWinHeight)/
        sqrt(_winWidth*_winWidth + _winHeight*_winHeight);
    
    _gX *= wRatio;
    _gY *= hRatio;

    _size *= sizeRatio;
    _defSize *= sizeRatio;
    _onSize *= sizeRatio;
    
    _winWidth = newWinWidth;
    _winHeight = newWinHeight;
}

// draw method
void
GrainVis::draw()
{
    float colR;
    float colG;
    float colB;
    float colA;
    
    double t_sec = GTime::instance()._sec - _triggerTime;
    if (_firstTrigger)
    {
        // slew size
        double mult = 0.0;
        if (_isOn)
        {
            mult = exp(-t_sec/(0.8*_durSec));
            
            colR = mult*_colR;
            colG = mult*_colG;
            colB = mult*_colB;
            colA = mult*_colA;
            //colA = _colA;

            _size = _defSize + (1.0 - mult)*(_onSize-_defSize);

            if (colB < 0.001)
                _isOn = false;
        }
        else
        {
            mult = 1.0-exp(-t_sec/(0.2*_durSec));
            
            colR = mult*_defR;
            colG = mult*_defG;
            colB = mult*_defB;
            colA = mult*_defA;
            //colA = _colA;
            _size = _defSize + (1.0 - mult)*(_onSize-_defSize);
        }
    }
    
    glColor4f(colR,colG,colB,colA);
    
    // disk version - for graphics cards that don't support GL_POINT_SMOOTH
    //       glTranslatef((GLfloat)gX,(GLfloat)gY,0.0);
    //       gluDisk(gluNewQuadric(),0,_size*0.5f, 64,1);
    
    // end disk version
    
    
    // point version (preferred - better quality)
    float prevSize = 1.0f;
    glGetFloatv(GL_POINT_SIZE, &prevSize);
    glPointSize(_size);
    glBegin(GL_POINTS);
    glVertex3f(_gX, _gY, 0.0);
    glEnd();
    glPointSize(prevSize);
    //end point version
}

void
GrainVis::trigger(float dur)
{
    _isOn = true;
    if (!_firstTrigger)
        _firstTrigger = true;
    _durSec = dur*0.001;
    _triggerTime = GTime::instance()._sec;
}

// move to
void
GrainVis::moveTo(float x, float y)
{
    _gX = x;
    _gY = y;
}

float
GrainVis::getX()
{
    return _gX;
}

float
GrainVis::getY()
{
    return _gY;
}
