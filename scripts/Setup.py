import os
import subprocess
import CheckPython
import winreg

CheckPython.ValidatePackages()

import Vulkan

os.chdir('../')

# Set HAKETON_ENGINE_PATH environment variable
def SetEnginePathEnvironmentVariable():
    engine_path = os.path.abspath('.')
    print(f"Setting HAKETON_ENGINE_PATH to: {engine_path}")
    
    try:
        # Set for current user
        key = winreg.OpenKey(winreg.HKEY_CURRENT_USER, r"Environment", 0, winreg.KEY_SET_VALUE)
        winreg.SetValueEx(key, "HAKETON_ENGINE_PATH", 0, winreg.REG_SZ, engine_path)
        winreg.CloseKey(key)
        
        # Also set for current session
        os.environ["HAKETON_ENGINE_PATH"] = engine_path
        
        print("✓ HAKETON_ENGINE_PATH environment variable set successfully!")
        print("  Note: You may need to restart your IDE/terminal for the change to take effect.")
        
    except Exception as e:
        print(f"⚠ Warning: Could not set environment variable: {e}")
        print(f"  Please manually set HAKETON_ENGINE_PATH to: {engine_path}")

SetEnginePathEnvironmentVariable()

if(not Vulkan.CheckVulkanSDK()):
    print("Vulkan SDK not installed.")

if(not Vulkan.CheckVulkanSDKDebugLibs()):
    print("Vulkan SDK debug libs not found.")

print("Runnging premake...")
subprocess.call(["vendor/premake/bin/premake5.exe", "vs2022"])