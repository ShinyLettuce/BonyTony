#pragma once

#include <tge/scene/SceneObjectDefinition.h>

namespace Tga
{
	struct ScriptUpdateContext
	{
		std::unordered_map<StringId, Property>* dynamicProperties;
		const std::unordered_map<StringId, Property>* staticProperties;
		float deltaTime;
	};

} // namespace Tga