//
// Created by blaze on 10.08.2024.
//
#pragma once

#include <complex>
class softClipper{
public:
    softClipper()
    = default;
    ~softClipper()=default;

    [[nodiscard]] float process(const float& x)
    {
        if(x<-1)
            y = -(2.f/3.f);
        if(x>=-1 && x<=1)
            y = x -static_cast<float> ((std::pow(x,3)/3.f));
        if(x>=1)
            y = (2.f/3.f);
        return y;
    }
private:
    float y{0.f};
};