#include "hkpch.h"
#include "LayerStack.h"

namespace Haketon {

	LayerStack::LayerStack()
	{
		m_LayerInsert = m_Layers.begin();
	}

	LayerStack::~LayerStack()
	{
		for (Layer* layer : m_Layers)
			delete layer;
	}

	void LayerStack::PushLayer(Layer* Layer)
	{
		m_LayerInsert = m_Layers.emplace(m_LayerInsert, Layer);
	}

	void LayerStack::PushOverlay(Layer* Overlay)
	{
		m_Layers.emplace_back(Overlay);
	}

	void LayerStack::PopLayer(Layer* Layer)
	{
		auto it = std::find(m_Layers.begin(), m_Layers.end(), Layer);
		if (it != m_Layers.end())
		{
			m_Layers.erase(it);
			m_LayerInsert--;
		}
	}

	void LayerStack::PopOverlay(Layer* Overlay)
	{
		auto it = std::find(m_Layers.begin(), m_Layers.end(), Overlay);
		if (it != m_Layers.end())
			m_Layers.erase(it);
	}
}