#include "SceneLoader.h"

#include <tge/scene/SceneSerialize.h>
#include <tge/scene/Scene.h>
#include <tge/engine.h>
#include <tge/settings/settings.h>
#include <tge/scene/SceneObject.h>
#include <tge/scene/SceneObjectDefinition.h>
#include <tge/scene/SceneObjectDefinitionManager.h>
#include <tge/scene/ScenePropertyTypes.h>
#include <tge/model/Model.h>
#include <tge/model/ModelFactory.h>
#include <tge/script/BaseProperties.h>
#include <tge/texture/TextureManager.h>
#include <tge/stringRegistry/StringRegistry.h>
#include <tge/model/ModelInstance.h>
#include <tge/script/jsondata.h>

#include <nlohmann/json.hpp>

#include <fstream>
#include <unordered_map>
#include <atomic>

#include "AmbienceManager.h"

#include "ThreadPool.h"
#include "Profiler.h"

namespace SceneLoader
{
	Tga::SceneObjectDefinitionManager globalSceneDefinitionManager;

	static std::unordered_map<const char*, SceneConfig> globalSceneStore;

	static const char* globalActiveScenePath = nullptr;
	static bool globalIsSceneLoaded = false;

	SceneConfig globalSceneConfigCopy;

	ThreadPool globalThreadPool(2);

	class CompiledSceneObject
	{
	public:
		void Load(std::shared_ptr<Tga::SceneObject> aSceneObject, std::vector<Tga::ScenePropertyDefinition>& aScenePropertyDefinitions)
		{
			myInts.clear();
			myFloats.clear();
			myBools.clear();
			myVector2fs.clear();
			myStringIds.clear();
			myAnimationClipReferences.clear();
			mySceneModels.clear();

			mySceneObject = aSceneObject;

			for (const auto& scenePropertyDefinition : aScenePropertyDefinitions)
			{
				if (scenePropertyDefinition.type == Tga::GetPropertyType<int>())
				{
					myInts.insert({ scenePropertyDefinition.name.GetString(), *scenePropertyDefinition.value.Get<int>() });
					continue;
				}

				if (scenePropertyDefinition.type == Tga::GetPropertyType<float>())
				{
					myFloats.insert({ scenePropertyDefinition.name.GetString(), *scenePropertyDefinition.value.Get<float>() });
					continue;
				}

				if (scenePropertyDefinition.type == Tga::GetPropertyType<bool>())
				{
					myBools.insert({ scenePropertyDefinition.name.GetString(), *scenePropertyDefinition.value.Get<bool>() });
					continue;
				}

				if (scenePropertyDefinition.type == Tga::GetPropertyType<Tga::Vector2f>())
				{
					myVector2fs.insert({ scenePropertyDefinition.name.GetString(), *scenePropertyDefinition.value.Get<Tga::Vector2f>() });
					continue;
				}

				if (scenePropertyDefinition.type == Tga::GetPropertyType<Tga::StringId>())
				{
					myStringIds.insert({ scenePropertyDefinition.name.GetString(), *scenePropertyDefinition.value.Get<Tga::StringId>() });
					continue;
				}

				if (scenePropertyDefinition.type == Tga::GetPropertyType<Tga::CopyOnWriteWrapper<Tga::SceneModel>>())
				{
					ScopedProfiler p("cso_load_scene_model");

					const Tga::SceneModel& sceneModel = scenePropertyDefinition.value.Get<Tga::CopyOnWriteWrapper<Tga::SceneModel>>()->Get();

					mySceneModels.insert({ scenePropertyDefinition.name.GetString(), sceneModel });
					continue;
				}

				if (scenePropertyDefinition.type == Tga::GetPropertyType<Tga::CopyOnWriteWrapper<Tga::AnimationClipReference>>())
				{
					const Tga::AnimationClipReference& animationClipReference = scenePropertyDefinition.value.Get<Tga::CopyOnWriteWrapper<Tga::AnimationClipReference>>()->Get();

					Tga::StringId path = animationClipReference.path;
					if (path.IsEmpty() || Tga::Settings::ResolveAssetPath(path.GetString()).empty())
					{
						//std::cout << "\033[31m" << "[SceneLoader.cpp] Failed while loading animation clip reference, cannot resolve asset path! " << "\"" << animationClipReference.path.GetString() << "\"" << "\033[0m" << '\n';
						continue;
					}

					myAnimationClipReferences.insert({ scenePropertyDefinition.name.GetString(), animationClipReference });

					continue;
				}
			}
		}

		int GetInt(std::string_view aId) const
		{
			assert(myInts.contains(aId));
			return myInts.at(aId);
		}

		float GetFloat(std::string_view aId) const
		{
			assert(myFloats.contains(aId));
			return myFloats.at(aId);
		}

		bool GetBool(std::string_view aId) const
		{
			assert(myBools.contains(aId));
			return myBools.at(aId);
		}

		Tga::Vector2f GetVector2f(std::string_view aId) const
		{
			assert(myVector2fs.contains(aId));
			return myVector2fs.at(aId);
		}

		Tga::Vector2f GetVector2fOrDefault(std::string_view aId, Tga::Vector2f aDefault) const
		{
			if (myVector2fs.contains(aId))
			{
				return myVector2fs.at(aId);
			}
			return aDefault;
		}

		Tga::StringId GetStringId(std::string_view aId) const
		{
			assert(myStringIds.contains(aId));
			return myStringIds.at(aId);
		}

