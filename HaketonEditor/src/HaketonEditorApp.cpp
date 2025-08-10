#include <Haketon.h>
#include <Haketon/Core/EntryPoint.h>

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
#include "DetailCustomization/Components/NativeScriptComponentDetailCustomization.h"
#include "GeneratedFiles/AutoReflection.gen.h"
#include "GeneratedFiles/HaketonEditorComponentSerialization.gen.h"

// TODO: This should only be done when truly running on windows
#include <windows.h>

#include "Events/EditorSceneEvents.h"
#include "Haketon/Core/IApplicationContext.h"
#include "Haketon/Core/Serialization/RapidJsonDeserializer.h"
#include "Haketon/Core/Serialization/RapidJsonSerializer.h"
#include "Haketon/Events/SceneEvents.h"
#include "Haketon/Utils/PlatformUtils.h"

using AttachFunc = Haketon::IApplicationContext* (*)(Haketon::Application*);
using DetachFunc = void (*)(Haketon::IApplicationContext*);

typedef Haketon::Application* (*CreateGameFunc)(Haketon::ApplicationCommandLineArgs args);
using DestroyGameFunc = void(*)(Haketon::Application*);

namespace Haketon
{
	class HaketonEditor : public Application
	{
	public:
		HaketonEditor(ApplicationCommandLineArgs args)
			: Application("Haketon Editor", args, true)
		{
			// Ensure ImGui context is properly shared across DLL boundary
			ShareImGuiContext();

			// Register editor-specific reflection types
			RegisterAllHaketonEditorTypes();
			RegisterHaketonEditorComponents();

			m_EditorScene = CreateRef<Scene>();
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
			
			PropertyEditor->RegisterComponentContentCustomization("NativeScriptComponent", []()
			{
            	return CreateRef<NativeScriptComponentDetailCustomization>();
			});

			LoadGame();
		}

		~HaketonEditor()
		{
			UnloadGame();
		}

		void OnEvent(Event& e) override
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
		}

		void RunImpl() override
		{
			if (m_AttachGameDeferred)
			{
				m_AttachGameDeferred = false;
				if (!m_GameLib)
				{
					HK_CORE_ERROR("No game loaded!");
					return;
				}

				auto attachFunc = (AttachFunc)GetProcAddress(m_GameLib, "AttachGameToHost");
				if (!attachFunc)
				{
					HK_CORE_ERROR("Could not load AttachGameToHost method");
					return;
				}

				m_ActiveGameContext = attachFunc(this);
			}
			else if (m_DetachGameDeferred)
			{
				m_DetachGameDeferred = false;
				if (!m_GameLib)
				{
					HK_CORE_ERROR("No game loaded!");
					return;
				}

				DetachFunc detachFunc = (DetachFunc)GetProcAddress(m_GameLib, "DetachGameFromHost");
				if (!detachFunc)
				{
					HK_CORE_ERROR("Could not load DetachGameFromHost method");
					return;
				}
			
				for (auto* layer : m_ActiveGameContext->CreatedLayers)
					PopLayer(layer);

				for (auto* layer : m_ActiveGameContext->CreatedOverlays)
					PopOverlay(layer);

				detachFunc(m_ActiveGameContext);
				m_ActiveGameContext = nullptr;
			}

			if (m_StopSceneDeferred)
			{
				m_RuntimeScene = nullptr;
				m_ActiveScene = m_EditorScene;
			}
		}

		bool OnOpenScene(SceneOpenEvent& e)
		{
			std::string path = e.GetPath();
			std::filesystem::path filePath(path);
			if (!filePath.empty() && std::filesystem::exists(filePath))
			{
				m_EditorScene = CreateRef<Scene>(path, filePath.stem().string());

				RapidJsonDeserializer rd;
				rd.ParseFile(path);
				rd.DeserializeScene(m_EditorScene);

				m_ActiveScene = m_EditorScene;

				ActiveSceneChangedEvent event;
				Application::OnEvent(event);
			}
			return true;
		}

