#pragma once
#include <Windows.h>
#include <windowsx.h>
#include "Common/BaseApplication.h"

namespace und {
    class WindowsApplication : public BaseApplication
    {
    public:
        WindowsApplication(GfxConfiguration &config);

        virtual int Initialize();
        virtual void Finalize();
        virtual void Tick(double DeltaTime);

        static LRESULT CALLBACK WindowProc(HWND hWnd,
            UINT message,
            WPARAM wParam,
            LPARAM lParam);

        inline HWND GetMainWindow() { return m_hWnd; };
    protected:
        HWND m_hWnd;
    };
}