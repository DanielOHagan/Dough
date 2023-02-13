#include "dough/rendering/CustomRenderState.h"

#include "dough/Logging.h"

namespace DOH {

	CustomRenderState::CustomRenderState()
	:	mRenderPassGraphicsPipelines({}),
		mPipelineCount(0)
	{}

	void CustomRenderState::addRenderPassGroup(const ERenderPass renderPass) {
		const auto& group = mRenderPassGraphicsPipelines.emplace(renderPass, GraphicsPipelineMap());
		if (!group.second) {
			//LOG_WARN("Render pass group already exists: " << ERenderPassStrings[static_cast<uint32_t>(renderPass)]);
			LOG_WARN("Render pass group already exists: " << static_cast<uint32_t>(renderPass));
		}
	}

	void CustomRenderState::addRenderPassGroup(const ERenderPass renderPass, const GraphicsPipelineMap& pipelineMap) {
		const auto& group = mRenderPassGraphicsPipelines.emplace(renderPass, pipelineMap);
		if (!group.second) {
			//LOG_WARN("Render pass group already exists: " << ERenderPassStrings[static_cast<uint32_t>(renderPass)]);
			LOG_WARN("Render pass group already exists: " << static_cast<uint32_t>(renderPass));
		} else {
			mPipelineCount += static_cast<uint32_t>(pipelineMap.size());
		}
	}

	void CustomRenderState::addPipelineToRenderPass(
		const ERenderPass renderPass,
		const std::string& name,
		const std::shared_ptr<GraphicsPipelineVulkan> graphicsPipeline
	) {
		const auto& group = mRenderPassGraphicsPipelines.find(renderPass);
		if (group != mRenderPassGraphicsPipelines.end()) {
			if (group->second.emplace(name, graphicsPipeline).second) {
				mPipelineCount++;
			} else {
				LOG_WARN("Failed to add pipeline: " << name << " to group: " << static_cast<uint32_t>(renderPass));
			}
		} else {
			//LOG_WARN("RenderPass does not have pipeline group: " << ERenderPassStrings[static_cast<uint32_t>(renderPass)]);
			LOG_WARN("RenderPass does not have pipeline group: " << static_cast<uint32_t>(renderPass));
		}
	}

	void CustomRenderState::closePipeline(VkDevice logicDevice, const ERenderPass renderPass, const std::string& name) {
		const auto group = getRenderPassGraphicsPipelineGroup(renderPass);
		if (group.has_value()) {
			const auto pipeline = group.value().find(name);
			if (pipeline != group.value().end()) {
				pipeline->second->close(logicDevice);
				mPipelineCount--;
			} else {
				LOG_WARN("Pipeline: " << name.c_str() << " not found in group: " << static_cast<uint32_t>(renderPass));
			}
		}
	}

	void CustomRenderState::closeRenderPassGroup(VkDevice logicDevice, const ERenderPass renderPass) {
		const auto group = mRenderPassGraphicsPipelines.find(renderPass);

		if (group != mRenderPassGraphicsPipelines.end()) {
			for (const auto& pipeline : group->second) {
				pipeline.second->close(logicDevice);
				mPipelineCount--;
			}
		}
	}

	void CustomRenderState::close(VkDevice logicDevice) {
		for (auto& pipelineGroup : mRenderPassGraphicsPipelines) {
			for (auto& pipeline : pipelineGroup.second) {
				pipeline.second->close(logicDevice);
			}

			pipelineGroup.second.clear();
		}

		mRenderPassGraphicsPipelines.clear();
		mPipelineCount = 0;
	}
}
