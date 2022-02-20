#include "Style.h"

void
Style::getBackgroundColor(float col[4])
{
    for (int i = 0; i < 4; i++)
        col[i] = _backgroundColor[i];
}

float
Style::getAxisLineWidth()
{
    return _axisLineWidth;
}

void
Style::getAxisXColor(float col[4])
{
    for (int i = 0; i < 4; i++)
        col[i] = _axisXColor[i];
}

void
Style::getAxisYColor(float col[4])
{
    for (int i = 0; i < 4; i++)
        col[i] = _axisYColor[i];
}

void
Style::getAxisZColor(float col[4])
{
    for (int i = 0; i < 4; i++)
        col[i] = _axisZColor[i];
}

float
Style::getUsageLineWidth()
{
    return _usageLineWidth;
}

void
Style::getUsageAColorCoeff(float colCoeff[4])
{
    for (int i = 0; i < 4; i++)
        colCoeff[i] = _usageAColCoeff[i];
}

void
Style::getUsageInsColorCoeff(float colCoeff[4])
{
    for (int i = 0; i < 4; i++)
        colCoeff[i] = _usageInsColCoeff[i];
}

float
Style::getManualLineWidth()
{
    return _manualLineWidth;
}

void
Style::getManualInsColor(float col[4])
{
    for (int i = 0; i < 4; i++)
        col[i] = _manualInsColor[i];
}

void
Style::getParamColor(float col[4])
{
    for (int i = 0; i < 4; i++)
        col[i] = _paramColor[i];
}

void
Style::getGrainColor(float col[4])
{
    for (int i = 0; i < 4; i++)
        col[i] = _grainColor[i];
}

void
Style::getGrainColorSelected(float col[4])
{
    for (int i = 0; i < 4; i++)
        col[i] = _grainColorSelected[i];
}

void
Style::getGrainVisColor(float col[4])
{
    for (int i = 0; i < 4; i++)
        col[i] = _grainVisColor[i];
}

void
Style::getGrainVisDefColor(float col[4])
{
    for (int i = 0; i < 4; i++)
        col[i] = _grainVisDefColor[i];
}

bool
Style::getRandSoundRectColor()
{
    return _randSoundRectColor;
}

void
Style::getSoundRectColorCoeff(float col[4])
{
    for (int i = 0; i < 4; i++)
        col[i] = _soundRectColorCoeff[i];
}

void
Style::getSoundRectColorSelected(float col[4])
{
    for (int i = 0; i < 4; i++)
        col[i] = _soundRectColorSelected[i];
}

float
Style::getSoundRectLineWidth()
{
    return _soundRectLineWidth;
}

void
Style::getWaveColor(float col[4])
{
    for (int i = 0; i < 4; i++)
        col[i] = _waveColor[i];
}

float
Style::getWaveLineWidth()
{
    return _waveLineWidth;
}

float
Style::getWavePointSize()
{
    return _wavePointSize;
}

