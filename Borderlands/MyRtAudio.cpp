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



//-----------------------------------------------------------------------------
// name: MyRtAudio.cpp
// implementation of wrapper class for RtAudio
// author: Chris Carlson
//   date: fall 2011
//-----------------------------------------------------------------------------

#include "globals.h"
#include "MyRtAudio.h"

using namespace std;

MyRtAudio::~MyRtAudio()
{
    // cleanup   
    delete _audio;
    // cerr << "rtaudio cleanup reached " << endl;
}

MyRtAudio::MyRtAudio(unsigned int numIns,
                     unsigned int numOuts,
                     unsigned int srate,
                     unsigned int *bufferSize,
                     RtAudioFormat format,
                     bool showWarnings)
{
    // configure RtAudio
    // create pointer to RtAudio object
    _audio = NULL;
    // create the object
    try
    {
        _audio = new RtAudio();
    } catch(RtAudioError &err)
    {
        err.printMessage();
        exit(1);
    }
    
    // io
    _numInputs = numIns;
    _numOutputs = numOuts;
    
    //check audio devices
    if (_audio->getDeviceCount() < 1)
    {
        // no audio available
        cout << "No Audio Devices Found!" << endl;
        exit(1);
    }
    
    // allow RtAudio to print msgs to stderr
    _audio->showWarnings(showWarnings);
    
    // store pointer to bufferSize
    _bufferSize = bufferSize;
    
    // set sample rate;
    _sampleRate = srate;
    
    // set format
    _format = format;
}


// set the audio callback and start the audio stream
void
MyRtAudio::openStream(RtAudioCallback callback)
{
    // create stream options
    RtAudio::StreamOptions options;
    //options.flags |= RTAUDIO_NONINTERLEAVED;
    
    RtAudio::StreamParameters iParams;
    RtAudio::StreamParameters oParams;
    
    // i/o params
    iParams.deviceId = _audio->getDefaultInputDevice();
    iParams.nChannels = _numInputs;
    iParams.firstChannel = 0;
    oParams.deviceId = _audio->getDefaultOutputDevice();
    oParams.nChannels = _numOutputs;
    oParams.firstChannel = 0;
    
    // open stream
    _audio->openStream(&oParams, &iParams,
                       FORMAT, _sampleRate,
                       _bufferSize,
                       callback, NULL, &options); 
}

// report the current buffer size
unsigned int
MyRtAudio::getBufferSize()
{        
    cerr << "Buffer size defined by RtAudio: " << (*_bufferSize) << endl;

    return (*_bufferSize);
}           

// start rtaudio stream
void
MyRtAudio::startStream()
{
    // start audio stream
    _audio->startStream();
}

// stop rtaudio stream
void
MyRtAudio::stopStream()
{
    // try to stop audio stream
    try
    {
        _audio->stopStream();
    }
    catch(RtAudioError &err)
    {
        err.printMessage();
    }
}

// close audio stream
void
MyRtAudio::closeStream()
{
    _audio->closeStream();
}

// report the stream latency
void
MyRtAudio::reportStreamLatency()
{
    // report latency
    cout << "Stream Latency: " << _audio->getStreamLatency() << " frames" << endl;    
}
