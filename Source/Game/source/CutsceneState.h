#pragma once
#include "InputMapper.h"
#include "State.h"
#include "Cutscene.h"
#include "tge/sprite/sprite.h"
#include "tge/videoplayer/video.h"

struct CutsceneStateHandles
{
	StateHandle pauseState;
	StateHandle gameState;
};

class CutsceneState : public State
{
public:
	CutsceneState() = default;
	~CutsceneState() override;
	void Init(CutsceneStateHandles someCutsceneStateHandles, InputMapper* anInputMapper, Timer* aTimer);

	void OnPush() override;
	void OnPop() override;	
	
	void OnGainFocus() override;
	void OnLoseFocus() override;

	StateUpdateResult Update() override;
	void Render() override;

private:

	CutsceneStateHandles myCutsceneStateHandles;
	
	InputMapper* myInputMapper;
	Timer* myTimer;
	Tga::Video myVideo;

	Tga::SpriteSharedData myVideoData;
};
