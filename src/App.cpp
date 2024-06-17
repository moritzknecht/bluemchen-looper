#include "./Config.h"
#include "daisysp.h"
#include "AudioLooper.h"
#include "SteppedClock.hpp"
#include "rings/dsp/fx/reverb.h"
#include "CustomUI.h"
#include "./UiHardware.h"
#include "./Helper.h"

#ifdef BLUEMCHEN
#include "kxmx_bluemchen.h"
#else
#include "daisy_patch.h"
#endif

using namespace daisy;
using namespace daisysp;
using namespace rings;
using namespace audiolooper;

#ifdef BLUEMCHEN
using namespace kxmx;
Bluemchen hw;
#else
DaisyPatch hw;
#endif

#include "./Menus.h"
daisy::UI    ui;
UiEventQueue eventQueue;

// Allocate memory for the left and right audio buffers
float DSY_SDRAM_BSS buffer_l[kBuffSize];
float DSY_SDRAM_BSS buffer_r[kBuffSize];

// Allocate memory for the reverb buffer
uint16_t reverb_buffer[65536];

// Initialize control variables
bool shouldRecord  = false;
int  loopLength    = 64;
int  loopRecorded  = 0;
int  crossFaderPos = 0;
int  loopLengthPos = 3;

bool  preventClear = false;
float last_cv1     = 0.0f;
float last_cv2     = 0.0f;

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
/*
float mapControl(float x,
                 float in_min,
                 float in_max,
                 float out_min,
                 float out_max)
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
*/

void InitUi()
{
    UI::SpecialControlIds specialControlIds;
    specialControlIds.okBttnId
        = bttnEncoder; // Encoder button is our okay button
    specialControlIds.menuEncoderId
        = encoderMain; // Encoder is used as the main menu navigation encoder

    // This is the canvas for the OLED display.
    UiCanvasDescriptor oledDisplayDescriptor;
    oledDisplayDescriptor.id_     = canvasOledDisplay; // the unique ID
    oledDisplayDescriptor.handle_ = &hw.display;   // a pointer to the display
    oledDisplayDescriptor.updateRateMs_      = 50; // 50ms == 20Hz
    oledDisplayDescriptor.screenSaverTimeOut = 5 * 1000;
    // oledDisplayDescriptor.screenSaverOn  = true;
    oledDisplayDescriptor.clearFunction_ = &ClearCanvas;
    oledDisplayDescriptor.flushFunction_ = &FlushCanvas;

    ui.Init(eventQueue,
            specialControlIds,
            {oledDisplayDescriptor},
            canvasOledDisplay);
}


bool released = true;
void GenerateUiEvents()
{
    if(hw.encoder.TimeHeldMs() >= 1000 && released == true
       && !hw.encoder.Increment() && mainMenu.GetSelectedItemIdx() == 0)
    {
        released = false;
        eventQueue.AddButtonPressed(bttnEncoder, 1);
    }

    if(hw.encoder.RisingEdge() && hw.encoder.TimeHeldMs() <= 0
       && !hw.encoder.Increment() && mainMenu.GetSelectedItemIdx() != 0)
    {
        released = false;
        eventQueue.AddButtonPressed(bttnEncoder, 1);
    }

    if(hw.encoder.FallingEdge())
    {
        released = true;
        eventQueue.AddButtonReleased(bttnEncoder);
    }

    const auto increments = hw.encoder.Increment();
    if(increments != 0)
        eventQueue.AddEncoderTurned(encoderMain, increments, 12);
}

