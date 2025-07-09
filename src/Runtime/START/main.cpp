#include <stdio.h>
#include <iostream>
#include <vector>
#include <GLFWApplication.h>
#include <time.h>
#include <windows.h>

using namespace nilou;
namespace nilou {

GfxConfiguration config(EPixelFormat::PF_B8G8R8A8, EPixelFormat::PF_D32FS8, 0, 1600, 900, L"Nilou");

BaseApplication *GetAppication()
{
    static BaseApplication *g_pApp;
    if (g_pApp == nullptr)
    {
        g_pApp = new GLFWApplication(config);
    }
    return g_pApp;
}

}

int main(int argc, char* argv[])
{        
    while (!IsDebuggerPresent()) 
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    const char* DefaultRHI = "vulkan";
    config.defaultRHI = DefaultRHI;
    
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

    return 0;
}