#include "dough/rendering/LineRenderer.h"

#include "dough/application/Application.h"
#include "dough/scene/geometry/primitives/Quad.h"
#include "dough/rendering/pipeline/ShaderDescriptorSetLayoutsVulkan.h"

#include "tracy/public/tracy/Tracy.hpp"

namespace DOH {

	void LineRenderer::init(VkDevice logicDevice, VkExtent2D swapChainExtent, ValueUniformInfo uboSize) {
		ZoneScoped;

		auto& context = Application::get().getRenderer().getContext();

		std::vector<std::reference_wrapper<DescriptorSetLayoutVulkan>> linePipelinesDescSets = { context.getCommonDescriptorSetLayouts().Ubo };
		//NOTE:: Line Renderer currently uses the same descriptor set layouts so the one instance can be used for both SCENE and UI
		std::shared_ptr<ShaderDescriptorSetLayoutsVulkan> lineShadersDescSetLayouts = std::make_shared<ShaderDescriptorSetLayoutsVulkan>(linePipelinesDescSets);
		std::initializer_list<VkDescriptorSet> descSets = { VK_NULL_HANDLE };
		std::shared_ptr<DescriptorSetsInstanceVulkan> descSetInstance = std::make_shared<DescriptorSetsInstanceVulkan>(descSets);
		{
			//Scene
			const StaticVertexInputLayout& sceneVertexLayout = StaticVertexInputLayout::get(SCENE_LINE_VERTEX_TYPE);
			const uint32_t line3dBatchSizeBytes = LINE_BATCH_MAX_LINE_COUNT * sceneVertexLayout.getStride();
			std::shared_ptr<VertexArrayVulkan> vao = context.createVertexArray();
			std::shared_ptr<VertexBufferVulkan> vbo = context.createVertexBuffer(
				sceneVertexLayout,
				static_cast<VkDeviceSize>(line3dBatchSizeBytes),
				VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
			);
			vao->addVertexBuffer(vbo);
			vao->getVertexBuffers()[0]->map(logicDevice, line3dBatchSizeBytes);

			//TODO:: Index buffer usage?

			mSceneLineRenderable = std::make_shared<SimpleRenderable>(vao, descSetInstance, false);

			mSceneLineVertexShader = context.createShader(EShaderStage::VERTEX, mSceneLineRendererVertexShaderPath);
			mSceneLineFragmentShader = context.createShader(EShaderStage::FRAGMENT, mSceneLineRendererFragmentShaderPath);
			mSceneLineShaderProgram = context.createShaderProgram(
				mSceneLineVertexShader,
				mSceneLineFragmentShader,
				lineShadersDescSetLayouts
			);

			mSceneLineGraphicsPipelineInfo = std::make_unique<GraphicsPipelineInstanceInfo>(
				sceneVertexLayout,
				*mSceneLineShaderProgram,
				ERenderPass::APP_SCENE
			);
			auto& optionalFields = mSceneLineGraphicsPipelineInfo->enableOptionalFields();
			optionalFields.PolygonMode = VK_POLYGON_MODE_LINE;
			optionalFields.CullMode = VK_CULL_MODE_NONE;
			optionalFields.Topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
			//optionalFields.Topology = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
			optionalFields.ClearRenderablesAfterDraw = false;

			//TODO:: Allow depth testing for line rendering?
			//optionalFields.setDepthTesting(true, VK_COMPARE_OP_LESS_OR_EQUAL);

			mSceneLineGraphicsPipeline = context.createGraphicsPipeline(*mSceneLineGraphicsPipelineInfo);
			mSceneLineGraphicsPipeline->init(
				context.getLogicDevice(),
				context.getSwapChain().getExtent(),
				context.getRenderPass(ERenderPass::APP_SCENE).get()
			);
			mSceneLineBatch = std::make_unique<RenderBatchLineList>(SCENE_LINE_VERTEX_TYPE, LINE_BATCH_MAX_LINE_COUNT);
			mSceneLineGraphicsPipeline->addRenderableToDraw(mSceneLineRenderable);
		}

		{
			//UI
			const StaticVertexInputLayout& uiVertexLayout = StaticVertexInputLayout::get(UI_LINE_VERTEX_TYPE);
			const uint32_t line2dBatchSizeBytes = LINE_BATCH_MAX_LINE_COUNT * uiVertexLayout.getStride();
			std::shared_ptr<VertexArrayVulkan> vao = context.createVertexArray();
			std::shared_ptr<VertexBufferVulkan> vbo = context.createVertexBuffer(
				uiVertexLayout,
				static_cast<VkDeviceSize>(line2dBatchSizeBytes),
				VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
			);
			vao->addVertexBuffer(vbo);
			vao->getVertexBuffers()[0]->map(logicDevice, line2dBatchSizeBytes);

			//TODO:: Index buffer usage

			mUiLineRenderable = std::make_shared<SimpleRenderable>(vao, descSetInstance, false);
			mUiLineVertexShader = context.createShader(EShaderStage::VERTEX, mUiLineRendererVertexShaderPath);
			mUiLineFragmentShader = context.createShader(EShaderStage::FRAGMENT, mUiLineRendererFragmentShaderPath);
			mUiLineShaderProgram = context.createShaderProgram(
				mUiLineVertexShader,
				mUiLineFragmentShader,
				lineShadersDescSetLayouts
			);

			mUiLineGraphicsPipelineInfo = std::make_unique<GraphicsPipelineInstanceInfo>(
				uiVertexLayout,
				*mUiLineShaderProgram,
				ERenderPass::APP_UI
			);
			auto& optionalFields = mUiLineGraphicsPipelineInfo->enableOptionalFields();
			optionalFields.PolygonMode = VK_POLYGON_MODE_LINE;
			optionalFields.CullMode = VK_CULL_MODE_NONE;
			optionalFields.Topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
			optionalFields.ClearRenderablesAfterDraw = false;

			//TODO:: Allow depth testing for line rendering?
			//mUiLineGraphicsPipelineInfo->getOptionalFields().setDepthTesting(true, VK_COMPARE_OP_LESS_OR_EQUAL);

			mUiLineGraphicsPipeline = context.createGraphicsPipeline(*mUiLineGraphicsPipelineInfo);
			mUiLineGraphicsPipeline->init(
				context.getLogicDevice(),
				context.getSwapChain().getExtent(),
				context.getRenderPass(ERenderPass::APP_UI).get()
			);
			mUiLineBatch = std::make_unique<RenderBatchLineList>(UI_LINE_VERTEX_TYPE, LINE_BATCH_MAX_LINE_COUNT);
			mUiLineGraphicsPipeline->addRenderableToDraw(mUiLineRenderable);
		}
	}

