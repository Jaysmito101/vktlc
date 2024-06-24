#include "game/scenes/MainScene.hpp"
#include "core/Application.hpp"

#include "game/components/TriangleUpDown.hpp"


// TODO: Make input system!
#include <GLFW/glfw3.h>

namespace tlc
{
	void MainScene::OnLoad(Bool isAsync)
	{
		(void) isAsync;
		
		log::Info("Loading Scene: {}", GetName());
		std::this_thread::sleep_for(std::chrono::seconds(1));	
	}

	void MainScene::OnUnload()
	{
		
	}

	void MainScene::OnStart()
	{
		auto renderer = GetECS()->CreateEntity("UpDownRenderer");
		auto rendererComponent = renderer->AddComponent<TriangleUpDown>();
		(void)rendererComponent;
		log::Info("MainScene started");
	}

	void MainScene::OnUpdate()
	{
		
		if (glfwGetKey(Window::Get()->GetHandle(), GLFW_KEY_A)) {
			Application::Get()->ChangeSceneAsync("TestScene");
		}
		else if (glfwGetKey(Window::Get()->GetHandle(), GLFW_KEY_C)) {
			Application::Get()->ChangeScene("TestScene");
		}

		if (glfwGetKey(Window::Get()->GetHandle(), GLFW_KEY_Q)) {
			GetECS()->GetEntities("UpDownRenderer")[0]->SetActive(false);
		}
		else {
			GetECS()->GetEntities("UpDownRenderer")[0]->SetActive(true);
		}
	}

	void MainScene::OnEnd()
	{
		log::Info("MainScene ended");
		
	}

	void MainScene::OnPause()
	{
	}

	void MainScene::OnResume()
	{
	}

	void MainScene::OnResize(U32 width, U32 height)
	{
		auto entity = GetECS()->GetEntities("UpDownRenderer")[0];
		entity->GetComponent<TriangleUpDown>()->Resize(width, height);
	}

}