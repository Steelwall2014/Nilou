#pragma once
#include "Interface.h"

#include "Common/GfxConfiguration.h"

namespace nilou {
    Interface IApplication
    {
    public:
        virtual int Initialize() = 0;
        virtual void Finalize() = 0;
        virtual void Tick(double DeltaTime) = 0;

        virtual bool IsQuit() = 0;
        virtual GfxConfiguration &GetConfiguration() = 0;
        virtual void SetWindowWidth(int width) = 0;
        virtual void SetWindowHeight(int height) = 0;
        virtual float GetTimeSinceStart() = 0;                  // 获取从开始到现在经过了多少时间，单位为秒
        virtual bool IsCursorEnabled() = 0;
    };

}