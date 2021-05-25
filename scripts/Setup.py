import os
import subprocess
import CheckPython

CheckPython.ValidatePackages()

import Vulkan

os.chdir('../')

if(not Vulkan.CheckVulkanSDK()):
    print("Vulkan SDK not installed.")

if(not Vulkan.CheckVulkanSDKDebugLibs()):
    print("Vulkan SDK debug libs not found.")

print("Runnging premake...")
subprocess.call(["vendor/premake/bin/premake5.exe", "vs2019"])