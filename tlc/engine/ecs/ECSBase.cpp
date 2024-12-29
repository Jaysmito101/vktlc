#include "engine/ecs/ECSBase.hpp"

namespace tlc
{ 
	ECS::ECS()
	{
		auto rootEntity = internal::EntityHolder("__Root");
		m_RootEntity = rootEntity.ID;
		m_Entities[rootEntity.ID] = rootEntity;
	}

	ECS::~ECS()
	{
	}

	Entity ECS::CreateEntity(const String& name, const Entity& parent)
	{
		if (name.empty()) {
			log::Warn("ECS::CreateEntity: Entity name cannot be empty!");
			return nullentity;
		}

		if (name.starts_with("__")) {
			log::Warn("ECS::CreateEntity: Entity name cannot start with '__', reserved for internal entities!");
			return nullentity;
		}

		auto entity = internal::EntityHolder(name);
		m_Entities[entity.ID] = entity;
		LinkInTree(parent, entity.ID);
		return entity.ID;
	}

	Bool ECS::IsChildOf(const Entity& parent, const Entity& child) const
	{
		auto entity = m_Entities.find(child);
		if (entity == m_Entities.end()) {
			return false;
		}
		return entity->second.Parent == parent;
	}

	Bool ECS::IsParentOf(const Entity& parent, const Entity& child) const
	{
		auto entity = m_Entities.find(parent);
		if (entity == m_Entities.end()) {
			return false;
		}
		return entity->second.Children.contains(child);
	}

	void ECS::PrintEntityTree() const {
		static const std::function<void(const Entity&, U32)> printTree = [&](const UUID& entity, U32 level) {
			auto entityHolder = m_Entities.find(entity);
			if (entityHolder == m_Entities.end()) {
				return;
			}

			auto& name = entityHolder->second.Name;
			auto& id = entityHolder->second.ID;

			for (U32 i = 0; i < level; i++) {
				log::Raw("  |");
			}
			String components = "";
			for (const auto& child : entityHolder->second.Components) {
				components += GetComponentName(child) + " ";
			}
			log::Raw("  |--> {} {{ {} }}\n", name, components);

			for (const auto& child : entityHolder->second.Children) {
				printTree(child, level + 1);
			}
		};
		printTree(m_RootEntity, 0);
	}

	void ECS::PrintSystems() const {
		for (const auto& [trigger, systems] : m_Systems) {
			log::Raw("Trigger: {}\n", SystemTriggerToString(trigger));
			for (const auto& system : systems) {
				log::Raw("  |--> {} (Priority: {})\n", system.Name, system.Priority);
			}
		}
	}


	void ECS::LinkInTree(const Entity& parent, const Entity& child)
	{
		TLC_ASSERT(parent != child, "ECS::LinkInTree: Parent and Child cannot be the same entity!");
		TLC_ASSERT(child != m_RootEntity, "ECS::LinkInTree: Cannot link the root entity!");

		auto parentActual = parent;
		if (parent == nullentity) {
			parentActual = m_RootEntity;
		}

		auto parentEntity = m_Entities.find(parentActual);
		auto childEntity = m_Entities.find(child);
		if (parentEntity == m_Entities.end() || childEntity == m_Entities.end()) {
			log::Warn("ECS::LinkInTree: Parent or Child entity not found!");
			return;
		}

		// if it is already a child of the parent, do nothing
		if (childEntity->second.Parent == parentActual) {
			return;
		}

		// Validate if the child is already a child of another parent
		if (childEntity->second.Parent != nullentity) {
			UnlinkInTree(childEntity->second.Parent, child);
		}

		parentEntity->second.Children.insert(child);
		childEntity->second.Parent = parentActual;
	}

	void ECS::UnlinkInTree(const Entity& parent, const Entity& child)
	{
		TLC_ASSERT(parent != child, "ECS::UnlinkInTree: Parent and Child cannot be the same entity!");
		TLC_ASSERT(child != m_RootEntity, "ECS::UnlinkInTree: Cannot unlink the root entity!");

		auto childEntity = m_Entities.find(child);
		if (childEntity == m_Entities.end()) {
			log::Warn("ECS::UnlinkInTree: Child entity not found!");
			return;
		}
		
		if (parent == nullentity) {
			// If the parent is null, just remove the parent
			childEntity->second.Parent = nullentity;
			return;
		}

		auto parentEntity = m_Entities.find(parent);
		if (parentEntity == m_Entities.end()) {
			log::Warn("ECS::UnlinkInTree: Parent entity not found!");
			return;
		}


		// if it is not a child of the parent, do nothing
		if (childEntity->second.Parent != parent) {
			return;
		}

		parentEntity->second.Children.erase(child);
		childEntity->second.Parent = nullentity;
	}

