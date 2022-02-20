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
//  AudioFileSet.cpp
//  Borderlands
//
//  Created by Christopher Carlson on 11/21/11.
//

#include <math.h>

#include "globals.h"
#include "AudioFileSet.h"

//---------------------------------------------------------------------------
// constructor
//---------------------------------------------------------------------------
AudioFileSet::AudioFileSet()
{
    // init fileset
    _fileSet = new vector<AudioFile *>;
}

//---------------------------------------------------------------------------
// Destructor
//---------------------------------------------------------------------------
AudioFileSet::~AudioFileSet()
{
    // delete each audio file object (and corresponding buffer, etc.)
    if (_fileSet != NULL)
    {
        for (int i = 0; i < _fileSet->size(); i++)
        {
            delete _fileSet->at(i);
        }
    }
}

//---------------------------------------------------------------------------
// Access file set externally (note this is not thread safe)
//---------------------------------------------------------------------------
vector<AudioFile *>
*AudioFileSet::getFileVector()
{
    return _fileSet;
}


//---------------------------------------------------------------------------
//  Search path and load all audio files into memory.  Convert to mono (L) 
//  if needed.
//---------------------------------------------------------------------------
int
AudioFileSet::loadFileSet(string localPath)
{
    // read through loop directory and attempt to load audio files into buffers
    
    // using dirent
    DIR *dir;
    struct dirent *ent;
    
    // counter var for number of files loaded
    int fileCounter = 0;
    
    // get directory
    dir = opendir(localPath.c_str());
    
    // if directory exists - jump on in
    if (dir != NULL)
    {    
        // print all the files and directories within directory
        while ((ent = readdir (dir)) != NULL)
        {
            // get filename
            string fileName = ent->d_name;
            
            // skip cd, top directory, other files
            if ((fileName == ".") || (fileName == "..") ||
                (fileName == ".DS_Store")|| (fileName == ".svn"))
            {
                continue;
            }
            
            // construct path
            string path = localPath + fileName;
            
            // temp struct that will hold the details of the file being read
            // (sample rate, num channels. etc.)
            SF_INFO sfinfo;
            
            // a pointer to the audio file to read
            SNDFILE *infile;
            
            // open the file for reading and get the header info
            if (!(infile = sf_open(path.c_str(), SFM_READ, &sfinfo)))
            {
                printf ("Not able to open input file %s.\n", fileName.c_str());
                // print the error message from libsndfile.
                puts(sf_strerror(NULL)) ;
                // skip to next
                continue;
            }
            
            // show the current file path (comment this eventually)
            printf("Loading '%s'... \n", fileName.c_str());
            
            // explore the file's info
            cout << "  Channels: " << sfinfo.channels << endl;
            cout << "  Frames: " << sfinfo.frames << endl;
            cout << "  Sample Rate: " << sfinfo.samplerate << endl;
            cout << "  Format: " << sfinfo.format << endl;
            
            // warn about sampling rate incompatibility
            if (sfinfo.samplerate != SRATE)
            {
                printf("\nWARNING: '%s' is sampled at a different rate from \
the current sample rate of %i\n",fileName.c_str(), SRATE);
            }
            
            //MONO CONVERSION SET ASIDE FOR NOW...  number of channels for each file is dealt with 
            //by external audio processing algorithms
            //notify user of mono conversion
            //            if (sfinfo.channels > 1){
            //  printf("\nWARNING: '%s' will be converted to mono using left channel \n\n",theFileName.c_str());
            //            }
            
            // read the contents of the file in chunks of 512 samples
            const int buffSize = 512;
            
            //we will convert the file to mono - left channel, but store the stereo frames temporarily
            SAMPLE stereoBuff[buffSize];
            
            //length of mono buffer
            //            unsigned int monoLength = sfinfo.frames/sfinfo.channels;
            
            
            //allocate memory for the new waveform (new audio file entry in the fileSet)
            //length corresponds to the number of frames * number of channels  (1 frame contains L, R pair or chans 1,2,3...)
            unsigned long fullSize = sfinfo.frames * sfinfo.channels;
            
            _fileSet->push_back(new AudioFile(fileName,
                                              path,
                                              sfinfo.channels,
                                              sfinfo.frames,
                                              sfinfo.samplerate,
                                              new SAMPLE[fullSize]));

                               
            // accumulate the samples
            unsigned long counter = 0;
            bool empty = false;
            do
            {
                // read the samples as doubles
#if USE_SAMPLES_DOUBLE
                sf_count_t count =
                    sf_read_double(infile, &stereoBuff[0], buffSize);
#else
                sf_count_t count =
                    sf_read_float(infile, &stereoBuff[0], buffSize);
#endif

#if NORMALISE_LOADED_SOUND_FILES
                SAMPLE maxVal = 0.0;
                for(int i = 0; i < buffSize; i++)
                {
                    if (fabs(stereoBuff[i]) > maxVal)
                        maxVal = fabs(stereoBuff[i]);
                }

                int normFactor = 1.0;
                if (maxVal > 1e-15)
                    normFactor = 1.0/maxVal;
                
                for(int i = 0; i < buffSize; i++)
                    stereoBuff[i] *= normFactor;
#endif
                
                // break if we reached the end of the file
                // print the sample values to screen
                for(int i = 0; i < buffSize; i++)
                {
                    if (counter < fullSize)
                    {
                        //if ((i % sfinfo.channels) == 0){
                        _fileSet->at(fileCounter)->_wave[counter] =
                            stereoBuff[i]*globalAtten;
                        counter++;
                    }     
                }
                
                if (count == 0) 
                {
                    empty = true;
                    continue;
                }
                
            } while(!empty);

            cout << counter << endl;
            //increment the file counter
            fileCounter++;
            
            // don't forget to close the file	
            sf_close(infile);
        }
        
        //close the directory that we've been navigating
        closedir(dir);
    }
    else
    {
        /* could not open directory */
        perror("");
        return 1;
    }

    return 0;
}
