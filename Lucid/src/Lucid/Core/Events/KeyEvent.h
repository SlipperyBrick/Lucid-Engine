#pragma once

#include "ldpch.h"

#include "Event.h"

#include "Lucid/Core/KeyCodes.h"

// Base class for a key event that contains the keycode
class KeyEvent : public Event
{

public:

	inline int GetKeyCode() const { return m_KeyCode; }

	// Implement category flags for key events
	virtual int GetCategoryFlags() const override { return EventCategoryKeyboard | EventCategoryInput; }

protected:

	// Only derived classes can call this constructor
	KeyEvent(int keycode)
		: m_KeyCode(keycode) {}

	int m_KeyCode;
};

// Key pressed event that contains the keycode of the key along with a repeat count if a key is held down
class KeyPressedEvent : public KeyEvent
{

public:

	KeyPressedEvent(int keycode, int repeatCount)
		: KeyEvent(keycode), m_RepeatCount(repeatCount) {}

	inline int GetRepeatCount() const { return m_RepeatCount; }

	// Override to print event data for debugging purposes
	std::string ToString() const override
	{
		std::stringstream ss;
		ss << "KeyPressedEvent: " << m_KeyCode << " (" << m_RepeatCount << " repeats)";

		return ss.str();
	}

	// Implement the static type for key pressed
	static EventType GetStaticType() { return EventType::KeyPressed; }

	virtual EventType GetEventType() const override { return GetStaticType(); }
	virtual const char* GetName() const override { return "KeyPressed"; }

private:

	int m_RepeatCount;
};

// Key released event that contains the keycode of the released key
class KeyReleasedEvent : public KeyEvent
{

public:

	KeyReleasedEvent(int keycode)
		: KeyEvent(keycode) {}

	// Override to print event data for debugging purposes
	std::string ToString() const override
	{
		std::stringstream ss;
		ss << "KeyReleasedEvent: " << m_KeyCode;

		return ss.str();
	}

	// Implement the static type for key released
	static EventType GetStaticType() { return EventType::KeyReleased; }

	virtual EventType GetEventType() const override { return GetStaticType(); }
	virtual const char* GetName() const override { return "KeyReleased"; }
};

// Similar to a KeyPressedEvent but doesn't store a repeat count, meant for short key presses (e.g, when naturally typing)
class KeyTypedEvent : public KeyEvent
{

public:

	KeyTypedEvent(int keycode)
		: KeyEvent(keycode) {}

	// Override to print event data for debugging purposes
	std::string ToString() const override
	{
		std::stringstream ss;
		ss << "KeyTypedEvent: " << m_KeyCode;

		return ss.str();
	}

	// Implement the static type for key typed
	static EventType GetStaticType() { return EventType::KeyTyped; }

	virtual EventType GetEventType() const override { return GetStaticType(); }
	virtual const char* GetName() const override { return "KeyTyped"; }
};