#pragma once

#include "ldpch.h"

#include "Event.h"

// Mouse move event that contains the x and y position of the mouse
class MouseMovedEvent : public Event
{

public:

	MouseMovedEvent(float x, float y)
		: m_MouseX(x), m_MouseY(y) {}

	inline float GetX() const { return m_MouseX; }
	inline float GetY() const { return m_MouseY; }

	// Override to print event data for debugging purposes
	std::string ToString() const override
	{
		std::stringstream ss;
		ss << "MouseMovedEvent: " << m_MouseX << ", " << m_MouseY;

		return ss.str();
	}

	// Implement the static type for mouse moved
	static EventType GetStaticType() { return EventType::MouseMoved; }

	virtual EventType GetEventType() const override { return GetStaticType(); }
	virtual const char* GetName() const override { return "MouseMoved"; }

	// Implement category flags for mouse button events
	virtual int GetCategoryFlags() const override { return EventCategoryMouse | EventCategoryInput; }

private:

	float m_MouseX;
	float m_MouseY;
};

// Mouse scroll event that contains the x and y offset of the mouse scroll (supports horizontal scrolling)
class MouseScrolledEvent : public Event
{

public:

	MouseScrolledEvent(float xOffset, float yOffset)
		: m_XOffset(xOffset), m_YOffset(yOffset) {}

	inline float GetXOffset() const { return m_XOffset; }
	inline float GetYOffset() const { return m_YOffset; }

	// Override to print event data for debugging purposes
	std::string ToString() const override
	{
		std::stringstream ss;
		ss << "MouseScrolledEvent: " << GetXOffset() << ", " << GetYOffset();

		return ss.str();
	}

	// Implement the static type for mouse scrolled
	static EventType GetStaticType() { return EventType::MouseScrolled; }

	virtual EventType GetEventType() const override { return GetStaticType(); }
	virtual const char* GetName() const override { return "MouseScrolled"; }

	// Implement category flags for mouse button events
	virtual int GetCategoryFlags() const override { return EventCategoryMouse | EventCategoryInput; }

private:

	float m_XOffset;
	float m_YOffset;
};

// Base class for a mouse button event that contains the mouse button
class MouseButtonEvent : public Event
{

public:

	inline int GetMouseButton() const { return m_Button; }

	// Implement category flags for mouse button events
	virtual int GetCategoryFlags() const override { return EventCategoryInput; }

protected:

	// Only derived classes can call this constructor
	MouseButtonEvent(int button)
		: m_Button(button) {}

	int m_Button;
};

// Mouse button pressed event that contains the mouse button pressed
class MouseButtonPressedEvent : public MouseButtonEvent
{

public:

	MouseButtonPressedEvent(int button)
		: MouseButtonEvent(button) {}

	// Override to print event data for debugging purposes
	std::string ToString() const override
	{
		std::stringstream ss;
		ss << "MouseButtonPressedEvent: " << m_Button;

		return ss.str();
	}

	// Implement the static type for mouse button pressed
	static EventType GetStaticType() { return EventType::MouseButtonPressed; }

	virtual EventType GetEventType() const override { return GetStaticType(); }
	virtual const char* GetName() const override { return "MouseButtonPressed"; }
};

// Mouse button released event that contains the mouse button released
class MouseButtonReleasedEvent : public MouseButtonEvent
{

public:

	MouseButtonReleasedEvent(int button)
		: MouseButtonEvent(button) {}

	// Override to print event data for debugging purposes
	std::string ToString() const override
	{
		std::stringstream ss;
		ss << "MouseButtonReleasedEvent: " << m_Button;

		return ss.str();
	}

	// Implement the static type for mouse button released
	static EventType GetStaticType() { return EventType::MouseButtonReleased; }

	virtual EventType GetEventType() const override { return GetStaticType(); }
	virtual const char* GetName() const override { return "MouseButtonReleased"; }
};