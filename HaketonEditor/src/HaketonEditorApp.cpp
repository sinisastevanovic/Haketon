#include "HaketonEditorApp.h"

#include "Haketon/Core/EntryPoint.h"
#include "EditorLayer.h"

#include "Modules/PropertyEditorModule.h"
#include "DetailCustomization/Components/TagComponentDetailCustomization.h"
#include "DetailCustomization/Properties/Vec2DetailCustomization.h"
#include "DetailCustomization/Properties/Vec3DetailCustomization.h"
#include "DetailCustomization/Properties/Vec4DetailCustomization.h"
#include "DetailCustomization/Properties/IVec2DetailCustomization.h"
#include "DetailCustomization/Properties/IVec3DetailCustomization.h"
#include "DetailCustomization/Properties/IVec4DetailCustomization.h"
#include "DetailCustomization/Properties/QuatDetailCustomization.h"
#include "DetailCustomization/Properties/ColorDetailCustomization.h"
#include "DetailCustomization/Properties/AssetHandlePropertyDetailCustomization.h"
#include "DetailCustomization/Components/NativeScriptComponentDetailCustomization.h"

#include "Haketon/Core/IApplicationContext.h"
#include "Haketon/Core/Serialization/RapidJsonDeserializer.h"
#include "Haketon/Core/Serialization/RapidJsonSerializer.h"
#include "Haketon/Utils/PlatformUtils.h"
#include "Haketon/Asset/AssetManager.h"

#include "GeneratedFiles/AutoReflection.gen.h"
#include "GeneratedFiles/HaketonEditorComponentSerialization.gen.h"

// TODO: This should only be done when truly running on windows
#include <windows.h>


using AttachFunc = Haketon::IApplicationContext* (*)(Haketon::Application*);
using DetachFunc = void (*)(Haketon::IApplicationContext*);

namespace Haketon
{
	HaketonEditor::HaketonEditor(ApplicationCommandLineArgs args)
		: Application("Haketon Editor", args, true)
	{
		// Ensure ImGui context is properly shared across DLL boundary
		ShareImGuiContext();

		// Register editor-specific reflection types
		RegisterAllHaketonEditorTypes();
		RegisterHaketonEditorComponents();

		m_EditorScene = AssetManager::CreateTransientAsset<Scene>();
		m_ActiveScene = m_EditorScene;

		PushLayer(new EditorLayer());

		ModuleManager::Get().AddModuleToList("PropertyEditor", new PropertyEditorModule());
		ModuleManager::Get().StartupModule("PropertyEditor");

		PropertyEditorModule* PropertyEditor = ModuleManager::LoadModuleChecked<PropertyEditorModule>("PropertyEditor");
		PropertyEditor->RegisterDetailCustomization("TagComponent", []()
		{
            return CreateRef<TagComponentDetailCustomization>();
		});
		
		PropertyEditor->RegisterPropertyDetailCustomization("Vector2", []()
		{
            return CreateRef<Vec2PropertyDetailCustomization>();
		});
		
		PropertyEditor->RegisterPropertyDetailCustomization("Vector3", []()
		{
            return CreateRef<Vec3PropertyDetailCustomization>();
		});
		
		PropertyEditor->RegisterPropertyDetailCustomization("Vector4", []()
		{
            return CreateRef<Vec4PropertyDetailCustomization>();
		});
		
		PropertyEditor->RegisterPropertyDetailCustomization("IVector2", []()
		{
            return CreateRef<IVec2PropertyDetailCustomization>();
		});
		
		PropertyEditor->RegisterPropertyDetailCustomization("IVector3", []()
		{
            return CreateRef<IVec3PropertyDetailCustomization>();
		});
		
		PropertyEditor->RegisterPropertyDetailCustomization("IVector4", []()
		{
            return CreateRef<IVec4PropertyDetailCustomization>();
		});
		
		PropertyEditor->RegisterPropertyDetailCustomization("Quaternion", []()
		{
            return CreateRef<QuatPropertyDetailCustomization>();
		});
		
		PropertyEditor->RegisterPropertyDetailCustomization("FColor", []()
		{
            return CreateRef<ColorPropertyDetailCustomization>();
		});
		
		PropertyEditor->RegisterPropertyDetailCustomization("AssetHandle", []()
		{
            return CreateRef<AssetHandlePropertyDetailCustomization>();
		});
		
		PropertyEditor->RegisterComponentContentCustomization("NativeScriptComponent", []()
		{
            return CreateRef<NativeScriptComponentDetailCustomization>();
		});

		if (args.Count == 2)
		{
			std::string gameLocation = args.Args[1];
			if (gameLocation.length() == 0)
			{
				HK_CORE_WARN("No game location specified");
			}
			else
			{
				OpenProject(gameLocation);
			}
		}
	}

