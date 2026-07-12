#include "FlipbookManager.h"
#include "tge/engine.h"
#include "tge/drawers/SpriteDrawer.h"
#include "tge/graphics/GraphicsEngine.h"
#include "tge/texture/TextureManager.h"

void FlipbookManager::RegisterFlipBook(const FlipBookPresets::FlipbookPreset aPreset, const FlipbookHandle aHandle, const bool aLooping)
{
	Flipbook& currentFlipbook = myFlipbooks[(int)aHandle];

	currentFlipbook.looping = aLooping;
	
	Tga::Engine& engine = *Tga::Engine::GetInstance();
	currentFlipbook.spriteData.myTexture = engine.GetTextureManager().GetTexture(aPreset.flipbookAssetPath);
	currentFlipbook.frameAmount = aPreset.frameAmount;

	const float addingUVX = 1.0f / static_cast<float>(aPreset.frameAmount);
	const float addingUVY = 1.0f / static_cast<float>(aPreset.frameAmount);

	const int frameAmount = static_cast<int>(aPreset.frameAmount);

	for (int j = 0; j < frameAmount; j++)
	{
		for (int i = 0; i < frameAmount; i++)
		{
			currentFlipbook.uvMap.push_back(UV(
				{
					addingUVX * i,
					addingUVY * j
				},
				{
					(addingUVX * i) + addingUVX,
					(addingUVY * j) + addingUVY
				}));
		}
	}
}

void FlipbookManager::InitPersistentFlipbooks()
{
	for (int i = 0; i < 3; ++i)
	{
		myPersistentFlipbook3DInstances.push_back(Flipbook3DInstance{});
	}
}

void FlipbookManager::RemoveAllLoopingInstances()
{
	std::erase_if(myFlipbookInstances, [this](const FlipbookInstance& aFlipbookInstance)
	{
		return myFlipbooks[(int)aFlipbookInstance.flipbookHandle].looping;
	});
}

void FlipbookManager::SetPersistentInstanceFlipbook(PersistentInstanceHandle anInstanceHandle, FlipbookHandle aFlipbookHandle)
{
	myPersistentFlipbook3DInstances[(int)anInstanceHandle].flipbookHandle = aFlipbookHandle;
	myPersistentFlipbook3DInstances[(int)anInstanceHandle].active = false;
}

bool FlipbookManager::GetPersistentInstanceActive(PersistentInstanceHandle anInstanceHandle) const
{
	return myPersistentFlipbook3DInstances[(int)anInstanceHandle].active;
}

void FlipbookManager::PlayPersistent(PersistentInstanceHandle anInstanceHandle, float aFrameUpdateRate)
{
	myPersistentFlipbook3DInstances[(int)anInstanceHandle].active = true;
	myPersistentFlipbook3DInstances[(int)anInstanceHandle].frameIndex = 0;
	myPersistentFlipbook3DInstances[(int)anInstanceHandle].frameUpdateRate = aFrameUpdateRate;
}

void FlipbookManager::MovePersistent(PersistentInstanceHandle anInstanceHandle, Tga::Matrix4x4f aTransform)
{
	myPersistentFlipbook3DInstances[(int)anInstanceHandle].transform = aTransform;
}

void FlipbookManager::PlayAt(FlipbookHandle aHandle, Tga::Vector2f aPosition, float aRotation)
{
	Tga::Vector2ui size = myFlipbooks[(int)aHandle].spriteData.myTexture->CalculateTextureSize();

	FlipbookInstance newInstance
	{
		.flipbookHandle = aHandle,
		.position = aPosition,
		.rotation = aRotation,
		.size = { static_cast<float>(size.x), static_cast<float>(size.y) }
	};

	myFlipbookInstances.emplace_back(newInstance);
}

