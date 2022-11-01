#pragma once
#include <iostream>
#include "Interface.h"

namespace nilou {
    class FrameVariables;
    Interface IDrawPass
    {
    public:
        IDrawPass() = default;
        virtual ~IDrawPass() {};

        virtual int Initialize(FrameVariables &frame) { return 0; }
        virtual void Draw(FrameVariables &frame) = 0;
    };
    Interface ISubPass
    {

    };
}