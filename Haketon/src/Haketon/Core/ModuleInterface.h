#pragma once
#include "Haketon/Core/Core.h"

class HK_API IModuleInterface
{
public:

    virtual ~IModuleInterface() {}

    virtual void StartupModule() {}

    virtual void ShutdownModule() {}
};