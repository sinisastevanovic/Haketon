#include <Haketon.h>
#include <Haketon/Core/EntryPoint.h>

#include "EditorLayer.h"

namespace Haketon
{
	class HaketonEditor : public Application
	{
	public:
		HaketonEditor()
			: Application("Haketon Editor")
		{
			PushLayer(new EditorLayer());
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
