#pragma once

#include "IPlug_include_in_plug_hdr.h"

const int kNumPresets = 1;

enum EParams
{
    kMicDist,
    kAngle,
    kDepth,
    kFreq,
    kNumParams
};

using namespace iplug;
using namespace igraphics;

class AmpMod final : public Plugin
{
public:
  AmpMod(const InstanceInfo& info);

#if IPLUG_DSP // http://bit.ly/2S64BDd
public:
    ~AmpMod();
    void ProcessBlock(sample** inputs, sample** outputs, int nFrames) override;
    void OnReset() override;
//    void OnParamChange(int paramIdx) override;
    void getBufferSize();
    void initBuffer();
    
private:
    double mDepth = 0.;
    double mFreq = 0.;
    
    double mDelaySam1 = 0.;
    double mDelaySam2 = 0.;
    
    sample* mpBuffer1 = NULL;
    sample* mpBuffer2 = NULL;
    
    int mReadIndex1 = 0;
    int mReadIndex2 = 0;
    
    int mBufferSize1 = 0;
    int mBufferSize2 = 0;
#endif
};
