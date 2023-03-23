#pragma once
#include <vector>
#include "BaseApplication.h"
#include <Common/InputManager.h>

class GLFWwindow;
namespace nilou {
    class IDrawPass;
    class GLFWApplication : public BaseApplication
    {
    public:
        GLFWApplication(GfxConfiguration &config);

        virtual bool Initialize() override;
        virtual bool Initialize_RenderThread() override;
        virtual void Finalize_RenderThread() override;
        virtual void Tick(double DeltaTime) override;
        virtual void Tick_RenderThread() override;
    private:
        GLFWwindow *window;
        void DispatchScreenResizeMessage();
        void DispatchMouseMoveMessage();
        void DispatchKeyMessage();
        void ProcessInput_RenderThread();
        void EnableCursor();
    };
}