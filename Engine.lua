-- This script defines all projects for the Haketon Engine SDK.
-- It is designed to be included by a game's premake5.lua file.

-- Include the engine's dependency definitions.
-- We pass our new root variable to the dependency script.
include (path.join(HAKETON_ENGINE_ROOT, "vendor/premake/premake_customization/solution_items.lua"))
include (path.join(HAKETON_ENGINE_ROOT, "Dependencies.lua"))

-- Define the dependency projects (GLFW, ImGui, etc.)
group "Dependencies"
	--include (path.join(HAKETON_ENGINE_ROOT, "vendor/premake"))
	include (path.join(HAKETON_ENGINE_ROOT, "Haketon/vendor/GLFW"))
	include (path.join(HAKETON_ENGINE_ROOT, "Haketon/vendor/Glad"))
	include (path.join(HAKETON_ENGINE_ROOT, "Haketon/vendor/imgui"))
group ""

-- Define the core engine and editor projects
include (path.join(HAKETON_ENGINE_ROOT, "Haketon"))
include (path.join(HAKETON_ENGINE_ROOT, "HaketonEditor"))
include (path.join(HAKETON_ENGINE_ROOT, "HaketonHeaderTool"))
include (path.join(HAKETON_ENGINE_ROOT, "HaketonHeaderTool.Tests"))