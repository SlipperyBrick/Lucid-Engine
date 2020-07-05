#pragma once

#include "ldpch.h"

// Enumerators for various event types
enum class EventType
{
	None = 0,

	// Window events
	WindowClose, WindowResize, WindowFocus, WindowLostFocus, WindowMoved,

	// Application events
	AppTick, AppUpdate, AppRender,

	// Keyboard events
	KeyPressed, KeyReleased, KeyTyped,

	// Mouse events
	MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled
};

// Organizes each event type into its individual category
enum EventCategory
{
	None = 0,
	EventCategoryApplication	= BIT(0),
	EventCategoryInput			= BIT(1),
	EventCategoryKeyboard		= BIT(2),
	EventCategoryMouse			= BIT(3),
	EventCategoryMouseButton	= BIT(4)
};

// Base class for all events
class Event
{

public:

	bool Handled = false;

	virtual EventType GetEventType() const = 0;
	virtual const char* GetName() const = 0;
	virtual int GetCategoryFlags() const = 0;

	// Can be overridden in derived classes to print more information pertaining to the event type
	virtual std::string ToString() const { return GetName(); }

	// Utility function for checking if an event is in a given category (for debugging purposes)
	inline bool IsInCategory(EventCategory category)
	{
		return GetCategoryFlags() & category;
	}
};

// Handles dispatching of events based on their type
class EventDispatcher
{

	// Using of an event types function (T&) that is passed by reference
	template<typename T>
	using EventFn = std::function<bool(T&)>;

public:

	// Recieves an event reference for any type of event
	EventDispatcher(Event& event)
		: m_Event(event)
	{
	}

	// Passes an event function that is implemented by the caller
	template<typename T>
	bool Dispatch(EventFn<T> func)
	{
		// Checks the event type of m_Event against an event functions static type
		if (m_Event.GetEventType() == T::GetStaticType())
		{
			// Run the event function and flag it as handled
			m_Event.Handled = func(*(T*)&m_Event);
			return true;
		}

		// If the event type doesn't match the event function passed, do not dispatch the event
		return false;
	}

private:

	Event& m_Event;
};

// Overloaded operator for logging events via the log class (for debugging purposes)
inline std::ostream& operator<<(std::ostream& os, const Event& e)
{
	return os << e.ToString();
}