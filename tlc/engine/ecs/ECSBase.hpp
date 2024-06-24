#pragma once

#include "core/Core.hpp"

#include "engine/ecs/Component.hpp"


namespace tlc {
	class Entity;

	class ECS
	{
	public:
		ECS();
		~ECS();

		List<Raw<Entity>> GetEntities(const String& name) const;
		Raw<Entity> GetEntity(UUID uuid) const;
		List<Raw<Entity>> GetAllEntities() const;

		void HandleDeletions();


		inline List<Raw<IComponent>> GetAllComponents() const {
			List<Raw<IComponent>> result;
			for (auto& pair : m_Components) {
				for (auto component : pair.second) {
					result.push_back(component);
				}
			}
			return result;
		}

		template <typename T>
		inline List<Raw<IComponent>> GetComponentsRaw() {
			auto& components = m_Components[typeid(T).hash_code()];
			List<Raw<IComponent>> result;
			for (auto component : components) {
				result.push_back(component);
			}
			return result;
		}

		template <typename T>
		inline List<Raw<T>> GetComponents() {
			auto& components = m_Components[typeid(T).hash_code()];
			List<Raw<T>> result;
			for (auto component : components) {
				result.push_back(static_cast<Raw<T>>(component));
			}
			return result;
		}

		template <typename T, typename... Args>
		inline Raw<T> CreateComponent(Raw<Entity> entity, Args&&... args)
		{
			static_assert(std::is_base_of<IComponent, T>::value, "T must be a subclass of IComponent");

			auto component = new T();
			component->Initiate(this, entity);
			// checkid there  is a setup function in the component
			if constexpr (requires(T component, Args... args) { component->Setup(std::forward<Args>(args)...); }){
				component->Setup(std::forward<Args>(args)...);
			}

			m_Components[typeid(T).hash_code()].push_back(component);

			component->Start();
			component->Resume();
			
			return component;
		}

		Raw<Entity> CreateEntity(String name, Raw<Entity> parent = nullptr);

	private:
		inline Raw<Entity> GetRootEntity() const { return m_RootEntity; }

	private:
		Raw<Entity> m_RootEntity = nullptr;
		List<Raw<Entity>> m_Entities;
		Map<Size, List<Raw<IComponent>>> m_Components;

		friend class Entity;
		friend class Component;
	};

}
