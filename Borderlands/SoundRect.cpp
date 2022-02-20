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
//  SoundRect.cpp
//  Borderlands
//
//  Created by Christopher Carlson on 11/30/11.
//


//TODO:  set and show name implementation

#include "Style.h"
#include "SoundRect.h"

SoundRect::~SoundRect()
{
    _buff = NULL;
}

// constructor 
SoundRect::SoundRect(Style *style,
                     unsigned int winWidth,
                     unsigned int winHeight)
{
    _style = style;
    
    _winWidth = winWidth;
    _winHeight = winHeight;
    
    // initializtion
    init();
    
    float xBorder = 100.0;
    float yBorder = 50.0;
    
    // translation coordinates
    _rX = xBorder + ((float)rand()/RAND_MAX) * (_winWidth - xBorder * 2.0);
    _rY = yBorder + ((float)rand()/RAND_MAX) * (_winHeight - yBorder * 2.0);
    
    // min w and h dim
    _minDim = 60.0f;
    
    // scale factor for randomization
    float scaleF = 0.5;
    
    // box corners
    setDimensions(_minDim + scaleF*((float)rand()/RAND_MAX)*_winWidth,
                   _minDim + scaleF*((float)rand()/RAND_MAX)*_winHeight);  
    
    // set _orientation
    if (randf() < 0.5)
        _orientation = true; //sideways
    else 
        _orientation = false;
    
    // waveform display upsampling
    setUps();
    
    // set color of rect
    randColor();
}

void
SoundRect::updateWinDimensions(unsigned int newWinWidth,
                                unsigned int newWinHeight)
{
    if ((_winWidth == newWinWidth) && 
        (_winHeight == newWinHeight))
        return;

    // update the rect pos and size if window size changed
    float wRatio = ((float)newWinWidth)/_winWidth;
    float hRatio = ((float)newWinHeight)/_winHeight;

    _rWidth *= wRatio;
    _rHeight *= hRatio;

    _rtop *= hRatio;
    _rbot *= hRatio;
    _rleft *= wRatio;
    _rright *= wRatio;

    _rX *= wRatio;
    _rY *= hRatio;
    
    // finally, update the window size
    _winWidth = newWinWidth;
    _winHeight = newWinHeight;
}

// other intialization code
void
SoundRect::init()
{    
    // selection state
    _isSelected = false;
    _buffMult = (double) 1.0 / globalAtten;
    
    // other params
    _showBuff = true;
    _pendingBuffState = true;
    
    // alpha pulsation stuff
    _aMin = 0.2f;
    _aMax = 0.3f;
    _aPhase = 2.0f*PI*((float)rand()/RAND_MAX);
    _lambda = 0.001;
    _pRate = ((float)rand()/RAND_MAX)*0.001;
    
    if (((float)rand()/RAND_MAX) > 0.5)
        _aTarg = _aMin;
    else
        _aTarg = _aMax;
    
    _buffAlphaMax = 0.75f;
    _buffAlpha = _buffAlphaMax;
    _buff = NULL;
    _buffFrames = 0;
    _buffChans = 0;
    _buffInc = 0;
    _lastX = 0;
    _lastY = 0;
    
    // get start time
    _startTime = GTime::instance()._sec;

    // set id
    //_id = ++boxId;
}

// set selection (highlight)
void
SoundRect::setSelectState(bool state)
{
    _isSelected = state;
}

// determine if mouse click is in selection range
bool SoundRect::select(float x, float y)
{    
    // check selection
    bool isInside = insideMe(x, y);
    // cout << "x " << x << "  y " << y << endl;
    //    cout << "_rleft " << _rleft << "_rright " << _rright <<
    // "_rtop " << _rtop << "_rbot " << _rbot  << endl;

    // if selected set selection position (center, top, right, bottom, left)
    
    // return selection state
    return isInside;
}

// process mouse motion during selection
void
SoundRect::move(float xDiff, float yDiff)
{
    _rX = _rX + xDiff;
    _rY = _rY + yDiff;
    updateCorners(_rWidth,_rHeight);
}

bool
SoundRect::getOrientation()
{
    return _orientation;
}

void
SoundRect::toggleOrientation()
{
    _orientation = !_orientation;
    setDimensions(_rHeight,_rWidth);
}

