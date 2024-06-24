#pragma once
#include "core/Core.hpp"
#include "engine/ecs/ECS.hpp"
#include "rendering/Renderer.hpp"

namespace tlc 
{
	class Application;

	class Scene 
	{
	public:
		Scene();
		virtual ~Scene();

		void Load(Bool isAsync);
		void Unload();
		void Start();
		void Update();
		void End();
		void Pause();
		void Resume();
		void Resize(U32 width, U32 height);

		virtual void OnLoad(Bool isAsync) = 0;
		virtual void OnUnload() = 0;
		virtual void OnStart() = 0;
		virtual void OnUpdate() = 0;
		virtual void OnEnd() = 0;
		virtual void OnPause() = 0;
		virtual void OnResume() = 0;
		virtual void OnResize(U32 width, U32 height) = 0;


		inline UUID GetUUID() const { return m_UUID; }
		inline Bool HasLoaded() const { return m_HasLoaded; }
		inline Bool IsPaused() const { return m_IsPaused; }
		inline const String& GetName() const { return m_Name; }

		inline Raw<ECS> GetECS() { return m_ECS.get(); }
		inline const Raw<ECS> GetECS() const { return m_ECS.get(); }

	private:
		inline void SetName(const String& name) { m_Name = name; }

	private:
		UUID m_UUID = 0;
		Bool m_HasLoaded = false;
		Bool m_IsPaused = false;

		String m_Name = "Unnamed Scene";
		
		Scope<ECS> m_ECS;

		friend class Application;
	};

}