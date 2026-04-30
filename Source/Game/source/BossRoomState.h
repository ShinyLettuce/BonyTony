#pragma once

#include "State.h"
#include "Camera.h"
#include "FlipbookManager.h"
#include "Player.h"
#include "Letterbox.h"
#include "SceneLoader.h"

class BossRoomState : public State
{
public:
	void Init(StateHandle aMainMenuStateHandle, StateHandle aCutsceneStateHandle, InputMapper* anInputMapper, Timer* aTimer);

	void OnPush() override;
	void OnPop() override;

	void OnGainFocus() override;
	void OnLoseFocus() override;

	StateUpdateResult Update() override;
	void Render() override;
private:
	struct Boss
	{
		std::shared_ptr<Tga::AnimatedModelInstance> animatedModelInstance;
		std::shared_ptr<Tga::AnimationPlayer> idleAnimationPlayer;
		bool isDead;
	};

	struct Timings
	{
		float time;
		float timeWhenDoorCloses;
		bool hasDoorClosed;
		float timeWhenBossStartsYapping;
		bool hasBossStartedYapping;
		float gibberishDelay;
		float gibberishTimer;
		
		bool hasTutorialStarted;
		float timeWhenTutorialStartsFadingIn;
		float tutorialDuration;
	};

	InputMapper* myInputMapper;
	Timer* myTimer;
	
	FlipbookManager myFlipBookManager;
	FlipbookManager::FlipbookHandle mySpeechBubbleFlipbookHandle;
	Tga::Vector2f mySpeechBubbleOffset = { -300.f, 350.f };
	static constexpr float mySpeechBubbleSpeed = 0.125f;
	static constexpr float mySpeechBubbleSize = 0.7f;
	
	Tga::Sprite2DInstanceData myTutorialInstance;
	Tga::SpriteSharedData myTutorialSharedData;
	static constexpr float myTutorialSizeMultiplier = 0.3f;
	static constexpr float myTutorialVerticalOffset = 0.27f;
	float myTutorialAlpha = 0.f;
	
	Tga::SpriteSharedData myFullscreenSharedData;

	StateHandle myMainMenuHandle;
	StateHandle myCutsceneStateHandle;

	Camera myCamera;
	Player myPlayer;
	Boss myBoss;

	Letterbox myLetterbox;

	Timings myTimings;

	inline static constexpr float myCameraHorizontalOffset = -100.0f;

	bool myStartedEndSequence;
	float myEndSequenceTime;
};