void FlipbookManager::PlayAt(FlipbookHandle aHandle, Tga::Vector2f aPosition, float aTimeStep, float aRotation)
{
	Tga::Vector2ui size = myFlipbooks[(int)aHandle].spriteData.myTexture->CalculateTextureSize();

	FlipbookInstance newInstance
	{
		.flipbookHandle = aHandle,
		.position = aPosition,
		.rotation = aRotation,
		.frameUpdateRate = aTimeStep,
		.size = { static_cast<float>(size.x), static_cast<float>(size.y) }
	};

	myFlipbookInstances.emplace_back(newInstance);
}

void FlipbookManager::PlayAt(FlipbookHandle aHandle, Tga::Matrix4x4f aTransform, float aTimeStep)
{
	Tga::Vector2ui size = myFlipbooks[(int)aHandle].spriteData.myTexture->CalculateTextureSize();

	Flipbook3DInstance newInstance
	{
		.id = static_cast<unsigned int>(myFlipbookInstances.size()),
		.flipbookHandle = aHandle,
		.transform = aTransform,
		.frameUpdateRate = aTimeStep,
	};

	myFlipbook3DInstances.emplace_back(newInstance);
}

void FlipbookManager::PlayAt(FlipbookHandle aHandle, Tga::Vector2f aPosition, float aRotation, bool aFlipped)
{
	Tga::Vector2ui size = myFlipbooks[(int)aHandle].spriteData.myTexture->CalculateTextureSize();

	FlipbookInstance newInstance
	{
		.flipped = aFlipped,
		.flipbookHandle = aHandle,
		.position = aPosition,
		.rotation = aRotation,
		.size = { static_cast<float>(size.x), static_cast<float>(size.y) }
	};

	myFlipbookInstances.emplace_back(newInstance);
}

void FlipbookManager::PlayAt(FlipbookHandle aHandle, Tga::Vector2f aPosition, Tga::Vector2f aSize, float aRotation)
{
	Tga::Vector2ui spriteSize = myFlipbooks[(int)aHandle].spriteData.myTexture->CalculateTextureSize();

	FlipbookInstance newInstance
	{
		.flipbookHandle = aHandle,
		.position = aPosition,
		.rotation = aRotation,
		.size = { static_cast<float>(spriteSize.x) * aSize.x, static_cast<float>(spriteSize.y) * aSize.y }
	};

	myFlipbookInstances.emplace_back(newInstance);
}

void FlipbookManager::PlayAt(FlipbookHandle aHandle, Tga::Vector2f aPosition, Tga::Vector2f aSize, float aTimeStep,float aRotation)
{
	Tga::Vector2ui spriteSize = myFlipbooks[(int)aHandle].spriteData.myTexture->CalculateTextureSize();

	FlipbookInstance newInstance
	{
		.flipbookHandle = aHandle,
		.position = aPosition,
		.rotation = aRotation,
		.frameUpdateRate = aTimeStep,
		.size = { static_cast<float>(spriteSize.x) * aSize.x, static_cast<float>(spriteSize.y) * aSize.y }
	};

	myFlipbookInstances.emplace_back(newInstance);
}

void FlipbookManager::PlayAt(FlipbookHandle aHandle, Tga::Matrix4x4f aTransform, float aSize, float aTimeStep)
{
	Flipbook3DInstance newInstance
	{
		.id = static_cast<unsigned int>(myFlipbookInstances.size()),
		.flipbookHandle = aHandle,
		.transform =(aTransform * aSize),
		.frameUpdateRate = aTimeStep
	};

	myFlipbook3DInstances.emplace_back(newInstance);
}

void FlipbookManager::PlayAt(FlipbookHandle aHandle, Tga::Vector2f aPosition, Tga::Vector2f aSize, float aRotation, bool aFlipped)
{
	Tga::Vector2ui spriteSize = myFlipbooks[(int)aHandle].spriteData.myTexture->CalculateTextureSize();
	
	FlipbookInstance newInstance
	{
		.flipped = aFlipped,
		.flipbookHandle = aHandle,
		.position = aPosition,
		.rotation = aRotation,
		.size = { static_cast<float>(spriteSize.x) * aSize.x, static_cast<float>(spriteSize.y) * aSize.y }
	};

	myFlipbookInstances.emplace_back(newInstance);
}

