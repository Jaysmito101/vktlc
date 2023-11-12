#pragma once

#include "core/Types.hpp"

#include <unordered_map>
#include <vector>
#include <functional>
#include <string>

#define BIND_EVENT_FN(fn) [this](auto&&... args) -> decltype(auto) { return this->fn(std::forward<decltype(args)>(args)...); }

namespace tlc
{

	using EventReciever = std::function<bool(void*)>;

	class EventManager
	{
	public:
		EventManager() = default;
		~EventManager() = default;

		void Subscribe(CString eventName, EventReciever callback, const String& callbackID = "");
		void Unsubscribe(CString recieverName, const String& callbackName);
		void UnsubscribeAll(CString recieverName);

		void RaiseEvent(CString eventName, void* paramsPtr = nullptr);

		inline static EventManager* Get() { if (!s_Instance) s_Instance = CreateScope<EventManager>(); return s_Instance.get(); }
		inline static void Shutdown() { s_Instance.reset(); }

	private:
		UnorderedMap<StringView, UnorderedMap<String, EventReciever>> m_EventSubscribers;

		static Scope<EventManager> s_Instance;
	};

}