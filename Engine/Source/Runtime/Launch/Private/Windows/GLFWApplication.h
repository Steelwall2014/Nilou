#pragma once
#include <vector>
#include "BaseApplication.h"
#include "InputManager.h"

class GLFWwindow;
namespace nilou {
    class IDrawPass;
    class GLFWApplication : public BaseApplication
    {
    public:
        GLFWApplication();

        virtual bool Initialize() override;
        virtual bool Initialize_RenderThread() override;
        virtual void Finalize_RenderThread() override;
        virtual void ProcessInput() override;
        virtual void Tick(double DeltaTime) override;
        virtual void Tick_RenderThread() override;
        virtual WindowContext* GetWindowContext() override { return window; }
    private:
        GLFWwindow *window = nullptr;
        std::atomic<bool> bShouldQuit{false};
        bool CreateWindow();
        void DispatchScreenResizeMessage();
        void DispatchMouseMoveMessage();
        void DispatchKeyMessage();
        void EnableCursor();
    };
}