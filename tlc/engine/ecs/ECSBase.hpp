#pragma once

#include "core/Core.hpp"

#include "engine/ecs/System.hpp"


namespace tlc {
	using Entity = UUID;

	const Entity nullentity = UUID::Zero();


	enum class SystemTrigger {
		OnComponentCreate,
		OnComponentDestroy,
		OnUpdate,
	};

	inline constexpr String SystemTriggerToString(SystemTrigger trigger) {
		switch (trigger) {
		case SystemTrigger::OnComponentCreate: return "OnComponentCreate";
		case SystemTrigger::OnComponentDestroy: return "OnComponentDestroy";
		case SystemTrigger::OnUpdate: return "OnUpdate";
		}
		return "Unknown";
	}

	namespace internal {
		struct EntityHolder {
			Entity ID = nullentity;
			Entity Parent = UUID::Zero();
			String Name = "Unnamed_Entity";
			Set<Entity> Children;
			List<UUID> Components;

			EntityHolder() = default;
			EntityHolder(String name) : Name(name), ID(UUID::New()) {}
		};

		struct ComponentHolder {
			Entity EntityID = nullentity;
			UUID ComponentID = UUID::Zero();
			String Name = "Unnamed_Component";
			Size Index = 0;

			ComponentHolder() = default;
			ComponentHolder(Entity entity, UUID component, String name) : EntityID(entity), ComponentID(component), Name(name) {}
		};

		struct ComponentPool {
			Size TypeSize = 0;
			Size TypeID = 0;
			Size Capacity = 10;
			Size Count = 0;
			List<U8> Data;
			UnorderedMap<UUID, ComponentHolder> Components;

			ComponentPool() = default;

			inline void Reserve(Size capacity) {
				EnsureCapacity(capacity);
			}

			template<typename T>
			inline UUID AddComponent(ComponentHolder holder, const T& component) {
				TLC_ASSERT(sizeof(T) == TypeSize, "ComponentPool::AddComponent: Component size mismatch!");
				TLC_ASSERT(typeid(T).hash_code() == TypeID, "ComponentPool::AddComponent: Component type mismatch!");
				return AddComponentRaw(holder, &component);
			}

			inline UUID AddComponentRaw(ComponentHolder holder, const void* component) {
				if (Components.find(holder.ComponentID) != Components.end()) {
					log::Warn("ComponentPool::AddComponent: Component already exists!");
					return UUID::Zero();
				}
				if (Count >= Capacity) {
					EnsureCapacity(Capacity * 2);
				}
				holder.Index = FindEmptySpot();
				FillSpot(holder.Index, component);
				Components[holder.ComponentID] = holder;

				return holder.ComponentID;
			}

			Bool HasComponent(const UUID& component) const {
				return Components.find(component) != Components.end();
			}

			template<typename T>
			inline const T& GetComponent(const UUID& component) const {
				TLC_ASSERT(sizeof(T) == TypeSize, "ComponentPool::GetComponent: Component size mismatch!");
				TLC_ASSERT(typeid(T).hash_code() == TypeID, "ComponentPool::GetComponent: Component type mismatch!");
				return *reinterpret_cast<const T*>(GetComponentRaw(component));
			}

			template<typename T>
			inline T& GetComponent(const UUID& component) {
				TLC_ASSERT(sizeof(T) == TypeSize, "ComponentPool::GetComponent: Component size mismatch!");
				TLC_ASSERT(typeid(T).hash_code() == TypeID, "ComponentPool::GetComponent: Component type mismatch!");
				return *reinterpret_cast<T*>(GetComponentRaw(component));
			}

			inline void* GetComponentRaw(const UUID& component) {
				auto it = Components.find(component);
				if (it == Components.end()) {
					log::Warn("ComponentPool::GetComponent: Component does not exist!");
					return nullptr;
				}
				return Data.data() + it->second.Index * (TypeSize + 1) + 1;
			}


			inline void RemoveComponent(const UUID& component) {
				auto it = Components.find(component);
				if (it == Components.end()) {
					log::Warn("ComponentPool::RemoveComponent: Component does not exist!");
					return;
				}
				ClearSpot(it->second.Index);
				Components.erase(it);
			}

			template<typename T>
			inline static ComponentPool Create(Size typeId = typeid(T).hash_code()) {
				ComponentPool pool;
				pool.TypeSize = sizeof(T);
				pool.TypeID = typeid(T).hash_code();
				pool.Data.resize(pool.Capacity * (pool.TypeSize + 1));
				return pool;
			}

			inline const ComponentHolder& GetHolder(const UUID& component) const {
				auto it = Components.find(component);
				if (it == Components.end()) {
					log::Fatal("ComponentPool::GetHolder: Component does not exist!");
				}
				return it->second;
			}