	List<Entity> ECS::FindByName(const String& name, const Entity& parent, Bool recursive) const {
		if (name.empty()) {
			return {};
		}

		List<Entity> result;
		if (parent == nullentity) {
			for (const auto& [_, entity] : m_Entities) {
				if (entity.Name == name) {
					result.emplace_back(entity.ID);
				}
			}
		} else {
			auto parentEntity = m_Entities.find(parent);
			if (parentEntity == m_Entities.end()) {
				return {};
			}

			// Simple BFS
			Queue<Entity> entitiesToSearch;
			entitiesToSearch.push(parent);

			while (!entitiesToSearch.empty()) {
				auto currentEntity = entitiesToSearch.front();
				entitiesToSearch.pop();

				auto entity = m_Entities.find(currentEntity);
				if (entity == m_Entities.end()) {
					continue;
				}

				if (entity->second.Name == name) {
					result.emplace_back(entity->second.ID);
				}

				if (recursive) {
					for (const auto& child : entity->second.Children) {
						entitiesToSearch.push(child);
					}
				}
			}
		}
		return result;
	}
	
	List<Entity> ECS::Find(const String& path, const Entity& parentIn) const {
		if (path.empty()) {
			return {};
		}
		auto items = utils::SplitString(path, "/");


		if (items.empty()) {
			return {};
		}

		auto parent = parentIn;

		List<Entity> result;
		for (auto i = 0; i < items.size(); i++) {
			auto name = items[i];
			auto entities = FindByName(name, parent, false);
			if (entities.empty()) {
				return {};
			}

			if (i == items.size() - 1) {
				result = entities;
				break;
			}

			parent = entities[0]; // We only care about the first entity
		}
		return result;		
	}

	List<Entity> ECS::CreatePath(const String& path, const Entity& parent) {
		auto items = utils::SplitString(path, "/");
		if (items.empty()) {
			return {};
		}

		List<Entity> entities;
		Entity currentParent = parent;
		for (const auto& item : items) {
			auto entityCurrent = FindByName(item, currentParent, false);

			// If the entity already exists, just set it as the current parent
			if (!entityCurrent.empty()) {
				entities.emplace_back(entityCurrent[0]);
				currentParent = entityCurrent[0];
				continue;
			}

			auto entity = CreateEntity(item, currentParent);
			if (entity == nullentity) {
				return {};
			}
			entities.emplace_back(entity);
			currentParent = entity;
		}

		return entities;
	}

	void ECS::DispatchSystems(SystemTrigger trigger, const List<UUID>& components) {
		auto systems = m_Systems.find(trigger);
		if (systems == m_Systems.end()) {
			return;
		}

		auto componentType = m_ComponentTypeMap[components[0]]; // All components are of the same type (assumption)
		auto filteredSystems = systems->second | std::ranges::views::filter([componentType](const auto& system) {
			return system.Filter == componentType;
		});

		for (const auto& system : filteredSystems) {
			for (const auto& component : components) {
				system.System->OnUpdate(this, GetComponentEntity(component), component);
			}
		}
	}

	void ECS::DeleteComponentApply(const UUID& component) {
		if (m_ComponentTypeMap.find(component) == m_ComponentTypeMap.end()) {
			log::Warn("ECS::DeleteComponentApply: Component does not exist!");
			return;
		}

		// remove from type map
		auto componentType = m_ComponentTypeMap[component];

		// remove from entity
		auto componentHolder = GetComponentHolder(component);

		m_ComponentTypeMap.erase(component);

		auto entity = m_Entities.find(componentHolder.EntityID);
		if (entity != m_Entities.end()) {
			auto& components = entity->second.Components;
			components.erase(std::remove(components.begin(), components.end(), component), components.end());
		}

		// remove from component pool
		auto pool = m_Components.find(componentType);
		if (pool != m_Components.end()) {
			pool->second.RemoveComponent(component);
		}
	}

	void ECS::MarkEntitiesForDeletion(const UUID& entity) {
		TLC_ASSERT(entity != m_RootEntity, "ECS::MarkEntitiesForDeletion: Cannot delete the root entity!");
		TLC_ASSERT(entity != nullentity, "ECS::MarkEntitiesForDeletion: Cannot delete a null entity!");
		TLC_ASSERT(m_Entities.find(entity) != m_Entities.end(), "ECS::MarkEntitiesForDeletion: Entity does not exist!");

		// Mark all components of the entity for deletion
		auto& components = GetComponents(entity);
		for (const auto& component : components) {
			DestroyComponent(component);
		}

		// Mark all children for deletion
		auto& children = GetChildren(entity);
		for (const auto& child : children) {
			MarkEntitiesForDeletion(child);
		}
	}

	void ECS::ApplyDeletions() {
		for (const auto& entity : m_EntitiesToRemove) {
			MarkEntitiesForDeletion(entity);
		}

		// Now actually delete the components
		for (const auto& component : m_ComponentsToRemove) {
			DeleteComponentApply(component);
		}

		for (const auto& entity : m_EntitiesToRemove) {
			UnlinkInTree(GetParent(entity), entity);
			m_Entities.erase(entity);
		}

		m_EntitiesToRemove.clear();
		m_ComponentsToRemove.clear();		
	}
}