// default stype
DefaultStyle::DefaultStyle()
{
    _backgroundColor[0] = 0.15;
    _backgroundColor[1] = 0.15;
    _backgroundColor[2] = 0.15;
    _backgroundColor[3] = 1.0;

    _axisLineWidth = 0.9;
    
    _axisXColor[0] = 1.0;
    _axisXColor[1] = 0.0;
    _axisXColor[2] = 0.0;
    _axisXColor[3] = 0.9;

    _axisYColor[0] = 0.0;
    _axisYColor[1] = 1.0;
    _axisYColor[2] = 0.0;
    _axisYColor[3] = 0.9;

    _axisZColor[0] = 0.0;
    _axisZColor[1] = 0.0;
    _axisZColor[2] = 1.0;
    _axisZColor[3] = 0.7;

    _usageLineWidth = 2.0;
    
    _usageAColCoeff[0] = 1.0;
    _usageAColCoeff[1] = 1.0;
    _usageAColCoeff[2] = 1.0;
    _usageAColCoeff[3] = 1.0;

    _usageInsColCoeff[0] = 1.0;
    _usageInsColCoeff[1] = 1.0;
    _usageInsColCoeff[2] = 1.0;
    _usageInsColCoeff[3] = 1.0;

    _manualLineWidth = 2.0;
    
    _manualInsColor[0] = 0.6;
    _manualInsColor[1] = 0.6;
    _manualInsColor[2] = 0.6;
    _manualInsColor[3] = 0.6;

    _paramColor[0] = 1.0;
    _paramColor[1] = 1.0;
    _paramColor[2] = 1.0;
    _paramColor[3] = 1.0;

    _grainColor[0] = 0.1;
    _grainColor[1] = 0.7;
    _grainColor[2] = 0.6;
    _grainColor[3] = 0.35;

    _grainColorSelected[0] = 0.0,
    _grainColorSelected[1] = 0.4;
    _grainColorSelected[2] = 0.7;
    _grainColorSelected[3] = 0.3;

    _grainVisColor[0] = 1.0;
    _grainVisColor[1] = 0.0; //1.0;
    _grainVisColor[2] = 0.0; //1.0;
    _grainVisColor[3] = 0.6;

    _grainVisDefColor[0] = 1.0;
    _grainVisDefColor[1] = 0.0; //1.0;
    _grainVisDefColor[2] = 0.0; //1.0;
    _grainVisDefColor[3] = 1.0;
        
    _randSoundRectColor = false;
    
    _soundRectColorCoeff[0] = 1.0;
    _soundRectColorCoeff[1] = 1.0;
    _soundRectColorCoeff[2] = 1.0;
    _soundRectColorCoeff[3] = 1.0;

    _soundRectColorSelected[0] = 0.1;
    _soundRectColorSelected[1] = 0.7;
    _soundRectColorSelected[2] = 0.6;
    _soundRectColorSelected[3] = 0.35;

    _soundRectLineWidth = 2.0;

    _waveColor[0] = 1.0;
    _waveColor[1] = 1.0;
    _waveColor[2] = 1.0;
    _waveColor[3] = 1.0;

    _waveLineWidth = 0.3;
    _wavePointSize = 1.0;
}

DefaultStyle::~DefaultStyle() {}

// purple stype
PurpleStyle::PurpleStyle()
{
    _backgroundColor[0] = 0.23;
    _backgroundColor[1] = 0.20;
    _backgroundColor[2] = 0.46;
    _backgroundColor[3] = 1.0;

    _axisLineWidth = 0.9;
    
    _axisXColor[0] = 1.0;
    _axisXColor[1] = 0.0;
    _axisXColor[2] = 0.0;
    _axisXColor[3] = 0.9;

    _axisYColor[0] = 0.0;
    _axisYColor[1] = 1.0;
    _axisYColor[2] = 0.0;
    _axisYColor[3] = 0.9;

    _axisZColor[0] = 0.0;
    _axisZColor[1] = 0.0;
    _axisZColor[2] = 1.0;
    _axisZColor[3] = 0.7;

    _usageLineWidth = 2.0;
    
    _usageAColCoeff[0] = 0.98;
    _usageAColCoeff[1] = 0.63;
    _usageAColCoeff[2] = 0.93;
    _usageAColCoeff[3] = 1.0;

    _usageInsColCoeff[0] = 0.98;
    _usageInsColCoeff[1] = 0.63;
    _usageInsColCoeff[2] = 0.92;
    _usageInsColCoeff[3] = 1.0;

    _manualLineWidth = 2.0;
    
    _manualInsColor[0] = 0.71;
    _manualInsColor[1] = 0.20;
    _manualInsColor[2] = 0.59;
    _manualInsColor[3] = 0.6;

    _paramColor[0] = 0.98;
    _paramColor[1] = 0.63;
    _paramColor[2] = 0.92;
    _paramColor[3] = 1.0;

    /*_grainColor[0] = 0.72;
      _grainColor[1] = 0.70;
      _grainColor[2] = 0.38;
      _grainColor[3] = 0.35;*/
    _grainColor[0] = 0.71;
    _grainColor[1] = 0.40;
    _grainColor[2] = 1.0;
    _grainColor[3] = 0.5;

    _grainColorSelected[0] = 0.71; //0.74,
    _grainColorSelected[1] = 0.40; //0.37;
    _grainColorSelected[2] = 1.0; //0.53;
    _grainColorSelected[3] = 0.9; //0.35;

    _grainVisColor[0] = 0.81; //1.0;
    _grainVisColor[1] = 0.0; //1.0;
    _grainVisColor[2] = 1.0;
    _grainVisColor[3] = 0.9; //0.6;

    _grainVisDefColor[0] = 0.81; //1.0;
    _grainVisDefColor[1] = 0.0; //1.0;
    _grainVisDefColor[2] = 1.0;
    _grainVisDefColor[3] = 1.0;
        
    _randSoundRectColor = false;
    
    _soundRectColorCoeff[0] = 1.0;
    _soundRectColorCoeff[1] = 0.60;
    _soundRectColorCoeff[2] = 1.0;
    _soundRectColorCoeff[3] = 1.0;

    _soundRectColorSelected[0] = 0.72;
    _soundRectColorSelected[1] = 0.70;
    _soundRectColorSelected[2] = 0.38;
    _soundRectColorSelected[3] = 0.35;

    _soundRectLineWidth = 3.0; //2.0;

    _waveColor[0] = 1.0;
    _waveColor[1] = 1.0;
    _waveColor[2] = 1.0;
    _waveColor[3] = 1.0;

    _waveLineWidth = 2.0; //1.0; //0.3;
    _wavePointSize = 1.0;
}