// color randomizer + alpha (roughly green/blue in color)
void
SoundRect::randColor()
{
    // color
    _colR = 0.4 + ((float)rand()/RAND_MAX)*0.3;

    float soundRectColorCoeff[4];
    _style->getSoundRectColorCoeff(soundRectColorCoeff);
        
    if (!_style->getRandSoundRectColor())
    {
        _colG = _colR;
        _colB = _colR;

        _colR *= soundRectColorCoeff[0];
        _colG *= soundRectColorCoeff[1];
        _colB *= soundRectColorCoeff[2];
    }
    else
    {
        _colG = 0.4 + ((float)rand()/RAND_MAX)*0.3;
        _colB = 0.4 + ((float)rand()/RAND_MAX)*0.3;
    }
    
    // _colG = 0.39f + ((float)rand()/RAND_MAX)*0.51f;
    // _colB = 0.27f + ((float)rand()/RAND_MAX)*0.63f;   
    _colA = _aMin + ((float)rand()/RAND_MAX) * 0.2f;
    // wPulse = 0.95 + ((float)rand()/RAND_MAX) *0.1;

    _colA *= soundRectColorCoeff[3];
}

// set width and height
void
SoundRect::setDimensions(float width, float height)
{    
    bool newWidth = false;
    bool newHeight = false;
    if (width >= _minDim)
    { 
        _rWidth = width;
        newWidth = true;
    }
    
    if (height >= _minDim)
    {
        _rHeight = height;
        newHeight = true;
    }
    
    if (newWidth || newHeight)
    {
        updateCorners(_rWidth, _rHeight);
        
        //update waveform params
        setUps();
        setWaveDisplayParams();
    }

    /*if ((width > 50.0f) && (width < (8.0f*_winWidth))){
        _rWidth = width;
        newSet = true;
    }
    if ( (height > 50.0f) && (width < (8.0f*_winHeight))){
        _rHeight = height;
        newSet = true;
    }
    
    if (newSet){
        updateCorners(_rWidth,_rHeight);
        
        //update waveform params
        setUps();
        setWaveDisplayParams();
    }
     */
}

// getters for width and height
float
SoundRect::getWidth()
{
    return _rWidth;
}

float
SoundRect::getHeight()
{
    return _rHeight;
}

// update box corners with new width values
void SoundRect::updateCorners(float width, float height)
{
    _rtop = _rY + height * 0.5f;
    _rbot = _rY - height * 0.5f;
    _rright = _rX + width * 0.5f;
    _rleft = _rX - width * 0.5f;
    //    cout << "Sound Rect " << _id << ": "
    //    << _rtop << ", " << _rright << ", " <<
    //    _rbot << ", " << _rleft << endl;
}

// set upsampling for waveform display
void
SoundRect::setUps()
{
    float sizeFactor = 10.0f;
    if (_orientation)
        _ups = (float) _winWidth/_rWidth;
    else
        _ups = (float) _winHeight/_rHeight;
    
    if (_ups < 1)
        _ups = 1;
}

////process mouse movement/selection
//void SoundRect::procMovement(int x, int y)
//{
//    for (int i = 0; i<5; i++){
//        if (pickedArray[i] == 1){
//            cout << "pickedArray[" << i <<"]: " << pickedArray[i]<<endl;
//            switch (i) {
//                case INSIDE:
//                    if (_lastX != (float)x){
//                        _rX += ((float)x - _lastX);
//                        updateCorners(_rWidth,_rHeight);
//                    }
//                    if (_lastY != (float)y){
//                        _rY -= ((float)y - _lastY);
//                        updateCorners(_rWidth,_rHeight);
//                    }
//                    //                    if (_lastY > 0){
//                    //                        _rHeight -= (y- _lastY);
//                    //                        updateCorners(_rWidth, _rHeight);
//                    //                    }
//                    //                    // m_top = (m_screenHeight - y) * m_screenScaleH;
//                    break;
//                case TOP:
//                    //m_bottom = (m_screenHeight - y) * m_screenScaleH;
//                    break;
//                case BOTTOM:
//                    //m_right = x * m_screenScaleW;
//                    break;
//                case LEFT:
//                    // m_left = x * m_screenScaleW;
//                    break;
//                case RIGHT:
//                    if (_lastX != (float)x){
//                        _rX += (float)x - _lastX;
//                        cout << "rx" << _rX << endl;
//                        updateCorners(_rWidth,_rHeight);
//                    }
//                    if (_lastY != (float)y){
//                        _rY -= ((float)y - _lastY);
//                        updateCorners(_rWidth,_rHeight);
//                    }
//                    break;
//                    
//                default:
//                    break;
//            }
//        }
//    }
//    
//    _lastX = (float)x;
//    _lastY = (float)y;
//}

