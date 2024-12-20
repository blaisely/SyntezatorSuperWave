//
// Created by blaze on 25.08.2024.
//
#pragma once

#include <memory>
#include <algorithm>
#include <math.h>
#define MOD 1000000007
inline double raw2dB(double raw)
{
    return 20.0*log10(raw);
}
inline double peakGainFor_Q(double Q)
{
    if (Q <= 0.707) return 1.0;
    return (Q*Q) / (pow((Q*Q - 0.25), 0.5));
}
inline double dBPeakGainFor_Q(double Q)
{
    return raw2dB(peakGainFor_Q(Q));
}
inline double dB2Raw(double dB)
{
    return pow(10.0, (dB / 20.0));
}
inline double sgn(double xn)
{
    return (xn > 0) - (xn < 0);
}
inline double softClipWaveShaper(double xn, double saturation)
{
    return sgn(xn)*(1.0 - exp(-fabs(saturation*xn)));
}
inline double hyperbolicTangent(double xn, double saturation)
{
    return std::tanh(saturation*xn)/std::tanh(saturation);
}
inline double arctangentSaturation(double x, double drive)
{
    drive = std::max(0.0, drive);
    double adjustedDrive = drive / 10.0; // Scale drive for smoother control

    // Apply saturation with dynamic scaling
    double saturatedSignal = atan(pow(adjustedDrive, 1.2) * x) / atan(adjustedDrive);

    // Blend with normalization based on the drive level
    double blend = (1.0 - adjustedDrive) * x + adjustedDrive * saturatedSignal;

    // Apply normalization to avoid loudness increase
    double normalizationFactor = 1.0 / (1.0 + (adjustedDrive*2));

    return blend * normalizationFactor;
}
inline float softClip (const float x)
{
    float y=0;
    if(x<-1)
        y = -(2.f/3.f);
    if(x>=-1 && x<=1)
        y = x -static_cast<float> ((std::pow(x,3)/3.f));
    if(x>=1)
        y = (2.f/3.f);
    return y;
}
static float fast_power(float base, int power) {
    float result = 1.0f;
    while (power > 0) {

        if (power % 2 == 1) {
            result = std::fmod((result * base), MOD);
        }
        base = std::fmod((base * base), MOD);
        power = power / 2;
    }
    return result;
}
inline float roundTwoDec(float var)
{
    // 37.66666 * 100 =3766.66
    // 3766.66 + .5 =3767.16    for rounding off value
    // then type cast to int so value is 3767
    // then divided by 100 so the value converted into 37.67
    float value = (int)(var * 100 + .5);
    return (float)value / 100;
}
