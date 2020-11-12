#include "AmpMod.h"
#include "IPlug_include_in_plug_src.h"
#include "IControls.h"
#include <math.h>

AmpMod::AmpMod(const InstanceInfo& info)
: Plugin(info, MakeConfig(kNumParams, kNumPresets))
{
    GetParam(kDepth)->InitDouble("Depth", 50., 0., 100., 0.01, "%");
    GetParam(kFreq)->InitFrequency("Frequency", 1., 1., 25., 0.1);
    GetParam(kProb)->InitDouble("Tri-Sq", 50., 0., 100., 0.1, "%");

#if IPLUG_EDITOR // http://bit.ly/2S64BDd
    mMakeGraphicsFunc = [&]() {
        return MakeGraphics(*this, PLUG_WIDTH, PLUG_HEIGHT, PLUG_FPS, GetScaleForScreen(PLUG_HEIGHT));
    };
    
    mLayoutFunc = [&](IGraphics* pGraphics) {
        pGraphics->AttachCornerResizer(EUIResizerMode::Scale, false);
        pGraphics->AttachPanelBackground(COLOR_WHITE);
        pGraphics->LoadFont("Roboto-Regular", ROBOTO_FN);
        const IRECT b = pGraphics->GetBounds();
        pGraphics->AttachControl(new ITextControl(b.GetMidVPadded(50).GetVShifted(-60), "Amplitude Modulation", IText(30)));
        
        pGraphics->AttachControl(new IVKnobControl(b.GetCentredInside(100).GetVShifted(20).GetHShifted(0), kDepth));
        pGraphics->AttachControl(new IVKnobControl(b.GetCentredInside(100).GetVShifted(20).GetHShifted(-90), kFreq));
        pGraphics->AttachControl(new IVKnobControl(b.GetCentredInside(100).GetVShifted(20).GetHShifted(90), kProb));
  };
#endif
}

#if IPLUG_DSP
void AmpMod::ProcessBlock(sample** inputs, sample** outputs, int nFrames)
{
    sample* in1 = inputs[0];
    sample* in2 = inputs[1];
    sample* out1 = outputs[0];
    sample* out2 = outputs[1];
    
    mDepth = GetParam(kDepth)->Value();
    double depth = (100. + mDepth) / 100.;
    
    for (int s = 0; s < nFrames; ++s, ++in1, ++in2, ++out1, ++out2)
    {
        getAmp();
        
        // Multiply buffer coefficients with output
        *out1 = (depth * mAmp1) * *in1;
        *out2 = (depth * mAmp2) * *in2;

        //increment the read index, wrapping if it goes out of bounds.
        mReadIndex1++;
        if(mReadIndex1 >= mBufferSize1) {
            mReadIndex1 = 0;
        }
        mReadIndex2++;
        if(mReadIndex2 >= mBufferSize2) {
            mReadIndex2 = 0;
        }
    }
}


void AmpMod::OnReset() {
    TRACE;
    
    mReadIndex1 = 0;
    mReadIndex2 = 0;
    mFreq = 1.;
    getAmp();
}

void AmpMod::initPos() {
    mFreq = GetParam(kFreq)->Value();
    mFreq = floor(mFreq * 10 + 0.5)/10;
    
    // Find the global position in the project
    int pos1 = (int) GetSamplePos();
    int pos2 = (int) GetSamplePos();
    
    // Compute Period
    mBufferSize1 = floor(GetSampleRate() / mFreq);
    mBufferSize2 = floor(GetSampleRate() / mFreq);
    
    // Get position with respect to the modulation wave
    if (mBufferSize1 != 0 && mBufferSize2 != 0) {
        mReadIndex1 = pos1 % mBufferSize1;
        mReadIndex2 = pos2 % mBufferSize2;
    }
    else {
        mReadIndex1 = 0;
        mReadIndex2 = 0;
    }
}

void AmpMod::getAmp() {
    mProb = GetParam(kProb)->Value()/100;
    
    // Get triangle and square wave amplitude
    if (mReadIndex1 < floor(mBufferSize1/2)) {
        mAmp1 = mProb*abs((2/M_PI)*asin(sin(M_PI*mFreq*mReadIndex1/GetSampleRate()))) + (1-mProb);
        mAmp2 = mProb*abs((2/M_PI)*asin(sin(M_PI*mFreq*mReadIndex2/GetSampleRate()))) + (1-mProb);
    }
    else {
      mAmp1 = mProb*abs((2/M_PI)*asin(sin(M_PI*mFreq*mReadIndex1/GetSampleRate())));
      mAmp2 = mProb*abs((2/M_PI)*asin(sin(M_PI*mFreq*mReadIndex2/GetSampleRate())));
    }
}

void AmpMod::OnParamChange(int paramIdx) {
    mFreq = 1.;
    initPos();
    getAmp();
}
#endif