void
SoundRect::associateSound(SAMPLE *buff,
                          unsigned long buffFrames,
                          unsigned int buffChans)
{    
    _buff = buff;
    _buffFrames = buffFrames;
    _buffChans = buffChans;
    //    if (_orientation)
    //        setDimensions((float)buffFrames/20000.f,_rHeight);
    //    else
    //        setDimensions(_rWidth,(float)buffFrames/30000.f);
    
    setWaveDisplayParams();
}

void
SoundRect::setWaveDisplayParams()
{
    if (_orientation)
        _buffInc = _buffFrames/(_ups*_rWidth);    
    else
        _buffInc = _buffFrames/(_ups*_rHeight);
}

// draw function
void
SoundRect::draw()
{
    glPushMatrix();
    //rect properties
    //    
    //    if (_colA > (_aMax - 0.005)){
    //        _aTarg = _aMin;
    //        _lambda = 0.999+_pRate;
    //    }else if (_colA < (_aMin + 0.005))
    //    {
    //        _aTarg = _aMax;
    //        _lambda = 0.999;
    //    }
    //    
    //_colA = 0.18f + 0.1f*sin(0.5f*PI*GTime::instance().sec + _aPhase);
    
    //draw rectangle
    glColor4f(_colR, _colG, _colB, _colA);
    
    //    double t = (GTime::instance().sec-_startTime);
    //    _rX += 0.1*sin(2*PI*0.12*t);
    //    _rY += 0.5*sin(2*PI*0.1*t);
    //    updateCorners(_rWidth,_rHeight);
    //    
    glBegin(GL_QUADS);
    glVertex3f(_rleft, _rtop, 0.0f);
    glVertex3f(_rright,  _rtop, 0.0f);
    glVertex3f(_rright, _rbot, 0.0f);
    glVertex3f(_rleft, _rbot, 0.0f);
    glEnd();
    
    if (_isSelected)
    {
        float soundRectColorSelected[4];
        _style->getSoundRectColorSelected(soundRectColorSelected);
        glColor4f(soundRectColorSelected[0], soundRectColorSelected[1],
                  soundRectColorSelected[2], soundRectColorSelected[3]);

        float lineWidth = _style->getSoundRectLineWidth();
        glLineWidth(lineWidth);
        
        glBegin(GL_LINE_STRIP);
        glVertex3f(_rleft, _rtop, 0.0f);
        glVertex3f(_rright, _rtop, 0.0f);
        glVertex3f(_rright, _rbot, 0.0f);
        glVertex3f(_rleft, _rbot, 0.0f);
        glVertex3f(_rleft, _rtop, 0.0f);
        glEnd();
    }
    
    // draw audio buffer
    if ((_buff) && (_showBuff || _pendingBuffState))
    {
        // fade in out waveform
        if (!_pendingBuffState)
        {
            _buffAlpha = 0.996*_buffAlpha;
            if (_buffAlpha < 0.001)
                _showBuff = false;
        }
        else
            _buffAlpha = 0.996*_buffAlpha + 0.004*_buffAlphaMax;
        
        
        // _buffMult = (_buffAlpha + 0.35)/globalAtten;;

        float lineWidth = _style->getWaveLineWidth();
        glLineWidth(lineWidth);
        
        float waveColor[4];
        _style->getWaveColor(waveColor);
        glColor4f(waveColor[0], waveColor[1],
                  waveColor[2], _colA*_buffAlpha*waveColor[3]);

        float pointSize = _style->getWavePointSize();
        glPointSize(pointSize);
        
        switch (_buffChans)
        {
            case 1:
                glBegin(GL_LINE_STRIP);
                if (_orientation)
                {    
                    for (int i = 0; i < _rWidth*_ups; i++)
                    {
                        float nextI = (float) i/_ups;
                        
                        glVertex3f((_rleft + nextI),
                                   _rY + 0.5f*_rHeight*
                                   (_buff[(int)floor(i*_buffInc)]),
                                   0.0f);
                    }
                }
                else
                {
                    for (int i  = 0; i < _rHeight*_ups;i++)
                    {
                        float nextI = (float) i/_ups;
                        
                        glVertex3f(_rX + 0.5f*_rWidth*
                                   (_buff[(int)floor(i*_buffInc)]),
                                   (_rbot + nextI),
                                   0.0f);
                    }
                }
                glEnd();
                break;
                
            case 2:     
                // left channel
                if (_orientation)
                {
                    glBegin(GL_LINE_STRIP);
                    for (int i  = 0; i < _rWidth*_ups;i++)
                    {
                        float nextI = (float) i/_ups;
                        // glVertex3f((_rleft + nextI),_rY + _rHeight*(0.24f * _buff[2*(int)floor(i*_buffInc)]*_buffMult+0.1f),0.0f);
                        glVertex3f((_rleft + nextI),
                                   _rY + 0.25*_rHeight + 0.25f*_rHeight*
                                   (_buff[2*(int)floor(i*_buffInc)]),
                                   0.0f);
                    }
                    
                    glEnd();
                    
                    // right channel
                    glBegin(GL_LINE_STRIP);
                    
                    for (int i = 0; i < _rWidth*_ups; i++)
                    {
                        float nextI = (float) i/_ups;
                        
                        glVertex3f((_rleft + nextI),
                                   _rY - 0.25*_rHeight + 0.25f*_rHeight*
                                   (_buff[2*(int)floor(i*_buffInc)+1]),
                                   0.0f);
                    }
                    glEnd();
                }
                else
                {
                    glBegin(GL_LINE_STRIP);
                    for (int i  = 0; i < _rHeight*_ups; i++)
                    {
                        float nextI = (float) i/_ups;
                        
                        // glVertex3f(_rX + _rWidth*(0.24f * _buff[2*(int)floor(i*_buffInc)]*_buffMult+0.1f),(_rbot + nextI),0.0f);
                        glVertex3f(_rX + 0.25*_rWidth + 0.25f*_rWidth*
                                   (_buff[2*(int)floor(i*_buffInc)]),
                                   (_rbot + nextI),
                                   0.0f);
                    }
                    
                    glEnd();
                    
                    // right channel
                    glBegin(GL_LINE_STRIP);
                    
                    for (int i  = 0; i < _rHeight*_ups; i++)
                    {
                        float nextI = (float) i/_ups;
                        
                        // glVertex3f(_rX + _rWidth*(0.24f * _buff[2*(int)floor(i*_buffInc)+1]-0.1f),(_rbot + nextI),0.0f);
                         glVertex3f(_rX - 0.25*_rWidth + 0.25f*_rWidth*
                                    (_buff[2*(int)floor(i*_buffInc)+1]),
                                    (_rbot + nextI),
                                    0.0f);
                    }
                    glEnd();
                }
                
            default:
                break;
        }
    }
    glPopMatrix();
}

