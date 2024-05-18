#include <time.h>
#include "daisy_patch.h"

using namespace daisy;

class SteppedClock
{
  public:
    SteppedClock();
    bool Process(bool clock, bool reset);
    long GetStep();
    bool GetRunning();
    void SetSteps(int steps);
    void SetAutoReset(bool reset);
    bool GetClockRunning();

  private:
    bool running      = false;
    bool advStep      = false;
    long seqStep      = -1;
    int  maxSteps     = 16;
    bool autoReset    = false;
    int  lastClock    = -1;
    bool clockRunning = false;
};