	HaketonEditor::~HaketonEditor()
	{
		// Critical: Clean up entities FIRST while game DLL is still loaded
		// (entities may have components defined in the DLL)
		if (m_RuntimeScene)
		{
			m_RuntimeScene->DestroyAllEntities();
			m_RuntimeScene.reset();
		}
		
		if (m_EditorScene)
		{
			m_EditorScene->DestroyAllEntities();
			m_EditorScene.reset();
		}
		
		// Now safe to unload game DLL after all entities are destroyed
		UnloadGame();
		
		// Reset ImGui context sharing flag so context can be properly destroyed
		// when the base Application destructor runs
		auto* imguiLayer = GetImGuiLayer();
		if (imguiLayer)
		{
			imguiLayer->SetContextSharedWithDLL(false);
			HK_CORE_INFO("ImGui context sharing reset - context can now be destroyed");
		}
		
		UnregisterHaketonEditorComponents();
	}

	void HaketonEditor::OnEvent(Event& e)
	{
		Application::OnEvent(e);

		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<ScenePlayEvent>(HK_BIND_EVENT_FN(OnScenePlayEvent));
		dispatcher.Dispatch<ScenePauseEvent>(HK_BIND_EVENT_FN(OnScenePauseEvent));
		dispatcher.Dispatch<SceneStopEvent>(HK_BIND_EVENT_FN(OnSceneStopEvent));
		dispatcher.Dispatch<SceneOpenEvent>(HK_BIND_EVENT_FN(OnOpenScene));
		dispatcher.Dispatch<SceneNewEvent>(HK_BIND_EVENT_FN(OnSceneNewEvent));
		dispatcher.Dispatch<SceneSaveEvent>(HK_BIND_EVENT_FN(OnSceneSave));
		dispatcher.Dispatch<SceneSaveAsEvent>(HK_BIND_EVENT_FN(OnSceneSaveAs));
		dispatcher.Dispatch<ProjectNewEvent>(HK_BIND_EVENT_FN(OnNewProject));
		dispatcher.Dispatch<ProjectOpenEvent>(HK_BIND_EVENT_FN(OnOpenProject));
		dispatcher.Dispatch<AssetOpenEvent>(HK_BIND_EVENT_FN(OnAssetOpenEvent));
	}

	void HaketonEditor::RunImpl()
	{
		if (m_DeferredAttachGameLayers)
		{
			m_DeferredAttachGameLayers = false;
			if (m_ActiveGameContext)
			{
				for (auto layer : m_ActiveGameContext->CreatedLayers)
				{
					PushLayer(layer);
				}

				for (auto layer : m_ActiveGameContext->CreatedOverlays)
				{
					PushOverlay(layer);
				}
			}
		}
	}

	void HaketonEditor::Shutdown()
	{
		auto* imguiLayer = GetImGuiLayer();
		if (imguiLayer)
		{
			imguiLayer->SetContextSharedWithDLL(false);
			HK_CORE_INFO("ImGui context sharing reset - context can now be destroyed");
		}
		
		// Scene cleanup is handled in destructor to ensure proper DLL cleanup order
		Application::Shutdown();
	}

