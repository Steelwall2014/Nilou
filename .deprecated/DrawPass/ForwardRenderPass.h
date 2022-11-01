#pragma once
#include "Interface/IDrawPass.h"
#include "Common/GfxStructures.h"

namespace und {
    class ForwardRenderPass : implements IDrawPass
    {
    public:
        ~ForwardRenderPass() = default;
        virtual int Initialize(FrameVariables &frame) override;
        virtual void Draw(FrameVariables &frame) final;
    };
}