#pragma once

#include "core/Core.hpp"
#include "engine/ecs/Component.hpp"

#include "engine/ecs/ECSBase.hpp"


namespace tlc {

	class Entity {
	public:
		List<Raw<IComponent>> GetComponentsOfType(Size typeID) const;
		void SetParent(Raw<Entity> parent);
		void Destroy();
		void SetActive(Bool active, U32 depth = 0);

		inline UUID GetUUID() const { return m_UUID; }

		template <typename T>
		inline List<Raw<T>> GetComponents() {
			auto components = GetComponentsOfType(typeid(T).hash_code());
			List<Raw<T>> result;
			for (auto component : components) {
				result.push_back(static_cast<Raw<T>>(component));
			}
			return result;
		}

		template <typename T>
		inline Raw<T> GetComponent() {
			auto components = GetComponentsOfType(typeid(T).hash_code());
			if (components.size() > 0) {
				return static_cast<Raw<T>>(components[0]);
			}
			return nullptr;
		}

		
		template <typename T, typename... Args>
		inline Raw<T> AddComponent(Args&&... args) {
			static_assert(std::is_base_of<IComponent, T>::value, "T must be a subclass of IComponent");
			auto component = m_ECS->CreateComponent<T, Args...>(this, std::forward<Args>(args)...);
			m_Components[typeid(T).hash_code()].push_back(component);
			return component;
		}

		inline List<Raw<IComponent>> GetComponents() const {
			List<Raw<IComponent>> result;
			for (auto& [typeID, components] : m_Components) {
				for (auto component : components) {
					result.push_back(component);
				}
			}
			return result;
		}

		inline Raw<Entity> GetParent() const { return m_Parent; }
		inline String GetName() const { return m_Name; }
		inline Bool IsMarkedForDeletion() const { return m_MarkedForDeletion; }
		inline Bool IsActive() const { return m_Active; }


	private:
		Entity(Raw<ECS> ecs, String name);
		~Entity();

		void AddChild(Raw<Entity> child);
		void RemoveChild(UUID child);
		inline void MarkForDeletion() { m_MarkedForDeletion = true; }


	private:
		List<Raw<Entity>> m_Children;
		Raw<Entity> m_Parent = nullptr;
		Raw<ECS> m_ECS = nullptr;
		UUID m_UUID = UUID::Zero();
		String m_Name;
		Map<Size, List<Raw<IComponent>>> m_Components;
		Bool m_MarkedForDeletion = false;
		Bool m_Active = true;



		friend class ECS;
	};

}