	bool HaketonEditor::OnOpenScene(SceneOpenEvent& e)
	{
		std::string path = FileDialogs::OpenFile(AssetUtils::GetFilterForAssetType(AssetType::Scene));
		std::filesystem::path filePath(path);
		return OpenScene(filePath);
	}

	bool HaketonEditor::OnSceneNewEvent(SceneNewEvent& e)
	{
		if (m_ActiveScene == m_RuntimeScene)
			return false;

		// Clean up old editor scene before replacing it
		if (m_EditorScene)
		{
			m_EditorScene->DestroyAllEntities();
		}
		
		m_EditorScene = AssetManager::CreateTransientAsset<Scene>();
		m_ActiveScene = m_EditorScene;
		ActiveSceneChangedEvent event;
		Application::OnEvent(event);
		return true;
	}

	bool HaketonEditor::SaveSceneAs()
	{
		if (m_ActiveScene == m_RuntimeScene)
			return false;
		
		std::filesystem::path filePath(FileDialogs::SaveFile(AssetUtils::GetFilterForAssetType(AssetType::Scene)));
		if(!filePath.empty())
		{
			RapidJsonSerializer rs;
			rs.SerializeScene(m_ActiveScene.get());
			rs.SaveToFile(filePath);

			AssetHandle newAssetHandle = AssetManager::ImportAsset(filePath);
			if (newAssetHandle.IsValid())
			{
				m_ActiveScene = AssetManager::GetAsset<Scene>(newAssetHandle);
				m_EditorScene = m_ActiveScene;
				ActiveSceneChangedEvent event;
				Application::OnEvent(event);
			}
			return true;
		}

		return false;
	}

	bool HaketonEditor::OnSceneSave(SceneSaveEvent& e)
	{
		if (m_ActiveScene == m_RuntimeScene)
			return true;
		
		if (m_ActiveScene != nullptr)
		{
			if (AssetManager::IsTransientAsset(m_ActiveScene->GetHandle()))
			{
				SaveSceneAs();
			}
			else
			{
				RapidJsonSerializer rs;
				rs.SerializeScene(m_ActiveScene.get());
				rs.SaveToFile(m_ActiveScene->GetPath());
				// TODO: We need to let the AssetManager know this asset changed!
			}
		}
		return true;
	}

	bool HaketonEditor::OnSceneSaveAs(SceneSaveAsEvent& e)
	{
		SaveSceneAs();
		return true;
	}
	
	bool HaketonEditor::OnScenePlayEvent(ScenePlayEvent& e)
	{
		m_RuntimeScene = Scene::Copy(m_EditorScene.get());
		m_ActiveScene = m_RuntimeScene;
		m_ActiveScene->SetPaused(false);
		ActiveSceneChangedEvent event;
		Application::OnEvent(event);

		return true;
	}

	bool HaketonEditor::OnScenePauseEvent(ScenePauseEvent& e)
	{
		if (m_ActiveScene == m_RuntimeScene)
		{
			m_ActiveScene->SetPaused(e.Paused());
		}
		return true;
	}

	bool HaketonEditor::OnSceneStopEvent(SceneStopEvent& e)
	{
		// Properly clean up runtime scene before destroying it
		if (m_RuntimeScene)
		{
			m_RuntimeScene->DestroyAllEntities();
		}
		
		m_RuntimeScene = nullptr;
		m_ActiveScene = m_EditorScene;
		ActiveSceneChangedEvent event;
		Application::OnEvent(event);
		return true;
	}

	bool HaketonEditor::OnAssetOpenEvent(AssetOpenEvent& e)
	{
		AssetHandle handle = e.GetHandle();
		if (handle == AssetHandle::Null())
			return true;

		const auto metaData = AssetManager::GetMetadata(handle);
		if (!metaData)
			return true;

		switch (metaData->Type)
		{
			case AssetType::Scene:
			{
				if (m_EditorScene)
				{
					m_EditorScene->DestroyAllEntities();
				}

				m_EditorScene = AssetManager::GetAsset<Scene>(handle);
				m_ActiveScene = m_EditorScene;

				ActiveSceneChangedEvent event;
				Application::OnEvent(event);
				break;
			}
			case AssetType::None:
			case AssetType::Texture:
			case AssetType::Mesh:
			case AssetType::Material:
			default: break;
		}

		return true;
	}

