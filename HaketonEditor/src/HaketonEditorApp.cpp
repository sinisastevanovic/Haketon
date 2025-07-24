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
#include "GeneratedFiles/AutoReflection.gen.h"

namespace Haketon
{
	class HaketonEditor : public Application
	{
	public:
		HaketonEditor(ApplicationCommandLineArgs args)
			: Application("Haketon Editor", args, true)
		{
			// Register editor-specific reflection types
			// TODO: Find another way... This is ugly. Also having these generated files inside the project sucks
			RegisterAllHaketonEditorTypes();
			
			PushLayer(new EditorLayer());

			GetModuleManager()->AddModuleToList("PropertyEditor", new PropertyEditorModule());
			GetModuleManager()->StartupModule("PropertyEditor");

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
		}

		~HaketonEditor()
		{

		}
	};

	Application* CreateApplication(ApplicationCommandLineArgs args)
	{
		return new HaketonEditor(args);
	}
}
