#include <Haketon.h>
#include <Haketon/Core/EntryPoint.h>

#include "EditorLayer.h"
#include "Panels/PropertyEditorModule.h"
#include "Panels/DetailCustomization/TagComponentDetailCustomization.h"

namespace Haketon
{
	class HaketonEditor : public Application
	{
	public:
		HaketonEditor()
			: Application("Haketon Editor")
		{
			PushLayer(new EditorLayer());

			GetModuleManager()->AddModuleToList("PropertyEditor", new PropertyEditorModule());
			GetModuleManager()->StartupModule("PropertyEditor");

			PropertyEditorModule* PropertyEditor = ModuleManager::LoadModuleChecked<PropertyEditorModule>("PropertyEditor");
			PropertyEditor->RegisterDetailCustomization("TagComponent", []()
			{
            	return CreateRef<TagComponentDetailCustomization>();
			});
		}

		~HaketonEditor()
		{

		}
	};

	Application* CreateApplication()
	{
		return new HaketonEditor();
	}
}
