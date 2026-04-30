#include "AudioManager.h"

#include <cassert>

#include "tge/stringRegistry/StringRegistry.h"

namespace AudioManager
{
    static std::vector<AudioPool> globalAudioPools;

    void AudioPool::AddClip(const char* aPath, bool aIsLooping)
    {
        myAudioPool.emplace_back();
        myAudioPool.back().Init(aPath, aIsLooping);
    }

    void AudioPool::Play()
    {
        const size_t size = myAudioPool.size();
       
        if (size == 1)
        {
            myAudioPool.front().Play();
        }
        else if (size > 1)
        {
            PlayRandom();
        }
    }
    
    void AudioPool::PlayRandom()
    {
        int randomNumber = rand() % myAudioPool.size();

        if (!myPreviouslyPlayed.empty())
        {
            bool isValid = false;
            while (!isValid)
            {
                randomNumber = rand() % myAudioPool.size();
                
                for (const auto previouslyPlayed : myPreviouslyPlayed)
                {
                    if (randomNumber != previouslyPlayed)
                    {
                        isValid = true;
                    }
                    else
                    {
                        isValid = false;
                        break;
                    }
                }
            }
        }
        
        myAudioPool.at(randomNumber).Play();
        
        myPreviouslyPlayed.push_back(randomNumber);
        if (myPreviouslyPlayed.size() >= myRepeatBuffer && myRepeatBuffer > 0)
        {
            myPreviouslyPlayed.erase(myPreviouslyPlayed.begin());
        }
    }

    void AudioPool::SetVolume(float aVolume)
    {
        for (auto& audio : myAudioPool)
        {
            audio.SetVolume(aVolume);
        }
    }

    void AudioPool::Stop()
    {
        for (auto& audio : myAudioPool)
        {
            audio.Stop();
        }
    }

    AudioType AudioPool::GetAudioType() const
    {
        return myAudioType;
    }

    void AudioPool::SetAudioType(const AudioType aAudioType)
    {
        myAudioType = aAudioType;
    }

    void AudioPool::SetRepeatBuffer(const int aRepeatBuffer)
    {
        myRepeatBuffer = aRepeatBuffer;
    }

    bool AudioPool::IsAudioPlaying()
    {
        for (auto& audio : myAudioPool)
        {
            if (audio.IsAudioPlayingNow())
            {
                return true;
            }
        }
        return false;
    }

    static std::unordered_map<std::string_view, AudioPoolHandle> globalIdToAudioPoolHandle;

    AudioPoolHandle MakeAudioPool(std::string_view aId)
    {
        AudioPoolHandle audioPool = static_cast<AudioPoolHandle>(globalAudioPools.size());
        globalAudioPools.push_back(AudioPool());
        if (!aId.empty())
        {
            globalIdToAudioPoolHandle.insert({aId, audioPool});
        }
        return audioPool;
    }
    
    AudioPool& GetAudioPoolByHandle(AudioPoolHandle aHandle)
    {
        return globalAudioPools.at(aHandle);
    }

    AudioPoolHandle GetAudioPoolHandleById(std::string_view aId)
    {
        assert(globalIdToAudioPoolHandle.contains(aId));
        
        return globalIdToAudioPoolHandle.at(aId);
    }
    
    void UpdateVolume(const int aMasterVolume, const int aMusicVolume, const int aMaxVolume)
    {
        const float masterVolume = static_cast<float>(aMasterVolume)/static_cast<float>(aMaxVolume);
        const float musicVolume = masterVolume * static_cast<float>(aMusicVolume)/static_cast<float>(aMaxVolume);
        
        for (auto& audioPool : globalAudioPools)
        {
            switch (audioPool.GetAudioType())
            {
                case AudioType::None:
                {
                    audioPool.SetVolume(masterVolume);
                    break;
                }
                case AudioType::Music:
                {
                    audioPool.SetVolume(musicVolume);
                    break;
                }
            }
        }
    }
}