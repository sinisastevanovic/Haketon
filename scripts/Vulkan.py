import os
import subprocess
import sys
from pathlib import Path

import Utils

from io import BytesIO
from urllib.request import urlopen
from zipfile import ZipFile

VULKAN_SDK = os.environ.get('VULKAN_SDK')
HAKETON_VULKAN_VERSION = '1.3.296.0'
VULKAN_SDK_INSTALLER_URL = f"https://sdk.lunarg.com/sdk/download/{HAKETON_VULKAN_VERSION}/windows/VulkanSDK-{HAKETON_VULKAN_VERSION}-Installer.exe"
VULKAN_SDK_EXE_PATH = 'Haketon/vendor/VulkanSDK/VulkanSDK.exe'

def InstallVulkanSDK():
    print('Downloading {} to {}'.format(VULKAN_SDK_INSTALLER_URL, VULKAN_SDK_EXE_PATH))
    os.makedirs(os.path.dirname(VULKAN_SDK_EXE_PATH), exist_ok=True)
    Utils.DownloadFile(VULKAN_SDK_INSTALLER_URL, VULKAN_SDK_EXE_PATH)
    print("Done!")
    print("Running Vulkan SDK installer...")
    os.startfile(os.path.abspath(VULKAN_SDK_EXE_PATH))
    print("Re-run this script after installation")

def InstallVulkanPrompt():
    print("Would you like to install the Vulkan SDK?")
    install = Utils.YesOrNo()
    if(install):
        InstallVulkanSDK()
        quit()

def CheckVulkanSDK():
    if(VULKAN_SDK is None):
        print("You don't have the Vulkan SDK installed!")
        InstallVulkanPrompt()
        return False
    elif(HAKETON_VULKAN_VERSION not in VULKAN_SDK):
        print(f"Located Vulkan SDK at {VULKAN_SDK}")
        print(f"You don't have the correct Vulkan SDK version! (Haketon requires {HAKETON_VULKAN_VERSION})")
        InstallVulkanPrompt()
        return False

    print(f"Correct Vulkan SDK located at {VULKAN_SDK}")
    return True

VulkanSDKDebugLibsURL = f"https://files.lunarg.com/SDK-{HAKETON_VULKAN_VERSION}/VulkanSDK-{HAKETON_VULKAN_VERSION}-DebugLibs.zip"
OutputDirectory = "Haketon/vendor/VulkanSDK"
TempZipFile = f"{OutputDirectory}/VulkanSDK.zip"

def CheckVulkanSDKDebugLibs():
    shadercdLib = Path(f"{os.environ.get('VULKAN_SDK')}/Lib/shaderc_sharedd.lib")
    if(not shadercdLib.exists()):
        print(f"No Vulkan SDK debug libs found. (Checked {shadercdLib})")
        print("Downloading", VulkanSDKDebugLibsURL)
        with urlopen(VulkanSDKDebugLibsURL) as zipresp:
            with ZipFile(BytesIO(zipresp.read())) as zfile:
                zfile.extractall(OutputDirectory)

    print(f"Vulkan SDK debug libs located at {OutputDirectory}")
    return True