		Tga::ModelInstance GetModelInstance(std::string_view aId)
		{
			Tga::Engine& engine = *Tga::Engine::GetInstance();
			Tga::TextureManager& textureManager = engine.GetTextureManager();

			const Tga::SceneModel& sceneModel = mySceneModels.at(aId);
			Tga::ModelInstance modelInstance = Tga::ModelFactory::GetInstance().GetModelInstance(sceneModel.path.GetString());

#if !defined(_RETAIL)
			myUniqueModelPaths.insert({ sceneModel.path.GetString() });
#endif 

			std::cout << "Load model " << sceneModel.path.GetString() << '\n';

			if (modelInstance.IsValid())
			{
				modelInstance.SetTransform(mySceneObject->GetTransform());

				for (int i = 0; i < 4; ++i)
				{
					for (int j = 0; j < 4; ++j)
					{
						Tga::StringId textureId = sceneModel.textures[i][j];
						Tga::Texture* texture = textureManager.GetTexture(textureId.GetString());

						if (texture)
						{
							modelInstance.SetTexture(i, j, texture);
						}
					}
				}
			}

			return modelInstance;
		}

		Tga::ModelInstance GetFirstModelInstance()
		{
			Tga::Engine& engine = *Tga::Engine::GetInstance();
			Tga::TextureManager& textureManager = engine.GetTextureManager();

			const Tga::SceneModel& sceneModel = mySceneModels.begin()->second;
			Tga::ModelInstance modelInstance = Tga::ModelFactory::GetInstance().GetModelInstance(sceneModel.path.GetString());

#if !defined(_RETAIL)
			myUniqueModelPaths.insert({ sceneModel.path.GetString() });
#endif

			std::cout << "Load model " << sceneModel.path.GetString() << '\n';

			if (modelInstance.IsValid())
			{
				modelInstance.SetTransform(mySceneObject->GetTransform());

				for (int i = 0; i < 4; ++i)
				{
					for (int j = 0; j < 4; ++j)
					{
						Tga::StringId textureId = sceneModel.textures[i][j];
						Tga::Texture* texture = textureManager.GetTexture(textureId.GetString());

						if (texture)
						{
							modelInstance.SetTexture(i, j, texture);
						}
					}
				}
			}

			return modelInstance;
		}

		Tga::AnimatedModelInstance GetAnimatedModelInstance(std::string_view aId) const
		{
			Tga::Engine& engine = *Tga::Engine::GetInstance();
			Tga::TextureManager& textureManager = engine.GetTextureManager();

			const Tga::SceneModel& sceneModel = mySceneModels.at(aId);
			Tga::AnimatedModelInstance animatedModelInstance = Tga::ModelFactory::GetInstance().GetAnimatedModelInstance(sceneModel.path.GetString());
			
			std::cout << "Load model " << sceneModel.path.GetString() << '\n';

			if (animatedModelInstance.IsValid())
			{
				animatedModelInstance.SetTransform(mySceneObject->GetTransform());

				for (int i = 0; i < 4; ++i)
				{
					for (int j = 0; j < 4; ++j)
					{
						Tga::StringId textureId = sceneModel.textures[i][j];
						Tga::Texture* texture = textureManager.GetTexture(textureId.GetString());

						if (texture)
						{
							animatedModelInstance.SetTexture(i, j, texture);
						}
					}
				}
			}

			return animatedModelInstance;
		}

		Tga::AnimationClipReference GetAnimationClipReference(std::string_view aId) const
		{
			assert(myAnimationClipReferences.contains(aId));
			return myAnimationClipReferences.at(aId);
		}

		void OutputUniqueModelPaths() const
		{
#if !defined(_RETAIL)
			std::cout << '\n';

			for (const auto& path : myUniqueModelPaths)
			{
				std::cout << '\"' << path << '\"' << ',' << '\n';
			}
#endif
		}
	private:
		std::shared_ptr<Tga::SceneObject> mySceneObject;

		std::unordered_map<std::string_view, int> myInts;
		std::unordered_map<std::string_view, float> myFloats;
		std::unordered_map<std::string_view, bool> myBools;
		std::unordered_map<std::string_view, Tga::Vector2f> myVector2fs;
		std::unordered_map<std::string_view, Tga::StringId> myStringIds;
		std::unordered_map<std::string_view, Tga::SceneModel> mySceneModels;
		std::unordered_map<std::string_view, Tga::AnimationClipReference> myAnimationClipReferences;

		std::set<std::string> myUniqueModelPaths;
	};

	static std::atomic<bool> globalKillPreload{ false };

	void Init()
	{
		globalSceneDefinitionManager.Init(Tga::Settings::GameAssetRoot().string().data());
	}

