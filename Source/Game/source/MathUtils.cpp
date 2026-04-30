#include "MathUtils.h"

#include <cmath>
#include <random>

int MathUtils::Modulo(int aNum, int aMod)
{
    return (aNum % aMod + aMod) % aMod;
}

float MathUtils::Min(float aA, float aB)
{
    return aA < aB ? aA : aB;
}

float MathUtils::Max(float aA, float aB)
{
    return aA > aB ? aA : aB;
}

float MathUtils::Clamp(float aValue, float aMin, float aMax)
{
    return MathUtils::Min(MathUtils::Max(aValue, aMin), aMax);
}

float MathUtils::Clamp01(float aValue)
{
    return MathUtils::Min(MathUtils::Max(aValue, 0.0f), 1.0f);
}

float MathUtils::LerpClamped(float aStart, float aEnd, float aPercent)
{
    return (aStart + MathUtils::Clamp01(aPercent) * (aEnd - aStart));
}

float MathUtils::LerpUnclamped(float aStart, float aEnd, float aPercent)
{
    return (aStart + aPercent * (aEnd - aStart));
}

float MathUtils::RandFloat(float aMin, float aMax)
{
	static std::mt19937 rEngine(6);
    std::uniform_real_distribution<float> rnd(aMin, aMax);
    return rnd(rEngine);
}

float MathUtils::EaseInOutBack(float aT)
{
    const float c1 = 1.70158f;
    const float c2 = c1 * 1.525f;

    return aT < 0.5f
        ? (std::pow(2.0f * aT, 2.0f) * ((c2 + 1.0f) * 2.0f * aT - c2)) / 2.0f
        : (std::pow(2.0f * aT - 2.0f, 2.0f) * ((c2 + 1.0f) * (aT * 2.0f - 2.0f) + c2) + 2.0f) / 2.0f;
}

struct SpringConfig
{
    float posPosCoefficient, posVelCoefficient;
    float velPosCoefficient, velVelCoefficient;
};

static void CalculateSpring(SpringConfig& outSpringConfig, float aDeltaTime, float aAngularFrequency, float aDampingRatio)
{
    constexpr float epsilon = 0.0001f;

    if (aDampingRatio < 0.0f)
    {
        aDampingRatio = 0.0f;
    }

    if (aAngularFrequency < 0.0f)
    {
        aAngularFrequency = 0.0f;
    }

    if (aAngularFrequency < epsilon)
    {
        outSpringConfig.posPosCoefficient = 1.0f; outSpringConfig.posVelCoefficient = 0.0f;
        outSpringConfig.velPosCoefficient = 0.0f; outSpringConfig.velVelCoefficient = 1.0f;
        return;
    }

    if (aDampingRatio > 1.0f + epsilon)
    {
        const float za = -aAngularFrequency * aDampingRatio;
        const float zb = aAngularFrequency * std::sqrt(aDampingRatio * aDampingRatio - 1.0f);
        const float z1 = za - zb;
        const float z2 = za + zb;

        const float e1 = std::exp(z1 * aDeltaTime);
        const float e2 = std::exp(z2 * aDeltaTime);

        const float invTwoZb = 1.0f / (2.0f * zb); // = 1 / (z2 - z1)

        const float e1_Over_TwoZb = e1 * invTwoZb;
        const float e2_Over_TwoZb = e2 * invTwoZb;

        const float z1e1_Over_TwoZb = z1 * e1_Over_TwoZb;
        const float z2e2_Over_TwoZb = z2 * e2_Over_TwoZb;

        outSpringConfig.posPosCoefficient = e1_Over_TwoZb * z2 - z2e2_Over_TwoZb + e2;
        outSpringConfig.posVelCoefficient = -e1_Over_TwoZb + e2_Over_TwoZb;

        outSpringConfig.velPosCoefficient = (z1e1_Over_TwoZb - z2e2_Over_TwoZb + e2) * z2;
        outSpringConfig.velVelCoefficient = -z1e1_Over_TwoZb + z2e2_Over_TwoZb;
    }
    else if (aDampingRatio < 1.0f - epsilon)
    {
        const float omegaZeta = aAngularFrequency * aDampingRatio;
        const float alpha = aAngularFrequency * std::sqrt(1.0f - aDampingRatio * aDampingRatio);

        const float expTerm = std::exp(-omegaZeta * aDeltaTime);
        const float cosTerm = std::cos(alpha * aDeltaTime);
        const float sinTerm = std::sin(alpha * aDeltaTime);

        const float invAlpha = 1.0f / alpha;

        const float expSin = expTerm * sinTerm;
        const float expCos = expTerm * cosTerm;
        const float expOmegaZetaSin_Over_Alpha = expTerm * omegaZeta * sinTerm * invAlpha;

        outSpringConfig.posPosCoefficient = expCos + expOmegaZetaSin_Over_Alpha;
        outSpringConfig.posVelCoefficient = expSin * invAlpha;

        outSpringConfig.velPosCoefficient = -expSin * alpha - omegaZeta * expOmegaZetaSin_Over_Alpha;
        outSpringConfig.velVelCoefficient = expCos - expOmegaZetaSin_Over_Alpha;
    }
    else
    {
        const float expTerm = std::exp(-aAngularFrequency * aDeltaTime);
        const float timeExp = aDeltaTime * expTerm;
        const float timeExpFreq = timeExp * aAngularFrequency;

        outSpringConfig.posPosCoefficient = timeExpFreq + expTerm;
        outSpringConfig.posVelCoefficient = timeExp;

        outSpringConfig.velPosCoefficient = -aAngularFrequency * timeExpFreq;
        outSpringConfig.velVelCoefficient = -timeExpFreq + expTerm;
    }
}

void MathUtils::Spring(float& aPosition, float& aVelocity, float aDeltaTime, float aFrequency, float aDamping)
{
    float previousPosition = aPosition; // - equilibrium
    float previousVelocity = aVelocity;

    SpringConfig spring;
    CalculateSpring(spring, aDeltaTime, aFrequency, aDamping);

    aPosition = previousPosition * spring.posPosCoefficient + previousVelocity * spring.posVelCoefficient; // + equilibrium
    aVelocity = previousPosition * spring.velPosCoefficient + previousVelocity * spring.velVelCoefficient;
}