	bool HaketonEditor::OnNewProject(ProjectNewEvent& e)
	{
		std::string projectPath = FileDialogs::SaveFile("Haketon Project (*.hkproject)\0*.hkproject\0");
		if (!projectPath.empty())
		{
			std::string projectName = std::filesystem::path(projectPath).stem().string();
			m_CurrentProject = Project::New(projectPath, projectName);
		
			if (m_CurrentProject)
			{
				HK_CORE_INFO("Created new project: {0}", projectName);
			}

			LoadGame();
		}
		return true;
	}

	bool HaketonEditor::OnOpenProject(ProjectOpenEvent& e)
	{
		std::string projectPath = FileDialogs::OpenFile("Haketon Project (*.hkproject)\0*.hkproject\0");
		OpenProject(projectPath);
		return true;
	}

	bool HaketonEditor::OpenScene(const std::filesystem::path& path)
	{
		return true;
	}

	bool HaketonEditor::OpenProject(const std::string& projectPath)
	{
		if (!projectPath.empty())
		{
			m_CurrentProject = Project::Load(projectPath);
		
			if (m_CurrentProject)
			{
				SetWindowTitle("Haketon Editor - " + m_CurrentProject->GetConfig().Name);
				HK_CORE_INFO("Opened project: {0}", m_CurrentProject->GetConfig().Name);

				//OpenScene(m_CurrentProject->GetAssetDirectory() + "/scenes/" + m_CurrentProject->GetConfig().StartupScene);
				LoadGame();
				
			}
		}
		return true;
	}

	void HaketonEditor::LoadGame()
	{
		if (m_ActiveGameContext)
			return;

		if (m_CurrentProject)
		{
			auto dllPath = m_CurrentProject->GetDllPath();
			if (!std::filesystem::exists(dllPath))
			{
				HK_CORE_ERROR("Game dll not found, please build project first: {}", dllPath);
				return;
			}

			HK_CORE_INFO("Loading game dll...");
			m_GameLib = LoadLibraryA(dllPath.c_str());
			if (!m_GameLib)
			{
				HK_CORE_ERROR("Could not load TestGame.dll");
				return;
			}

			auto attachFunc = (AttachFunc)GetProcAddress(m_GameLib, "AttachGameToHost");
			if (!attachFunc)
			{
				HK_CORE_ERROR("Could not load AttachGameToHost method");
				return;
			}

			m_ActiveGameContext = attachFunc(this);
			HK_CORE_INFO("Game loaded successfully.");

			AssetManager::Init();
			
			m_DeferredAttachGameLayers = true;
			CurrentProjectChangedEvent event(m_CurrentProject.get());
			Application::OnEvent(event);
		}
	}
	
	void HaketonEditor::UnloadGame()
	{
		HK_CORE_INFO("Unloading game dll...");
		
		if (m_GameLib)
		{
			DetachFunc detachFunc = (DetachFunc)GetProcAddress(m_GameLib, "DetachGameFromHost");
			if (!detachFunc)
			{
				HK_CORE_ERROR("Could not load DetachGameFromHost method");
				return;
			}

			AssetManager::Shutdown();
			
			if (m_ActiveGameContext)
			{
				for (auto* layer : m_ActiveGameContext->CreatedLayers)
					PopLayer(layer);

				for (auto* layer : m_ActiveGameContext->CreatedOverlays)
					PopOverlay(layer);

				detachFunc(m_ActiveGameContext);
				m_ActiveGameContext = nullptr;
			}

			FreeLibrary(m_GameLib);
			m_GameLib = nullptr;
		}
	}

	Application* CreateApplication(ApplicationCommandLineArgs args)
	{
		return new HaketonEditor(args);
	}
}
