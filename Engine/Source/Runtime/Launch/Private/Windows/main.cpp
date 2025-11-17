#include <stdio.h>
#include <iostream>
#include <vector>
#include <time.h>
#include "GLFWApplication.h"
#include "HAL/CrashHandler.h"
#include "HAL/PlatformMisc.h"

using namespace nilou;
namespace nilou {

BaseApplication *GetAppication()
{
    static BaseApplication *g_pApp;
    if (g_pApp == nullptr)
    {
        g_pApp = new GLFWApplication();
    }
    return g_pApp;
}

}

int main(int argc, char* argv[])
{        
    FCrashHandler::Initialize();

    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-waitfordebugger") == 0)
        {
            while (!FPlatformMisc::IsDebuggerPresent()) 
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }
    }
    GGfxConfig.defaultRHI = "vulkan";
    GGfxConfig.SwapChainFormat = EPixelFormat::PF_B8G8R8A8;
    GGfxConfig.DepthFormat = EPixelFormat::PF_D32FS8;
    GGfxConfig.screenWidth = 1600;
    GGfxConfig.screenHeight = 900;
    GGfxConfig.appName = L"Nilou";
    
    int ret;

    if ((ret = GetAppication()->Initialize()) != true) {
        printf("App Initialize failed, will exit now.");
        return ret;
    }

    clock_t DeltaTime, lastFrame = 0;
    while (!GetAppication()->IsQuit()) {
        clock_t currentFrame = clock();
        DeltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        GetAppication()->Tick(DeltaTime/1000.f);
    }

    GetAppication()->Finalize();

    FCrashHandler::Shutdown();

    return 0;
}