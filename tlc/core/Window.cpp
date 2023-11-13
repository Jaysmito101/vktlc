#include "core/Window.hpp"

#include "GLFW/glfw3.h"

namespace tlc
{
	Scope<Window> Window::s_Instance = nullptr;

	static void GLFWErrorCallback(I32 error, const char* description)
	{
		log::Error("GLFW Error ({0}): {1}", error, description);
	}

	static void GLFWWindowPosCallback(GLFWwindow* window, I32 x, I32 y)
	{
		(void)window;
		EventManager<EventType::WindowPos, I32, I32>::Get()->RaiseEvent(x, y);
	}

	static void GLFWWindowSizeCallback(GLFWwindow* window, I32 width, I32 height)
	{
		(void)window;
		auto size = MakePair<I32, I32>(width, height);
		EventManager<EventType::WindowSize, I32, I32>::Get()->RaiseEvent(width, height);
	}

	static void GLFWWindowCloseCallback(GLFWwindow* window)
	{
		(void)window;
		EventManager<EventType::WindowClose>::Get()->RaiseEvent();
	}

	static void GLFWWindowCursorPosCallback(GLFWwindow* window, F64 x, F64 y)
	{
		(void)window;
		EventManager<EventType::WindowCursorPos, F64, F64>::Get()->RaiseEvent(x, y);
	}

	static void GLFWWindowMouseButtonCallback(GLFWwindow* window, I32 button, I32 action, I32 mods)
	{
		(void)window;
		EventManager<EventType::WindowMouseButton, I32, I32, I32>::Get()->RaiseEvent(button, action, mods);
	}

	static void GLFWWindowKeyCallback(GLFWwindow* window, I32 key, I32 scancode, I32 action, I32 mods)
	{
		(void)window;
		EventManager<EventType::WindowKey, I32, I32, I32, I32>::Get()->RaiseEvent(key, scancode, action, mods);
	}

	static void GLFWWindowCharCallback(GLFWwindow* window, U32 keycode)
	{
		(void)window;
		EventManager<EventType::WindowChar, U32>::Get()->RaiseEvent(keycode);
	}

	static void GLFWWindowScrollCallback(GLFWwindow* window, F64 xoffset, F64 yoffset)
	{
		(void)window;
		EventManager<EventType::WindowScroll, F64, F64>::Get()->RaiseEvent(xoffset, yoffset);
	}

	Window::Window()
	{
		if (s_Instance != nullptr)
		{
			log::Fatal("Window already exists!");			
		}

		log::Info("Creating window");

		if (!glfwInit())
		{
			log::Fatal("Failed to setup GLFW!");
		}

		if (!glfwVulkanSupported())
		{
			log::Fatal("Vulkan not supported!");
		}

		if (!CreateWindowHandle())
		{
			log::Fatal("Failed to create window!");
		}

	}

	Window::~Window()
	{
		log::Info("Destroying window");
		glfwDestroyWindow(m_Handle);
		glfwTerminate();
	}

	void Window::SetSize(I32 width, I32 height)
	{
		glfwSetWindowSize(m_Handle, width, height);
	}

	void Window::SetPosition(I32 x, I32 y)
	{
		glfwSetWindowPos(m_Handle, x, y);
	}

	void Window::SetTitle(const String& title)
	{
		glfwSetWindowTitle(m_Handle, title.c_str());
	}

	void Window::SetFullscreen(Bool value)
	{
		if (m_IsFullscreen == value) return;

		if (value)
		{
			m_LastPosition = GetPosition();
			m_LastSize = GetSize();

			const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());

			if (mode == nullptr)
			{
				log::Error("Failed to get video mode!");
				return;
			}

			glfwSetWindowMonitor(
				m_Handle,
				glfwGetPrimaryMonitor(),
				0,
				0,
				mode->width,
				mode->height,
				mode->refreshRate
			);

			SetSize(mode->width, mode->height);
			SetPosition(0, 0);

			m_IsFullscreen = true;
		}
		else
		{
			glfwSetWindowMonitor(
				m_Handle,
				nullptr,
				m_LastPosition.x,
				m_LastPosition.y,
				m_LastSize.x,
				m_LastSize.y,
				0
			);

			SetSize(m_LastSize.x, m_LastSize.y);
			SetPosition(m_LastPosition.x, m_LastPosition.y);

			m_IsFullscreen = false;
		}

	}

	void Window::SetCursorPos(F32 x, F32 y)
	{
		glfwSetCursorPos(m_Handle, static_cast<F64>(x), static_cast<F64>(y));
	}

	Pair<I32, I32> Window::GetSize()
	{
		I32 width = 0, height = 0;
		glfwGetWindowSize(m_Handle, &width, &height);
		return MakePair<I32, I32>(width, height);
	}

	Pair<I32, I32> Window::GetPosition()
	{
		I32 x = 0, y = 0;
		glfwGetWindowPos(m_Handle, &x, &y);
		return MakePair<I32, I32>(x, y);
	}

	Pair<F32, F32> Window::GetCursorPos()
	{
		F64 x = 0, y = 0;
		glfwGetCursorPos(m_Handle, &x, &y);
		return MakePair<F32, F32>(static_cast<F32>(x), static_cast<F32>(y));
	}

	void Window::Update()
	{
		glfwPollEvents();
	}

	Bool Window::CreateWindowHandle()
	{
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

		m_Handle = glfwCreateWindow(
			512,
			512,
			"TLC Window",
			nullptr,
			nullptr
		);

		if (m_Handle == nullptr)
		{
			return false;
		}

		glfwSetWindowUserPointer(m_Handle, this);

		SetupCallbacks();

		return true;
	}

	void Window::SetupCallbacks()
	{
		glfwSetErrorCallback(GLFWErrorCallback);
		glfwSetCharCallback(m_Handle, GLFWWindowCharCallback);
		glfwSetCursorPosCallback(m_Handle, GLFWWindowCursorPosCallback);
		glfwSetKeyCallback(m_Handle, GLFWWindowKeyCallback);
		glfwSetMouseButtonCallback(m_Handle, GLFWWindowMouseButtonCallback);
		glfwSetScrollCallback(m_Handle, GLFWWindowScrollCallback);
		glfwSetWindowSizeCallback(m_Handle, GLFWWindowSizeCallback);
		glfwSetWindowPosCallback(m_Handle, GLFWWindowPosCallback);
		glfwSetWindowCloseCallback(m_Handle, GLFWWindowCloseCallback);
	}

	

}
