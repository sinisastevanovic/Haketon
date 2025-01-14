-- Haketon Dependencies

VULKAN_SDK = os.getenv("VULKAN_SDK")

IncludeDir = {}
IncludeDir["stb_image"] = "%{wks.location}/Haketon/vendor/stb_image"
IncludeDir["GLFW"] = "%{wks.location}/Haketon/vendor/GLFW/include"
IncludeDir["Glad"] = "%{wks.location}/Haketon/vendor/Glad/include"
IncludeDir["ImGui"] = "%{wks.location}/Haketon/vendor/ImGui"
IncludeDir["ImGuizmo"] = "%{wks.location}/Haketon/vendor/ImGuizmo"
IncludeDir["glm"] = "%{wks.location}/Haketon/vendor/glm"
IncludeDir["entt"] = "%{wks.location}/Haketon/vendor/entt/include"
IncludeDir["rapidjson"] = "%{wks.location}/Haketon/vendor/rapidjson/include"
IncludeDir["rttr"] = "%{wks.location}/Haketon/vendor/rttr/include"
IncludeDir["shaderc"] = "%{wks.location}/Haketon/vendor/shaderc/include"
IncludeDir["SPIRV_Cross"] = "%{wks.location}/Haketon/vendor/SPIRV-Cross"
IncludeDir["VulkanSDK"] = "%{VULKAN_SDK}/Include"

LibraryDir = {}
LibraryDir["VulkanSDK"] = "%{VULKAN_SDK}/Lib"
LibraryDir["VulkanSDK_Debug"] = "%{VULKAN_SDK}/Lib"
LibraryDir["VulkanSDK_DLL"] = "%{VULKAN_SDK}/Bin"

Library = {}
Library["Vulkan"] = "%{VULKAN_SDK}/Lib/vulkan-1.lib"

Library["ShaderC_Debug"] = "%{VULKAN_SDK}/Lib/shaderc_sharedd.lib"
Library["SPIRV_Cross_Debug"] = "%{VULKAN_SDK}/Lib/spirv-cross-cored.lib"
Library["SPIRV_Cross_GLSL_Debug"] = "%{VULKAN_SDK}/Lib/spirv-cross-glsld.lib"
Library["SPIRV_Tools_Debug"] = "%{VULKAN_SDK}/Lib/SPIRV-Toolsd.lib"

Library["ShaderC_Release"] = "%{VULKAN_SDK}/Lib/shaderc_shared.lib"
Library["SPIRV_Cross_Release"] = "%{VULKAN_SDK}/Lib/spirv-cross-core.lib"
Library["SPIRV_Cross_GLSL_Release"] = "%{VULKAN_SDK}/Lib/spirv-cross-glsl.lib"