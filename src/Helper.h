#pragma once
#include <stdint.h> // Include this for standard integer types if needed

// Function prototypes
double mapControl(double      input,
                  double      inMin,
                  double      inMax,
                  double      outMin,
                  double      outMax,
                  const char* scaleType = "linear");
double mapLinear(double input,
                 double inMin,
                 double inMax,
                 double outMin,
                 double outMax);
double
mapLog(double input, double inMin, double inMax, double outMin, double outMax);
double
mapExp(double input, double inMin, double inMax, double outMin, double outMax);

double mapControl(double      input,
                  double      inMin,
                  double      inMax,
                  double      outMin,
                  double      outMax,
                  const char* scaleType)
{
    if(scaleType[0] == 'l' || scaleType[0] == 'L')
    {
        return mapLinear(input, inMin, inMax, outMin, outMax);
    }
    else if(scaleType[0] == 'l' || scaleType[0] == 'L')
    {
        return mapLog(input, inMin, inMax, outMin, outMax);
    }
    else if(scaleType[0] == 'e' || scaleType[0] == 'E')
    {
        return mapExp(input, inMin, inMax, outMin, outMax);
    }
    else
    {
        // Default to linear if scale type is unrecognized
        return mapLinear(input, inMin, inMax, outMin, outMax);
    }
}

double mapLinear(double input,
                 double inMin,
                 double inMax,
                 double outMin,
                 double outMax)
{
    return ((input - inMin) * (outMax - outMin)) / (inMax - inMin) + outMin;
}

double
mapLog(double input, double inMin, double inMax, double outMin, double outMax)
{
    // Ensure positive values
    if(inMin <= 0 || inMax <= 0 || outMin <= 0 || outMax <= 0 || input <= 0)
    {
        return input; // Return input as is in case of invalid values
    }
    double logInMin           = 0.0;
    double logInMax           = log(inMax / inMin);
    double logOutMin          = log(outMin);
    double logOutMax          = log(outMax);
    double logInput           = log(input / inMin);
    double normalizedLogInput = (logInput - logInMin) / (logInMax - logInMin);
    return exp(normalizedLogInput * (logOutMax - logOutMin) + logOutMin)
           * outMin;
}

double
mapExp(double input, double inMin, double inMax, double outMin, double outMax)
{
    double normalizedInput = (input - inMin) / (inMax - inMin);
    return outMin * pow(outMax / outMin, normalizedInput);
}