// Audio callback function for processing audio and control updates
void AudioCallback(AudioHandle::InputBuffer  in,
                   AudioHandle::OutputBuffer out,
                   size_t                    size)
{
    // Process the controls
    hw.ProcessAllControls();

    GenerateUiEvents();

#ifdef BLUEMCHEN
    float cv1_val = cv1.Process();
    bool  trig    = false;
    if(cv1_val >= TRIG_TRESHOLD && last_cv1 < TRIG_TRESHOLD)
    {
        trig = true;
    }
    else if(cv1_val < TRIG_TRESHOLD && last_cv1 >= TRIG_TRESHOLD)
    {
        // CV1 input has crossed the threshold downwards
        trig = false;
    }
    last_cv1 = cv1_val;

    float cv2_val = cv2.Process();
    bool  reset   = false;
    if(cv2_val >= TRIG_TRESHOLD && last_cv2 < TRIG_TRESHOLD)
    {
        // cv2 input has crossed the threshold upwards
        // Trigger your event here (e.g., toggling an LED)
        reset = true;
    }
    else if(cv2_val < TRIG_TRESHOLD && last_cv2 >= TRIG_TRESHOLD)
    {
        // cv2 input has crossed the threshold downwards
        reset = false;
    }
    last_cv2 = cv2_val;
#endif
#ifndef BLUEMCHEN

    bool trig  = hw.gate_input[0].Trig();
    bool reset = hw.gate_input[1].Trig();

#endif

    bool newStep = steppedClock.Process(trig, reset);
    loopView.SetCurrentStep((int)steppedClock.GetStep());

    float knob1Value = knob1.Process();
    float knob2Value = knob2.Process();

    float freq1 = mapControl(knob1Value, 0, 1, 20, 20000, "exp");
    float freq2 = mapControl(knob2Value, 0, 1, 20, 20000, "exp");
    svf_l.SetFreq(freq1);
    svf_r.SetFreq(freq1);
    svf_l.SetRes(fx1ResoValue.Get());
    svf_r.SetRes(fx1ResoValue.Get());

    reverb.set_amount(fx1RevAmountValue.Get());

    svf2_l.SetFreq(freq2);
    svf2_r.SetFreq(freq2);

    // Map control to loop length
    /* float len  = mapControl((float)loopLengthPos, 0.0, 5.0, 0.5, 1.0);
    loopLength = pow(2, (int)(len * 8));*/
    loopLength = loopLengths[barListValues.GetIndex()];
    loopView.SetLoopLength(loopLength);
    // Start or stop the looper based on the clock
    if(!steppedClock.GetRunning())
    {
        looper_l.Stop();
        looper_r.Stop();
    }

    if(steppedClock.GetRunning()
       && (looper_l.GetState() == AudioLooper::State::STOPPED
           || !steppedClock.GetClockRunning())
       && newStep && steppedClock.GetStep() == 0 && loopView.GetEditing())
    {
        looper_l.Start();
        looper_r.Start();
    }

    // Toggle recording state on encoder press
    if(hw.encoder.RisingEdge() && hw.encoder.TimeHeldMs() <= 0.f
       && mainMenu.GetSelectedItemIdx() == 0 && loopView.GetEditing())
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
            crossFaderPos = CROSSFADER_RESOLUTION;
            fadeLeft.SetPos(1.0);
            fadeRight.SetPos(1.0);
            loopView.SetCrossFaderPos(crossFaderPos);
        }
    }

    if(newStep && steppedClock.GetStep() == 0 && shouldRecord)
    {
        looper_l.TrigRecord();
        looper_r.TrigRecord();
        shouldRecord = false;
        loopRecorded = 0;
    }

    // Clear the loop if the button is held longer than CROSSFADER_RESOLUTION0 ms
    if(hw.encoder.TimeHeldMs() >= 300.f && hw.encoder.TimeHeldMs() < 1000
       && hw.encoder.Increment() && !preventClear
       && mainMenu.GetSelectedItemIdx() == 0 && loopView.GetEditing())
    {
        shouldRecord = false;
        looper_l.Stop();
        looper_r.Stop();
        looper_l.Clear();
        looper_r.Clear();

        crossFaderPos = 0;
        loopRecorded  = 0;
        fadeLeft.SetPos(0.0);
        fadeRight.SetPos(0.0);
        loopView.SetCrossFaderPos(crossFaderPos);
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
    /*
    if(hw.encoder.Increment() && hw.encoder.TimeHeldMs() <= 0
       && (looper_l.GetState() == AudioLooper::State::STOPPED
           || looper_l.GetState() == AudioLooper::State::EMPTY))
    {
        loopLengthPos += hw.encoder.Increment();
        loopLengthPos = (loopLengthPos % 6 + 6) % 6;
    }
    */
    if(hw.encoder.Increment() && hw.encoder.TimeHeldMs() <= 0
       && (looper_l.GetState() == AudioLooper::State::PLAYING
           || looper_l.GetState() == AudioLooper::State::REC_FIRST)
       && mainMenu.GetSelectedItemIdx() == 0 && loopView.GetEditing())
    {
        int newValue = crossFaderPos + hw.encoder.Increment();
        if(newValue > CROSSFADER_RESOLUTION)
        {
            newValue = CROSSFADER_RESOLUTION;
        }
        else if(newValue < 0)
        {
            newValue = 0;
        }

        crossFaderPos = newValue;
        fadeLeft.SetPos((float)crossFaderPos / CROSSFADER_RESOLUTION);
        fadeRight.SetPos((float)crossFaderPos / CROSSFADER_RESOLUTION);
        loopView.SetCrossFaderPos(crossFaderPos);
    }

    loopView.SetLoopRecorded(loopRecorded);
    /*
    reverb.set_amount(mapControl(cutoff, 20, 20000, 0, 1));
    reverb.set_diffusion(0.625f);
    reverb.set_time(0.35f + 0.63f * 0.2);
    reverb.set_input_gain(0.2f);
    reverb.set_lp(0.3f + 0.5 * 0.6f);
*/

    // Process audio signals

    //float loop[2][size];
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

        reverb.Process(&sig_l, &sig_r, 1);

        //loop[0][i] = sig_l;
        // loop[1][i] = sig_r;
        float fade_l = fadeLeft.Process(in_l, sig_l);
        float fade_r = fadeRight.Process(in_r, sig_r);

        float mid  = 0.5f * (fade_l + fade_r);
        float side = 0.5f * (fade_l - fade_r);

        side *= midsideValue.Get();

        out[0][i] = mid + side;
        out[1][i] = mid - side;
    }
}

