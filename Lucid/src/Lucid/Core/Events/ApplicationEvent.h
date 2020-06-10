#pragma once

#include "ldpch.h"

#include "Event.h"

// Window resize event that stores the new width and height of the resized window
class WindowResizeEvent : public Event
{

public:

	WindowResizeEvent(unsigned int width, unsigned int height)
		: m_Width(width), m_Height(height) {}

	inline unsigned int GetWidth() const { return m_Width; }
	inline unsigned int GetHeight() const { return m_Height; }

	// Override to print event data for debugging purposes
	std::string ToString() const override
	{
		std::stringstream ss;
		ss << "WindowResizeEvent: " << m_Width << ", " << m_Height;

		return ss.str();
	}

	// Implement the static type for window resize
	static EventType GetStaticType() { return EventType::WindowResize; }

	virtual EventType GetEventType() const override { return GetStaticType(); }
	virtual const char* GetName() const override { return "WindowResize"; }

	// Implement category flags for a window resize event
	virtual int GetCategoryFlags() const override { return EventCategoryApplication; }

private:

	unsigned int m_Width;
	unsigned int m_Height;
};

// Window close event
class WindowCloseEvent : public Event
{

public:

	WindowCloseEvent() {}

	// Implement the static type for window close
	static EventType GetStaticType() { return EventType::WindowClose; }

	virtual EventType GetEventType() const override { return GetStaticType(); }
	virtual const char* GetName() const override { return "WindowClose"; }

	// Implement category flags for a window close event
	virtual int GetCategoryFlags() const override { return EventCategoryApplication; }
};

// Application tick event
class AppTickEvent : public Event
{

public:

	AppTickEvent() {}

	// Implement the static type for app tick
	static EventType GetStaticType() { return EventType::AppTick; }

	virtual EventType GetEventType() const override { return GetStaticType(); }
	virtual const char* GetName() const override { return "AppTick"; }

	// Implement category flags for an app tick event
	virtual int GetCategoryFlags() const override { return EventCategoryApplication; }
};

// Application update event
class AppUpdateEvent : public Event
{

public:

	AppUpdateEvent() {}

	// Implement the static type for app update
	static EventType GetStaticType() { return EventType::AppUpdate; }

	virtual EventType GetEventType() const override { return GetStaticType(); }
	virtual const char* GetName() const override { return "AppUpdate"; }

	// Implement category flags for an app update event
	virtual int GetCategoryFlags() const override { return EventCategoryApplication; }
};

// Application render event
class AppRenderEvent : public Event
{

public:

	AppRenderEvent() {}

	// Implement the static type for app render
	static EventType GetStaticType() { return EventType::AppRender; }

	virtual EventType GetEventType() const override { return GetStaticType(); }
	virtual const char* GetName() const override { return "AppRender"; }

	// Implement category flags for an app render event
	virtual int GetCategoryFlags() const override { return EventCategoryApplication; }
};