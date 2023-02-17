#pragma once
#include <vector>
#include "Common/BaseApplication.h"

class GLFWwindow;
namespace nilou {
    class IDrawPass;
    class GLFWApplication : public BaseApplication
    {
    public:
        GLFWApplication(GfxConfiguration &config);

        virtual bool Initialize() override;
        virtual bool Initialize_RenderThread() override;
        virtual void Finalize() override;
        virtual void Tick(double DeltaTime) override;
    private:
        GLFWwindow *window;
        void processInput();
        void EnableCursor();
    };
}