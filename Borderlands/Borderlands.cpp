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
//  main.cpp
//  Borderlands
//
//  Created by Christopher Carlson on 11/13/11.
//


//my includes
#include "globals.h"

//graphics includes
#ifdef __MACOSX_CORE__
#include <GLUT/glut.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#endif

#include "RtAudio.h"

//other libraries
#include <iostream>
#include <vector>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <cstdio>
#include <cstdlib>
#include <sstream>

//audio related
#include "MyRtAudio.h"
#include "AudioFileSet.h"
#include "Window.h"

//graphics related
#include "SoundRect.h"

//graphics and audio related
#include "GrainCluster.h"

#include "Style.h"

using namespace std;

//-----------------------------------------------------------------------------
// Defines a point in a 3D space (coords x, y and z)
//-----------------------------------------------------------------------------
struct pt3d
{ 
    pt3d( GLfloat _x, GLfloat _y, GLfloat _z ) :
    x(_x), y(_y), z(_z) {};
    
    float x;
    float y;
    float z;
};

//-----------------------------------------------------------------------------
// Shared Data Structures, Global parameters
//-----------------------------------------------------------------------------
// audio system
MyRtAudio * _audio = NULL;
// library path
string _audioPath = "./loops/";
// parameter string
string _paramString = "";
// desired audio buffer size 
unsigned int _buffSize = 1024;
// audio files
vector <AudioFile *> *_sounds = NULL;
// audio file visualization objects
vector <SoundRect *> *_soundViews = NULL;
// grain cloud audio objects
vector<GrainCluster *> *_grainCloud = NULL;
// grain cloud visualization objects
vector<GrainClusterVis *> *_grainCloudVis = NULL;
// cloud counter
unsigned int _numClouds = 0;

//global time increment - samples per second
//global time is incremented in audio callback
const double _sampTimeSec = (double) 1.0 / (double)SRATE;


//Initial camera movement vars
//my position
pt3d _position(0.0,0.0,0.0f);


//ENUMS 
// user selection mode
enum
{
    RECT,
    CLOUD
};
enum
{
    MOVE,
    RESIZE
};
// default selection mode
int _selectionMode = CLOUD;
int _dragMode = MOVE;
bool _resizeDir = false; //for rects
// rubber band select params
int _rbAnchorX = -1;
int _rb_AnchorY = -1;

// not used yet - for multiple selection
vector<int> *_selectionIndices = new vector<int>;

// selection helper vars
int _selectedCloud = -1;
int _selectedRect = -1;
bool _menuFlag = true;
int _selectionIndex = 0;

// cloud parameter changing
enum
{
    NUMGRAINS,
    DURATION,
    WINDOW,
    MOTIONX,
    MOTIONY,
    MOTIONXY,
    DIRECTION,
    OVERLAP,
    PITCH,
    ANIMATE,
    P_LFO_FREQ,
    P_LFO_AMT,
    SPATIALIZE,
    VOLUME
};
// flag indicating parameter change
bool _paramChanged = false;
unsigned int _currentParam = NUMGRAINS;
double _lastParamChangeTime = 0.0;
double _tempParamVal = -1.0;

// mouse coordinate initialization
int _mouseX = -1;
int _mouseY = -1;
long _veryHighNumber = 50000000;
long _lastDragX = _veryHighNumber;
long _lastDragY = _veryHighNumber;

//
unsigned int _screenWidth;
unsigned int _screenHeight;

unsigned int _winWidth;
unsigned int _winHeight;

// NOTE: start in windowed mode instead of fullscreen
// otherwise, when quitting fullscreen, the windows would stay maximized
// and would still take the whole screen (but with a top bar)
bool _isFullScreen = false;

Style *_style = NULL;

float _sidewaysMoveSpeed = 10.0f;
float _upDownMoveSpeed = 10.0f;

// for negative value entry
bool _negativeFlag = false;

bool _audioBounce = false;
FILE *_bounceFile = NULL;
char _bounceFileName[255];

//--------------------------------------------------------------------------------
// FUNCTION PROTOTYPES
//--------------------------------------------------------------------------------
void idleFunc();
void displayFunc();
void reshape(int w, int h);
void specialFunc(int key, int x, int y);
void keyboardFunc(unsigned char key, int x, int y);
void keyUpFunc(unsigned char key, int x, int y);
void deselect(int mode);

void mouseFunc(int button, int state, int x, int y);
void mouseDrag(int x, int y);
void mousePassiveMotion(int x, int y);
void updateMouseCoords(int x, int y);
void initialize();
void drawString(GLfloat x, GLfloat y, GLfloat z, const char * str, GLfloat scale);
void drawUsage();
void drawManual();
void drawParam();
void drawBounceFile();
void drawAxis();
void drawFps();

int audioCallback(void *outputBuffer, void *inputBuffer,
                  unsigned int numFrames, double streamTime,
                  RtAudioStreamStatus status, void *userData);
void cleaningFunction();


//--------------------------------------------------------------------------------
// Cleanup code
//--------------------------------------------------------------------------------
void
cleaningFunction()
{
    try
    {
        _audio->stopStream();
        _audio->closeStream();
    }
    catch (RtAudioError &err)
    {
        err.printMessage();
    }
    if (_sounds != NULL)
        delete _sounds;
    if (_audio != NULL)
        delete _audio;
    
    if (_grainCloud != NULL)
    {
        delete _grainCloud;
    }    
    
    if (_grainCloudVis != NULL)
    {
        delete _grainCloudVis;
    }
    if (_soundViews != NULL)
    {
        delete _soundViews;
    }
    if (_selectionIndices != NULL)
    {
        delete _selectionIndices;
    }

    if (_style != NULL)
    {
        delete _style;
        _style = NULL;
    }
}

void
updateWindowDimensions(int newWinWidth, int newWinHeight)
{
    // rects
    if (_soundViews)
    {
        for (int i = 0; i < _soundViews->size(); i++)
            (*_soundViews)[i]->updateWinDimensions(newWinWidth,newWinHeight);
    }
    
    // grains
    if (_grainCloudVis)
    {
        for (int i = 0; i < _grainCloudVis->size(); i++)
            (*_grainCloudVis)[i]->updateWinDimensions(newWinWidth,newWinHeight);
    }
}

//================================================================================
//   Audio Callback
//================================================================================

// audio callback
int
audioCallback(void *outputBuffer, void *inputBuffer,
              unsigned int numFrames, double streamTime,
              RtAudioStreamStatus status, void *userData)
{
    // cast audio buffers
    SAMPLE *out = (SAMPLE *)outputBuffer;
    SAMPLE *in = (SAMPLE *)inputBuffer;
    
    memset(out, 0, sizeof(SAMPLE)*numFrames*CHANNELS);
    if (!_menuFlag)
    {
        for(int i = 0; i < _grainCloud->size(); i++)
        {
            _grainCloud->at(i)->nextBuffer(out, numFrames);
        }
    }
    GTime::instance()._sec += numFrames*_sampTimeSec;
    // cout << GTime::instance()._sec<<endl;

    if (_bounceFile != NULL)
        fwrite(out, sizeof(float), numFrames*CHANNELS, _bounceFile);
    
    return 0;
}

