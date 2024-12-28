#pragma once


#include "core/Core.hpp"

namespace tlc
{
    class ISystem
    {
    public:
        virtual ~ISystem() = default;
        virtual void OnUpdate() = 0;

        virtual void OnLoad() {}
        virtual void OnUnload() {}
    };
} // namespace tlc