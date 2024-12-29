#pragma once


#include "core/Core.hpp"

namespace tlc
{
    class ECS;

    class ISystem
    {
    public:
        virtual ~ISystem() = default;
        virtual void OnUpdate(Raw<ECS> ecs, const UUID& entity, const UUID& component) = 0;

        virtual void OnLoad() {}
        virtual void OnUnload() {}
    };
} // namespace tlc