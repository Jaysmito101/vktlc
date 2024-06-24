#include "game/scenes/TestScene.hpp"
#include "core/Application.hpp"

#include "game/components/TriangleLeftRight.hpp"


// TODO: Make input system!
#include <GLFW/glfw3.h>

namespace tlc
{
	void TestScene::OnLoad(Bool isAsync)
	{
		(void) isAsync;
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}

	void TestScene::OnUnload()
	{
	}

	void TestScene::OnStart()
	{
		auto renderer = GetECS()->CreateEntity("Renderer");
		auto rendererComponent = renderer->AddComponent<TriangleLeftRight>();
		(void)rendererComponent;
		log::Info("TestScene started");
	}

	void TestScene::OnUpdate()
	{
		if (glfwGetKey(Window::Get()->GetHandle(), GLFW_KEY_A)) {
			Application::Get()->ChangeSceneAsync("MainScene");
		}
		else if (glfwGetKey(Window::Get()->GetHandle(), GLFW_KEY_C)) {
			Application::Get()->ChangeScene("MainScene");
		}
		
		if (glfwGetKey(Window::Get()->GetHandle(), GLFW_KEY_Q) && GetECS()->GetEntities("Renderer").size() > 0) {
			GetECS()->GetEntities("Renderer")[0]->Destroy();
		}
	}

	void TestScene::OnEnd()
	{
		log::Info("TestScene ended");
	}

	void TestScene::OnPause()
	{
	}

	void TestScene::OnResume()
	{
	}

	void TestScene::OnResize(U32 width, U32 height)
	{
		(void) width; (void) height;
		auto entity = GetECS()->GetEntities("UpDownRenderer")[0];
		entity->GetComponent<TriangleLeftRight>()->Resize(width, height);
	}

}