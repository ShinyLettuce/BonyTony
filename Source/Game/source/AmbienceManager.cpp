#include "AmbienceManager.h"

#include "SceneLoader.h"
#include "imgui/imgui.h"
#include "tge/math/CommonMath.h"


#if defined(_DEBUG)
#include "tge/Engine.h"
#include "tge/drawers/SpriteDrawer.h"
#include "tge/graphics/GraphicsEngine.h"
#include "tge/texture/TextureManager.h"
#include "tge/drawers/SpriteDrawer.h"
#endif

void AmbienceManager::Init(std::vector<Ambience>* aAmbiences)
{
    myAmbiences = aAmbiences;

    #if defined(_DEBUG)
    myInstances.clear();
    mySharedData.myTexture = Tga::Engine::GetInstance()->GetTextureManager().GetTexture("Sprites/AmbienceDebug.png");
    #endif
    
    for (auto& ambience : *myAmbiences)
    {
        const size_t oldSize = myAudioMap.size();
        ambience.defaultVolume = ambience.volume;
        
        myAudioMap.try_emplace(ambience.path, Tga::Audio{});
        
        if (oldSize != myAudioMap.size())
        {
            myAudioMap.at(ambience.path).audio.Init(ambience.path.GetString(), true);
            myAudioMap.at(ambience.path).audio.Play();
        }
        
        #if defined(_DEBUG)
        myInstances.emplace_back();
        myInstances.back().myPosition = ambience.position;
        myInstances.back().mySize = mySharedData.myTexture->CalculateTextureSize();
        #endif
    }
    
    myIsPlaying = true;
}

void AmbienceManager::Update(const Tga::Vector2f aListenerPosition)
{
    #if !defined(_RETAIL)
    ImGui::Begin("Ambience");
    #endif
    
    for (auto& ambience : *myAmbiences)
    {
        const float distanceToListener = Tga::Vector2f::Distance(aListenerPosition, ambience.position);
        float volume = ambience.volume;
        
        if (distanceToListener > ambience.muteDistance)
        {
            volume = 0.f;
        }
        else if (distanceToListener > ambience.maxVolumeDistance)
        {
            const float maxDistance = ambience.muteDistance - ambience.maxVolumeDistance;
            const float listenerDistance = distanceToListener - ambience.maxVolumeDistance;
            volume = ambience.volume * (1.f - (listenerDistance / maxDistance));
        }
        
        auto& audioData = myAudioMap.at(ambience.path);
        
        audioData.volume += volume;
        audioData.volume = Tga::Clamp(audioData.volume, 0.f, ambience.volume);
        
        #if !defined(_RETAIL)
        ImGui::Text("Audio: %s", ambience.path.GetString());
        ImGui::Text("Position: %fx %fy", ambience.position.x, ambience.position.y);
        ImGui::Text("Distance to ambience: %f", distanceToListener);
        ImGui::Text("Mute Distance: %f", ambience.muteDistance);
        ImGui::Text("Max Volume Distance: %f", ambience.maxVolumeDistance);
        ImGui::Text("Default Volume: %f", ambience.volume);
        ImGui::NewLine();
        #endif
    }
    
    for (auto& [path, audioData] : myAudioMap)
    {
        audioData.audio.SetVolume(audioData.volume);
        
        #if !defined(_RETAIL)
        ImGui::Text("%s's Volume: %f", path.GetString(), audioData.volume);
        #endif
        
        audioData.volume = 0;
    }
    
    #if !defined(_RETAIL)
    if (ImGui::Button("Toggle all ambience"))
	{
		if (myIsPlaying)
		{
		    StopAll();
		}
        else
        {
            PlayAll();
        }
	}
    ImGui::End();
    #endif
}

void AmbienceManager::StopAll()
{
    myIsPlaying = false;
    for (auto& [path, audioData] : myAudioMap)
    {
        audioData.audio.Stop();
    }
}

void AmbienceManager::PlayAll()
{
    myIsPlaying = true;
    for (auto& [path, audioData] : myAudioMap)
    {
        audioData.audio.Play();
    }
}

void AmbienceManager::RenderDebugVisuals() const
{
    #if defined(_DEBUG)
    
    const Tga::Engine& engine = *Tga::Engine::GetInstance();
	Tga::SpriteDrawer& spriteDrawer = engine.GetGraphicsEngine().GetSpriteDrawer();
    
    for (auto& instance : myInstances)
    {
	    spriteDrawer.Draw(mySharedData, instance);
    }
    
    #endif
}

void AmbienceManager::UpdateVolume(const int aMasterVolume, const int aMaxVolume) const
{
    const float masterVolume = static_cast<float>(aMasterVolume)/static_cast<float>(aMaxVolume);
    
    for (auto& ambience : *myAmbiences)
    {
        ambience.volume = ambience.defaultVolume * masterVolume;
    }
}