	void LineRenderer::close(VkDevice logicDevice) {
		ZoneScoped;
		
		
		//TODO:: All of these resources are closed immediately. Should switch to deletion queue through mContext.closeGpuResource()

		mSceneLineRenderable->getVao().close(logicDevice);
		//mContext.addGpuResourceToClose(mSceneLineRenderable->getVaoPtr());
		mSceneLineShaderProgram->close(logicDevice);
		//mContext.addGpuResourceToClose(mSceneLineVertexShader);
		//mContext.addGpuResourceToClose(mSceneLineFragmentShader);
		mSceneLineGraphicsPipeline->close(logicDevice);

		mUiLineRenderable->getVao().close(logicDevice);
		//mContext.addGpuResourceToClose(mUiLineRenderable->getVaoPtr());
		mUiLineShaderProgram->close(logicDevice);
		//mContext.addGpuResourceToClose(mUiLineVertexShader);
		//mContext.addGpuResourceToClose(mUiLineFragmentShader);
		mUiLineGraphicsPipeline->close(logicDevice);
	}

	void LineRenderer::drawScene(
		VkDevice logicDevice,
		const uint32_t imageIndex,
		void* ubo,
		size_t uboSize,
		VkCommandBuffer cmd,
		CurrentBindingsState& currentBindings,
		AppDebugInfo& debugInfo
	) {
		ZoneScoped;

		if (mSceneLineBatch->getLineCount() > 0) {
			const uint32_t uboSlot = 0;
			mSceneLineRenderable->getDescriptorSetsInstance()->getDescriptorSets()[uboSlot] = Application::get().getRenderer().getContext().getSceneCameraData().DescriptorSets[imageIndex];
			if (currentBindings.Pipeline != mSceneLineGraphicsPipeline->get()) {
				mSceneLineGraphicsPipeline->bind(cmd);
				debugInfo.PipelineBinds++;
				currentBindings.Pipeline = mSceneLineGraphicsPipeline->get();
			}

			mSceneLineRenderable->getVao().setDrawCount(mSceneLineBatch->getVertexCount());

			Application::get().getRenderer().getContext().bindSceneUboToPipeline(
				cmd,
				*mUiLineGraphicsPipeline,
				imageIndex,
				currentBindings,
				debugInfo
			);

			//Offset by 1 because UBO already bound to this pipeline
			mSceneLineGraphicsPipeline->recordDrawCommands(cmd, currentBindings, 1);
			debugInfo.SceneDrawCalls += mSceneLineGraphicsPipeline->getVaoDrawCount();

			mSceneLineRenderable->getVao().getVertexBuffers()[0]->setDataMapped(
				logicDevice,
				mSceneLineBatch->getData().data(),
				mSceneLineBatch->getLineCount() * RenderBatchLineList::LINE_3D_SIZE
			);
		}

		mSceneLineBatch->reset();
	}

