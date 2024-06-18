#include "dough/rendering/CustomRenderState.h"

#include "dough/Logging.h"
#include "dough/application/Application.h"

#include <tracy/public/tracy/Tracy.hpp>

namespace DOH {

	CustomRenderState::CustomRenderState(const char* name)
	:	mRenderPassGraphicsPipelines({}),
		mName(name),
		mPipelineCount(0)
	{}

	void CustomRenderState::addRenderPassGroup(const ERenderPass renderPass) {
		ZoneScoped;

		const auto& group = mRenderPassGraphicsPipelines.emplace(renderPass, GraphicsPipelineMap());
		if (!group.second) {
			LOG_WARN("Render pass group already exists: " << ERenderPassStrings[static_cast<uint32_t>(renderPass)]);
		}
	}

	void CustomRenderState::addRenderPassGroup(const ERenderPass renderPass, const GraphicsPipelineMap& pipelineMap) {
		ZoneScoped;

		const auto& group = mRenderPassGraphicsPipelines.emplace(renderPass, pipelineMap);
		if (!group.second) {
			LOG_WARN("Render pass group already exists: " << ERenderPassStrings[static_cast<uint32_t>(renderPass)]);
		} else {
			mPipelineCount += static_cast<uint32_t>(pipelineMap.size());
		}
	}

	void CustomRenderState::addPipelineToRenderPass(
		const ERenderPass renderPass,
		const std::string& name,
		const std::shared_ptr<GraphicsPipelineVulkan> graphicsPipeline
	) {
		ZoneScoped;

		const auto& group = mRenderPassGraphicsPipelines.find(renderPass);
		if (group != mRenderPassGraphicsPipelines.end()) {
			if (group->second.emplace(name, graphicsPipeline).second) {
				mPipelineCount++;
			} else {
				LOG_WARN("Failed to add pipeline: " << name << " to group: " << ERenderPassStrings[static_cast<uint32_t>(renderPass)]);
			}
		} else {
			LOG_WARN("RenderPass does not have pipeline group: " << ERenderPassStrings[static_cast<uint32_t>(renderPass)]);
		}
	}

	void CustomRenderState::closePipeline(VkDevice logicDevice, const ERenderPass renderPass, const std::string& name) {
		ZoneScoped;

		std::optional<std::reference_wrapper<GraphicsPipelineMap>> group = getRenderPassGraphicsPipelineGroup(renderPass);
		if (group.has_value()) {
			const auto pipeline = group->get().find(name);
			if (pipeline != group->get().end()) {
				auto& renderer = Application::get().getRenderer();
				renderer.closeGpuResource(pipeline->second);
				mPipelineCount--;
				group->get().erase(pipeline);
			} else {
				LOG_WARN("Pipeline: " << name.c_str() << " not found in group: " << ERenderPassStrings[static_cast<uint32_t>(renderPass)]);
			}
		}
	}

	void CustomRenderState::closeRenderPassGroup(VkDevice logicDevice, const ERenderPass renderPass) {
		ZoneScoped;

		const auto group = mRenderPassGraphicsPipelines.find(renderPass);

		if (group != mRenderPassGraphicsPipelines.end()) {
			for (const auto& pipeline : group->second) {
				auto& renderer = Application::get().getRenderer();
				renderer.closeGpuResource(pipeline.second);
				mPipelineCount--;
			}

			mRenderPassGraphicsPipelines.erase(group);
		}
	}

	void CustomRenderState::close(VkDevice logicDevice) {
		ZoneScoped;

		for (auto& pipelineGroup : mRenderPassGraphicsPipelines) {
			for (auto& pipeline : pipelineGroup.second) {
				auto& renderer = Application::get().getRenderer();
				renderer.closeGpuResource(pipeline.second);
			}

			pipelineGroup.second.clear();
		}

		clearRenderPassGroups();
	}
}