// audio file bounce
void
toggleAudioBounce()
{
    if (!_audioBounce)
        _audioBounce = true;
    else
    {
        _audioBounce = false;

        memset(_bounceFileName, 0, 255);
    }

    if (_audioBounce)
    {
        long tm = time(NULL);
        
        int rnd = rand()%1000000;
        sprintf(_bounceFileName, "./bounces/borderlands-%ld-%d.raw", tm, rnd);

        if (_bounceFile != NULL)
            fclose(_bounceFile);
        
        _bounceFile = fopen(_bounceFileName, "wb");
    }
    else
    {
        if (_bounceFile != NULL)
        {
            fclose(_bounceFile);
            _bounceFile = NULL;
        }
    }
}

//================================================================================
//   GRAPHICS/GLUT
//================================================================================
void
windowInit()
{    
    // assign glut function calls
    glutIdleFunc(idleFunc);
    glutReshapeFunc(reshape);
    glutDisplayFunc(displayFunc);
    glutMouseFunc(mouseFunc);
    glutMotionFunc(mouseDrag);
    glutPassiveMotionFunc(mousePassiveMotion);
    glutKeyboardFunc(keyboardFunc);
    glutKeyboardUpFunc(keyUpFunc);
    // set the special function - called on special keys events (fn, arrows, pgDown, etc)
    glutSpecialFunc(specialFunc);
}

void
setWindowedMode()
{
    _winWidth = 0.6*_screenWidth;
    _winHeight = 0.6*_screenHeight;
    glutReshapeWindow(_winWidth,_winHeight);
    
    int centerXCorner = _screenWidth/2 - _winWidth/2;
    int centerYCorner = _screenHeight/2 - _winHeight/2;
    glutPositionWindow(centerXCorner,centerYCorner);
}

void
toggleFullScreen()
{
    if (_isFullScreen)
    {
        setWindowedMode();
        
        _isFullScreen = false;
        /*
        glutLeaveGameMode();
        cout << "left game mode" << endl;
        windowInit();
        glutMainLoop();
        */
    }
    else
    {    
        /*
        //glutFullScreen();
        string res;
        char buffer[33];
        _screenWidth = glutGet(GLUT_SCREEN_WIDTH);
        sprintf(buffer, "%d", _screenWidth);
        res.append(buffer);
        res.append("x");
        char buffer2[33];
        _screenHeight = glutGet(GLUT_SCREEN_HEIGHT);
        sprintf(buffer2,"%d",_screenHeight);
        res.append(buffer2);
        res.append(":32@75");
        cout << res.c_str() << endl;
        
        //presentation view
        //set resolution/refresh rate
        glutGameModeString(res.c_str());
        glutEnterGameMode();
        cout << "entered game mode" << endl;

                windowInit();
        glutMainLoop();
        */
        glutFullScreen();
        
        _isFullScreen = true;
    }
    
    //glutDisplayFunc(displayFunc);
}

//-----------------------------------------------------------
//  GLUT Initialization
//-----------------------------------------------------------
void
initialize()
{
    //_style = new DefaultStyle();
    //_style = new PurpleStyle();
    _style = new PsycheStyle();
    
    // initial window settings
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);    
    
    _screenWidth = glutGet(GLUT_SCREEN_WIDTH);
    _screenHeight = glutGet(GLUT_SCREEN_HEIGHT);

    _winWidth = 0.6*_screenWidth;
    _winHeight = 0.6*_screenHeight;
    
    glutInitWindowSize(_winWidth,_winHeight);
    glutInitWindowPosition (0, 0);
    glutCreateWindow("Borderlands");

    setWindowedMode();

    /*
    //Game Mode   
    string res;
    char buffer[33];
    _screenWidth = glutGet(GLUT_SCREEN_WIDTH);
    sprintf(buffer, "%d", _screenWidth);
    res.append(buffer);
    res.append("x");
    char buffer2[33];
    _screenHeight = glutGet(GLUT_SCREEN_HEIGHT);
    sprintf(buffer2,"%d",_screenHeight);
    res.append(buffer2);
    res.append(":32@75");
    cout << res.c_str() << endl;
    
    //presentation view
    //set resolution/refresh rate
    glutGameModeString(res.c_str());
    glutEnterGameMode();
    
    */
        
    // full screen end
    float bgColor[4];
    _style->getBackgroundColor(bgColor);
        
    // initial state
    glClearColor(bgColor[0], bgColor[1], bgColor[2], bgColor[3]);
    
    // enable depth buffer updates
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    // set polys with counterclockwise winding to be front facing 
    // this is gl default anyway
    glFrontFace(GL_CCW);
    // set fill mode
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glPolygonMode(GL_FRONT_AND_BACK, GL_POLYGON_SMOOTH);
    // enable transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // antialias lines and points
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_POINT_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT,GL_NICEST);
    glHint(GL_POINT_SMOOTH_HINT,GL_NICEST);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
    
    // assign glut function calls
    glutIdleFunc(idleFunc);
    glutReshapeFunc(reshape);
    glutDisplayFunc(displayFunc);
    glutMouseFunc(mouseFunc);
    glutMotionFunc(mouseDrag);
    glutPassiveMotionFunc(mousePassiveMotion);
    glutKeyboardFunc(keyboardFunc);
    glutKeyboardUpFunc(keyUpFunc);
    // set the special function - called on special keys events (fn, arrows, pgDown, etc)
    glutSpecialFunc(specialFunc);
}



//------------------------------------------------------------------------------
// GLUT display function
//------------------------------------------------------------------------------
void
displayFunc()
{
    // clear color and depth buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearDepth(1.0);
    
    // PUSH //save current transform
    glPushMatrix();
    
    glLoadIdentity();
    
    // update viewer position
    
    // translate the screen to the position of our camera
    glTranslatef(-_position.x, -_position.y, -_position.z);
    if (!_menuFlag)
    {
        // render rectangles
        if (_soundViews)
        {
            for (int i = 0; i < _soundViews->size(); i++)
            {
                _soundViews->at(i)->draw();
            }
        }
        
         // render grain clouds if they exist
         if (_grainCloudVis)
         {
             for (int i = 0; i < _grainCloudVis->size(); i++)
             {
                 _grainCloudVis->at(i)->draw();
             }
         }
        
         // print current param if editing
        if ((_selectedCloud >= 0) || (_selectedRect >= 0))
            drawParam();

        if (_bounceFile != NULL)
            drawBounceFile();
    }
    else
    {
        drawUsage();
        drawManual();
    }
    
    //drawUsage();
    drawFps();
    
    // POP ---//restore state
    glPopMatrix();
    
    // flush and swap

    // renders and empties buffers
    glFlush();
    // brings hidden buffer to the front (using double buffering for smoother graphics
    glutSwapBuffers();
}