void
SoundRect::toggleWaveDisplay()
{
    _pendingBuffState = !_pendingBuffState;
    if (_pendingBuffState)
        _showBuff = true;
}

//return id
//unsigned int SoundRect::getId()
//{
//    return _id;
//}


// check to see if a coordinate is inside this rectangle
bool
SoundRect::insideMe(float x, float y)
{
    if ((x > _rleft) && (x < _rright))
    {
        if ((y > _rbot) && (y < _rtop))
            return true;
    }
    return false;
}

// return normalized position values in x and y
bool
SoundRect::getNormedPosition(double *positionsX,
                             double *positionsY,
                             float x, float y,unsigned int idx)
{
    bool trigger = false;
    // cout << "grainX:  " << x << " grainY: " << y << " _rleft " << _rleft << " _rright" << _rright << " _rtop " << _rtop << " _rbottom " << _rbot <<  endl;
    if (insideMe(x,y))
    {
        trigger = true;
        if (_orientation)
        {
            positionsX[idx] = (double)((x-_rleft) / _rWidth);
            positionsY[idx] = (double)((y-_rbot) / _rHeight);
        }
        else
        {
            positionsY[idx] = (double)((x-_rleft) / _rWidth);
            positionsX[idx] = (double)((y-_rbot) / _rHeight);
        }
        // cout << positionsX[idx] << ", " << positionsY[idx]<<endl;
        if ((positionsY[idx] < 0.0)|| (positionsY[idx] > 1.0))
            cout <<
                "problem with x trigger pos - see soundrect get normed pos" << endl;
        if ((positionsY[idx] < 0.0)|| (positionsY[idx] > 1.0))
            cout <<
                "problem with x trigger pos - see soundrect get normed pos" << endl;
    }
    
    return trigger;
}

// set name
void
SoundRect::setName(char *name) {}
