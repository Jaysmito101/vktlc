#include "core/EventManager.hpp"

namespace tlc
{
	Scope<EventManager> EventManager::s_Instance = nullptr;

	

	void EventManager::Subscribe(CString eventName, EventReciever callback, const String& callbackIDIn)
	{
		if (m_EventSubscribers.find(eventName) == m_EventSubscribers.end())
			m_EventSubscribers[eventName] = std::unordered_map<String, EventReciever>();

		String callbackID = callbackIDIn;

		if (callbackID == "")
			callbackID = "callback" + std::to_string(m_EventSubscribers[eventName].size());
		
		m_EventSubscribers[eventName][callbackID] = (callback);
	}

	void EventManager::Unsubscribe(CString recieverName, const String& callbackName)
	{
		if (m_EventSubscribers.find(recieverName) == m_EventSubscribers.end())
			return;
		
		if (m_EventSubscribers[recieverName].find(callbackName) == m_EventSubscribers[recieverName].end())
			return;

		m_EventSubscribers[recieverName].erase(callbackName);
	}

	void EventManager::UnsubscribeAll(CString recieverName)
	{
		if (m_EventSubscribers.find(recieverName) == m_EventSubscribers.end())
			return;

		m_EventSubscribers[recieverName].clear();
	}

	void EventManager::RaiseEvent(CString eventName, void* paramsPtr)
	{
		if (m_EventSubscribers.find(eventName) == m_EventSubscribers.end())
			return;

		for (const auto& [callbackID, callback] : m_EventSubscribers[eventName])
			if (callback(paramsPtr))
				break;
	}
}