	void StartPreloadProcess()
	{
		globalThreadPool.Enqueue([]() {
			const char* paths1[]
			{
				"models\\BreakableMeshes\\SM_WoodBoxDestruction_Beta.fbx",
				"models\\Characters\\SK_PlayerCharacter.fbx",
				"models\\Characters\\SM_ProjectileRevolver.fbx",
				"models\\Characters\\SM_Revolver.fbx",
				"models\\Characters\\SM_Shotgun.fbx",
				"models\\PlatformMeshes\\Foreground\\Level1\\SM_Level1_Borderwall.fbx",
				"models\\PlatformMeshes\\SM_Baseblock1x0.5Level1.fbx",
				"models\\PlatformMeshes\\SM_Baseblock1x1Level1.fbx",
				"models\\Props\\SM_EN_CagedWallLamp.fbx",
				"models\\Props\\SM_EN_Ceilinglamp.fbx",
				"models\\Props\\SM_EN_Crowbar.fbx",
				"models\\Props\\SM_EN_Pipe_1.fbx",
				"models\\Props\\SM_EN_Pipe_2.fbx",
				"models\\Props\\SM_EN_Pipe_3.fbx",
				"models\\Props\\Upgrade_Crates\\SM_Pistol_Upgrade_Crate.fbx",
				"models\\Rooms\\Level1\\AndreRooms\\Room20.1_8x20.fbx",
				"models\\Rooms\\Level1\\AndreRooms\\Room20_8x20.fbx",
				"models\\Rooms\\Level1\\AndreRooms\\Room21.1_8x20.fbx",
				"models\\Rooms\\Level1\\AndreRooms\\Room21_8x20.fbx",
				"models\\Rooms\\Level1\\AndreRooms\\Room22.1_8x20.fbx",
				"models\\Rooms\\Level1\\AndreRooms\\Room22_8x20.fbx",
				"models\\Rooms\\Level1\\AndreRooms\\Room23.1_8x20.fbx",
				"models\\Rooms\\Level1\\AndreRooms\\Room23_8x20.fbx",
				"models\\Rooms\\Level1\\AndreRooms\\Room24_8x20.fbx",
				"models\\Rooms\\Level1\\AndreRooms\\Room25.1_8x20.fbx",
				"models\\Rooms\\Level1\\AndreRooms\\Room25_8x20.fbx",
				"models\\Rooms\\Level1\\AndreRooms\\Room26_8x20.fbx",
				"models\\Rooms\\Level1\\AndreRooms\\Room27_8x20.fbx",
				"models\\Rooms\\Level1\\AndreRooms\\Room28_8x20.fbx",
				"models\\Rooms\\Level1\\AndreRooms\\Room29_8x20.fbx",
				"models\\Rooms\\Level1\\AndreRooms\\Room30_8x20.fbx",
				"models\\Rooms\\Level1\\AndreRooms\\Room31_8x20.fbx",
				"models\\Rooms\\Level1\\AndreRooms\\Room32.1_8x20.fbx",
				"models\\Rooms\\Level1\\AndreRooms\\Room32_8x20.fbx",
				"models\\Rooms\\Level1\\AndreRooms\\Room33_8x20.fbx",
				"models\\Rooms\\Level1\\AndreRooms\\Room34.1_8x20.fbx",
				"models\\Rooms\\Level1\\AndreRooms\\Room34_8x20.fbx",
				"models\\Rooms\\Level1\\SM_GunRoom3.1.fbx",
				"models\\Rooms\\Level1\\SM_Room10_8x20.fbx",
				"models\\Rooms\\Level1\\SM_Room1_1_8x20.fbx",
				"models\\Rooms\\Level1\\SM_Room1_2_8x20.fbx",
				"models\\Rooms\\Level1\\SM_Room2_8x20.fbx",
				"models\\Rooms\\Level1\\SM_Room3_8x20.fbx",
				"models\\Rooms\\Level1\\SM_Room4_8x20.fbx",
				"models\\Rooms\\Level1\\SM_Room5_8x20.fbx",
				"models\\Rooms\\Level1\\SM_Room6_8x20.fbx",
				"models\\Rooms\\Level1\\SM_Room7_8x20.fbx",
				"models\\Rooms\\Level1\\SM_Room8_8x20.fbx",
				"models\\Rooms\\Level1\\SM_Room9_8x20.fbx",
				"models\\Setdressing\\Level1\\SM_Setdressing_Level1_BoxGroup1.fbx",
				"models\\Setdressing\\Level1\\SM_Setdressing_Level1_End.fbx",
				"models\\Setdressing\\Level1\\SM_Setdressing_Level1_Large_Hangout.fbx",
				"models\\Setdressing\\Level1\\SM_Setdressing_Level1_Long_Hangout.fbx",
				"models\\Setdressing\\Level1\\SM_Setdressing_Level1_Pipes_and_Boxes.fbx",
				"models\\Setdressing\\Level1\\SM_Setdressing_Level1_Pipes_and_Doors.fbx",
				"models\\Setdressing\\Level1\\SM_Setdressing_Level1_Sidepipe.fbx",
				"models\\Setdressing\\Level1\\SM_Setdressing_Level1_Small_Hangoutspot.fbx",
				"models\\Setdressing\\Level1\\SM_Setdressing_Level1_Small_Ledge.fbx",
				"models\\Setdressing\\Level1\\SM_Setdressing_Level1_Small_Rooms.fbx",
				"models\\Setdressing\\Level1\\SM_Setdressing_Level1_WineCellar.fbx",
				"models\\Setdressing\\Level1\\Setdressing_Level1_Boxes_and_Gunes.fbx",
				"models\\Setdressing\\Level1\\Setdressing_Level1_Wine_and_Boxes.fbx",
				"testing\\PlaceHolderTutorials\\Tutorial1.fbx",
				"testing\\PlaceHolderTutorials\\Tutorial2.fbx",
				"testing\\PlaceHolderTutorials\\Tutorial3.fbx"
			};

			const char* paths2[]
			{
				"models\\BreakableMeshes\\SM_MetallicBoxDestruction_Beta.fbx",
				"models\\BreakableMeshes\\SM_WoodBoxDestruction_Beta.fbx",
				"models\\Characters\\SK_PlayerCharacter.fbx",
				"models\\Characters\\SM_ProjectileRevolver.fbx",
				"models\\Characters\\SM_Revolver.fbx",
				"models\\Characters\\SM_Shotgun.fbx",
				"models\\PlatformMeshes\\BackGround Lvl 2\\BackgroundRoomsLevel2\\SM_ArtRoom - A.fbx",
				"models\\PlatformMeshes\\BackGround Lvl 2\\BackgroundRoomsLevel2\\SM_ArtRoom - B.fbx",
				"models\\PlatformMeshes\\BackGround Lvl 2\\BackgroundRoomsLevel2\\SM_BossDoorRoom.fbx",
				"models\\PlatformMeshes\\BackGround Lvl 2\\BackgroundRoomsLevel2\\SM_CasinoRoom - A.fbx",
				"models\\PlatformMeshes\\BackGround Lvl 2\\BackgroundRoomsLevel2\\SM_CasinoRoom - B.fbx",
				"models\\PlatformMeshes\\BackGround Lvl 2\\BackgroundRoomsLevel2\\SM_CasinoRoom - C.fbx",
				"models\\PlatformMeshes\\BackGround Lvl 2\\BackgroundRoomsLevel2\\SM_CasinoRoom - D.fbx",
				"models\\PlatformMeshes\\BackGround Lvl 2\\BackgroundRoomsLevel2\\SM_ChandelierRoom.fbx",
				"models\\PlatformMeshes\\BackGround Lvl 2\\BackgroundRoomsLevel2\\SM_ElevatorsRoom.fbx",
				"models\\PlatformMeshes\\BackGround Lvl 2\\BackgroundRoomsLevel2\\SM_GamesRoom.fbx",
				"models\\PlatformMeshes\\BackGround Lvl 2\\BackgroundRoomsLevel2\\SM_HotelRoom - A.fbx",
				"models\\PlatformMeshes\\BackGround Lvl 2\\BackgroundRoomsLevel2\\SM_HotelRoom - B.fbx",
				"models\\PlatformMeshes\\BackGround Lvl 2\\BackgroundRoomsLevel2\\SM_HotelRoom - C.fbx",
				"models\\PlatformMeshes\\BackGround Lvl 2\\BackgroundRoomsLevel2\\SM_HotelRoom - D.fbx",
				"models\\PlatformMeshes\\BackGround Lvl 2\\BackgroundRoomsLevel2\\SM_LastRooms_A.fbx",
				"models\\PlatformMeshes\\BackGround Lvl 2\\SM_BackGround_1_Block_Lvl2.fbx",
				"models\\PlatformMeshes\\BackGround Lvl 2\\SM_BackGround_2_Block_Lvl2.fbx",
				"models\\PlatformMeshes\\BackGround Lvl 2\\SM_BackGround_4_Block_Lvl2.fbx",
				"models\\PlatformMeshes\\BackGround Lvl 2\\SM_SideWallBlock_Lvl2.fbx",
				"models\\PlatformMeshes\\Background_Level1\\SM_Backgroundplane2_4x4Level1.fbx",
				"models\\PlatformMeshes\\ChandelierSupport.fbx",
				"models\\PlatformMeshes\\Foreground\\Level2\\SM_ChandelierFOREGROUND.fbx",
				"models\\PlatformMeshes\\Foreground\\Level2\\SM_ChandelierWheelFOREGROUND.fbx",
				"models\\PlatformMeshes\\SM_Baseblock1x0.3Level2.fbx",
				"models\\PlatformMeshes\\SM_Baseblock1x0.5Level2.fbx",
				"models\\PlatformMeshes\\SM_Baseblock1x1Level2.fbx",
				"models\\PlatformMeshes\\SM_Baseblock1x1Level2VARIATION.fbx",
				"models\\PlatformMeshes\\SM_ChandelierWheel.fbx",
				"models\\PlatformMeshes\\SM_Chandelier_4_Level2.fbx",
				"models\\PlatformMeshes\\SM_Chandelier_5_Level2.fbx",
				"models\\PlatformMeshes\\SM_CornerPieceL.fbx",
				"models\\PlatformMeshes\\SM_CornerPieceR.fbx",
				"models\\PlatformMeshes\\SM_ForegroundCane.fbx",
				"models\\PlatformMeshes\\SM_Pool_3_Level2.fbx",
				"models\\Props\\SM_ArtPiece1.fbx",
				"models\\Props\\SM_ArtPiece2.fbx",
				"models\\Props\\SM_ArtPiece3.fbx",
				"models\\Props\\SM_ArtPiece4.fbx",
				"models\\Props\\SM_ArtPieceStefan.fbx",
				"models\\Props\\SM_ChandelierChains.fbx",
				"models\\Props\\SM_EN_Pipe_1.fbx",
				"models\\Props\\SM_EN_Pipe_2.fbx",
				"models\\Props\\SM_EN_Pipe_3.fbx",
				"models\\Props\\SM_HallLampCasino.fbx",
				"models\\Props\\SM_HotelDoorClosed.fbx",
				"models\\Props\\SM_LuggageCart.fbx",
				"models\\Props\\SM_PotPlant1.fbx",
				"models\\Props\\SM_PotPlant2.fbx",
				"models\\Props\\SM_PotPlant3.fbx",
				"models\\Props\\SM_Railing.fbx",
				"models\\Props\\SM_Roullette.fbx",
				"models\\Props\\SM_ServiceCart.fbx",
				"models\\Props\\SM_SideTableLamp.fbx",
				"models\\Props\\SM_Sign_A.fbx",
				"models\\Props\\SM_SlotMachine2.fbx",
				"models\\Props\\SM_StatueBoss - Afbx.fbx",
				"models\\Props\\Upgrade_Crates\\SM_Pistol_Upgrade_Crate.fbx",
				"testing\\PlaceHolderTutorials\\Tutorial3.fbx",
			};

			std::cout << "Start preload process" << '\n';

			for (const char* path : paths1)
			{
				if (globalKillPreload.load())
				{
					return;
				}

				std::cout << "Preload model " << path << '\n';

				Tga::ModelFactory::GetInstance().GetModelInstance(path);
				Tga::ModelFactory::GetInstance().GetAnimatedModelInstance(path);
			}

			for (const char* path : paths2)
			{
				if (globalKillPreload.load())
				{
					return;
				}

				std::cout << "Preload model " << path << '\n';

				Tga::ModelFactory::GetInstance().GetModelInstance(path);
				Tga::ModelFactory::GetInstance().GetAnimatedModelInstance(path);
			}
			});
	}

