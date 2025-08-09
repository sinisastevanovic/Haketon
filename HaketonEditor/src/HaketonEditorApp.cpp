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

typedef Haketon::Application* (*CreateGameFunc)(Haketon::ApplicationCommandLineArgs args);
using DestroyGameFunc = void(*)(Haketon::Application*);

namespace Haketon
{
	class HaketonEditor : public EditorApplication
	{
	public:
		HaketonEditor(ApplicationCommandLineArgs args)
			: EditorApplication("Haketon Editor", args, true)
		{
			// Ensure ImGui context is properly shared across DLL boundary
			ShareImGuiContext();

			// Register editor-specific reflection types
			RegisterAllHaketonEditorTypes();
			RegisterHaketonEditorComponents();

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

	private:
		void LoadGame()
		{
			if (m_GameApp)
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

				auto createGame = (CreateGameFunc)GetProcAddress(m_GameLib, "CreateApplication");
				if (!createGame)
				{
					HK_CORE_ERROR("Could not load CreateApplication method");
					return;
				}

				char* gameArg[] = { const_cast<char*>("PIE") };
				m_GameApp = createGame({1, gameArg});
				HK_CORE_INFO("Game loaded successfully.");
			}
		}
		
		void UnloadGame()
		{
			HK_CORE_INFO("Unloading game dll...");
			if (m_GameApp)
			{
				DestroyGameFunc destroyGame = (DestroyGameFunc)GetProcAddress(m_GameLib, "DestroyApplication");
				destroyGame(m_GameApp);
				m_GameApp = nullptr;
			}
			if (m_GameLib)
			{
				FreeLibrary(m_GameLib);
				m_GameLib = nullptr;
			}
		}
		
		Haketon::Application* m_GameApp = nullptr;
		HMODULE m_GameLib = nullptr;
	};

	Application* CreateApplication(ApplicationCommandLineArgs args)
	{
		return new HaketonEditor(args);
	}
}
