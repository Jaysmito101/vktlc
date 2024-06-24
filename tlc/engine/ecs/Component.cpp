#include "engine/ecs/Component.hpp"
#include "engine/ecs/ECS.hpp"

namespace tlc {
	
	void IComponent::Initiate(Raw<ECS> ecs, Raw<Entity> entity)
	{
		m_UUID = utils::GenerateUUID();
		m_ECS = ecs;
		m_Entity = entity;
	}

	IComponent::~IComponent()
	{
	}

	void IComponent::Start()
	{
		if (m_HasStarted) {
			return;
		}

		this->OnStart();

		m_HasStarted = true;
	}

	void IComponent::Update()
	{
		if (!m_IsActive) return;
		this->OnUpdate();
	}

	void IComponent::End()
	{
		if (!m_HasStarted) {
			return;
		}

		this->OnEnd();
		m_HasStarted = false;
	}

	void IComponent::Pause()
	{
		if (!m_IsActive) return;
		this->OnPause();
	}

	void IComponent::Resume()
	{
		if (m_IsActive) return;
		this->OnResume();
	}

	void IComponent::Destroy()
	{
		m_MarkedForDeletion = true;
	}
}