	void KillPreloadProcess()
	{
		globalKillPreload.store(true);
		globalThreadPool.Await();

		std::cout << "Killed preload process" << '\n';
	}

	void PackScene(const char* aPath)
	{
		std::filesystem::path path = (Tga::Settings::GameAssetRoot() / aPath);
		std::filesystem::path levelDataPath = path;
		std::filesystem::path packDataPath = path;

		std::filesystem::path levelDataRelativePath = aPath;

		levelDataPath.replace_extension(".leveldata");
		packDataPath.replace_extension(".pack");
		levelDataRelativePath.replace_extension(".leveldata");

		if (path.empty())
			return;

		if (path.extension().empty())
		{
			path = path.replace_extension(".tgs");
		}

		if (std::filesystem::is_directory(path.parent_path()) == false)
		{
			fs::permissions(path.parent_path().parent_path(), fs::perms::all);
			std::filesystem::create_directories(path.parent_path());
		}

		if (std::filesystem::is_directory(levelDataPath) == false)
		{
			fs::permissions(levelDataPath.parent_path(), fs::perms::all);
			std::filesystem::create_directories(levelDataPath);
		}

		nlohmann::json packJson;

		for (const fs::directory_entry& file : fs::directory_iterator(levelDataPath))
		{
			//std::cout << "Serializing file " << file << '\n';

			std::ifstream _if(file.path(), std::ios::in);

			if (_if.peek() == std::ifstream::traits_type::eof())
			{
				continue;
			}

			nlohmann::json objectData;
			_if >> objectData;

			std::string uuid = file.path().stem().string();

			packJson[uuid] = objectData;
			packJson[uuid]["uuid"] = uuid;
		}

		std::ofstream _of(packDataPath, std::ios::trunc);
		fs::permissions(packDataPath, fs::perms::all);
		_of << packJson.dump();

		std::cout << "Created pack " << packDataPath << '\n';
	}

