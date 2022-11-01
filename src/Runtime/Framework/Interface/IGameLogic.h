#pragma once
#include "IRuntimeModule.h"

namespace nilou {
    Interface IGameLogic : implements IRuntimeModule
    {
       public:
        int Initialize() override = 0;
        void Finalize() override = 0;
        void Tick(double DeltaTime) override = 0;

        virtual void OnKeyboardKeyDown() {}
        virtual void OnKeyboardKeyUp() {}
        virtual void OnKeyboardKeyClick() {}

        virtual void OnMouseLeftDown() {}
        virtual void OnMouseLeftUp() {}
        virtual void OnMouseLeftClick() {}

        virtual void OnMouseRightDown() {}
        virtual void OnMouseRightUp() {}
        virtual void OnMouseRightClick() {}

        virtual void OnMouseMove() {}
        virtual void OnMouseDrag() {}
    };

    extern IGameLogic *g_pGameLogic;
}