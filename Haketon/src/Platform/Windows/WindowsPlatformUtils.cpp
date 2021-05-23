#include "hkpch.h"
#include "Haketon/Utils/PlatformUtils.h"
#include "Haketon/Core/Application.h"

#include <commdlg.h>
#include <GLFW/glfw3.h>
#include <combaseapi.h>

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>


namespace Haketon
{
    std::string FileDialogs::OpenFile(const char* Filter)
    {
        OPENFILENAMEA ofn;          
        CHAR szFile[260] = { 0 };   
        ZeroMemory(&ofn, sizeof(OPENFILENAME));
        ofn.lStructSize = sizeof(OPENFILENAME);
        ofn.hwndOwner = glfwGetWin32Window((GLFWwindow*)Application::Get().GetWindow().GetNativeWindow());
        ofn.lpstrFile = szFile;
        ofn.nMaxFile = sizeof(szFile);
        ofn.lpstrFilter = Filter;
        ofn.nFilterIndex = 1;
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
        if(GetOpenFileNameA(&ofn) == TRUE)
        {
            return ofn.lpstrFile;
        }

        return std::string();     
    }
    
    std::string FileDialogs::SaveFile(const char* Filter)
    {
        OPENFILENAMEA ofn;          
        CHAR szFile[260] = { 0 };   
        ZeroMemory(&ofn, sizeof(OPENFILENAME));
        ofn.lStructSize = sizeof(OPENFILENAME);
        ofn.hwndOwner = glfwGetWin32Window((GLFWwindow*)Application::Get().GetWindow().GetNativeWindow());
        ofn.lpstrFile = szFile;
        ofn.nMaxFile = sizeof(szFile);
        ofn.lpstrFilter = Filter;
        ofn.nFilterIndex = 1;
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
        if(GetSaveFileNameA(&ofn) == TRUE)
        {
            return ofn.lpstrFile;
        }

        return std::string(); 
    }


    
    void PlatformMisc::CreateGuid(struct FGuid& Result)
    {
        CoCreateGuid((GUID*)&Result);
    }
}
