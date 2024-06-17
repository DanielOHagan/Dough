#pragma once

#include "dough/rendering/renderables/IRenderable.h"
#include "dough/rendering/pipeline/ShaderDescriptorSetLayoutsVulkan.h"

namespace DOH {

	class SimpleRenderable : public IRenderable {
	private:
		std::shared_ptr<VertexArrayVulkan> mVao;
		std::shared_ptr<DescriptorSetsInstanceVulkan> mDescriptorSetsInstance;
		void* mPushConstantPtr;
		bool mIndexed;

	public:
		SimpleRenderable(std::shared_ptr<VertexArrayVulkan> vao, std::shared_ptr<DescriptorSetsInstanceVulkan> descSetsInstance = nullptr, bool isIndexed = true, void* pushConstantPtr = nullptr)
		:	mVao(vao),
			mDescriptorSetsInstance(descSetsInstance),
			mPushConstantPtr(pushConstantPtr),
			mIndexed(isIndexed)
		{}

		virtual VertexArrayVulkan& getVao() const override { return *mVao; }
		virtual std::shared_ptr<VertexArrayVulkan> getVaoPtr() const override { return mVao; }
		virtual void* getPushConstantPtr() const override { return mPushConstantPtr; }
		virtual bool isIndexed() const override { return mIndexed; }

		inline void setPushConstantPtr(void* ptr) { mPushConstantPtr = ptr; }

		virtual bool hasDescriptorSetsInstance() const override { return mDescriptorSetsInstance != nullptr; }
		virtual std::shared_ptr<DescriptorSetsInstanceVulkan> getDescriptorSetsInstance() const override { return mDescriptorSetsInstance; };
	};
}
