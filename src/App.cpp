#include "daisysp.h"
#include "../bluemchen/kxmx_bluemchen.h"
#include <string.h>
#include "AudioLooper.h"
#include "SteppedClock.hpp"
#include "../dsp/fx/reverb.h"

using namespace kxmx;
using namespace daisy;
using namespace daisysp;
using namespace torus;
using namespace audiolooper;

Bluemchen hw;

// Define the audio buffer size to accommodate 60 seconds of audio at 48kHz sampling rate
#define kBuffSize 48000 * 60 // 60 seconds at 48kHz

// Allocate memory for the left and right audio buffers
float DSY_SDRAM_BSS buffer_l[kBuffSize];
float DSY_SDRAM_BSS buffer_r[kBuffSize];

// Allocate memory for the reverb buffer
uint16_t reverb_buffer[32768];

// Initialize control variables
bool shouldRecord  = false;
int  loopLength    = 16;
int  loopRecorded  = 0;
int  crossFaderPos = 0;
int  loopLengthPos = 0;
int  channel       = 0;
bool preventClear  = false;

// Instantiate DSP components and parameters
SteppedClock steppedClock;
Reverb       reverb;
AudioLooper  looper_l;
AudioLooper  looper_r;
Svf          svf_l;
Svf          svf_r;
Svf          svf2_l;
Svf          svf2_r;
Parameter    knob1;
Parameter    knob2;
CrossFade    fadeLeft;
CrossFade    fadeRight;
Parameter    cv1;
Parameter    cv2;

// Map a control value from one range to another
float mapControl(float x,
                 float in_min,
                 float in_max,
                 float out_min,
                 float out_max)
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

// Update the OLED display with current information
void UpdateOled()
{
    hw.display.Fill(false);

    hw.display.SetCursor(0, 0);
    std::string str  = "seqStep  ";
    char       *cstr = &str[0];
    sprintf(cstr, "seqStep %ld", steppedClock.GetStep());
    hw.display.WriteString(cstr, Font_6x8, true);

    hw.display.SetCursor(0, 24);
    std::string str3  = "stepsRecorded  ";
    char       *cstr3 = &str3[0];
    sprintf(cstr3, "stepsRecorded %d", loopRecorded);
    hw.display.WriteString(cstr3, Font_6x8, true);

    hw.display.SetCursor(0, 36);
    std::string str4  = "play  ";
    char       *cstr4 = &str4[0];
    sprintf(cstr4, "state %d", (int)looper_l.GetState());
    hw.display.WriteString(cstr4, Font_6x8, true);

    hw.display.SetCursor(0, 48);
    std::string str5  = "bars  ";
    char       *cstr5 = &str5[0];
    sprintf(cstr5, "bars %d", (int)(loopLength / 16));
    hw.display.WriteString(cstr5, Font_6x8, true);

    hw.display.Update();
}

// Update the controls by processing the inputs
void UpdateControls()
{
    hw.ProcessAllControls();
    knob1.Process();
    knob2.Process();
    cv1.Process();
    cv2.Process();
}