//------------------------------------------------------------------------------
// GLUT reshape function
//------------------------------------------------------------------------------
void
reshape(int w, int h)
{
    glViewport(0, 0, (GLsizei) w, (GLsizei) h);
    _winWidth = w;
    _winHeight = h;

    updateWindowDimensions(_winWidth, _winHeight);
        
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, (GLdouble) _winWidth, 0.0, (GLdouble)_winHeight, -10.0, 10.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
     //translate the screen to the position of our camera
    glTranslatef(-_position.x, -_position.y, -_position.z);
    //request redisplay
    glutPostRedisplay();
}

//-----------------------------------------------------------------------------
// GlUT idle function 
//-----------------------------------------------------------------------------
void
idleFunc()
{
    // render the scene
    glutPostRedisplay();
}

///-----------------------------------------------------------------------------
// name: drawAxis()
// desc: draw 3d axis
//-----------------------------------------------------------------------------
void
drawAxis()
{
    // PUSH -- //store state
    glPushMatrix();
    
    // specify vertices with this drawing mode
    glBegin(GL_LINES);
    float axisLineWidth = _style->getAxisLineWidth();
    glLineWidth(axisLineWidth);

    // x axis
    float axisXColor[4];
    _style->getAxisXColor(axisXColor);
    glColor4f(axisXColor[0], axisXColor[1], axisXColor[2], axisXColor[3]);
    glVertex3f(0,0,0);
    glVertex3f(_winWidth,0,0);
    
    // y axis
    float axisYColor[4];
    _style->getAxisYColor(axisYColor);
    glColor4f(axisYColor[0], axisYColor[1], axisYColor[2], axisYColor[3]);
    glVertex3f(0,0,0);
    glVertex3f(0,_winHeight,0);
    
    // z axis
    float axisZColor[4];
    _style->getAxisZColor(axisZColor);
    glColor4f(axisZColor[0], axisZColor[1], axisZColor[2], axisZColor[3]);
    glVertex3f(0,0,0);
    glVertex3f(0,0,400);
    
    // stop drawing
    glEnd();
    
    // POP -- //restore state
    glPopMatrix();
}

void
drawFps()
{
    static int frame = 0;
    static int timebase = 0;
    
    frame++;
	int time = glutGet(GLUT_ELAPSED_TIME);

    static float fps = 0.0;
    
	if (time - timebase > 1000) {
		fps = frame*1000.0/(time-timebase);
	 	timebase = time;
		frame = 0;
    }
    
    // Draw
    char fpsStr[255];
    sprintf(fpsStr, "%d fps", (int)fps);

    float a = 0.6f + 0.2*sin(0.8*PI*GTime::instance()._sec);
        
    float paramColor[4];
    _style->getParamColor(paramColor);
    glColor4f(paramColor[0], paramColor[1],
              paramColor[2], a*paramColor[3]);
    
    glLineWidth(2.0f);
    
    const float scale = 0.06;
    drawString(0.0 + (float)_winWidth*scale*0.25, // x
               0.0 + (float)_winWidth*scale*0.25, // y
               0.5f,
               fpsStr,
               (float)_winWidth*scale);
}

//-----------------------------------------------------------------------------
// Display simple string
// desc: from sndpeek source - Ge Wang, et al
//-----------------------------------------------------------------------------
void
drawString(GLfloat x, GLfloat y, GLfloat z,
           const char * str, GLfloat scale = 1.0f)
{
    GLint len = strlen(str);
    GLint i;
    
    glPushMatrix();
    glTranslatef(x, _winHeight-y, z);
    glScalef(.001f*scale, .001f*scale, .001f*scale);
    
    for(i = 0; i < len; i++)
        glutStrokeCharacter(GLUT_STROKE_MONO_ROMAN, str[i]);
    
    glPopMatrix();
}

//-----------------------------------------------------------------------------
// Show usage on screen.  TODO:  add usage info 
//-----------------------------------------------------------------------------
void
drawUsage()
{
    float smallSize = 0.03f;
    float mediumSize = 0.04f;
    float usageLineWidth = _style->getUsageLineWidth();
    glLineWidth(usageLineWidth);
    
    float a = 0.6f + 0.2*sin(0.8*PI*GTime::instance()._sec);

    float colCoeffA[4];
    _style->getUsageAColorCoeff(colCoeffA);
    glColor4f(a*colCoeffA[0], a*colCoeffA[1],
              a*colCoeffA[2], a*colCoeffA[3]);
    drawString(_winWidth/2.0f + 0.2f*(float)_winWidth,
               (float)_winHeight/2.0f,
               0.5f,
               "BORDERLANDS",
               (float)_winWidth*0.1f);
   
    a = 0.6f + 0.2*sin(0.9*PI*GTime::instance()._sec);
    float insColor = a*0.4f;

    float colCoeffIns[4];
    _style->getUsageInsColorCoeff(colCoeffIns);
    glColor4f(insColor*colCoeffIns[0],
              insColor*colCoeffIns[1],
              insColor*colCoeffIns[2],
              a*colCoeffIns[3]);
    // key info
    drawString(_winWidth/2.0f + 0.2f*(float)_winWidth + 10.0,
               (float)_winHeight/2.0f + 30.0, 0.5f,"CLICK TO START",
               (float)_winWidth*0.04f);

    a = 0.6f + 0.2*sin(1.0*PI*GTime::instance()._sec);
    insColor = a*0.4f;
    glColor4f(insColor*colCoeffIns[0], insColor*colCoeffIns[1],
              insColor*colCoeffIns[2], a*colCoeffIns[3]);
    
    // key info
    drawString(_winWidth/2.0f + 0.2f*(float)_winWidth+10.0,
               (float)_winHeight/2.0f + 50.0, 0.5f,
               "ESCAPE TO QUIT",
               (float)_winWidth*0.04f);
}

//-----------------------------------------------------------------------------
// Show all the commands and keys on screen.
//-----------------------------------------------------------------------------
void
drawManual()
{
    float smallSize = 0.03f;
    float mediumSize = 0.04f;

    float manualLineWidth = _style->getManualLineWidth();
    glLineWidth(manualLineWidth);

    float insColor[4];
    _style->getManualInsColor(insColor);
    glColor4f(insColor[0], insColor[1],
              insColor[2], insColor[3]);

    char *commands[] =
        {
            "Entry/Exit",
            "------------",
            "ESC              Quit",
            "? key            Hide/Show Title Screen",
            "",
            "Rectangles",
            "------------",
            "Left click       Select",
            "Drag             Move",
            "TAB key          Cycle selection of overlapping rectangles under mouse",
            "R key + drag     Resize",
            "F key            Flip orientation",
            "",
            "Cloud Addition and Selection",
            "------------",
            "G key (+ shift)  Add (remove) cloud to/from end of collection",
            "Delete key       Remove selected cloud",
            "Left click       Select",
            "Drag             Move",
            "",
            "Cloud Parameters",
            "------------",
            "V key (+ shift)  Add (remove) voices",
            "A key            Toggle cloud on/off",
            "D key (+ shift)  Increment (decrement) duration",
            "D key + numbers  Enter duration value (ms) - press Enter to accept",
            "S key (+ shift)  Increment (decrement) overlap",
            "S key + numbers  Enter overlap value - press Enter to accept",
            "Z key (+ shift)  Increment (decrement) pitch",
            "Z key + numbers  Enter pitch value - press Enter to accept",
            "W key            Change window type (HANNING, TRIANGLE, EXPDEC, REXPDEC, SINC, RANDOM)",
            "W key + ",
            "1 through 6      Jump to specific window type",
            "F key            Switch grain direction (FORWARD, BACKWARD, RANDOM)",
            "R key            Enable mouse control of XY extent of grain position randomness",
            "X key            Enable mouse control of X extent of grain position randomness",
            "Y key            Enable mouse control of Y extent of grain position randomness",
            "T key            Switch spatialization modes",
            "L key (+ shift)  Adjust playback rate LFO frequency",
            "K key (+ shift)  Adjust playback rate LFO amplitude",
            "B key (+ shift)  Adjust cloud volume in dB",
            "------------",
            "Spacebar         Toggle audio bounce"
        };
            
    float x = (float)_winWidth*0.1;
    float y = (float)_winHeight*0.1;
    
    float yStep = (float)_winWidth*0.01f;
    float size = (float)_winWidth*0.06f;
    
    for (int i = 0; i < sizeof(commands)/sizeof(char *); i++)
    {
        const char *cmd = commands[i];
        drawString(x, y + i*yStep, 0.5,
                   cmd, size);
    }
}

