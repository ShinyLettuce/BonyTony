#pragma once

#include <cmath>

namespace MathUtils
{
    int Modulo(int aNum, int aMod);
    float Min(float aA, float aB);
    float Max(float aA, float aB);
    float Clamp(float aValue, float aMin, float aMax);
    float Clamp01(float aValue);

    float LerpClamped(float aStart, float aEnd, float aPercent);
    float LerpUnclamped(float aStart, float aEnd, float aPercent);

    float RandFloat(float aMin, float aMax);
    
    template<typename T>
    T Decay(T aCurrent, T aTarget, float aFactor, float aDeltaTime)
    {
        return aTarget + (aCurrent - aTarget) * std::exp(-aFactor * aDeltaTime);
    }

    float EaseInOutBack(float aT);

    void Spring(float& aPosition, float& aVelocity, float aDeltaTime, float aFrequency, float aDamping);
}