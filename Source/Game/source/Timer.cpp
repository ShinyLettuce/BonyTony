#include "Timer.h"

#include "tge/math/FMath.h"

void Timer::SetIsPaused(bool aPauseState)
{
	myIsPaused = aPauseState;
	if (!myIsPaused)
	{
		myWasJustResumed = true;
	}
}

Timer::Timer(float aFixedTickTime) : myFixedTickTime(aFixedTickTime)
{
	myTimeAtCreation = static_cast<double>(myClock.now().time_since_epoch().count());
	myCurrentFixedRateTickCount = GetFixedRateTickCount();
	myLastFixedRateTickCount = myCurrentFixedRateTickCount;
}
   
void Timer::Update()
{
	if (myIsPaused)
	{
		return;
	}
	if (myWasJustResumed)
	{
		myCurrentFrameTime = (myClock.now().time_since_epoch().count() - myTimeAtCreation) / 1000000000.0;
		myLastFrameTime = myCurrentFrameTime - myFixedTickTime;
		myWasJustResumed = false;
	}
	else
	{
		myLastFrameTime = myCurrentFrameTime;
		myCurrentFrameTime = (myClock.now().time_since_epoch().count() - myTimeAtCreation) / 1000000000.0;
	}
	myLastFixedRateTickCount = myCurrentFixedRateTickCount;
	myCurrentFixedRateTickCount = GetFixedRateTickCount();

	if (myTimeState != TimeScaleState::None)
	{
		const float blend = pow(0.5f, myTimeScaleData.lerpSpeed * GetUnscaledDeltaTime());
		myTimeScale = FMath::Lerp(myTimeScaleData.lerpToScale, myTimeScale, blend);
		
		if (myTimeScale > myTimeScaleData.lerpToScale - 0.1f && myTimeScale < myTimeScaleData.lerpToScale + 0.1f)
		{
			if (myBulletTimeDuration < myBulletTimeTimer)
			{
				myTimeScale = myTimeScaleData.lerpToScale;
				if (myTimeState == TimeScaleState::Bullet)
				{
					myTimeScaleData.lerpToScale = myTimeScaleDataBuffer.lerpToScale;
					myTimeScaleData.lerpSpeed = myTimeScaleDataBuffer.lerpSpeed;
					myTimeState = TimeScaleState::Lerp;
				}
				else
				{
					myTimeState = TimeScaleState::None;
				}
			}
			else
			{
				myBulletTimeTimer += GetUnscaledDeltaTime();
			}
		}
	}
}

void Timer::BulletTime(float aTimeScale, float aBulletTimeDuration, float aLerpToSpeed, float aLerpFromSpeed)
{
	myTimeScaleData.lerpToScale = aTimeScale;
	myTimeScaleData.lerpSpeed = aLerpToSpeed;
	myTimeScaleDataBuffer.lerpToScale = 1;
	myTimeScaleDataBuffer.lerpSpeed = aLerpFromSpeed;
	myBulletTimeDuration = aBulletTimeDuration;
	myBulletTimeTimer = 0;
	myTimeState = TimeScaleState::Bullet;
}

void Timer::LerpToTimeScale(float aTimeScale, float aLerpSpeed)
{
	myTimeScaleData.lerpToScale = aTimeScale;
	myTimeScaleData.lerpSpeed = aLerpSpeed;
	myTimeState = TimeScaleState::Lerp;
	myBulletTimeDuration = 0;
	myBulletTimeTimer = 1;
}

void Timer::LerpResetTimeScale(float aLerpSpeed)
{
	myTimeScaleData.lerpToScale = 1;
	myTimeScaleData.lerpSpeed = aLerpSpeed;
	myTimeState = TimeScaleState::Lerp;
	myBulletTimeDuration = 0;
	myBulletTimeTimer = 1;
}

void Timer::SetTimeScale(float aTimeScale)
{
	myTimeScale = aTimeScale;
}

void Timer::ResetTimeScale()
{
	myTimeScale = 1.0f;
}

float Timer::GetDeltaTime() const
{
	return static_cast<float>(myCurrentFrameTime - myLastFrameTime) * myTimeScale;
}

float Timer::GetUnscaledDeltaTime() const
{
	return static_cast<float>(myCurrentFrameTime - myLastFrameTime);
}

double Timer::GetTotalTime() const
{
	return myCurrentFrameTime;
}

int Timer::GetFixedRateTickDeltaCount() const
{
	return static_cast<int>(myCurrentFixedRateTickCount - myLastFixedRateTickCount);
}

uint64_t Timer::GetFixedRateTickCount() const
{
	return static_cast<uint64_t>(myCurrentFrameTime / myFixedTickTime);
}