void
drawParam()
{
    if ((_numClouds > 0) && (_selectedCloud >=0))
    {
        GrainClusterVis *cloudVis= _grainCloudVis->at(_selectedCloud);
        GrainCluster *cloud = _grainCloud->at(_selectedCloud);
        float cloudX = cloudVis->getX();
        float cloudY = cloudVis->getY();
        string value;
        ostringstream sinput;
        ostringstream sinput2;
        float a = 0.7f + 0.3*sin(1.6*PI*GTime::instance()._sec);

        float paramColor[4];
        _style->getParamColor(paramColor);
        glColor4f(paramColor[0], paramColor[1],
                  paramColor[2], a*paramColor[3]);
        
        switch (_currentParam)
        {
            case NUMGRAINS:
                value = "Voices: ";
                sinput << cloud->getNumVoices();            
                value = value+ sinput.str();
                drawString((GLfloat)_mouseX,
                           (GLfloat)(_winHeight-_mouseY),
                           0.0,
                           value.c_str(),
                           100.0f);
                break;
                
            case DURATION:
                value = "Duration: ";
                if (_paramString == "")
                {
                    sinput << cloud->getDurationMs();
                    value = value + sinput.str() + " ms";
                }
                else
                {
                    value = value + _paramString + " ms";
                }
                
                drawString((GLfloat)_mouseX,
                           (GLfloat)(_winHeight-_mouseY),
                           0.0,
                           value.c_str(),
                           100.0f);
                // value = "Duration (ms): " + cloud->getDurationMs();
                break;
                
            case WINDOW:
                switch (cloud->getWindowType())
                {
                    case HANNING:
                        value = "Window: HANNING";
                        break;
                        
                    case TRIANGLE:
                        value = "Window: TRIANGLE";
                        break;
                        
                    case REXPDEC:
                        value = "Window: REXPDEC";
                        break;
                        
                    case EXPDEC:
                        value = "Window: EXPDEC";
                        break;
                        
                    case SINC:
                        value = "Window: SINC";
                        break;
                        
                    case RANDOM_WIN:
                        value = "Window: RANDOM_WIN";
                        break;
                        
                    default:
                        value = "";
                        break;
                }
                
                drawString((GLfloat)_mouseX,
                           (GLfloat)(_winHeight-_mouseY),
                           0.0,
                           value.c_str(),
                           100.0f);
                break;
                
            case MOTIONX:
                value = "X: ";
                sinput << cloudVis->getXRandExtent();
                value = value + sinput.str();
                drawString((GLfloat)_mouseX,
                           (GLfloat)(_winHeight-_mouseY),
                           0.0,
                           value.c_str(),
                           100.0f);
                break;
                
            case MOTIONY:
                value = "Y: ";
                sinput << cloudVis->getYRandExtent();
                value = value + sinput.str();
                drawString((GLfloat)_mouseX,
                           (GLfloat)(_winHeight-_mouseY),
                           0.0,
                           value.c_str(),
                           100.0f);
                break;
                
            case MOTIONXY:
                value = "X,Y: ";
                sinput << cloudVis->getXRandExtent();
                value = value + sinput.str() + ", ";
                sinput2 << cloudVis->getYRandExtent();
                value = value + sinput2.str();
                drawString((GLfloat)_mouseX,
                           (GLfloat)(_winHeight-_mouseY),
                           0.0,
                           value.c_str(),
                           100.0f);
                break;
                
            case DIRECTION:
                switch(cloud->getDirection())
                {
                    case FORWARD:
                        value = "Direction: FORWARD";
                        break;
                        
                    case BACKWARD:
                        value = "Direction: BACKWARD";
                        break;
                        
                    case RANDOM_DIR:
                        value = "Direction: RANDOM";
                        break;
                        
                    default:
                        value = "";
                        break;
                }
                drawString((GLfloat)_mouseX,
                           (GLfloat)(_winHeight-_mouseY),
                           0.0,
                           value.c_str(),
                           100.0f);  
                break;
                
            case SPATIALIZE:
                switch(cloud->getSpatialMode())
                {
                    case UNITY:
                        value = "Spatial Mode: UNITY";
                        break;
                        
                    case STEREO:
                        value = "Spatial Mode: STEREO";
                        break;
                        
                    case AROUND:
                        value = "Spatial Mode: AROUND";
                        break;
                        
                    default:
                        value = "";
                        break;
                }
                drawString((GLfloat)_mouseX,
                           (GLfloat)(_winHeight-_mouseY),
                           0.0,
                           value.c_str(),
                           100.0f);  
                break;
                
            case VOLUME:
                value = "Volume (dB): ";
                if (_paramString == "")
                {
                    sinput << cloud->getVolumeDb();
                    value = value + sinput.str();
                }
                else
                {
                    value = value + _paramString;
                }
                drawString((GLfloat)_mouseX,
                           (GLfloat)(_winHeight-_mouseY),
                           0.0,
                           value.c_str(),
                           100.0f);
                break;
                
            case OVERLAP:
                value = "Overlap: ";
                if (_paramString == "")
                {
                    sinput << cloud->getOverlap();
                    value = value + sinput.str();
                }
                else
                {
                    value = value + _paramString;
                }
                drawString((GLfloat)_mouseX,
                           (GLfloat)(_winHeight-_mouseY),
                           0.0,
                           value.c_str(),
                           100.0f);
                // value = "Duration (ms): " + cloud->getDurationMs();
                break;
                
            case PITCH:
                value = "Pitch: ";
                if (_paramString == "")
                {
                    sinput << cloud->getPitch();
                    value = value + sinput.str();
                }
                else
                {
                    value = value + _paramString;
                }
                drawString((GLfloat)_mouseX,
                           (GLfloat)(_winHeight-_mouseY),
                           0.0,
                           value.c_str(),
                           100.0f);
                // value = "Duration (ms): " + cloud->getDurationMs();
                break;
                
            case P_LFO_FREQ:
                value = "Pitch LFO Freq: ";
                if (_paramString == "")
                {
                    sinput << cloud->getPitchLFOFreq();
                    value = value + sinput.str();
                }
                else
                {
                    value = value + _paramString;
                }
                drawString((GLfloat)_mouseX,
                           (GLfloat)(_winHeight-_mouseY),
                           0.0,
                           value.c_str(),
                           100.0f);
                // value = "Duration (ms): " + cloud->getDurationMs();
                break;
                
            case P_LFO_AMT:
                value = "Pitch LFO Amount: ";
                if (_paramString == "")
                {
                    sinput << cloud->getPitchLFOAmount();
                    value = value + sinput.str();
                }
                else
                {
                    value = value + _paramString;
                }
                drawString((GLfloat)_mouseX,
                           (GLfloat)(_winHeight-_mouseY),
                           0.0,
                           value.c_str(),
                           100.0f);
                // value = "Duration (ms): " + cloud->getDurationMs();
                break;
                
            default:
                break;
        }
    }
}