void Init(float samplerate)
{
    reverb.Init(reverb_buffer);

    knob1.Init(hw.controls[hw.CTRL_1], 0, 1, Parameter::LINEAR);
    knob2.Init(hw.controls[hw.CTRL_2], 0, 1, Parameter::LINEAR);
    // knob2.Init(hw.controls[hw.CTRL_2], 0.0f, 2.0f, Parameter::LINEAR);

    steppedClock.SetAutoReset(false);
    cv1.Init(hw.controls[hw.CTRL_3], -5000.0f, 5000.0f, Parameter::EXPONENTIAL);
    cv2.Init(hw.controls[hw.CTRL_4], -5000.0f, 5000.0f, Parameter::EXPONENTIAL);

    looper_l.Init(buffer_l, kBuffSize);
    looper_r.Init(buffer_r, kBuffSize);

    looper_l.SetMode(audiolooper::AudioLooper::Mode::REPLACE);
    looper_r.SetMode(audiolooper::AudioLooper::Mode::REPLACE);
    looper_l.Clear();
    looper_r.Clear();
    looper_l.Stop();
    looper_r.Stop();

    svf_l.Init(samplerate);
    svf_r.Init(samplerate);
    svf2_l.Init(samplerate);
    svf2_r.Init(samplerate);

    svf_l.SetRes(0.0);
    svf_r.SetRes(0.0);
    svf2_l.SetRes(0.0);
    svf2_r.SetRes(0.0);
    svf2_l.SetFreq(20000);
    svf2_r.SetFreq(20000);

    fadeLeft.Init(CROSSFADE_CPOW);
    fadeRight.Init(CROSSFADE_CPOW);
}

// Main function to initialize hardware and start audio processing
int main(void)
{
    hw.Init();
    // hw.SetAudioSampleRate(daisy::SaiHandle::Config::SampleRate::SAI_48KHZ);
    hw.StartAdc();

    float samplerate = hw.AudioSampleRate();
    Init(samplerate);

    InitUi();
    InitUiPages();
    ui.OpenPage(mainMenu);
    hw.StartAudio(AudioCallback);

    while(1)
    {
        ui.Process();
    }
}