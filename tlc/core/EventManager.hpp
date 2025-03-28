#pragma once

#include "core/Types.hpp"

#include <unordered_map>
#include <vector>
#include <functional>
#include <string>

#define BIND_EVENT_FN(fn) [this](auto&&... args) -> decltype(auto) { return this->fn(std::forward<decltype(args)>(args)...); }

namespace tlc
{

	enum EventType
	{
		None = 0,
		WindowClose, WindowSize, WindowPos, WindowCursorPos, WindowMouseButton,
		WindowKey, WindowChar, WindowScroll, WindowFocus, WindowFramebufferSize,
		SwapchainRecreate
	};


	template<EventType Type, typename... ParamsData>
	class EventManager
	{
	public:
		EventManager() = default;
		~EventManager() = default;

		void Subscribe(std::function<void(ParamsData...)> callback, String callbackID = "")
		{
			if (callbackID.empty())
				callbackID = "callback" + std::to_string(rand());

			m_EventSubscribers.emplace(callbackID, callback);
		}

		void Unsubscribe(const String& callbackName)
		{
			auto it = m_EventSubscribers.find(callbackName);
			if (it != m_EventSubscribers.end())
				m_EventSubscribers.erase(it);
		}

		void UnsubscribeAll()
		{
			m_EventSubscribers.clear();
		}

		void RaiseEvent(ParamsData ...params)
		{
			for (auto& callback : m_EventSubscribers)
			{
				callback.second(params...);
			}
		}

		inline static EventManager* Get() { if (!s_Instance) s_Instance = CreateScope<EventManager<Type, ParamsData...>>(); return s_Instance.get(); }
		inline static void Shutdown() { s_Instance.reset(); }

	private:
		UnorderedMap<String, std::function<void(ParamsData...)>> m_EventSubscribers;

		static Scope<EventManager<Type, ParamsData...>> s_Instance;
	};


	template<EventType Type, typename... ParamsData>
	Scope<EventManager<Type, ParamsData...>> EventManager<Type, ParamsData...>::s_Instance = nullptr;
}