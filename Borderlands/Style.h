#ifndef STYLE_H
#define STYLE_H

// interface
class Style
{
public:
    Style() {}
    virtual ~Style() {}

    void getBackgroundColor(float col[4]);

    float getAxisLineWidth();
    void getAxisXColor(float col[4]);
    void getAxisYColor(float col[4]);
    void getAxisZColor(float col[4]);

    float getUsageLineWidth();
    void getUsageAColorCoeff(float colCoeff[4]);
    void getUsageInsColorCoeff(float colCoeff[4]);

    float getManualLineWidth();
    void getManualInsColor(float col[4]);

    void getParamColor(float col[4]);

    void getGrainColor(float col[4]);
    void getGrainColorSelected(float col[4]);

    void getGrainVisColor(float col[4]);
    void getGrainVisDefColor(float col[4]);
    
    bool getRandSoundRectColor();
    void getSoundRectColorCoeff(float col[4]);
    void getSoundRectColorSelected(float col[4]);
    float getSoundRectLineWidth();

    void getWaveColor(float col[4]);
    float getWaveLineWidth();
    float getWavePointSize();
    
protected:
    float _backgroundColor[4];

    float _axisLineWidth;
    float _axisXColor[4];
    float _axisYColor[4];
    float _axisZColor[4];

    float _usageLineWidth;
    float _usageAColCoeff[4];
    float _usageInsColCoeff[4];

    float _manualLineWidth;
    float _manualInsColor[4];

    float _paramColor[4];

    float _grainColor[4];
    float _grainColorSelected[4];

    float _grainVisColor[4];
    float _grainVisDefColor[4];
    
    bool _randSoundRectColor;
    float _soundRectColorCoeff[4];
    float _soundRectColorSelected[4];
    float _soundRectLineWidth;

    float _waveColor[4];
    float _waveLineWidth;
    float _wavePointSize;
};

// default style
class DefaultStyle : public Style
{
public:
    DefaultStyle();
    virtual ~DefaultStyle();
};

// purple style
class PurpleStyle : public Style
{
public:
    PurpleStyle();
    virtual ~PurpleStyle();
};

// psyche style
class PsycheStyle : public Style
{
public:
    PsycheStyle();
    virtual ~PsycheStyle();
};

#endif
