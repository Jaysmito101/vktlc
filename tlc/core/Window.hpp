#pragma once
#include "core/Core.hpp"

struct GLFWwindow;

namespace tlc
{
	class Window
	{
	public:
		Window();
		~Window();

		void SetSize(I32 width, I32 height);
		void SetPosition(I32 x, I32 y);	
		void SetTitle(const String& title);
		void SetFullscreen(Bool value);
		void SetCursorPos(F32 x, F32 y);

		Pair<I32, I32> GetSize();
		Pair<I32, I32> GetPosition();
		Pair<F32, F32> GetCursorPos();

		void Update();

		inline Bool IsFullscreen() { return m_IsFullscreen; }
		inline void ToggleFullscreen() { SetFullscreen(!m_IsFullscreen); }

		inline GLFWwindow* GetHandle() { return m_Handle; }

		inline static Window* Get() { if (!s_Instance) s_Instance = CreateScope<Window>(); return s_Instance.get(); }
		inline static void Shutdown() { s_Instance.reset(); }

	private:
		Bool CreateWindowHandle();
		void SetupCallbacks();

	private:
		GLFWwindow* m_Handle = nullptr;
		Bool m_IsFullscreen = false;
		Pair<I32, I32> m_LastPosition = MakePair<I32, I32>(0, 0);
		Pair<I32, I32> m_LastSize = MakePair<I32, I32>(0, 0);

		static Scope<Window> s_Instance;
	};

}
