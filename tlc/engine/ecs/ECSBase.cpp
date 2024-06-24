#include "engine/ecs/ECSBase.hpp"
#include "engine/ecs/Entity.hpp"

namespace tlc
{ 

	ECS::ECS()
	{
		auto rootEntity = new Entity(this, "__Root");
		m_RootEntity = rootEntity;
		m_Entities.push_back(rootEntity);
	}

	ECS::~ECS()
	{
		for (auto entity : m_Entities) {
			DeleteRaw(entity);
		}
		m_Entities.clear();

		for (auto& pair : m_Components) {
			for (auto component : pair.second) {
				component->End();
				DeleteRaw(component);
			}
		}
		m_Components.clear();
	}

	List<Raw<Entity>> ECS::GetEntities(const String& name) const
	{
		List<Raw<Entity>> result;
		for (auto entity : m_Entities) {
			if (entity->GetName() == name) {
				result.push_back(entity);
			}
		}
		return result;
	}

	Raw<Entity> ECS::GetEntity(UUID uuid) const
	{
		for (auto entity : m_Entities) {
			if (entity->GetUUID() == uuid) {
				return entity;
			}
		}
		return nullptr;
	}

	List<Raw<Entity>> ECS::GetAllEntities() const
	{
		return m_Entities;
	}

	void ECS::HandleDeletions()
	{
		List<Raw<Entity>> entitiesToDelete;
		for (auto entity : m_Entities) 
		{
			if (entity->IsMarkedForDeletion()) 
			{
				entitiesToDelete.push_back(entity);
			}
		}

		for (auto entity : entitiesToDelete) 
		{
			m_Entities.erase( std::find(m_Entities.begin(), m_Entities.end(), entity) );
			DeleteRaw(entity);
		}

		for (auto& pair : m_Components) 
		{
			for (auto component : pair.second) 
			{
				List<Raw<IComponent>> componentsToDelete;

				if (component->IsMarkedForDeletion()) 
				{
					componentsToDelete.push_back(component);
				}


				for (auto comp : componentsToDelete)
				{
					comp->End();
					m_Components[pair.first].erase(std::find(m_Components[pair.first].begin(), m_Components[pair.first].end(), comp));
					DeleteRaw(comp);
				}
			}
		}
	}

	Raw<Entity> ECS::CreateEntity(String name, Raw<Entity> parent) 
	{
		auto entity = new Entity(this, name);
		entity->SetParent(parent);
		m_Entities.push_back(entity);		
		return entity;
	}

}