PurpleStyle::~PurpleStyle() {}

// psyche stype
PsycheStyle::PsycheStyle()
{
    _backgroundColor[0] = 0.0;
    _backgroundColor[1] = 1.0;
    _backgroundColor[2] = 1.0;
    _backgroundColor[3] = 1.0;

    _axisLineWidth = 0.9;
    
    _axisXColor[0] = 1.0;
    _axisXColor[1] = 0.0;
    _axisXColor[2] = 0.0;
    _axisXColor[3] = 0.9;

    _axisYColor[0] = 0.0;
    _axisYColor[1] = 1.0;
    _axisYColor[2] = 0.0;
    _axisYColor[3] = 0.9;

    _axisZColor[0] = 0.0;
    _axisZColor[1] = 0.0;
    _axisZColor[2] = 1.0;
    _axisZColor[3] = 0.7;

    _usageLineWidth = 2.0;
    
    _usageAColCoeff[0] = 0.0;
    _usageAColCoeff[1] = 0.0;
    _usageAColCoeff[2] = 0.0;
    _usageAColCoeff[3] = 1.0;

    _usageInsColCoeff[0] = 0.0;
    _usageInsColCoeff[1] = 0.0;
    _usageInsColCoeff[2] = 0.0;
    _usageInsColCoeff[3] = 1.0;

    _manualLineWidth = 2.0;
    
    _manualInsColor[0] = 0.0;
    _manualInsColor[1] = 0.0;
    _manualInsColor[2] = 0.0;
    _manualInsColor[3] = 0.6;

    _paramColor[0] = 1.0;
    _paramColor[1] = 1.0;
    _paramColor[2] = 1.0;
    _paramColor[3] = 1.0;

    _grainColor[0] = 1.0; //0.42;
    _grainColor[1] = 1.0; //0.0;
    _grainColor[2] = 1.0;
    _grainColor[3] = 0.5;

    _grainColorSelected[0] = 1.0; //0.42;
    _grainColorSelected[1] = 1.0; //0.0;
    _grainColorSelected[2] = 1.0;
    _grainColorSelected[3] = 0.9;

    _grainVisColor[0] = 0.42;
    _grainVisColor[1] = 0.34;
    _grainVisColor[2] = 1.0;
    _grainVisColor[3] = 0.9;

    _grainVisDefColor[0] = 0.42;
    _grainVisDefColor[1] = 0.34;
    _grainVisDefColor[2] = 1.0;
    _grainVisDefColor[3] = 1.0;
        
    _randSoundRectColor = false;
    
    _soundRectColorCoeff[0] = 1.0;
    _soundRectColorCoeff[1] = 0.0;
    _soundRectColorCoeff[2] = 1.0;
    _soundRectColorCoeff[3] = 1.0; //1.5;

    _soundRectColorSelected[0] = 1.0;
    _soundRectColorSelected[1] = 0.0;
    _soundRectColorSelected[2] = 1.0;
    _soundRectColorSelected[3] = 0.75;

    _soundRectLineWidth = 3.0;

    _waveColor[0] = 0.0; //1.0;
    _waveColor[1] = 0.0; //1.0;
    _waveColor[2] = 0.0; //1.0;
    _waveColor[3] = 1.0;

    //_waveLineWidth = 0.3;
    _waveLineWidth = 2.0;
    _wavePointSize = 1.0;
}

PsycheStyle::~PsycheStyle() {}