		private:
			inline void EnsureCapacity(Size capacity) {
				if (capacity <= Capacity && capacity != 0) {
					return;
				}
				Data.resize(capacity * (TypeSize + 1));
				// Ensure the newly allocated memory is zeroed out
				std::memset(Data.data() + Capacity * (TypeSize + 1), 0, (capacity - Capacity) * (TypeSize + 1));
				Capacity = capacity;
			}

			inline Size FindEmptySpot() {
				for (Size i = 0; i < Capacity; i++) {
					if (Data[i * (TypeSize + 1)] == 0) {
						return i;
					}
				}
				return -1;
			}

			inline void FillSpot(Size index, const void* data) {
				Count++;
				Data[index * (TypeSize + 1)] = 1;
				std::memcpy(Data.data() + index * (TypeSize + 1) + 1, data, TypeSize);
			}

			inline void ClearSpot(Size index) {
				Count--;
				Data[index * (TypeSize + 1)] = 0;
			}
		};

		struct SystemHolder {
			Ref<ISystem> System = nullptr;
			String Name = "Unnamed_System";
			U32 Priority = 0;
			Size Filter;			
			SystemTrigger Trigger = SystemTrigger::OnUpdate;
			UUID ID = UUID::Zero();

			SystemHolder(Ref<ISystem> system, String name, U32 priority, SystemTrigger trigger = SystemTrigger::OnUpdate) : System(system), Name(name), Priority(priority), Trigger(trigger) {}
		};
	}


	class ECS
	{
	public:
		ECS();
		~ECS();

		Entity CreateEntity(const String& name = "Unnamed_Entity", const Entity& parent = nullentity);

		Bool IsChildOf(const Entity& parent, const Entity& child) const;
		Bool IsParentOf(const Entity& parent, const Entity& child) const;

		inline Entity GetParent(const Entity& entity) const { return m_Entities.at(entity).Parent; }
		inline const List<Entity> GetChildren(const UUID& entity) const { auto& children = m_Entities.at(entity).Children; return List<Entity>(children.begin(), children.end()); }
		inline const String& GetEntityName(const Entity& entity) const { return m_Entities.at(entity).Name; }
		inline Bool IsValidEntity(const Entity& entity) const { return m_Entities.find(entity) != m_Entities.end(); }
		inline const List<UUID>& GetComponents(const Entity& entity) const { return m_Entities.at(entity).Components; }

		List<Entity> FindByName(const String& name, const Entity& parent = nullentity, Bool recursive = false) const;
		List<Entity> Find(const String& path, const Entity& parent = nullentity) const;
		List<Entity> CreatePath(const String& path, const Entity& parent = nullentity);

		void PrintEntityTree() const;
		void PrintSystems() const;

		void ApplyDeletions();

		// These functions are used to destroy entities and components
		// but they just mark them for deletion, they are not actually deleted
		// until ApplyDeletions is called
		inline void DestroyEntity(const Entity& entity) { m_EntitiesToRemove.insert(entity); }
		inline void DestroyComponent(const UUID& component) { m_ComponentsToRemove.insert(component); DispatchSystems(SystemTrigger::OnComponentDestroy, { component }); }


		template<typename T>
		inline UUID CreateComponent(const Entity& entity, const String& name = std::format("Component<{0}>", std::string(typeid(T).name())), const T& component = T()) {
			if (!IsValidEntity(entity)) {
				log::Warn("ECS::CreateComponent: Entity does not exist!");
				return UUID::Zero();
			}
			const auto componentTypeID = typeid(T).hash_code();
			auto componentId = Assure<T>().AddComponent(internal::ComponentHolder(entity, UUID::New(), name), component);
			m_ComponentTypeMap[componentId] = componentTypeID;
			m_Entities[entity].Components.push_back(componentId);
			DispatchSystems(SystemTrigger::OnComponentCreate, { componentId });
			return componentId;
		}

		inline void* GetComponentRaw(const UUID& component) {
			auto typeId = m_ComponentTypeMap[component];
			auto it = m_Components.find(typeId);
			if (it == m_Components.end()) {			
				if (it->second.HasComponent(component)) {
					return it->second.GetComponentRaw(component);
				}
			}
		}

		template <typename T>
		inline UUID GetComponentIDFromEntity(const Entity& entity) {
			auto& components = GetComponents(entity);
			auto typeId = typeid(T).hash_code();
			auto it = std::find_if(components.begin(), components.end(), [this, typeId](const auto& component) {
				return m_ComponentTypeMap[component] == typeId;
			});
			if (it == components.end()) {
				log::Warn("ECS::GetComponentIDFromEntity: Component does not exist!");
				return UUID::Zero();
			}
			return *it;
		}