void
drawBounceFile()
{
    if (_bounceFile == NULL)
        return;

    float a = 0.6f + 0.2*sin(0.8*PI*GTime::instance()._sec);

    float paramColor[4];
    _style->getParamColor(paramColor);
    glColor4f(paramColor[0], paramColor[1],
              paramColor[2], a*paramColor[3]);

    char str[255];
    sprintf(str, "*** Bouncing to: %s ***",
            _bounceFileName);

#define BORDER 0.02
    drawString(BORDER*(float)_winWidth,
               (float)_winHeight - BORDER*_winHeight,
               0.5f,
               str,
               (float)_winWidth*0.1f);
}

//================================================================================
//   INTERACTION/GLUT
//================================================================================


// update mouse coords based on mousemovement
void
updateMouseCoords(int x, int y)
{
    _mouseX = x + _position.x;
    _mouseY = (_winHeight - (y-_position.y));
}

//-----------------------------------------------------------------------------
// Handle special function keys (arrows, etc)
//-----------------------------------------------------------------------------
void
specialFunc(int key, int x, int y)
{
    // cout << "special key" << key <<endl;
    
    switch (key)
    {
        case GLUT_KEY_LEFT:
            // move to the left
            _position.x -=  _sidewaysMoveSpeed;
            _mouseX -=_sidewaysMoveSpeed;
            break;
            
        case GLUT_KEY_RIGHT:
            // move to the right
            _position.x += _sidewaysMoveSpeed;
            _mouseX +=_sidewaysMoveSpeed;
            break;
            
        case GLUT_KEY_DOWN:
            // move backward
            _position.y -= _upDownMoveSpeed;
            _mouseY +=_sidewaysMoveSpeed;
            break;
            
        case GLUT_KEY_UP:
            // move forward
            _position.y += _upDownMoveSpeed;
            _mouseY -=_sidewaysMoveSpeed;
            break;
            
        case 14:
            // cout << "shift down" << endl;
            break;
            
        case 15://shift in
            // cout << "shift up" << endl;
            break;
            
        default:
            break;
    }
    // request redisplay
    glutPostRedisplay();
}

//-----------------------------------------------------------------------------
// Handle key up events (other than special keys)
//-----------------------------------------------------------------------------
void
keyUpFunc(unsigned char key, int x, int y)
{
    // cout << key << endl;
    switch (key)
    {
        case 'a':
            break;
            
        case 'R':
        case 'r':
            _dragMode = MOVE;
            _lastDragX = _veryHighNumber;
            _lastDragY = _veryHighNumber;
            break;
            
        default:
            break;
    }
    
    glutPostRedisplay();
}

