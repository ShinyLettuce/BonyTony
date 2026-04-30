#include <stdafx.h>
#include "AnimationClip.h"

#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>

#include <tge/settings/settings.h>

using namespace Tga;


static std::unordered_map<StringId, AnimationClip> locLoadedClips;

AnimationClip* Tga::GetAnimationClip(StringId path)
{
	auto it = locLoadedClips.find(path);
	if (it != locLoadedClips.end())
		return &it->second;

	std::filesystem::path resolvedPath = Tga::Settings::GameAssetRoot() / path.GetString();
	if (!fs::exists(resolvedPath))
		return nullptr;

	AnimationClip& clip = locLoadedClips[path];

	std::ifstream file(resolvedPath, std::ios::in);
	nlohmann::json json;
	file >> json;
	file.close();

	clip.animationSourcePath = StringRegistry::RegisterOrGetString(json.value("animation_source_path", ""));
	clip.previewModelPath = StringRegistry::RegisterOrGetString(json.value("preview_model_path", ""));

	clip.startTime = json.value("start_time", 0.f);
	clip.endTime =json.value("end_time", 0.f);
	clip.cycleOffsetPercentage = json.value("cycle_offset", 0.f);
	clip.isLooping = json.value("is_looping", false);

	return &clip;
}

AnimationClip* Tga::GetOrCreateAnimationClip(StringId path)
{
	AnimationClip* clip = GetAnimationClip(path);

	if (clip == nullptr)
		clip = &locLoadedClips[path];

	return clip;
}


void Tga::SaveAnimationClip(StringId path)
{
	std::filesystem::path resolvedPath = Tga::Settings::GameAssetRoot() / path.GetString();
	AnimationClip& clip = locLoadedClips[path];

	nlohmann::json json = {
	{ "animation_source_path", clip.animationSourcePath.GetString()},
	{ "preview_model_path", clip.previewModelPath.GetString()},
	{ "start_time", clip.startTime},
	{ "end_time", clip.endTime},
	{ "cycle_offset", clip.cycleOffsetPercentage},
	{ "is_looping", clip.isLooping},
	};

	std::ofstream fout(resolvedPath, std::ios::trunc);
	fs::permissions(resolvedPath, fs::perms::all);
	fout << json.dump(2);
}