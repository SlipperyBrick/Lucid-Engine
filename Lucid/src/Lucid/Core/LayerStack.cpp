#include "ldpch.h"

#include "LayerStack.h"

LayerStack::LayerStack()
{
}

LayerStack::~LayerStack()
{
	for (Layer* layer : m_Layers)
	{
		delete layer;
	}
}

// Pushes a layer into the layer stack
void LayerStack::PushLayer(Layer* layer)
{
	m_Layers.emplace(m_Layers.begin() + m_LayerInsertIndex, layer);
	m_LayerInsertIndex++;
}

// Pushes an overlay into the layer stack
void LayerStack::PushOverlay(Layer* overlay)
{
	m_Layers.emplace_back(overlay);
}

// Pops a layer from the layer stack
void LayerStack::PopLayer(Layer* layer)
{
	auto it = std::find(m_Layers.begin(), m_Layers.end(), layer);

	if (it != m_Layers.end())
	{
		m_Layers.erase(it);
		m_LayerInsertIndex--;
	}

}

// Pops and overlay from the layer stack
void LayerStack::PopOverlay(Layer* overlay)
{
	auto it = std::find(m_Layers.begin(), m_Layers.end(), overlay);

	if (it != m_Layers.end())
	{
		m_Layers.erase(it);
	}
}