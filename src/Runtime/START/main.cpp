#include <stdio.h>
#include <iostream>
#include <vector>
#include <GLFWApplication.h>
#include <time.h>

using namespace nilou;
namespace nilou {
    GfxConfiguration config(8, 8, 8, 8, 32, 0, 0, 1600, 900, L"test");
    BaseApplication *g_pApp = new GLFWApplication(config);
}

int main()
{
    int ret;

    if ((ret = g_pApp->Initialize()) != 0) {
        printf("App Initialize failed, will exit now.");
        return ret;
    }



    //for (auto &module : run_time_modules) {
    //    if ((ret = module->Initialize()) != 0) {
    //        std::cerr << "Failed. err = " << ret;
    //        return EXIT_FAILURE;
    //    }
    //}

    clock_t DeltaTime, lastFrame = 0;
    while (!g_pApp->IsQuit()) {
        clock_t currentFrame = clock();
        DeltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        g_pApp->Tick(DeltaTime/1000.f);
    }

    g_pApp->Finalize();

    return 0;
}