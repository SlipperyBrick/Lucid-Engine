#pragma once

#include <string>

#include "Lucid/Core/Events/Event.h"
#include "Lucid/Core/Timestep.h"

// Interface for creation of layer abstraction
class Layer
{

public:

	Layer(const std::string& debugName = "Layer");
	virtual ~Layer();

	virtual void OnAttach() {}
	virtual void OnDetach() {}
	virtual void OnUpdate(Timestep ts) {}
	virtual void OnImGuiRender() {}
	virtual void OnEvent(Event& event) {}

	inline const std::string& GetName() const { return m_DebugName; }

protected:

	std::string m_DebugName;
};