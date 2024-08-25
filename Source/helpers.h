//
// Created by blaze on 25.08.2024.
//
#pragma once

#include <memory>
#include <algorithm>
#include <math.h>

inline double raw2dB(double raw)
{
    return 20.0*log10(raw);
}
inline double peakGainFor_Q(double Q)
{
    // --- no resonance at or below unity
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
