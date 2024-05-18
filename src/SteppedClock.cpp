#include "SteppedClock.hpp"


SteppedClock::SteppedClock()
{
    // srand(time(NULL));
}

void SteppedClock::SetSteps(int steps)
{
    maxSteps = steps;
}

void SteppedClock::SetAutoReset(bool reset)
{
    autoReset = reset;
}


bool SteppedClock::Process(bool gate, bool reset)
{
    uint32_t now = System::GetNow();
    if(lastClock == -1)
    {
        lastClock = now;
    }
    bool stopped = reset;
    if(autoReset)
    {
        stopped = reset || (now - lastClock > 500);
    }
    if(now - lastClock > 500)
    {
        seqStep      = -1;
        lastClock    = -1;
        clockRunning = false;
    }
    if(stopped)
    {
        seqStep   = -1;
        running   = false;
        lastClock = -1;
    }
    // advStep = false;
    advStep = gate;
    if(advStep)
    {
        lastClock    = System::GetNow();
        running      = true;
        clockRunning = true;
        seqStep++;
        if(seqStep >= maxSteps)
        {
            seqStep = 0;
        }
        advStep = false;
        return true;
    }
    return false;
}

long SteppedClock::GetStep()
{
    return seqStep;
}

bool SteppedClock::GetRunning()
{
    return running;
}

bool SteppedClock::GetClockRunning()
{
    return clockRunning;
}