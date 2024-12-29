#pragma once

#include "core/Core.hpp"
#include "engine/ecs/System.hpp"
#include "engine/ecs/ECSBase.hpp"

namespace tlc
{
    template<typename... Ts>
    class ComponentsQuery : public HomoTuple<UUID, sizeof...(Ts)>
    {
    public:
        ComponentsQuery<Ts...>() : HomoTuple<UUID, sizeof...(Ts)>() {}

    private:
        using SelfType = ComponentsQuery<Ts...>;
        using BaseType = HomoTuple<UUID, sizeof...(Ts)>;

        friend class ECS;

        template <int N>
        static bool MatchEntityImpl(Raw<ECS> ecs, const Entity& entity, const List<UUID>& components)
        {
            auto typeId = typeid(NthType<N, Ts...>).hash_code();
            auto component = std::find_if(components.begin(), components.end(), [typeId, ecs](const auto& comp) {
                return ecs->GetComponentTypeID(comp) == typeId;
            });

            if (component == components.end())
            {
                return false;
            }

            if constexpr (N < sizeof...(Ts) - 1) {
                return MatchEntityImpl<N + 1>(ecs, entity, components);
            }
            return true;
            
        }

        // Check if an entity has all the components of the query
        static bool MatchEntity(Raw<ECS> ecs, const Entity& entity)
        {
            auto& components = ecs->GetComponents(entity);
            return MatchEntityImpl<0>(ecs, entity, components);
        }

        // Check if an entity has the injected component (SelfType)
        static bool IsEntityInjected(Raw<ECS> ecs, const Entity& entity)
        {
            auto& components = ecs->GetComponents(entity);
            return std::any_of(components.begin(), components.end(), [ecs](const auto& comp) {
                return ecs->GetComponentTypeID(comp) == typeid(SelfType).hash_code();
            });
        }

        template <int N>
        static void InjectImpl(Raw<ECS> ecs, const Entity& entity, SelfType& value, const List<UUID>& components)
        {
            auto typeId = typeid(NthType<N, Ts...>).hash_code();
            auto component = std::find_if(components.begin(), components.end(), [typeId, ecs](const auto& comp) {
                return ecs->GetComponentTypeID(comp) == typeId;
            });

            if (component != components.end())
            {
                std::get<N>(value) = *component;
            }

            if constexpr (N < sizeof...(Ts) - 1) {
                InjectImpl<N + 1>(ecs, entity, value, components);
            }
        }

        // Inject the component (SelfType) into the entity, also capture the component ids
        static void Inject(Raw<ECS> ecs, const Entity& entity)
        {
            auto value = SelfType();
            auto& components = ecs->GetComponents(entity);
            InjectImpl<0>(ecs, entity, value, components);
            auto nameOfType = std::string(typeid(SelfType).name());
            const auto name = std::vformat("QueryComponent<{0}>", std::make_format_args(nameOfType));
            ecs->CreateComponent<SelfType>(entity, name, value);
        }

        static void InjectIfNeeded(Raw<ECS> ecs, const Entity& entity)
        {
            if (MatchEntity(ecs, entity) && !IsEntityInjected(ecs, entity))
            {
                log::Error("Injecting {} into entity {}", typeid(SelfType).name(), entity);
                Inject(ecs, entity);
            }
        }

        class InjectorAdd : public ISystem
        {
        public:
            virtual void OnUpdate(Raw<ECS> ecs, const List<Entity>& entities, const List<UUID>& components) override
            {
                (void)components;

                // usually this will be a single entity
                // because of the trigger of this system
                for (const auto& entity : entities) 
                {
                    SelfType::InjectIfNeeded(ecs, entity);
                }
            }
        };

        class InjectorRem : public ISystem
        {
        public:
            virtual void OnUpdate(Raw<ECS> ecs, const List<Entity>& entities, const List<UUID>& components) override
            {
                (void)components;
                (void)ecs;

                // usually this will be a single entity
                // because of the trigger of this system
                for (const auto& entity : entities) 
                {
                    // SelfType::InjectIfNeeded(m_ECS, entity);
                }
            }
        };
    };
    
    template<int N, typename... Ts>
    inline void RegisterSystemWithQueryImpl(Raw<ECS> ecs, Ref<ISystem> systemAdd, Ref<ISystem> systemRem, const String& name) {        
        if constexpr (N < sizeof...(Ts)) {
            using ComponentType = NthType<N, Ts...>;
            auto componentTypeName = std::string(typeid(ComponentType).name());
            ecs->RegisterSystem<ComponentType>(systemAdd, SystemTrigger::OnComponentCreate, 100000, std::vformat("__QueryInjectorAdd<{0} : Listner<{1}>>", std::make_format_args(name, componentTypeName)));
            ecs->RegisterSystem<ComponentType>(systemRem, SystemTrigger::OnComponentDestroy, 100000, std::vformat("__QueryInjectorRem<{0} : Listner<{1}>>", std::make_format_args(name, componentTypeName)));
            RegisterSystemWithQueryImpl<N + 1, Ts...>(ecs, systemAdd, systemRem, name);
        }
    }

    template<typename... Ts>
	inline UUID ECS::RegisterSystemWithQuery(Ref<ISystem> system, SystemTrigger trigger, U32 priority, const String& name) {
        using QueryType = ComponentsQuery<Ts...>;
        auto query = QueryType();
        auto queryTypeName = std::string(typeid(QueryType).name());

        auto systemAdd = CreateRef<typename QueryType::InjectorAdd>();
        auto systemRem = CreateRef<typename QueryType::InjectorRem>();
        RegisterSystemWithQueryImpl<0, Ts...>(this, systemAdd, systemRem, queryTypeName);
        return RegisterSystem<ComponentsQuery<Ts...>>(system, trigger, priority, name);
	}
}