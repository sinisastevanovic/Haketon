#pragma once

#include <Haketon.h>
#include <windows.h>

#include "Events/EditorAssetEvents.h"
#include "Events/EditorSceneEvents.h"
#include "Haketon/Events/SceneEvents.h"

namespace Haketon
{
	class IApplicationContext;

	class HaketonEditor : public Application
	{
	public:
		HaketonEditor(ApplicationCommandLineArgs args);
		~HaketonEditor() override;

		void OnEvent(Event& e) override;
		void RunImpl() override;
		void Shutdown() override;

	private:
		// Scene event handlers
		bool OnOpenScene(SceneOpenEvent& e);
		bool OnSceneNewEvent(SceneNewEvent& e);
		bool OnSceneSave(SceneSaveEvent& e);
		bool OnSceneSaveAs(SceneSaveAsEvent& e);
		bool OnScenePlayEvent(ScenePlayEvent& e);
		bool OnScenePauseEvent(ScenePauseEvent& e);
		bool OnSceneStopEvent(SceneStopEvent& e);
		bool OnAssetOpenEvent(AssetOpenEvent& e);

		// Project event handlers
		bool OnNewProject(ProjectNewEvent& e);
		bool OnOpenProject(ProjectOpenEvent& e);

		// Helper methods
		bool OpenScene(const std::filesystem::path& path);
		bool SaveSceneAs();
		bool OpenProject(const std::string& projectPath);
		void LoadGame();
		void UnloadGame();

		// Member variables
		IApplicationContext* m_ActiveGameContext = nullptr;
		HMODULE m_GameLib = nullptr;
		bool m_DeferredAttachGameLayers = false;

		Ref<Scene> m_EditorScene;
		Ref<Scene> m_RuntimeScene;
		Ref<Project> m_CurrentProject;
	};
}