	void LineRenderer::drawUi(
		VkDevice logicDevice,
		const uint32_t imageIndex,
		void* ubo,
		size_t uboSize,
		VkCommandBuffer cmd,
		CurrentBindingsState& currentBindings,
		AppDebugInfo& debugInfo
	) {
		ZoneScoped;

		if (mUiLineBatch->getLineCount() > 0) {
			const uint32_t uboSlot = 0;
			mSceneLineRenderable->getDescriptorSetsInstance()->getDescriptorSets()[uboSlot] = Application::get().getRenderer().getContext().getSceneCameraData().DescriptorSets[imageIndex];
			if (currentBindings.Pipeline != mUiLineGraphicsPipeline->get()) {
				mUiLineGraphicsPipeline->bind(cmd);
				debugInfo.PipelineBinds++;
				currentBindings.Pipeline = mUiLineGraphicsPipeline->get();
			}

			mUiLineRenderable->getVao().setDrawCount(mUiLineBatch->getVertexCount());

			Application::get().getRenderer().getContext().bindUiUboToPipeline(
				cmd,
				*mUiLineGraphicsPipeline,
				imageIndex,
				currentBindings,
				debugInfo
			);

			//Offset by 1 because UBO already bound to this pipeline
			mUiLineGraphicsPipeline->recordDrawCommands(cmd, currentBindings, 1);
			debugInfo.UiDrawCalls += mUiLineGraphicsPipeline->getVaoDrawCount();

			mUiLineRenderable->getVao().getVertexBuffers()[0]->setDataMapped(
				logicDevice,
				mUiLineBatch->getData().data(),
				mUiLineBatch->getLineCount() * RenderBatchLineList::LINE_2D_SIZE
			);
		}

		mUiLineBatch->reset();
	}

	void LineRenderer::recreateGraphicsPipelines(
		VkDevice logicDevice,
		VkExtent2D extent,
		const RenderPassVulkan& sceneRenderPass,
		const RenderPassVulkan& uiRenderPass,
		VkDescriptorPool descPool
	) {
		ZoneScoped;

		mSceneLineGraphicsPipeline->resize(logicDevice, extent, sceneRenderPass.get());
		mUiLineGraphicsPipeline->resize(logicDevice, extent, uiRenderPass.get());
	}

	void LineRenderer::drawLineScene(const glm::vec3& start, const glm::vec3& end, const glm::vec4& colour) {
		ZoneScoped;

		if (mSceneLineBatch->hasSpace()) {
			mSceneLineBatch->add3d(start, end, colour);
		} else {
			LOG_WARN("Scene line batch max line count reached: " << mSceneLineBatch->getMaxLineCount());
		}
	}

	void LineRenderer::drawLineUi(const glm::vec2& start, const glm::vec2& end, const glm::vec4& colour) {
		ZoneScoped;

		if (mUiLineBatch->hasSpace()) {
			mUiLineBatch->add2d(start, end, colour);
		} else {
			LOG_WARN("UI line batch max line count reached: " << mUiLineBatch->getMaxLineCount());
		}
	}

	void LineRenderer::drawQuadScene(const Quad& quad, const glm::vec4& colour) {
		ZoneScoped;

		if (mSceneLineBatch->hasSpace(4)) {
			glm::vec3 botLeft = { quad.Position.x, quad.Position.y, quad.Position.z };
			glm::vec3 botRight = { quad.Position.x + quad.Size.x, quad.Position.y, quad.Position.z };
			glm::vec3 topLeft = { quad.Position.x, quad.Position.y + quad.Size.y, quad.Position.z };
			glm::vec3 topRight = { quad.Position.x + quad.Size.x, quad.Position.y + quad.Size.y, quad.Position.z };

			mSceneLineBatch->add3d(botLeft, topLeft, colour);
			mSceneLineBatch->add3d(topLeft, topRight, colour);
			mSceneLineBatch->add3d(topRight, botRight, colour);
			mSceneLineBatch->add3d(botRight, botLeft, colour);
		} else {
			LOG_WARN("Scene line batch does not have space for " << 4 << " more lines.");
		}
	}

	void LineRenderer::drawQuadUi(const Quad& quad, const glm::vec4& colour) {
		ZoneScoped;

		if (mUiLineBatch->hasSpace(4)) {
			glm::vec3 botLeft = { quad.Position.x, quad.Position.y, quad.Position.z };
			glm::vec3 botRight = { quad.Position.x + quad.Size.x, quad.Position.y, quad.Position.z };
			glm::vec3 topLeft = { quad.Position.x, quad.Position.y + quad.Size.y, quad.Position.z };
			glm::vec3 topRight = { quad.Position.x + quad.Size.x, quad.Position.y + quad.Size.y, quad.Position.z };

			mUiLineBatch->add2d(botLeft, topLeft, colour);
			mUiLineBatch->add2d(topLeft, topRight, colour);
			mUiLineBatch->add2d(topRight, botRight, colour);
			mUiLineBatch->add2d(botRight, botLeft, colour);
		} else {
			LOG_WARN("Ui line batch does not have space for " << 4 << " more lines.");
		}
	}
}
