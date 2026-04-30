#pragma once
#include "SceneLoader.h"
#include "tge/math/Vector2.h"
#include "tge/texture/TextureManager.h"

class LevelTrigger
{	
	enum class AnimationState
	{
		None,
		Opening,
		Closing,
	};
	
	struct LevelTriggerData
	{
		Tga::StringId sceneToLoad;
		Tga::Vector2f position;
		Tga::Vector2f size;
		std::shared_ptr<Tga::AnimatedModelInstance> modelInstance;
		std::shared_ptr<Tga::AnimationPlayer> openAnimation;
		std::shared_ptr<Tga::AnimationPlayer> closeAnimation;
		bool active;
		bool exists;
		float fadeOutTime;
		float delayTime;
		AnimationState animationState = AnimationState::None;
	};


public:
	
	struct AudioSequenceData
	{
		AudioManager::AudioPoolHandle audioPoolHandle;
		float bgmFadeStart;
		float bgmFadeDuration;
		float timeUntilFadeOut;
	};

	void Init(const SceneLoader::LevelTriggerConfig& aLevelTriggerConfig, const AudioSequenceData& aAudioSequence);

	Tga::Vector2f GetPosition() const;
	Tga::Vector2f GetSize() const;
	void ActivateTrigger();
	void UpdateDelayTimer(float aDeltaTime);
	bool DelayTimerFinished() const;
	float GetFadeOutTime() const { return myLevelTriggerData.fadeOutTime; };
	Tga::StringId GetPath() const { return myLevelTriggerData.sceneToLoad; }
	bool GetActive() const;
	bool GetExists() const;
	
	
	void UpdateAnimation(float aDeltaTime);
	void OpenDoor();
	void CloseDoor();
	void ResetDoor();
	
	const AudioSequenceData& GetAudioSequenceData() const;
	bool GetHasSfxPlayed() const;
	void SetHasSfxPlayed(const bool aHasPlayed);
	
	void Render();

private:
	AudioSequenceData myAudioSequenceData{};
	bool mySfxHasPlayed = false;
	
	LevelTriggerData myLevelTriggerData{};
	float myFadeOutTimer = 0.f;
	float myDelayTimer = 0.f;

	Tga::SpriteSharedData myFadeOutSharedData;
};
