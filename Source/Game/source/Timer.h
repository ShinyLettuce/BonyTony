#pragma once
#include <cstdint>
#include <chrono>

class Timer
{
	public:
		Timer(float aFixedTickTime = 1.f / 60.f);
		Timer(const Timer& aTimer) = delete;
		Timer& operator=(const Timer& aTimer) = delete;

		void Update();
		void BulletTime(float aTimeScale, float aBulletTimeDuration, float aLerpToSpeed, float aLerpFromSpeed);
		void SetTimeScale(float aTimeScale);
		void LerpToTimeScale(float aTimeScale, float aLerpSpeed);

		void ResetTimeScale();
		void LerpResetTimeScale(float aLerpSpeed);

		float GetDeltaTime() const;
		float GetUnscaledDeltaTime() const;
		double GetTotalTime() const;

		int GetFixedRateTickDeltaCount() const;
		uint64_t GetFixedRateTickCount() const;

		void SetIsPaused(bool aPauseState);

	private:
		
		struct TimeScaleLerp
		{
			float lerpToScale = 0;
			float lerpSpeed = 1;
		};

		enum class TimeScaleState
		{
			None,
			Lerp,
			Bullet
		};
		
		double myTimeAtCreation = 0;
		double myCurrentFrameTime = 0;
		double myLastFrameTime = 0;

		uint64_t myCurrentFixedRateTickCount = 0;
		uint64_t myLastFixedRateTickCount = 0;

		float myFixedTickTime = 0;
		
		TimeScaleState myTimeState = TimeScaleState::None;
		float myTimeScale = 1;
		float myBulletTimeTimer = 0;
		float myBulletTimeDuration = 0;
		TimeScaleLerp myTimeScaleData;
		TimeScaleLerp myTimeScaleDataBuffer;
		
		bool myWasJustResumed = false;
		bool myIsPaused = false;
		std::chrono::high_resolution_clock myClock;
};