//-----------------------------------------------------------------------------
// Handle key down events (other than special keys)
//-----------------------------------------------------------------------------
void
keyboardFunc(unsigned char key, int x, int y)
{
    int modkey = glutGetModifiers();
    // cout << "key pressed " << key << endl;
    
    switch(key)
    {
        case 9: // tab key       
            if (_selectionIndices->size() > 1)
            {
                _soundViews->at(_selectedRect)->setSelectState(false);
                _selectionIndex++;
                if (_selectionIndex >= _selectionIndices->size())
                    _selectionIndex = 0;
                
                _selectedRect = _selectionIndices->at(_selectionIndex);
                _soundViews->at(_selectedRect)->setSelectState(true);
            }
            break;
            
        case '1':
            _paramString.push_back('1');
            if (_currentParam == WINDOW)
            {
                if (_selectedCloud >=0)
                    _grainCloud->at(_selectedCloud)->setWindowType(0);
            }
            break;
            
        case '2':
            _paramString.push_back('2');
            if (_currentParam == WINDOW)
            {
                if (_selectedCloud >=0)
                    _grainCloud->at(_selectedCloud)->setWindowType(1);
            }
            break;
            
        case '3':
            _paramString.push_back('3');
            if (_currentParam == WINDOW)
            {
                if (_selectedCloud >=0)
                    _grainCloud->at(_selectedCloud)->setWindowType(2);
            }
            break;
            
        case '4':
            _paramString.push_back('4');
            if (_currentParam == WINDOW)
            {
                if (_selectedCloud >=0)
                    _grainCloud->at(_selectedCloud)->setWindowType(3);
            }
            break;
            
        case'5':
            _paramString.push_back('5');
            if (_currentParam == WINDOW)
            {
                if (_selectedCloud >=0)
                    _grainCloud->at(_selectedCloud)->setWindowType(4);
            }
            break;
            
        case'6':
            _paramString.push_back('6');
            if (_currentParam == WINDOW)
            {
                if (_selectedCloud >=0)
                    _grainCloud->at(_selectedCloud)->setWindowType(5);
            }
            break;
            
        case'7':
            _paramString.push_back('7');
            break;

        case'8':
            _paramString.push_back('8');
            break;

        case'9':
            _paramString.push_back('9');
            break;

        case '0':
            _paramString.push_back('0');
            break;

        case '.':
            _paramString.push_back('.');            
            break;
            
        case 13: // enter key - for saving param string
            if (_paramString != "")
            {
                float value = atof(_paramString.c_str());
                
                // cout << "value received " << value << endl;
                switch (_currentParam)
                {
                    case DURATION:
                        if (_selectedCloud >=0)
                        {
                            if (value < 1.0)
                                value = 1.0;
                            
                            _grainCloud->at(_selectedCloud)->setDurationMs(value);
                        }
                        break;
                        
                    case OVERLAP:
                        if (_selectedCloud >=0)
                            _grainCloud->at(_selectedCloud)->setOverlap(value);
                        break;
                        
                    case PITCH:
                        if (_selectedCloud >=0)
                            _grainCloud->at(_selectedCloud)->setPitch(value);
                        break;
                        
                    case P_LFO_FREQ:
                        if (_selectedCloud >=0)
                            _grainCloud->at(_selectedCloud)->setPitchLFOFreq(value);
                        break;
                        
                    case P_LFO_AMT:
                        if (_selectedCloud >=0)
                            _grainCloud->at(_selectedCloud)->setPitchLFOAmount(value);
                        break;
                        
                    case VOLUME:
                        if (_selectedCloud >=0)
                            _grainCloud->at(_selectedCloud)->setVolumeDb(value);
                        
                    default:
                        break;
                }
                
                _paramString = "";
            }
            // cout << "enter key caught" << endl;
            break;
            
        case 27: // esc key
            cleaningFunction();
            exit(1);
            break;
            
        case 'Q':// spatialization
        case 'q':  
            break;
            
        case 'O':
        case 'o':
            toggleFullScreen();
            break;
            
        case 'T':
        case 't':
            _paramString = "";
            if (_selectedCloud >=0)
            {
                if (_currentParam != SPATIALIZE)
                    _currentParam = SPATIALIZE;
                else
                {
                    if (modkey == GLUT_ACTIVE_SHIFT)
                    {
                        if (_selectedCloud >=0)
                        {
                            int spat = _grainCloud->at(_selectedCloud)->
                                getSpatialMode();
                            _grainCloud->at(_selectedCloud)->
                                setSpatialMode(spat - 1,-1);       
                        }
                    }
                    else
                    {
                        if (_selectedCloud >=0)
                        {
                            int spat = _grainCloud->at(_selectedCloud)->
                                getSpatialMode();
                            _grainCloud->at(_selectedCloud)->
                                setSpatialMode(spat + 1, -1);
                        }
                    }
                }
            }
            break;
            
        case 'S': // overlap control 
        case 's':
            _paramString = "";
            if (_currentParam != OVERLAP)
                _currentParam = OVERLAP;
            else
            {
                if (modkey == GLUT_ACTIVE_SHIFT)
                {
                    if (_selectedCloud >=0)
                    {
                        float overlap =  _grainCloud->at(_selectedCloud)->
                            getOverlap();
                        _grainCloud->at(_selectedCloud)->setOverlap(overlap - 0.01f);
                    }
                }
                else
                {
                    if (_selectedCloud >=0)
                    {
                        float overlap =  _grainCloud->at(_selectedCloud)->
                            getOverlap();
                        _grainCloud->at(_selectedCloud)->setOverlap(overlap + 0.01f);
                    }
                }
            }
            break;
            
        case 'R':
        case 'r':
            if (_selectedCloud >=0)
                _currentParam = MOTIONXY;
            // toggle selection modes
            _dragMode = RESIZE;
            break;
            
        case 'F': // direction
        case 'f':
            _paramString = "";
            if (_selectedCloud >=0)
            {
                if (_currentParam != DIRECTION)
                    _currentParam = DIRECTION;
                else
                {
                    if (modkey == GLUT_ACTIVE_SHIFT)
                    {
                        if (_selectedCloud >=0)
                        {
                            int dir = _grainCloud->at(_selectedCloud)->
                                getDirection();
                            _grainCloud->at(_selectedCloud)->
                                setDirection(dir - 1);
                            
                        }
                    }
                    else
                    {
                        if (_selectedCloud >=0)
                        {
                            int dir = _grainCloud->at(_selectedCloud)->
                                getDirection();
                            _grainCloud->at(_selectedCloud)->
                                setDirection(dir + 1);
                        }
                    }
                }
            }
            
            if (_selectedRect >=0)
                _soundViews->at(_selectedRect)->toggleOrientation();
            
            // cerr << "Looking from the front" << endl;
            break;
            
        case 'P': // waveform display on/off
        case 'p':
            // for (int i = 0; i < _soundViews->size();i++){
            //     _soundViews->at(i)->toggleWaveDisplay();
            // }
            break;
            
        case 'W': // window editing for grain
        case 'w':
            _paramString = "";
            if (_currentParam != WINDOW)
                _currentParam = WINDOW;
            else
            {
                if (modkey == GLUT_ACTIVE_SHIFT)
                {
                    if (_selectedCloud >=0)
                    {
                        int win = _grainCloud->at(_selectedCloud)->
                            getWindowType();
                        _grainCloud->at(_selectedCloud)->setWindowType(win - 1);
                    }
                }
                else
                {
                    if (_selectedCloud >=0)
                    {
                        int win = _grainCloud->at(_selectedCloud)->
                            getWindowType();
                        _grainCloud->at(_selectedCloud)->
                            setWindowType(win + 1);
                    }
                }
            }
            break;
            
        case 'B':
        case 'b':
            // cloud volume
            _paramString = "";
            if (_currentParam != VOLUME)
                _currentParam = VOLUME;
            else
            {
                if (modkey == GLUT_ACTIVE_SHIFT)
                {
                    if (_selectedCloud >=0)
                    {
                        float vol = _grainCloud->at(_selectedCloud)->
                            getVolumeDb();
                        _grainCloud->at(_selectedCloud)->
                            setVolumeDb(vol - 0.5f);
                    }
                }
                else
                {
                    if (_selectedCloud >=0)
                    {
                        float vol = _grainCloud->at(_selectedCloud)->
                            getVolumeDb();
                        _grainCloud->at(_selectedCloud)->
                            setVolumeDb(vol + 0.5f);
                    }
                }
            }
            break;
            
        case '/': // switch to menu view
        case '?':
            _menuFlag = !_menuFlag;
            break;
            
        case 'G':
        case 'g':
            _paramString = "";
            deselect(RECT);
            if (_grainCloud != NULL)
            {
                if (modkey == GLUT_ACTIVE_SHIFT)
                {
                    if (_grainCloud->size() > 0)
                    {
                        _grainCloud->pop_back();
                        _grainCloudVis->pop_back();
                        _numClouds-=1;
                        // cout << "cloud removed" << endl;
                    }
                    if (_numClouds == 0)
                        _selectedCloud = -1;
                    else
                    {
                        // still have a cloud so select
                        _selectedCloud = _numClouds-1;
                        _grainCloudVis->at(_selectedCloud)->setSelectState(true);
                    }
                    
                    break;    
                }
                else
                {
                    int numVoices = 8; // initial number of voices
                    int idx = _grainCloud->size();
                    if (_selectedCloud >=0)
                    {
                        if (_numClouds > 0)
                            _grainCloudVis->at(_selectedCloud)->
                                setSelectState(false);
                    }
                    
                    _selectedCloud = idx;

                    // create audio
                    _grainCloud->
                        push_back(new GrainCluster(_style,
                                                   _sounds,numVoices));
                    // create visualization
                    _grainCloudVis->
                        push_back(new GrainClusterVis(_style,
                                                      _winWidth,
                                                      _winHeight,
                                                      _mouseX,
                                                      _mouseY,
                                                      numVoices,
                                                      _soundViews));
                    // select new cloud
                    _grainCloudVis->at(idx)->setSelectState(true);
                    // register visualization with audio
                    _grainCloud->at(idx)->registerVis(_grainCloudVis->at(idx));
                    // _grainCloud->at(idx)->toggleActive();
                    _numClouds+=1;
                }
                // cout << "cloud added" << endl;
                // grainControl->newCluster(_mouseX,_mouseY,1);
            }
            break;
            
        case 'V': // grain voices (add, delete)
        case 'v':
            _paramString = "";
            if (_currentParam != NUMGRAINS)
                _currentParam = NUMGRAINS;
            else
            {
                if (modkey == GLUT_ACTIVE_SHIFT)
                {
                    if (_selectedCloud >=0)
                    {
                        if (_grainCloud)
                            _grainCloud->at(_selectedCloud)->removeGrain();
                        // cout << "grain removed" << endl;
                    }
                }
                else
                {
                    if (_selectedCloud >=0)
                    {
                        if (_grainCloud)
                            _grainCloud->at(_selectedCloud)->addGrain();
                        // cout << "grain added" << endl;
                    }
                }
            }
            break;
        
        case 'D':
        case 'd':
            _paramString = "";
            if (_currentParam != DURATION)
                _currentParam = DURATION;
            else
            {
                if (modkey == GLUT_ACTIVE_SHIFT)
                {
                    if (_selectedCloud >=0)
                    {
                        float dur = _grainCloud->at(_selectedCloud)->
                            getDurationMs();
                        _grainCloud->at(_selectedCloud)->
                            setDurationMs(dur - 5.0f);
                    }
                }
                else
                {
                    if (_selectedCloud >=0)
                    {
                        float dur = _grainCloud->at(_selectedCloud)->
                            getDurationMs();
                        _grainCloud->at(_selectedCloud)->
                            setDurationMs(dur + 5.0f);
                    }
                }
            }
            break;
            
        case 'I':
        case 'i':
            break;
            
        case 'L':
        case 'l':
            _paramString = "";
            if (_currentParam != P_LFO_FREQ)
                _currentParam = P_LFO_FREQ;
            else
            {
                if (modkey == GLUT_ACTIVE_SHIFT)
                {
                    if (_selectedCloud >=0)
                    {
                        float LFOFreq = _grainCloud->at(_selectedCloud)->
                            getPitchLFOFreq();
                        _grainCloud->at(_selectedCloud)->
                            setPitchLFOFreq(LFOFreq - 0.01f);
                    }
                }
                else
                {
                    if (_selectedCloud >=0)
                    {
                        float LFOFreq = _grainCloud->at(_selectedCloud)->
                            getPitchLFOFreq();
                        _grainCloud->at(_selectedCloud)->
                            setPitchLFOFreq(LFOFreq + 0.01f);
                    }
                }
            }
            break;
            
        case 'K':
        case 'k':
            _paramString = "";
            if (_currentParam != P_LFO_AMT)
                _currentParam = P_LFO_AMT;
            else
            {
                if (modkey == GLUT_ACTIVE_SHIFT)
                {
                    if (_selectedCloud >=0)
                    {
                        float LFOAmt = _grainCloud->at(_selectedCloud)->
                            getPitchLFOAmount();
                        _grainCloud->at(_selectedCloud)->
                            setPitchLFOAmount(LFOAmt - 0.001f);
                    }
                }
                else
                {
                    if (_selectedCloud >=0)
                    {
                        float LFOAmnt = _grainCloud->at(_selectedCloud)->
                            getPitchLFOAmount();
                        _grainCloud->at(_selectedCloud)->
                            setPitchLFOAmount(LFOAmnt + 0.001f);
                    }
                }
            }
            break;
            
        case 'H':
        case 'h':
            break;
            
            //case ' ': // add delete
            //break;
            
        case 'X':
        case 'x':
            _paramString = "";
            if (_selectedCloud >= 0)
                _currentParam = MOTIONX;
            break;
            
        case 'Y':
        case 'y':
            _paramString = "";
            if (_selectedCloud >= 0)
                _currentParam = MOTIONY;
            break;
            
        case 'Z':
        case 'z':
            _paramString = "";
            if (_currentParam != PITCH)
                _currentParam = PITCH;
            else
            {
                if (modkey == GLUT_ACTIVE_SHIFT)
                {
                    if (_selectedCloud >=0)
                    {
                        float pitch =  _grainCloud->at(_selectedCloud)->
                            getPitch();
                        _grainCloud->at(_selectedCloud)->
                            setPitch(pitch - 0.01f);
                    }
                }
                else
                {
                    if (_selectedCloud >=0)
                    {
                        float pitch =  _grainCloud->at(_selectedCloud)->
                            getPitch();
                        _grainCloud->at(_selectedCloud)->
                            setPitch(pitch + 0.01f);
                    }
                }
            }
            break;
                        
        case '-':
        case '_':
            _paramString.insert(0, "-");
            break;
            
        case 127: // delete selected
        case 8: // delete (tested on mac keyboard)
            if (_paramString == "")
            {
                if (_selectedCloud >=0)
                {
                    _grainCloud->erase(_grainCloud->begin() + _selectedCloud);
                    _grainCloudVis->erase(_grainCloudVis->begin() + _selectedCloud);
                    _selectedCloud = -1;
                    _numClouds-=1;
                }
            }
            else
            {
                if (_paramString.size () > 0)
                    _paramString.resize(_paramString.size () - 1);
            }
            break;
            
        case 'A':
        case 'a':
            _paramString = "";
            if (_selectedCloud >=0)
                _grainCloud->at(_selectedCloud)->toggleActive();
            break;
            
        case '=':
        case '+':               
            break;
            
        case ',':
        case '<':
            break;
            
        case '>':
            break;

        case ' ':
            toggleAudioBounce();
            break;
            
        default:
            // cout << "key pressed " << key << endl;
            break;
    }
    glutPostRedisplay();
}

