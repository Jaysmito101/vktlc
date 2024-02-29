#include "engine/Scene.hpp"

namespace tlc
{

	Scene::Scene()
	{
		m_UUID = utils::GenerateUUID();
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
		OnStart();
	}

	void Scene::Update()
	{
		OnUpdate();
	}

	void Scene::End()
	{
		OnEnd();
	}

	void Scene::Pause()
	{
		OnPause();
		m_IsPaused = true;
	}

	void Scene::Resume()
	{
		OnResume();
		m_IsPaused = false;
	}

	void Scene::Resize(U32 width, U32 height)
	{
		OnResize(width, height);
	}

}