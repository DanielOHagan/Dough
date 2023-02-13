#pragma once

#include "dough/Core.h"
#include "dough/rendering/pipeline/GraphicsPipelineVulkan.h"

namespace DOH {

	using GraphicsPipelineMap = std::unordered_map<std::string, std::shared_ptr<GraphicsPipelineVulkan>>;
	using RenderPassGraphicsPipelineMap = std::unordered_map<ERenderPass, GraphicsPipelineMap>;

	/**
	* A group of objects required for rendering a specified custom state.
	*
	* TODO::
	*	Allow for CustomRenderStates to be "merged", or for multiple CustomRenderStates or
	*	so they changed more dynamically.
	*	One case in which this would help is in EditorAppLogic and its inner app.
	*	The two AppLogic classes may both require a CustomRenderState at some point,
	*	for now only one instance of CustomRenderState is supported.
	*	Currently the CustomRenderState is the assumed owner of the stored GraphicsPipelines,
	*	in future it might be very useful to have support for multiple CustomRenderStates that
	*	use 1 or more of the same GraphicsPipelines.
	*
	* TODO::
	*	Is this a suitable place for a camera for this render state or camera for each render pass.
	*
	* TODO::
	*	Allow for an optimised version of this for production runtime. No map, only an array for enabled pipelines
	*	(maybe a separate one for disabled ones).
	*
	*/
	class CustomRenderState {

	private:
		//Rendering resources
		RenderPassGraphicsPipelineMap mRenderPassGraphicsPipelines;
		//std::unordered_map<uint32_t id OR string and have pipelines hold references, std::reference_wrapper<ShaderProgramVulkan>> mShaderPrograms;

		//Render State info
		uint32_t mPipelineCount;
		//std::string mName; //Debug info

	public:
		CustomRenderState();
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

		inline std::optional<GraphicsPipelineMap> getRenderPassGraphicsPipelineGroup(const ERenderPass renderPass) {
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

		inline void clearRenderPassGraphicsPipelines(const ERenderPass renderPass) {
			const auto groupItr = mRenderPassGraphicsPipelines.find(renderPass);
			if (groupItr != mRenderPassGraphicsPipelines.end()) {
				groupItr->second.clear();
			}
		}
		inline void clearRenderPassGroups() { mRenderPassGraphicsPipelines.clear(); }
		
		inline uint32_t getRenderPassGroupCount() const { return static_cast<uint32_t>(mRenderPassGraphicsPipelines.size()); }
		inline uint32_t getPipelineCount() const { return mPipelineCount; }
	};
}