//-----------------------------------------------------------------------------
// Handle mouse clicks, etc.
//-----------------------------------------------------------------------------

// handle deselections
void
deselect(int shapeType)
{
    switch (shapeType)
    {
        case CLOUD:
            if (_selectedCloud >=0)
            {
                _grainCloudVis->at(_selectedCloud)->setSelectState(false);
                // reset selected cloud
                _selectedCloud = -1;
                // cout << "deselecting cloud" <<endl;
            }
            
        case RECT:
            if (_selectedRect >= 0)
            {
                // cout << "deselecting rect" << endl;
                _soundViews->at(_selectedRect)->setSelectState(false);
                _selectedRect = -1;
            }
    }
}



// mouse clicks
void
mouseFunc(int button, int state, int x, int y)
{
    // cout << "button " << button << endl;

    // look for selections if button is down
    if ((button == GLUT_LEFT_BUTTON) ||
        (button == GLUT_RIGHT_BUTTON) && (state == GLUT_DOWN))
    {            
        _paramString = "";
        
        // hide menu
        if (_menuFlag)
            _menuFlag = false;
                
        deselect(CLOUD);
        
        // deselect existing selections
        deselect(RECT);
        
        // exit parameter editing
        _currentParam = -1;
                
        _lastDragX = _veryHighNumber;
        _lastDragY = _veryHighNumber;
        
        // first check grain clouds to see if we have selection
        for (int i = 0; i < _grainCloudVis->size(); i++)
        {
            if (_grainCloudVis->at(i)->select(_mouseX, _mouseY))
            {
                _grainCloudVis->at(i)->setSelectState(true);
                _selectedCloud = i;
                break;
            }            
        }
                
                
        // clear selection buffer
        if (_selectionIndices)
            delete _selectionIndices;
        
        // allocate new buffer
        _selectionIndices = new vector<int>;
        _selectionIndex = 0;
        
        // if grain cloud is not selected - search for rectangle selection
        if (_selectedCloud < 0)
        {
            // search for selections
            
            //set resize direction to horizontal
            _resizeDir = false;
            for (int i = 0; i < _soundViews->size(); i++)
            {
                if (_soundViews->at(i)->select(_mouseX,_mouseY))
                {
                    _selectionIndices->push_back(i);
                    //_soundViews->at(i)->setSelectState(true);
                    //_selectedRect = i;
                    //break;
                }
            }
            
            if (_selectionIndices->size() > 0)
            {
                _selectedRect = _selectionIndices->at(0);
                _soundViews->at(_selectedRect)->setSelectState(true);
            }
        }
        
    }
    
    
    // handle button up
    // if ((button == GLUT_LEFT_BUTTON) && (state == GLUT_UP)){
    //     _lastDragX = -1;
    //     _lastDragY = -1;
    //     _dragMode = MOVE;
    // }
}

