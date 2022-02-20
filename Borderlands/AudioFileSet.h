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
//  AudioFileSet.h
//  Borderlands
//
//  Created by Christopher Carlson on 11/21/11.
//


#ifndef AUDIOFILESET_H
#define AUDIOFILESET_H

#include <vector>
#include <string>
#include  <iostream>
using namespace std;

#include "sndfile.h"
#include "dirent.h"
#include "globals.h"


// basic encapsulation of an audio file
struct AudioFile
{    
    AudioFile(string name, string path,
              unsigned int numChan, unsigned long numFrames,
              unsigned int sampleRate, SAMPLE *wave)
    {
        cout << numFrames << endl;
        
        _name = name;
        _path = path;
        _frames = numFrames;
        _lengthSamps = numFrames * numChan;
        _channels = numChan;
        _sampleRate = sampleRate;
        _wave = wave;

    }
    
    virtual ~AudioFile()
    {
        if (_wave != NULL)
            delete []_wave;
    }
    
    string _name;
    string _path;
    SAMPLE *_wave;
    unsigned long _frames;
    unsigned long _lengthSamps;
    unsigned int _channels;
    unsigned int _sampleRate;
};


class AudioFileSet
{    
public:
    AudioFileSet();

    virtual ~AudioFileSet();
    
    //read in all audio files contained in 
    int loadFileSet(string path);
    
    //return the audio vector- note, the intension is for the files to be
    //read only.  if write access is needed in the future - thread safety will
    //need to be considered
    vector<AudioFile *> *getFileVector();
    
    
private:    
    vector<AudioFile *> *_fileSet;
};

#endif
