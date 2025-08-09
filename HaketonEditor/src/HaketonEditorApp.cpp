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
		}

		~HaketonEditor()
		{
		}

	private:
		
	};

	Application* CreateApplication(ApplicationCommandLineArgs args)
	{
		return new HaketonEditor(args);
	}
}