//-----------------------------------------------------------------------------
// Handle mouse movement during button press (drag)
//-----------------------------------------------------------------------------
void
mouseDrag(int x, int y)
{
    // update mouse coordinates based on drag position
    updateMouseCoords(x,y);
    int xDiff = 0;
    int yDiff = 0;
    
    if (_selectedCloud >= 0)
        _grainCloudVis->at(_selectedCloud)->
            updateCloudPosition(_mouseX,_mouseY);
    else
    {    
        switch (_dragMode)
        {
            case MOVE:
                if( (_lastDragX != _veryHighNumber) &&
                    (_lastDragY != _veryHighNumber))
                {    
                    if (_selectedRect >=0)
                        //movement case
                        _soundViews->at(_selectedRect)->
                            move(_mouseX - _lastDragX,_mouseY - _lastDragY);
                }
                _lastDragX = _mouseX;
                _lastDragY = _mouseY;
                break;
                
            case RESIZE:
                if((_lastDragX != _veryHighNumber) &&
                    (_lastDragY != _veryHighNumber))
                {
                    // cout << "drag ok" << endl;
                    // for width height - use screen coords
                    if (_selectedRect >= 0)
                    {
                        xDiff = x - _lastDragX;
                        yDiff = y - _lastDragY;
                        
                        // get width and height
                        float newWidth = _soundViews->at(_selectedRect)->getWidth();
                        float newHeight = _soundViews->at(_selectedRect)->getHeight();
                        
                        int thresh = 0;
                        
                        // check motion mag
                        if (xDiff < -thresh)
                            newWidth = newWidth * 0.8 +
                                0.2*(newWidth * (1.1 + abs(xDiff/50.0)));
                        else
                        {
                            if (xDiff > thresh)
                                newWidth = newWidth * 0.8 +
                                    0.2*(newWidth * (0.85 - abs(xDiff/50.0)));
                        }
                        if (yDiff > thresh)
                        {
                            newHeight = newHeight * 0.8 +
                                0.2*(newHeight * (1.1 + abs(yDiff/50.0)));
                        }else
                        {
                            if (yDiff < -thresh)
                                newHeight = newHeight * 0.8 +
                                    0.2*(newHeight * (0.85 - abs(yDiff/50.0)));
                        }
                        
                        // update width and height
                        _soundViews->at(_selectedRect)->
                            setDimensions(newWidth,newHeight);
                    }
                }   
                _lastDragX = x;
                _lastDragY = y;
                break;
                
            default:
                break;
        }
    }
}



//-----------------------------------------------------------------------------
// Handle mouse movement anytime
//-----------------------------------------------------------------------------
void
mousePassiveMotion(int x, int y)
{
    updateMouseCoords(x,y);
    
    if (_selectedCloud >=0)
    {
        switch (_currentParam)
        {
                case MOTIONX:
                    _grainCloudVis->at(_selectedCloud)->setXRandExtent(_mouseX);
                    break;
                    
                case MOTIONY:
                    _grainCloudVis->at(_selectedCloud)->setYRandExtent(_mouseY);
                    break;
                    
                case MOTIONXY:
                    _grainCloudVis->at(_selectedCloud)->setRandExtent(_mouseX,
                                                                      _mouseY);
                    break;
                    
                default:
                    break;
            }
        }
    // case NUMGRAINS:
    //   break;
    // case DURATION:
    // _grainCloud->at(_selectedCloud)->setDurationMs((_mouseY/_winHeight)*4000.0f);
    //            
    // default:
    // break;
    //   }
    // }
    // process rectangles
    // for (int i = 0; i < _soundViews->size(); i++)
    //     _soundViews->at(i)->procMovement(x, y);
    //    
}

//-----------------------------------------------------------------------------//
// MAIN
//-----------------------------------------------------------------------------//
int
main(int argc, char ** argv)
{
    // init random number generator
    srand(time(NULL));

    //start time
    
    //-------------Graphics Initialization--------//
    
    // initialize GLUT
    glutInit( &argc, argv );
    
    // initialize graphics
    initialize();
    
    // load sounds
    AudioFileSet newFileMgr;
    
    if (newFileMgr.loadFileSet(_audioPath) == 1)
    {
        goto cleanup;
    }
    
    _sounds = newFileMgr.getFileVector();
    cout << "Sounds loaded successfully..." << endl;
    
    // create visual representation of sounds    
    _soundViews = new vector<SoundRect *>;
    for (int i = 0; i < _sounds->size(); i++)
    {
        _soundViews->push_back(new SoundRect(_style, _winWidth, _winHeight));
        _soundViews->at(i)->associateSound(_sounds->at(i)->_wave,
                                           _sounds->at(i)->_frames,
                                           _sounds->at(i)->_channels);
    }
    
    // init grain cloud vector and corresponding view vector
    _grainCloud = new vector<GrainCluster *>;
    _grainCloudVis = new vector<GrainClusterVis *>;
    
    //-------------Audio Configuration-----------//
    
    // configure RtAudio
    // create the object
    try
    {
        _audio = new MyRtAudio(1,
                               CHANNELS,
                               SRATE,
                               &_buffSize,
                               FORMAT,
                               true);
    }
    catch (RtAudioError & err)
    {
        err.printMessage();
        exit(1);
    }    
    try
    {
        // open audio stream/assign callback
        _audio->openStream(&audioCallback);
        
        // get new buffer size
        _buffSize = _audio->getBufferSize();
        
        // start audio stream
        _audio->startStream();
        
        // report latency
        _audio->reportStreamLatency();        
        
    }
    catch(RtAudioError & err)
    {
        err.printMessage();
        goto cleanup;
    }
    
    // start graphics
    // let GLUT handle the current thread from here
    glutMainLoop();
    
    cout <<"Something went wrong...shouldn't be here" << endl;
    
    // cleanup routine
cleanup:
    cleaningFunction();
    
    // done
    return 0;
}
