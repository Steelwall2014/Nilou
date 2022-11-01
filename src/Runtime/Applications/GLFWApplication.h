#pragma once
#include <vector>
#include "Common/BaseApplication.h"

class GLFWwindow;
namespace nilou {
    class IDrawPass;
    class IRuntimeModule;
    class GLFWApplication : public BaseApplication
    {
    public:
        GLFWApplication(GfxConfiguration &config);

        virtual int Initialize();
        virtual void Finalize();
        virtual void Tick(double DeltaTime);
    private:
        GLFWwindow *window;
        // std::vector<IDrawPass *> m_DrawPasses;
        // std::vector<IRuntimeModule *> run_time_modules;
        void processInput();
        void EnableCursor();
    };
}