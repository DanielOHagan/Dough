#pragma once

#include "dough/rendering/buffer/VertexArrayVulkan.h"

namespace DOH {

	class IRenderable {
	public:
		virtual VertexArrayVulkan& getVao() const = 0;
		virtual void* getPushConstantPtr() const = 0;
	};
}
