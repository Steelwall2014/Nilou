#pragma once
#include "WindowsApplication.h"

namespace und {
	class OpenGLApplication : public WindowsApplication
	{
    public:
        OpenGLApplication(GfxConfiguration &config)
            : WindowsApplication(config) {};

        virtual int Initialize();
        virtual void Finalize();
        virtual void Tick(double DeltaTime);

    private:
        HDC   m_hDC;
        HGLRC m_RenderContext;
	};
}