#pragma once

#include "dough/rendering/renderables/IRenderable.h"

namespace DOH {

	//Simple renderable implementation that fits the bare minimum to work
	class SimpleRenderable : public IRenderable {
	private:
		std::shared_ptr<VertexArrayVulkan> mVao;
		void* mPushConstantPtr;
		bool mIndexed;

	public:
		SimpleRenderable(std::shared_ptr<VertexArrayVulkan> vao, bool isIndexed = true)
		:	mVao(vao),
			mPushConstantPtr(nullptr),
			mIndexed(isIndexed)
		{}

		virtual VertexArrayVulkan& getVao() const override { return *mVao; }
		virtual void* getPushConstantPtr() const override { return mPushConstantPtr; }
		virtual bool isIndexed() const override { return mIndexed; }

		inline void setPushConstantPtr(void* ptr) { mPushConstantPtr = ptr; }
	};
}
