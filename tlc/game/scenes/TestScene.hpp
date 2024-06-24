
#pragma once


#include "engine/Scene.hpp"


namespace tlc
{

	class TestScene : public Scene
	{
	public:
		virtual void OnLoad(Bool isAsync) override;
		virtual void OnUnload() override;
		virtual void OnStart() override;
		virtual void OnUpdate() override;
		virtual void OnEnd() override;
		virtual void OnPause() override;
		virtual void OnResume() override;
		virtual void OnResize(U32 width, U32 height) override;

	private:
	};

}
