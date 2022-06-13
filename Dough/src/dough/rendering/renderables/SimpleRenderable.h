#pragma once

#include "dough/rendering/renderables/IRenderable.h"

namespace DOH {

	//Simple renderable implementation that fits the bare minimum to work
	class SimpleRenderable : public IRenderable {
	private:
		std::shared_ptr<VertexArrayVulkan> mVao;
		void* mPushConstantPtr;

	public:
		SimpleRenderable(std::shared_ptr<VertexArrayVulkan> vao)
		:	mVao(vao),
			mPushConstantPtr(nullptr)
		{}

		virtual VertexArrayVulkan& getVao() const override { return *mVao; };
		virtual void* getPushConstantPtr() const override { return mPushConstantPtr; }

		inline void setPushConstantPtr(void* ptr) { mPushConstantPtr = ptr; }
	};
}