		bool OnSceneNewEvent(SceneNewEvent& e)
		{
			if (m_ActiveScene == m_RuntimeScene)
				return false;

			m_EditorScene = CreateRef<Scene>();
			m_ActiveScene = m_EditorScene;
			ActiveSceneChangedEvent event;
			Application::OnEvent(event);
			return true;
		}

		bool SaveSceneAs()
		{
			if (m_ActiveScene == m_RuntimeScene)
				return false;
			
			std::filesystem::path filePath(FileDialogs::SaveFile("Haketon Scene (*.haketon)\0*.haketon\0"));
			if(!filePath.empty())
			{
				RapidJsonSerializer rs;
				rs.SerializeScene(m_ActiveScene);
				std::string Result = rs.GetString();

				std::ofstream Fout(filePath);
				Fout << Result.c_str();
			
				m_ActiveScene->SetPath(filePath.string());
				m_ActiveScene->SetName(filePath.stem().string());
				ActiveSceneChangedEvent event;
				Application::OnEvent(event);
				return true;
			}

			return false;
		}

		bool OnSceneSave(SceneSaveEvent& e)
		{
			if (m_ActiveScene == m_RuntimeScene)
				return true;
			
			if (m_ActiveScene != nullptr)
			{
				if (m_ActiveScene->IsTransient())
				{
					SaveSceneAs();
				}
				else
				{
					RapidJsonSerializer rs;
					rs.SerializeScene(m_ActiveScene);
					std::string Result = rs.GetString();
					if(m_ActiveScene->GetPath().length() > 0)
					{
						std::filesystem::path Path = m_ActiveScene->GetPath();
						if(!std::filesystem::exists(Path.parent_path()))
							std::filesystem::create_directory(Path.parent_path());

						std::ofstream Fout(m_ActiveScene->GetPath());
						Fout << Result.c_str();
					}
				}
			}
			return true;
		}

		bool OnSceneSaveAs(SceneSaveAsEvent& e)
		{
			SaveSceneAs();
			return true;
		}
		
		bool OnScenePlayEvent(ScenePlayEvent& e)
		{
			m_RuntimeScene = Scene::Copy(m_EditorScene);
			m_ActiveScene = m_RuntimeScene;
			m_ActiveScene->SetPaused(false);
			
			m_AttachGameDeferred = true;
			m_DetachGameDeferred = false;
			return true;
		}

		bool OnScenePauseEvent(ScenePauseEvent& e)
		{
			if (m_ActiveScene == m_RuntimeScene)
			{
				m_ActiveScene->SetPaused(e.Paused());
			}
			return true;
		}

		bool OnSceneStopEvent(SceneStopEvent& e)
		{
			m_DetachGameDeferred = true;
			m_AttachGameDeferred = false;

			m_StopSceneDeferred = true;
			return true;
		}

	private:
		void LoadGame()
		{
			if (m_ActiveGameContext)
				return;
		
			auto args = GetCommandLineArgs();
			if (args.Count == 2)
			{
				std::string gameLocation = args.Args[1];
				if (gameLocation.length() == 0)
				{
					HK_CORE_WARN("No game location specified");
					return;
				}

				HK_CORE_INFO("Loading game dll...");
				m_GameLib = LoadLibraryA(gameLocation.c_str());
				if (!m_GameLib)
				{
					HK_CORE_ERROR("Could not load TestGame.dll");
					return;
				}

				/*auto attachFunc = (AttachFunc)GetProcAddress(m_GameLib, "AttachGameToHost");
				if (!attachFunc)
				{
					HK_CORE_ERROR("Could not load AttachGameToHost method");
					return;
				}

				m_ActiveGameContext = attachFunc(this);*/
				HK_CORE_INFO("Game loaded successfully.");
			}
		}
		
		void UnloadGame()
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

		IApplicationContext* m_ActiveGameContext = nullptr;
		HMODULE m_GameLib = nullptr;
		bool m_AttachGameDeferred = false;
		bool m_DetachGameDeferred = false;
		bool m_StopSceneDeferred = false;

		Ref<Scene> m_EditorScene;
		Ref<Scene> m_RuntimeScene;
	};

	Application* CreateApplication(ApplicationCommandLineArgs args)
	{
		return new HaketonEditor(args);
	}
}