		template <typename T>
		inline T& GetComponentFromEntity(const Entity& entity, const T& defaultValue = T()) {
			auto component = GetComponentIDFromEntity<T>(entity);
			if (component == UUID::Zero()) {
				return defaultValue;
			}
			return GetComponent<T>(component);
		}

		template<typename T>
		inline T& GetComponent(const UUID& component, const T& defaultValue = T()) {
			auto& pool = Assure<T>();
			if (!pool.HasComponent(component)) {
				log::Warn("ECS::GetComponent: Component does not exist!");
				return defaultValue;
			}
			return pool.GetComponent<T>(component);
		}

		template<typename T>
		inline const T& GetComponent(const UUID& component, const T& defaultValue = T()) const {
			auto& pool = Assure<T>();
			if (!pool.HasComponent(component)) {
				log::Warn("ECS::GetComponent: Component does not exist!");
				return defaultValue;
			}
			return pool.GetComponent<T>(component);
		}

		inline const String& GetComponentName(const UUID& component) const {
			return GetComponentHolder(component).Name;
		}

		inline const UUID& GetComponentEntity(const UUID& component) const {
			return GetComponentHolder(component).EntityID;
		}

		inline Size GetComponentTypeID(const UUID& component) const {
			return m_ComponentTypeMap.at(component);
		}

		template<typename Ts, typename SystemType> requires std::derived_from<SystemType, ISystem>
		inline UUID RegisterSystem(Ref<SystemType> system, SystemTrigger trigger = SystemTrigger::OnUpdate, U32 priority = 0, const String& name = std::format("System<{0}>", std::string(typeid(SystemType).name()))) {
			if (!name.starts_with("__")) {
				priority = std::clamp(priority, 0u, 1000u); // allowed range for normal systems
			}
			auto holder = internal::SystemHolder(system, name, priority, trigger);
			holder.Filter = typeid(Ts).hash_code();
			holder.ID = UUID::New();
			auto& systems = m_Systems[trigger]; // We want a default empty list if it doesn't exist
			systems.push_back(holder);
			std::sort(systems.begin(), systems.end(), [](const auto& a, const auto& b) { return a.Priority > b.Priority; });
			system->OnLoad();
			return holder.ID;
		}

		template<typename... Ts>
		inline UUID RegisterSystemWithQuery(Ref<ISystem> system, SystemTrigger trigger = SystemTrigger::OnUpdate, U32 priority = 0, const String& name = std::format("SystemQuery<{0}>", std::string(typeid(Ts).name())));

	private:
		void LinkInTree(const UUID& parent, const UUID& child);
		void UnlinkInTree(const UUID& parent, const UUID& child);

		void MarkEntitiesForDeletion(const UUID& entity);
		void DeleteComponentApply(const UUID& component);


		// NOTE: This function expects the TypeId for all the components to be the same
		void DispatchSystems(SystemTrigger trigger, const List<UUID>& components);

		template<typename T>
		inline internal::ComponentPool& Assure(Size typeId = typeid(T).hash_code()) {
			auto it = m_Components.find(typeId);
			if (it == m_Components.end()) {
				m_Components[typeId] = internal::ComponentPool::Create<T>();
			}
			return m_Components[typeId];
		}

		inline const internal::ComponentHolder& GetComponentHolder(const UUID& component) {
			auto typeId = m_ComponentTypeMap[component];
			auto it = m_Components.find(typeId);
			if (it == m_Components.end()) {
				log::Fatal("ECS::GetComponentHolder: Component does not exist!");
			}
			return it->second.GetHolder(component);
		}

		inline const internal::ComponentHolder& GetComponentHolder(const UUID& component) const {
			auto typeId = m_ComponentTypeMap.find(component);
			if (typeId == m_ComponentTypeMap.end()) {
				log::Fatal("ECS::GetComponentHolder: Component does not exist!");
			}
			auto it = m_Components.find(typeId->second);
			if (it == m_Components.end()) {
				log::Fatal("ECS::GetComponentHolder: Component does not exist!");
			}
			return it->second.GetHolder(component);
		}
		

	private:
		Entity m_RootEntity = UUID::Zero();
		UnorderedMap<Entity, internal::EntityHolder> m_Entities;
		UnorderedMap<Size, internal::ComponentPool> m_Components;
		UnorderedMap<UUID, Size> m_ComponentTypeMap;
		UnorderedMap<SystemTrigger, List<internal::SystemHolder>> m_Systems;

		Set<Entity> m_EntitiesToRemove;
		Set<UUID> m_ComponentsToRemove;
	};

}
