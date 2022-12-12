#include <stdio.h>
#include <iostream>
#include <vector>
#include <GLFWApplication.h>
#include <time.h>

using namespace nilou;
namespace nilou {
}

int main()
{
    int ret;

    if ((ret = GetAppication()->Initialize()) != 0) {
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
    while (!GetAppication()->IsQuit()) {
        clock_t currentFrame = clock();
        DeltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        GetAppication()->Tick(DeltaTime/1000.f);
    }

    GetAppication()->Finalize();

    return 0;
}