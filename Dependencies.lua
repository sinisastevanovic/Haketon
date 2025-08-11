-- Haketon Dependencies

VULKAN_SDK = os.getenv("VULKAN_SDK")
IncludeDir = {}
IncludeDir["spdlog"] = path.join(HAKETON_ENGINE_ROOT, "Haketon/vendor/spdlog/include")
IncludeDir["stb_image"] = path.join(HAKETON_ENGINE_ROOT, "Haketon/vendor/stb_image")
IncludeDir["GLFW"] = path.join(HAKETON_ENGINE_ROOT, "Haketon/vendor/GLFW/include")
IncludeDir["Glad"] = path.join(HAKETON_ENGINE_ROOT, "Haketon/vendor/Glad/include")
IncludeDir["ImGui"] = path.join(HAKETON_ENGINE_ROOT, "Haketon/vendor/imgui")
IncludeDir["ImGuizmo"] = path.join(HAKETON_ENGINE_ROOT, "Haketon/vendor/ImGuizmo")
IncludeDir["glm"] = path.join(HAKETON_ENGINE_ROOT, "Haketon/vendor/glm")
IncludeDir["entt"] = path.join(HAKETON_ENGINE_ROOT, "Haketon/vendor/entt/include")
IncludeDir["rapidjson"] = path.join(HAKETON_ENGINE_ROOT, "Haketon/vendor/rapidjson/include")
IncludeDir["rttr"] = path.join(HAKETON_ENGINE_ROOT, "Haketon/vendor/rttr/include")
IncludeDir["shaderc"] = path.join(HAKETON_ENGINE_ROOT, "Haketon/vendor/shaderc/include")
IncludeDir["SPIRV_Cross"] = path.join(HAKETON_ENGINE_ROOT, "Haketon/vendor/SPIRV-Cross")
IncludeDir["HaketonEngine"] = path.join(HAKETON_ENGINE_ROOT, "Haketon/src")
IncludeDir["HaketonEditor"] = path.join(HAKETON_ENGINE_ROOT, "HaketonEditor/src")
IncludeDir["VulkanSDK"] = "%{VULKAN_SDK}/Include"

LibraryDir = {}
LibraryDir["VulkanSDK"] = "%{VULKAN_SDK}/Lib"
LibraryDir["VulkanSDK_Debug"] = "%{VULKAN_SDK}/Lib"
LibraryDir["VulkanSDK_DLL"] = "%{VULKAN_SDK}/Bin"
LibraryDir["RTTRStatic"] = path.join(HAKETON_ENGINE_ROOT, "Haketon/vendor/rttr/static/lib")
LibraryDir["RTTRDllLib"] = path.join(HAKETON_ENGINE_ROOT, "Haketon/vendor/rttr/dll/lib")
LibraryDir["RTTRDllBin"] = path.join(HAKETON_ENGINE_ROOT, "Haketon/vendor/rttr/dll/bin")

Library = {}
Library["Vulkan"] = "%{VULKAN_SDK}/Lib/vulkan-1.lib"

Library["ShaderC_Debug"] = "%{VULKAN_SDK}/Lib/shaderc_sharedd.lib"
Library["SPIRV_Cross_Debug"] = "%{VULKAN_SDK}/Lib/spirv-cross-cored.lib"
Library["SPIRV_Cross_GLSL_Debug"] = "%{VULKAN_SDK}/Lib/spirv-cross-glsld.lib"
Library["SPIRV_Tools_Debug"] = "%{VULKAN_SDK}/Lib/SPIRV-Toolsd.lib"

Library["ShaderC_Release"] = "%{VULKAN_SDK}/Lib/shaderc_shared.lib"
Library["SPIRV_Cross_Release"] = "%{VULKAN_SDK}/Lib/spirv-cross-core.lib"
Library["SPIRV_Cross_GLSL_Release"] = "%{VULKAN_SDK}/Lib/spirv-cross-glsl.lib"