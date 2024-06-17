#pragma once

#include "dough/Core.h"
#include "dough/rendering/pipeline/GraphicsPipelineVulkan.h"

namespace DOH {

	using GraphicsPipelineMap = std::unordered_map<std::string, std::shared_ptr<GraphicsPipelineVulkan>>;
	using RenderPassGraphicsPipelineMap = std::unordered_map<ERenderPass, GraphicsPipelineMap>;

	/** 
	* TODO::
	*	Allow for an optimised version of this for production runtime. No map, only an array for enabled pipelines
	*	(maybe a separate one for disabled ones).
	*
	*/
	class CustomRenderState {
	private:
		RenderPassGraphicsPipelineMap mRenderPassGraphicsPipelines;

		//Render State info
		const char* mName;
		uint32_t mPipelineCount;

	public:
		CustomRenderState(const char* name);
		CustomRenderState(const CustomRenderState& copy) = delete;
		CustomRenderState operator=(const CustomRenderState& assignment) = delete;

		void addRenderPassGroup(const ERenderPass renderPass);
		void addRenderPassGroup(const ERenderPass renderPass, const GraphicsPipelineMap& pipelineMap);
		void addPipelineToRenderPass(
			const ERenderPass renderPass,
			const std::string& name,
			const std::shared_ptr<GraphicsPipelineVulkan> graphicsPipeline
		);
		void closePipeline(VkDevice logicDevice, const ERenderPass renderPass, const std::string& name);
		void closeRenderPassGroup(VkDevice logicDevice, const ERenderPass renderPass);
		void close(VkDevice logicDevice);

		inline std::optional<std::reference_wrapper<GraphicsPipelineMap>> getRenderPassGraphicsPipelineGroup(const ERenderPass renderPass) {
			return { mRenderPassGraphicsPipelines[renderPass] };
		}
		inline RenderPassGraphicsPipelineMap& getRenderPassGraphicsPipelineMap() { return mRenderPassGraphicsPipelines; }

		inline bool hasRenderPassGraphicsPipelineGroup(const ERenderPass renderPass) const {
			return mRenderPassGraphicsPipelines.find(renderPass) != mRenderPassGraphicsPipelines.end();
		}
		inline bool hasRenderPassGraphicsPipeline(const ERenderPass renderPass, const std::string& name) {
			const auto group = mRenderPassGraphicsPipelines.find(renderPass);
			return group != mRenderPassGraphicsPipelines.end() && (group->second.find(name) != group->second.end());
		}

		/**
		* Clear the list of graphics pipelines for given render pass. Does NOT close their GPU resources.
		*/
		inline void clearRenderPassGraphicsPipelines(const ERenderPass renderPass) {
			const auto groupItr = mRenderPassGraphicsPipelines.find(renderPass);
			if (groupItr != mRenderPassGraphicsPipelines.end()) {
				mPipelineCount -= static_cast<uint32_t>(groupItr->second.size());
				groupItr->second.clear();
			}
		}
		/**
		* Clear the entire map, all graphics pipelines for all render passes. Does NOT close their GPU resources.
		*/
		inline void clearRenderPassGroups() { mRenderPassGraphicsPipelines.clear(); mPipelineCount = 0; }
		
		inline uint32_t getRenderPassGroupCount() const { return static_cast<uint32_t>(mRenderPassGraphicsPipelines.size()); }
		inline uint32_t getPipelineCount() const { return mPipelineCount; }
		inline const char* getName() const { return mName; }
	};
}
