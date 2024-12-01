#include "engine/ecs/Entity.hpp"


namespace tlc {
	Entity::Entity(Raw<ECS> ecs, String name)
	{
		m_ECS = ecs;
		m_UUID = UUID::New();
		m_Name = name;
	}

	Entity::~Entity()
	{
	}

	void Entity::AddChild(Raw<Entity> child)
	{
		if (child != nullptr && std::find(m_Children.begin(), m_Children.end(), child) == m_Children.end()) {
			m_Children.push_back(child);
		}
	}

	void Entity::RemoveChild(UUID child)
	{
		for (auto it = m_Children.begin(); it != m_Children.end(); ++it) {
			if ((*it)->GetUUID()  == child) {
				m_Children.erase(it);
				break;
			}
		}
	}

	List<Raw<IComponent>> Entity::GetComponentsOfType(Size typeID) const
	{
		if (m_Components.find(typeID) != m_Components.end()) {
			return m_Components.at(typeID);
		}
		return List<Raw<IComponent>>();
	}

	void Entity::SetParent(Raw<Entity> parent)
	{
		if (parent == nullptr) {
			parent = m_ECS->GetRootEntity();
		}

		if (m_Parent != nullptr) {
			m_Parent->RemoveChild(m_UUID);
		}

		m_Parent = parent;

		if (m_Parent != nullptr) {
			m_Parent->AddChild(this);

			if (m_Parent->IsMarkedForDeletion()) {
				Destroy();
			}

			SetActive(m_Parent->IsActive(), 1);
		}
	}


	void Entity::Destroy()
	{
		m_MarkedForDeletion = true;
		for (auto& [typeID, components] : m_Components) {
			for (auto component : components) {
				component->Destroy();
			}
		}

		for (auto child : m_Children) {
			child->Destroy();
		}
	}

	void Entity::SetActive(Bool active, U32 depth)
	{
		auto sateBefore = m_Active;

		if (depth == 0) {
			m_Active = active;
		}
		else if (m_Active && !active) {
			m_Active = active;
		}

		for (auto child : m_Children) {
			child->SetActive(active, depth + 1);
		}

		if (sateBefore && !m_Active) {
			for (auto& [typeID, components] : m_Components) {
				for (auto component : components) {
					component->SetActive(false);
					component->Pause();
				}
			}
		}
		else if (!sateBefore && m_Active) {
			for (auto& [typeID, components] : m_Components) {
				for (auto component : components) {
					component->SetActive(true);
					component->Resume();
				}
			}
		}
	}
}