void FlipbookManager::Update(const float aDeltaTime)
{
	//TODO: AHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH
	std::erase_if(myFlipbookInstances, [this](const FlipbookInstance& aFlipbookInstance)
	{
		return (!myFlipbooks[(int)aFlipbookInstance.flipbookHandle].looping &&
			(myFlipbooks[(int)aFlipbookInstance.flipbookHandle].frameAmount * myFlipbooks[(int)aFlipbookInstance.flipbookHandle].frameAmount) < aFlipbookInstance.frameIndex);
	});

	std::erase_if(myFlipbook3DInstances, [this](const Flipbook3DInstance& aFlipbookInstance)
	{
		return (!myFlipbooks[(int)aFlipbookInstance.flipbookHandle].looping &&
			(myFlipbooks[(int)aFlipbookInstance.flipbookHandle].frameAmount * myFlipbooks[(int)aFlipbookInstance.flipbookHandle].frameAmount) < aFlipbookInstance.frameIndex);
	});
	
	for (FlipbookInstance& flipbookInstance : myFlipbookInstances)
	{
		flipbookInstance.timer += aDeltaTime;
		
		if (flipbookInstance.timer >= flipbookInstance.frameUpdateRate)
		{
			flipbookInstance.frameIndex++;
			flipbookInstance.timer = 0.0f;
		}

		if (myFlipbooks[(int)flipbookInstance.flipbookHandle].looping &&
			(myFlipbooks[(int)flipbookInstance.flipbookHandle].frameAmount * myFlipbooks[(int)flipbookInstance.flipbookHandle].frameAmount
			<= flipbookInstance.frameIndex))
		{
			flipbookInstance.frameIndex = 0;
		}
	}

	for (Flipbook3DInstance& flipbookInstance : myFlipbook3DInstances)
	{
		flipbookInstance.timer += aDeltaTime;
		
		if (flipbookInstance.timer >= flipbookInstance.frameUpdateRate)
		{
			flipbookInstance.frameIndex++;
			flipbookInstance.timer = 0.0f;
		}

		if (myFlipbooks[(int)flipbookInstance.flipbookHandle].looping &&
			(myFlipbooks[(int)flipbookInstance.flipbookHandle].frameAmount * myFlipbooks[(int)flipbookInstance.flipbookHandle].frameAmount
			<= flipbookInstance.frameIndex))
		{
			flipbookInstance.frameIndex = 0;
		}
	}

	for (Flipbook3DInstance& flipbookInstance : myPersistentFlipbook3DInstances)
	{
		flipbookInstance.timer += aDeltaTime;
		
		if (flipbookInstance.timer >= flipbookInstance.frameUpdateRate)
		{
			flipbookInstance.frameIndex++;
			flipbookInstance.timer = 0.0f;
		}

		if ((myFlipbooks[(int)flipbookInstance.flipbookHandle].frameAmount * myFlipbooks[(int)flipbookInstance.flipbookHandle].frameAmount
			<= flipbookInstance.frameIndex))
		{
			flipbookInstance.active = false;
		}
	}
}

