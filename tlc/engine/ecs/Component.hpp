#pragma once

#include "core/Core.hpp"

namespace tlc {

	class ECS;
	class Entity;

	class IComponent
	{
	private:
		void Start();
		void Update();
		void End();
		void Pause();
		void Resume();

	public:
		inline const UUID& GetUUID() const { return m_UUID; }
		inline Bool IsMarkedForDeletion() const { return m_MarkedForDeletion; }
		inline Bool IsActive() const { return m_IsActive; }
		inline void SetActive(Bool active) { m_IsActive = active; }

		void Destroy();

	protected:
		virtual void OnStart() {}
		virtual void OnUpdate() {}
		virtual void OnEnd() {}
		virtual void OnPause() {}
		virtual void OnResume() {}

		void Initiate(Raw<ECS> ecs, Raw<Entity> entity);
		IComponent() = default;
		virtual ~IComponent();

		void MarkForDeletion() { m_MarkedForDeletion = true; }

	private:
		UUID m_UUID;
		Raw<ECS> m_ECS = nullptr;
		Raw<Entity> m_Entity = nullptr;
		Bool m_MarkedForDeletion = false;
		Bool m_IsActive = true;
		Bool m_HasStarted = false;

		friend class Scene;
		friend class Entity;
		friend class ECS;
	};

}
