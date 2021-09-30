// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include "AirSim.h"
#include "Misc/Paths.h"
#include "Modules/ModuleManager.h"
#include "Modules/ModuleInterface.h"
#include <Interfaces/IPluginManager.h>

class FAirSim : public IModuleInterface
{
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
    void* MotionCoreDllHandle;
    void* ControlCoreDllHandle;
};

IMPLEMENT_MODULE(FAirSim, AirSim)

void FAirSim::StartupModule()
{
    //plugin startup
    UE_LOG(LogTemp, Log, TEXT("StartupModule: AirSim plugin"));
    FString path = IPluginManager::Get().FindPlugin("AirSim")->GetBaseDir();
    {
        FString dllpath = path + "/Source/AirLib/deps/ControlCore/lib/x64/Release/ControlCore.dll";
        ControlCoreDllHandle = FPlatformProcess::GetDllHandle(*dllpath);
        if (!ControlCoreDllHandle) {
            UE_LOG(LogTemp, Error, TEXT("Failed to load ControlCore library."));
        }
    }
    {
        FString dllpath = path + "/Source/AirLib/deps/MotionCore/lib/x64/Release/MotionCore.dll";
        MotionCoreDllHandle = FPlatformProcess::GetDllHandle(*dllpath);
        if (!MotionCoreDllHandle) {
            UE_LOG(LogTemp, Error, TEXT("Failed to load MotionCore library."));
        }
    }
}

void FAirSim::ShutdownModule()
{
    //plugin shutdown
    FPlatformProcess::FreeDllHandle(ControlCoreDllHandle);
    FPlatformProcess::FreeDllHandle(MotionCoreDllHandle);
}