#pragma once

#include "dough/rendering/renderables/IRenderable.h"
#include "dough/rendering/pipeline_2/ShaderUniformLayoutVulkan_2.h"

namespace DOH {

	//Simple renderable implementation that fits the bare minimum to work
	class SimpleRenderable : public IRenderable {
	private:
		std::shared_ptr<VertexArrayVulkan> mVao;
		void* mPushConstantPtr;
		bool mIndexed;

	public:
		SimpleRenderable(std::shared_ptr<VertexArrayVulkan> vao, bool isIndexed = true, void* pushConstantPtr = nullptr)
		:	mVao(vao),
			mPushConstantPtr(pushConstantPtr),
			mIndexed(isIndexed)
		{}

		virtual VertexArrayVulkan& getVao() const override { return *mVao; }
		virtual void* getPushConstantPtr() const override { return mPushConstantPtr; }
		virtual bool isIndexed() const override { return mIndexed; }

		inline void setPushConstantPtr(void* ptr) { mPushConstantPtr = ptr; }
	};

	//TEMP:: Change to allow for shader uniform sets per renderable
	class SimpleRenderable_2 : public IRenderable {
	private:
		std::shared_ptr<VertexArrayVulkan> mVao;
		std::shared_ptr<ShaderUniformSetsInstanceVulkan> mShaderResourceData;
		void* mPushConstantPtr;
		bool mIndexed;

	public:
		SimpleRenderable_2(std::shared_ptr<VertexArrayVulkan> vao, std::shared_ptr<ShaderUniformSetsInstanceVulkan> shaderResourceData = nullptr, bool isIndexed = true, void* pushConstantPtr = nullptr)
		:	mVao(vao),
			mShaderResourceData(shaderResourceData),
			mPushConstantPtr(pushConstantPtr),
			mIndexed(isIndexed)
		{}

		virtual VertexArrayVulkan& getVao() const override { return *mVao; }
		virtual void* getPushConstantPtr() const override { return mPushConstantPtr; }
		virtual bool isIndexed() const override { return mIndexed; }

		inline void setPushConstantPtr(void* ptr) { mPushConstantPtr = ptr; }

		virtual std::shared_ptr<ShaderUniformSetsInstanceVulkan> getShaderResourceData() const override { return mShaderResourceData; };
	};
}
