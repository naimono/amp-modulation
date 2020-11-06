#include "AmpMod.h"
#include "IPlug_include_in_plug_src.h"
#include "IControls.h"
#include <math.h>

AmpMod::AmpMod(const InstanceInfo& info)
: Plugin(info, MakeConfig(kNumParams, kNumPresets))
{
    GetParam(kDepth)->InitDouble("Depth", 50., 0., 100., 0.01, "%");
    GetParam(kFreq)->InitDouble("Frequency", 1., 1., 25., 0.1, "Hz");
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
AmpMod::~AmpMod() {
    // Delete buffer if already there
    if(mpBuffer1) {
        delete [] mpBuffer1;
    }
    if(mpBuffer2) {
        delete [] mpBuffer2;
    }
}

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
        // Read Delayed output
        sample yn1 = mpBuffer1[mReadIndex1];
        sample yn2 = mpBuffer1[mReadIndex2];
        
        // Multiply buffer coefficients with output
        *out1 = (depth * yn1) * *in1;
        *out2 = (depth * yn2) * *in2;
        
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

void AmpMod::initBuffer() {
    // Initialize buffers with coefficients
    
    if(mpBuffer1) {
        for (int i = 0; i < mBufferSize1; i++) {
            mpBuffer1[i] = abs((2/M_PI)*asin(sin(M_PI*mFreq*i)));
        }
    }
    mReadIndex1 = 0;
    
    if(mpBuffer2) {
        for (int i = 0; i < mBufferSize2; i++) {
            mpBuffer2[i] = abs((2/M_PI)*asin(sin(M_PI*mFreq*i)));
        }
    }
    mReadIndex2 = 0;
}

void AmpMod::OnReset() {
    TRACE;
    
    // Set Buffer Size
    getBufferSize();
    
    // Delete buffer if already there
    if(mpBuffer1) {
        delete [] mpBuffer1;
    }
    if(mpBuffer2) {
        delete [] mpBuffer2;
    }
    
    // Create new buffer in heap
    mpBuffer1 = new double[mBufferSize1];
    mpBuffer2 = new double[mBufferSize2];
    
    initBuffer();
}

void AmpMod::getBufferSize() {
    // Get Frequency from GUI
    mFreq = GetParam(kFreq)->Value();
    
    // Compute Period
    mBufferSize1 = floor(GetSampleRate() / mFreq);
    mBufferSize2 = floor(GetSampleRate() / mFreq);
}

void AmpMod::OnParamChange(int paramIdx) {
    // Set Buffer Size
    getBufferSize();

    // Delete buffer if already there
    if(mpBuffer1) {
        delete [] mpBuffer1;
    }
    if(mpBuffer2) {
        delete [] mpBuffer2;
    }

    // Create new buffer in heap
    mpBuffer1 = new double[mBufferSize1];
    mpBuffer2 = new double[mBufferSize2];

    initBuffer();
}
#endif
