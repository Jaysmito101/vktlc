#include "engine/Scene.hpp"

namespace tlc
{

	Scene::Scene()
	{
		m_UUID = utils::GenerateUUID();
		m_ECS = CreateScope<ECS>();
	}

	Scene::~Scene()
	{
	}

	void Scene::Load(Bool isAsync)
	{
		OnLoad(isAsync);
		m_HasLoaded = true;
	}

	void Scene::Unload()
	{
		OnUnload();
		m_HasLoaded = false;
	}

	void Scene::Start()
	{
		auto components = m_ECS->GetAllComponents();
		for (auto& component : components)
		{
			component->Start();
		}

		OnStart();
	}

	void Scene::Update()
	{
		OnUpdate();

		auto components = m_ECS->GetAllComponents();
		for (auto& component : components)
		{
			component->Update();
		}


		m_ECS->HandleDeletions();
	}

	void Scene::End()
	{
		OnEnd();

		auto components = m_ECS->GetAllComponents();
		for (auto& component : components)
		{
			component->End();
		}
	}

	void Scene::Pause()
	{
		OnPause();

		auto components = m_ECS->GetAllComponents();
		for (auto& component : components)
		{
			component->Pause();
		}

		m_IsPaused = true;
	}

	void Scene::Resume()
	{
		OnResume();

		auto components = m_ECS->GetAllComponents();
		for (auto& component : components)
		{
			component->Resume();
		}


		m_IsPaused = false;
	}

	void Scene::Resize(U32 width, U32 height)
	{
		OnResize(width, height);
	}

}