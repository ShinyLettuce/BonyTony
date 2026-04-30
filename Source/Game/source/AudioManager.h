#pragma once

#include <vector>

#include <tge/math/Vector2.h>
#include <tge/audio/audio.h>

namespace AudioManager
{
    enum class AudioType
    {
        None,
        Music,
    };
    
    class AudioPool
    {
    public: 
        void AddClip(const char* aPath, bool aIsLooping = false);
        void Play();
        void PlayRandom();
        void SetVolume(float aVolume);
        void Stop();
        AudioType GetAudioType() const;
        void SetAudioType(const AudioType aAudioType);
        void SetRepeatBuffer(const int aRepeatBuffer);
        bool IsAudioPlaying();
        
    private:
        std::vector<Tga::Audio> myAudioPool;
        std::vector<int> myPreviouslyPlayed;
        int myRepeatBuffer = 0;
        AudioType myAudioType = AudioType::None;
    };
    
    using AudioPoolHandle = int;
    
    AudioPoolHandle MakeAudioPool(std::string_view aId = "");
    AudioPool& GetAudioPoolByHandle(AudioPoolHandle aHandle);
    AudioPoolHandle GetAudioPoolHandleById(std::string_view aId);
    
    void UpdateVolume(int aMasterVolume, int aMusicVolume, int aMaxVolume);
}

namespace AudioHandles
{
    inline AudioManager::AudioPoolHandle shotgunShot;
    inline AudioManager::AudioPoolHandle reload;
    inline AudioManager::AudioPoolHandle shotgunShotNoAmmo;
    inline AudioManager::AudioPoolHandle revolverShot;
    inline AudioManager::AudioPoolHandle revolverShotNoAmmo;
    inline AudioManager::AudioPoolHandle batSwingAndHit;
    inline AudioManager::AudioPoolHandle pistolWhipAndHit;
    inline AudioManager::AudioPoolHandle woodenCrateDestroyed;
    inline AudioManager::AudioPoolHandle bulletproofGlassDestroyed;
    inline AudioManager::AudioPoolHandle shotGunPowerShot;
    inline AudioManager::AudioPoolHandle playerLandingOnGround;
    inline AudioManager::AudioPoolHandle playerHurt;
    inline AudioManager::AudioPoolHandle enemyHurt;
    inline AudioManager::AudioPoolHandle enemyReassemble;
    inline AudioManager::AudioPoolHandle bulletHittingIronCrate;
    inline AudioManager::AudioPoolHandle playerCollideWithWallOrCeiling;
    inline AudioManager::AudioPoolHandle bulletHittingWallOrFloor;
    inline AudioManager::AudioPoolHandle enemyDeathPoofCloud;
    inline AudioManager::AudioPoolHandle hoverButton;
    inline AudioManager::AudioPoolHandle clickButton;
    inline AudioManager::AudioPoolHandle windowChange;
    inline AudioManager::AudioPoolHandle level1Ambience;
    inline AudioManager::AudioPoolHandle level2Ambience;
    
    inline AudioManager::AudioPoolHandle elevatorDing;
    inline AudioManager::AudioPoolHandle elevatorClose;
    inline AudioManager::AudioPoolHandle elevatorOpen;
    inline AudioManager::AudioPoolHandle elevatorAmbienceAndMusic;
    inline AudioManager::AudioPoolHandle uiBack;
    inline AudioManager::AudioPoolHandle bossRoomAmbience;
    inline AudioManager::AudioPoolHandle bossDoorOpening;
    inline AudioManager::AudioPoolHandle level1Music;
    inline AudioManager::AudioPoolHandle level2IntroMusic;
    inline AudioManager::AudioPoolHandle level2Music;
    inline AudioManager::AudioPoolHandle gibberish;
    
    inline AudioManager::AudioPoolHandle introCutscene;
    inline AudioManager::AudioPoolHandle endingCutscene;

    inline AudioManager::AudioPoolHandle mainMenuMusic;
}