void FlipbookManager::Render() const
{
	Tga::Engine& engine = *Tga::Engine::GetInstance();
	Tga::SpriteDrawer& spriteDrawer = engine.GetGraphicsEngine().GetSpriteDrawer();

	for (auto& instance : myFlipbookInstances)
	{
		unsigned int frameAmount = myFlipbooks[(int)instance.flipbookHandle].frameAmount;

		if ((frameAmount * frameAmount - 1) < instance.frameIndex)
		{
			continue;
		}

		const std::vector<UV>& uvMap = myFlipbooks[(int)instance.flipbookHandle].uvMap;

		if (instance.flipped)
		{
			Tga::Sprite2DInstanceData spriteInstance
			{
				.myPosition = { instance.position.x, instance.position.y },
				.myPivot = instance.pivot,
				.mySize = instance.size / static_cast<float>(frameAmount),
				.myTextureRect =
				{
					.myStartX = uvMap[instance.frameIndex].end.x,
					.myStartY = uvMap[instance.frameIndex].start.y,
					.myEndX = uvMap[instance.frameIndex].start.x,
					.myEndY = uvMap[instance.frameIndex].end.y
				},
				.myRotation = instance.rotation
			};
			spriteDrawer.Draw(myFlipbooks[(int)instance.flipbookHandle].spriteData, spriteInstance);
		}
		else
		{
			Tga::Sprite2DInstanceData spriteInstance
			{
				.myPosition = { instance.position.x, instance.position.y },
				.myPivot = instance.pivot,
				.mySize = instance.size / static_cast<float>(frameAmount),
				.myTextureRect =
				{
					.myStartX = uvMap[instance.frameIndex].start.x,
					.myStartY = uvMap[instance.frameIndex].start.y,
					.myEndX = uvMap[instance.frameIndex].end.x,
					.myEndY = uvMap[instance.frameIndex].end.y
				},
				.myRotation = instance.rotation
			};
			spriteDrawer.Draw(myFlipbooks[(int)instance.flipbookHandle].spriteData, spriteInstance);
		}
	}

	for (auto& instance : myFlipbook3DInstances)
	{
		unsigned int frameAmount = myFlipbooks[(int)instance.flipbookHandle].frameAmount;

		if ((frameAmount * frameAmount - 1) < instance.frameIndex)
		{
			continue;
		}

		const std::vector<UV>& uvMap = myFlipbooks[(int)instance.flipbookHandle].uvMap;

		if (instance.flipped)
		{
			Tga::Sprite3DInstanceData spriteInstance
			{
				.myTransform = instance.transform,
				.myTextureRect =
				{
					.myStartX = uvMap[instance.frameIndex].end.x,
					.myStartY = uvMap[instance.frameIndex].start.y,
					.myEndX = uvMap[instance.frameIndex].start.x,
					.myEndY = uvMap[instance.frameIndex].end.y
				},
			};
			spriteDrawer.Draw(myFlipbooks[(int)instance.flipbookHandle].spriteData, spriteInstance);
		}
		else
		{
			Tga::Sprite3DInstanceData spriteInstance
			{
				.myTransform = instance.transform,
				.myTextureRect =
				{
					.myStartX = uvMap[instance.frameIndex].start.x,
					.myStartY = uvMap[instance.frameIndex].start.y,
					.myEndX = uvMap[instance.frameIndex].end.x,
					.myEndY = uvMap[instance.frameIndex].end.y
				},
			};
			
			spriteDrawer.Draw(myFlipbooks[(int)instance.flipbookHandle].spriteData, spriteInstance);
		}
	}

	for (auto& instance : myPersistentFlipbook3DInstances)
	{
		unsigned int frameAmount = myFlipbooks[(int)instance.flipbookHandle].frameAmount;

		if (!instance.active || (frameAmount * frameAmount - 1) < instance.frameIndex)
		{
			continue;
		}

		const std::vector<UV>& uvMap = myFlipbooks[(int)instance.flipbookHandle].uvMap;
		{
			Tga::Sprite3DInstanceData spriteInstance
			{
				.myTransform = instance.transform,
				.myTextureRect =
				{
					.myStartX = uvMap[instance.frameIndex].start.x,
					.myStartY = uvMap[instance.frameIndex].start.y,
					.myEndX = uvMap[instance.frameIndex].end.x,
					.myEndY = uvMap[instance.frameIndex].end.y
				},
			};
			
			spriteDrawer.Draw(myFlipbooks[(int)instance.flipbookHandle].spriteData, spriteInstance);
		}
	}
}
