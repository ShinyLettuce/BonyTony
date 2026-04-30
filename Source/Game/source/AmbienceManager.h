#pragma once
#include "AudioManager.h"
#include "tge/math/Vector2.h"
#include "tge/sprite/sprite.h"
#include "tge/stringRegistry/StringRegistry.h"

struct AudioData
{
    Tga::Audio audio;
    float volume;
};

struct Ambience
{
	Tga::StringId path;
    Tga::Vector2f position;
    float defaultVolume;
    float volume;
    float maxVolumeDistance;
    float muteDistance;  
};

class AmbienceManager
{
public:
    void Init(std::vector<Ambience>* aAmbiences);
    void Update(const Tga::Vector2f aListenerPosition);
    void StopAll();
    void PlayAll();
    void RenderDebugVisuals () const;
    void UpdateVolume(const int aMasterVolume, const int aMaxVolume) const;
private:
    std::vector<Ambience>* myAmbiences = nullptr;
    std::unordered_map<Tga::StringId, AudioData> myAudioMap;
    
    std::vector<Tga::Sprite2DInstanceData> myInstances;
    Tga::SpriteSharedData mySharedData;
    
    bool myIsPlaying = false;
};