	bool LoadSceneByPack(const char* aPath, Tga::Scene& outScene)
	{
		std::filesystem::path path = Tga::Settings::GameAssetRoot() / aPath;
		std::filesystem::path packDataPath = path;
		packDataPath.replace_extension(".pack");

		std::ifstream _if(packDataPath, std::ios::in);
		nlohmann::json packJson;
		_if >> packJson;

		std::filesystem::path resolvedTgsPath = Tga::Settings::GameAssetRoot() / aPath;

		std::string stem = resolvedTgsPath.stem().string();
		outScene.SetPath(aPath);
		outScene.SetName(stem.data());
		outScene.ClearScene();

		for (const nlohmann::json& objectData : packJson)
		{
			Tga::StringId objectPath = Tga::StringRegistry::RegisterOrGetString(objectData.value("path", "").data());

			Tga::SceneObject object;
			object.SetPath(nullptr, objectPath);
			object.SetName(objectData.value("name", "unknown").data());

			if (objectData.contains("translation"))
			{
				std::array<float, 3> translation = objectData["translation"].get<std::array<float, 3>>();
				object.GetTRS().translation = { translation[0], translation[1], translation[2] };
			}

			if (objectData.contains("rotation"))
			{
				std::array<float, 3> rotation = objectData["rotation"].get<std::array<float, 3>>();
				object.GetTRS().rotation = { rotation[0], rotation[1], rotation[2] };
			}

			if (objectData.contains("scale"))
			{
				std::array<float, 3> scale = objectData["scale"].get<std::array<float, 3>>();
				object.GetTRS().scale = { scale[0], scale[1], scale[2] };
			}

			if (objectData.contains("object-definition"))
			{
				object.SetSceneObjectDefintionName(Tga::StringRegistry::RegisterOrGetString(objectData["object-definition"].get<std::string>()));
			}

			std::vector<Tga::SceneProperty>& properties = object.EditPropertyOverrides();

			if (objectData.contains("properties"))
			{
				for (auto& propertyJson : objectData["properties"])
				{
					Tga::SceneProperty propertyDefinition = {};
					propertyDefinition.name = Tga::StringRegistry::RegisterOrGetString(propertyJson["name"]);
					propertyDefinition.type = Tga::PropertyTypeRegistry::GetPropertyType(Tga::StringRegistry::RegisterOrGetString(propertyJson["type"]));

					Tga::JsonData valueJson = { propertyJson["value"] };
					propertyDefinition.value = Tga::Property::CreateFromJson(propertyDefinition.type, valueJson);

					properties.push_back(propertyDefinition);
				}
			}

			outScene.CreateSceneObject(objectData["uuid"].get<std::string>().data(), object);
		}

		return true;
	}

