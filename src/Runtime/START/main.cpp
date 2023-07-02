#include <stdio.h>
#include <iostream>
#include <vector>
#include <GLFWApplication.h>
#include <time.h>

using namespace nilou;
namespace nilou {

GfxConfiguration config(EPixelFormat::PF_B8G8R8A8_sRGB, EPixelFormat::PF_D32FS8, 0, 1600, 900, L"Nilou");

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
    const char* DefaultRHI = "opengl";
    if (argc >= 3 && !strcmp(argv[1], "-rhi"))
        DefaultRHI = argv[2];
    assert(!strcmp(DefaultRHI, "opengl") || !strcmp(DefaultRHI, "vulkan"));
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