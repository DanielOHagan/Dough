#pragma once

#include "dough/rendering/buffer/VertexArrayVulkan.h"

namespace DOH {

	class DescriptorSetsInstanceVulkan;

	/** 
	* Renderables are objects that hold the information for issuing draw commands, they DO NOT own the data.
	*/
	class IRenderable {
	public:
		virtual VertexArrayVulkan& getVao() const = 0;
		virtual std::shared_ptr<VertexArrayVulkan> getVaoPtr() const = 0;
		virtual void* getPushConstantPtr() const = 0;

		virtual bool isIndexed() const = 0;

		virtual bool hasDescriptorSetsInstance() const { return false; }
		virtual std::shared_ptr<DescriptorSetsInstanceVulkan> getDescriptorSetsInstance() const { return nullptr; };
	};
}