	SceneConfig& SceneLoader::LoadSceneByPath(const char* aPath)
	{
		std::cout << "[SceneLoader.cpp] Trying to load scene..." << '\n';

		ScopedProfiler everything("load_scene_by_path");

		SceneConfig sceneConfig;
		sceneConfig.enemieyConfigs.reserve(32);
		sceneConfig.tileConfigs.reserve(2048);
		sceneConfig.modelConfigs.reserve(1024);
		sceneConfig.crateConfigs.reserve(256);
		sceneConfig.ambiences.reserve(32);

		Tga::Scene scene;

		bool hasLoadedMeleeEnemySharedData = false;
		bool hasLoadedRangedEnemySharedData = false;

		{
			globalActiveScenePath = aPath;
			globalIsSceneLoaded = true;

			if (globalSceneStore.contains(aPath))
			{
				globalSceneConfigCopy = globalSceneStore.at(aPath);
				return globalSceneConfigCopy;
			}

			ScopedProfiler profiler("load_scene_by_pack");

			SceneLoader::LoadSceneByPack(aPath, scene);
		}

		std::cout << "[SceneLoader.cpp] Parsing scene..." << '\n';

		std::vector<Tga::ScenePropertyDefinition> scenePropertyDefinitions;

		CompiledSceneObject cso;

		for (const auto& idAndSceneObject : scene.GetSceneObjects())
		{
			ScopedProfiler iterationProfiler("load_scene_object");

			scenePropertyDefinitions.clear();

			std::shared_ptr<Tga::SceneObject> sceneObject = idAndSceneObject.second;
			sceneObject->CalculateCombinedPropertySet(globalSceneDefinitionManager, scenePropertyDefinitions);

			{
				ScopedProfiler profiler("cso_load");
				cso.Load(sceneObject, scenePropertyDefinitions);
			}		

#if defined(_DEBUG)
			std::cout << "[SceneLoader.cpp] Loaded \"" << sceneObject->GetName() << "\"" << "\n";
#endif

			const Tga::StringId id = sceneObject->GetSceneObjectDefinitionName();

			if (id == "tile"_tgaid || id == "Tile"_tgaid ||
				id == "TileFull_Level1"_tgaid || id == "TileHalf_Level1"_tgaid || id == "TileSlim_Level1"_tgaid ||
				id == "TileFull_Level2"_tgaid || id == "TileHalf_Level2"_tgaid || id == "TileSlim_Level2"_tgaid ||
				id == "TileFullHighligh_Level2"_tgaid)
				{
				TileConfig tileConfig;

				tileConfig.modelInstance = cso.GetModelInstance("Model");
				tileConfig.size = cso.GetVector2fOrDefault("Size", Tga::Vector2f{ 100.0f, 100.0f });

				Tga::Vector3f position = tileConfig.modelInstance.GetTransform().GetPosition();
				tileConfig.position = Tga::Vector2f{ position.x, position.y };

				sceneConfig.tileConfigs.emplace_back(std::move(tileConfig));
			}
			else if (id == "Crate_Wood"_tgaid || id == "Crate_Metal"_tgaid)
			{
				CrateConfig crateConfig;

				crateConfig.modelInstance = cso.GetModelInstance("Model");
				crateConfig.animatedModelInstance = std::make_shared<Tga::AnimatedModelInstance>(cso.GetAnimatedModelInstance("Model"));
				crateConfig.breakClipReference = cso.GetAnimationClipReference("Break Clip");
				crateConfig.size = cso.GetVector2fOrDefault("Size", Tga::Vector2f{ 100.0f, 100.0f });
				crateConfig.metal = cso.GetBool("Is Metal");
				Tga::Vector3f position = crateConfig.modelInstance.GetTransform().GetPosition();
				crateConfig.position = Tga::Vector2f{ position.x, position.y };

				sceneConfig.crateConfigs.emplace_back(std::move(crateConfig));
			}
			else if (id == "Enemy_Melee"_tgaid || id == "Enemy_Ranged"_tgaid)
			{
				EnemyConfig enemyConfig;

				enemyConfig.modelInstance = std::make_shared<Tga::AnimatedModelInstance>(cso.GetAnimatedModelInstance("Model"));
				enemyConfig.modelInstanceDeath = std::make_shared<Tga::AnimatedModelInstance>(cso.GetAnimatedModelInstance("Death Model"));
				enemyConfig.meleeClipReference = cso.GetAnimationClipReference("Animation Melee");
				enemyConfig.idleClipReference = cso.GetAnimationClipReference("Animation Idle");
				enemyConfig.deathClipReference = cso.GetAnimationClipReference("Animation Death");

				if (id == "Enemy_Melee"_tgaid)
				{
					enemyConfig.hasGun = false;

					if (!hasLoadedMeleeEnemySharedData)
					{
						hasLoadedMeleeEnemySharedData = true;

						sceneConfig.enemySharedConfig.knockBackForce = cso.GetVector2f("Melee Knockback");
					}
				}
				else
				{
					enemyConfig.projectileModelInstance = cso.GetModelInstance("Projectile Model");
					enemyConfig.weaponModelInstance = cso.GetModelInstance("Weapon Model");
					enemyConfig.modelInstanceNoHand = std::make_shared<Tga::AnimatedModelInstance>(cso.GetAnimatedModelInstance("No Hand Model"));
					enemyConfig.hasGun = true;

					if (!hasLoadedRangedEnemySharedData)
					{
						hasLoadedRangedEnemySharedData = true;

						sceneConfig.enemySharedConfig.detectionRange = cso.GetFloat("Detection Range");
						sceneConfig.enemySharedConfig.detectionAngle = cso.GetFloat("Detection Angle");
						sceneConfig.enemySharedConfig.aimSpeed = cso.GetFloat("Aim Speed");
						sceneConfig.enemySharedConfig.distanceToFire = cso.GetFloat("Distance to Fire");
						sceneConfig.enemySharedConfig.shotCooldown = cso.GetFloat("Shot Cooldown");
						sceneConfig.enemySharedConfig.deathDuration = cso.GetFloat("Death Duration");
						sceneConfig.enemySharedConfig.projectileSpeed = cso.GetFloat("Projectile Speed");
						sceneConfig.enemySharedConfig.projectileKnockBackForce = cso.GetFloat("Projectile Knockback");
					}
				}

				Tga::Vector3f position = enemyConfig.modelInstance->GetTransform().GetPosition();
				enemyConfig.position = Tga::Vector2f{ position.x, position.y };
				sceneConfig.enemieyConfigs.emplace_back(std::move(enemyConfig));
			}
			else if (id == "Player"_tgaid)
			{
				CameraConfig cameraConfig;

				cameraConfig.depth = cso.GetFloat("CameraDepth");
				cameraConfig.height = cso.GetFloat("CameraHeight");
				cameraConfig.fov = cso.GetFloat("CameraFov");

				PlayerConfig playerConfig;

				playerConfig.modelInstance = cso.GetModelInstance("Model");
				playerConfig.shotgunModelInstance = cso.GetModelInstance("Shotgun Model");
				playerConfig.revolverModelInstance = cso.GetModelInstance("Revolver Model");

				playerConfig.playerArmPivotHeight = cso.GetFloat("Arm Pivot Height");
				playerConfig.playerGroundedGravity = cso.GetFloat("Grounded Gravity");
				playerConfig.playerGroundedFriction = cso.GetFloat("Grounded Friction");
				playerConfig.playerAirFriction = cso.GetFloat("Air Friction");
				playerConfig.stunDuration = cso.GetFloat("Stun Duration");
				playerConfig.bulletTimeTimeScale = cso.GetFloat("Bullet Time Timescale");
				playerConfig.bulletTimeLerpToSpeed = cso.GetFloat("Bullet Time Lerp To Speed");
				playerConfig.bulletTimeLerpFromSpeed = cso.GetFloat("Bullet Time Lerp From Speed");
				playerConfig.timeInBulletTime = cso.GetFloat("Bullet Time Time In Bullet Time");
				playerConfig.timeInBulletTime = cso.GetFloat("Bullet Time Time In Bullet Time");

				playerConfig.revolverData.maxClip = cso.GetInt("Revolver Clip");
				playerConfig.revolverData.timeBetweenShots = cso.GetFloat("Revolver Cooldown");
				playerConfig.revolverData.fallTime = cso.GetFloat("Revolver Fall Time");
				playerConfig.revolverData.gravityConstant = cso.GetFloat("Revolver Gravity");
				playerConfig.revolverData.maxDistance = cso.GetFloat("Revolver Knockback Distance");
				playerConfig.revolverData.timeToMaxDistance = cso.GetFloat("Revolver Knockback Duration");
				playerConfig.revolverData.range = cso.GetFloat("Revolver Range");

				playerConfig.shotgunData.maxClip = cso.GetInt("Shotgun Clip");
				playerConfig.shotgunData.timeBetweenShots = cso.GetFloat("Shotgun Cooldown");
				playerConfig.shotgunData.fallTime = cso.GetFloat("Shotgun Fall Time");
				playerConfig.shotgunData.gravityConstant = cso.GetFloat("Shotgun Gravity");
				playerConfig.shotgunData.maxDistance = cso.GetFloat("Shotgun Knockback Distance");
				playerConfig.shotgunData.timeToMaxDistance = cso.GetFloat("Shotgun Knockback Duration");
				playerConfig.shotgunData.range = cso.GetFloat("Shotgun Range");
				playerConfig.shotgunBulletAmount = cso.GetInt("Shotgun Bullet Amount");
				playerConfig.shotgunSpreadAngle = cso.GetFloat("Shotgun Bullet Spread Angle");
				playerConfig.shotgunHangtime = cso.GetFloat("Shotgun Hangtime");

				playerConfig.powerShotData.maxClip = cso.GetInt("Shotgun Clip");
				playerConfig.powerShotData.timeBetweenShots = cso.GetFloat("Power Shot Duration");
				playerConfig.powerShotData.fallTime = cso.GetFloat("Power Shot Fall Time");
				playerConfig.powerShotData.gravityConstant = cso.GetFloat("Power Shot Gravity");
				playerConfig.powerShotData.maxDistance = cso.GetFloat("Power Shot Knockback Distance");
				playerConfig.powerShotData.timeToMaxDistance = cso.GetFloat("Power Shot Knockback Duration");
				playerConfig.powerShotData.range = cso.GetFloat("Shotgun Range");

				Tga::Vector3f position = playerConfig.modelInstance.GetTransform().GetPosition();
				playerConfig.position = Tga::Vector2f{ position.x, position.y };
				sceneConfig.cameraConfig = cameraConfig;
				sceneConfig.playerConfig = playerConfig;
			}
			else if (id == "TheBoss"_tgaid)
			{
				BossConfig bossConfig;

				bossConfig.animatedModelInstance = std::make_shared<Tga::AnimatedModelInstance>(cso.GetAnimatedModelInstance("Model"));
				bossConfig.idleClipReference = cso.GetAnimationClipReference("IdleAnimation");
				
				sceneConfig.bossConfig = bossConfig;
			}
			else if (id == "Pickup"_tgaid)
			{
				bool isRevolverPickup = false;
				bool isShotgunPickup = false;
				bool isPowershotPickup = false;

				PickupConfig pickupConfig;

				pickupConfig.modelInstance = cso.GetModelInstance("Model");
				pickupConfig.size = cso.GetVector2fOrDefault("Size", Tga::Vector2f{ 100.0f, 100.0f });
				isShotgunPickup = cso.GetBool("Is Shotgun Pickup");
				isRevolverPickup = cso.GetBool("Is Revolver Pickup");
				isPowershotPickup = cso.GetBool("Is Powershot Pickup");

				Tga::Vector3f position = pickupConfig.modelInstance.GetTransform().GetPosition();
				pickupConfig.position = Tga::Vector2f{ position.x, position.y };

				const int typeCount = (isRevolverPickup + isShotgunPickup + isPowershotPickup);

				if (typeCount == 0)
				{
					std::cout << "[SceneLoader.cpp] Warning: Pickup " << sceneObject->GetName() << " has to have at least one type!" << '\n';
				}
				else if (typeCount > 1)
				{
					std::cout << "[SceneLoader.cpp] Warning: Pickup " << sceneObject->GetName() << " cannot be of more than one type!" << '\n';
				}
				else
				{
					if (isRevolverPickup)
					{
						pickupConfig.type = PickupType::Revolver;
					}

					if (isShotgunPickup)
					{
						pickupConfig.type = PickupType::Shotgun;
					}

					if (isPowershotPickup)
					{
						pickupConfig.type = PickupType::PowerShot;
					}
				}

				sceneConfig.pickupConfigs.emplace_back(std::move(pickupConfig));
			}
			else if (id == "LevelEndTrigger"_tgaid)
			{
				LevelTriggerConfig levelTriggerConfig;

				levelTriggerConfig.exists = true;
				levelTriggerConfig.modelInstance = std::make_shared<Tga::AnimatedModelInstance>(cso.GetAnimatedModelInstance("Model"));
				levelTriggerConfig.closeAnimation = cso.GetAnimationClipReference("Close");
				levelTriggerConfig.openAnimation = cso.GetAnimationClipReference("Open");
				levelTriggerConfig.size = cso.GetVector2fOrDefault("Size", Tga::Vector2f{ 200.0f, 200.0f });
				levelTriggerConfig.sceneToLoad = cso.GetStringId("Scene To Load");

				Tga::Vector3f position = levelTriggerConfig.modelInstance->GetTransform().GetPosition();
				levelTriggerConfig.position = Tga::Vector2f{ sceneObject->GetPosition().x, sceneObject->GetPosition().y };

				sceneConfig.levelTriggerConfig = levelTriggerConfig;
			}
			else if (id == "Ambience"_tgaid)
			{
				Ambience ambience;
				ambience.position = Tga::Vector2f{ sceneObject->GetPosition().x, sceneObject->GetPosition().y };

				ambience.path = cso.GetStringId("Path");
				ambience.volume = cso.GetFloat("Volume");
				ambience.maxVolumeDistance = cso.GetFloat("Max Volume Distance");
				ambience.muteDistance = cso.GetFloat("Mute Distance");

				sceneConfig.ambiences.push_back(ambience);
			}
			else if (id == "SceneMeta"_tgaid)
			{
				SceneMetaConfig sceneMetaConfig
				{
					SceneType::Unknown
				};

				bool isBossRoom = cso.GetBool("Is Boss Room");
				bool isLevel1 = cso.GetBool("Level1");
				bool isLevel2 = cso.GetBool("Level2");

				if (isBossRoom)
				{
					sceneMetaConfig.type = SceneType::BossScene;
				}
				else if (isLevel1)
				{
					sceneMetaConfig.type = SceneType::Level1;
				}
				else if (isLevel2)
				{
					sceneMetaConfig.type = SceneType::Level2;
				}

				sceneConfig.metaConfig = sceneMetaConfig;
			}
			else
			{
				ModelConfig modelConfig;
				modelConfig.modelInstance = cso.GetFirstModelInstance();

				sceneConfig.modelConfigs.push_back(modelConfig);
			}
		}

		cso.OutputUniqueModelPaths();

		globalSceneStore.insert({ aPath, sceneConfig });

		globalSceneConfigCopy = globalSceneStore.at(globalActiveScenePath);
		return globalSceneConfigCopy;
	}

	SceneConfig& GetActiveScene()
	{
		if (!globalIsSceneLoaded)
		{
			std::cout <<
				"[SceneLoader.cpp] SUPER DUPER ERROR: No scene is currently loaded, any access to scene config will crash."
				<< "\n";
			exit(-1);
		}

		return globalSceneConfigCopy;
	}
}