// Audio callback function for processing audio and control updates
void AudioCallback(AudioHandle::InputBuffer  in,
                   AudioHandle::OutputBuffer out,
                   size_t                    size)
{
    // Process the controls
    hw.ProcessAllControls();
    bool  clock   = cv1.Process() > 0.2;
    bool  reset   = cv2.Process() > 0.2;
    bool  newStep = steppedClock.Process(clock, reset);
    float cutoff  = knob1.Process();
    float cutoff2 = knob2.Process();

    svf_l.SetFreq(cutoff);
    svf_r.SetFreq(cutoff);
    svf2_l.SetFreq(cutoff2);
    svf2_r.SetFreq(cutoff2);

    // Map control to loop length
    float len  = mapControl((float)loopLengthPos, 0.0, 5.0, 0.5, 1.0);
    loopLength = pow(2, (int)(len * 8));

    // Start or stop the looper based on the clock
    if(!steppedClock.GetRunning())
    {
        looper_l.Stop();
        looper_r.Stop();
    }

    if(steppedClock.GetRunning()
       && (looper_l.GetState() == AudioLooper::State::STOPPED
           || !steppedClock.GetClockRunning())
       && newStep && steppedClock.GetStep() == 0)
    {
        looper_l.Start();
        looper_r.Start();
    }

    // Toggle recording state on encoder press
    if(hw.encoder.FallingEdge())
    {
        shouldRecord = true;
    }

    // Manage loop recording and stopping
    if(newStep && looper_l.Recording())
    {
        loopRecorded++;
        if(loopRecorded >= loopLength)
        {
            looper_l.TrigRecord();
            looper_r.TrigRecord();
            fadeLeft.SetPos(1.0);
            fadeRight.SetPos(1.0);
        }
    }

    if(newStep && steppedClock.GetStep() == 0 && shouldRecord)
    {
        looper_l.TrigRecord();
        looper_r.TrigRecord();
        shouldRecord = false;
        loopRecorded = 0;
    }

    // Clear the loop if the button is held longer than 1000 ms
    if(hw.encoder.TimeHeldMs() >= 1000.f && !hw.encoder.Increment()
       && !preventClear)
    {
        looper_l.Clear();
        looper_r.Clear();
        fadeLeft.SetPos(0.0);
        fadeRight.SetPos(0.0);
    }

    if(hw.encoder.TimeHeldMs() >= 300)
    {
        shouldRecord = false;
    }

    /*
    if(hw.encoder.TimeHeldMs() >= 200.f && hw.encoder.Increment())
    {
        preventClear = true;
        channel += hw.encoder.Increment();
        channel = (channel % 3 + 3) % 3;
    }
    */

    if(preventClear && hw.encoder.TimeHeldMs() <= 0.f)
    {
        preventClear = false;
    }

    if(hw.encoder.Increment() && hw.encoder.TimeHeldMs() <= 0
       && looper_l.GetState() == AudioLooper::State::STOPPED)
    {
        loopLengthPos += hw.encoder.Increment();
        loopLengthPos = (loopLengthPos % 6 + 6) % 6;
    }
    if(hw.encoder.Increment() && hw.encoder.TimeHeldMs() <= 0
       && looper_l.GetState() != AudioLooper::State::STOPPED)
    {
    }

    reverb.set_amount(mapControl(cutoff, 20, 20000, 0, 1));
    reverb.set_diffusion(0.625f);
    reverb.set_time(0.35f + 0.63f * 0.2);
    reverb.set_input_gain(0.2f);
    reverb.set_lp(0.3f + 0.5 * 0.6f);

    // Process audio signals
    for(size_t i = 0; i < size; i++)
    {
        float in_l = in[0][i];
        float in_r = in[1][i];

        float sig_l = looper_l.Process(in_l);
        float sig_r = looper_r.Process(in_r);

        svf_l.Process(sig_l);
        svf_r.Process(sig_r);
        svf2_l.Process(svf_l.High());
        svf2_r.Process(svf_r.High());

        sig_l = svf2_l.Low();
        sig_r = svf2_r.Low();

        // reverb.Process(&sig_l, &sig_r, size);

        out[0][i] = fadeLeft.Process(in_l, sig_l);
        out[1][i] = fadeRight.Process(in_r, sig_r);
    }
}

// Main function to initialize hardware and start audio processing
int main(void)
{
    hw.Init();
    hw.StartAdc();
    reverb.Init(reverb_buffer);
    knob1.Init(hw.controls[1], 20, 20000, Parameter::LOGARITHMIC);
    knob2.Init(hw.controls[2], 20, 20000, Parameter::LOGARITHMIC);
    steppedClock.SetAutoReset(true);
    cv1.Init(hw.controls[hw.CTRL_3], 0.001f, 1.0f, Parameter::LINEAR);
    cv2.Init(hw.controls[hw.CTRL_4], 0.001f, 1.0f, Parameter::LINEAR);

    float samplerate = hw.AudioSampleRate();
    looper_l.Init(buffer_l, kBuffSize);
    looper_r.Init(buffer_r, kBuffSize);
    looper_l.Clear();
    looper_r.Clear();
    svf_l.Init(samplerate);
    svf_r.Init(samplerate);
    svf2_l.Init(samplerate);
    svf2_r.Init(samplerate);

    svf_l.SetRes(0.0);
    svf_r.SetRes(0.0);
    svf2_l.SetRes(0.0);
    svf2_r.SetRes(0.0);

    fadeLeft.Init(CROSSFADE_CPOW);
    fadeRight.Init(CROSSFADE_CPOW);

    hw.StartAudio(AudioCallback);
}