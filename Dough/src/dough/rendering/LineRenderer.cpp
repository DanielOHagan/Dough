#include "dough/rendering/LineRenderer.h"

#include "dough/application/Application.h"
#include "dough/scene/geometry/primitives/Quad.h"
#include "dough/rendering/pipeline/ShaderDescriptorSetLayoutsVulkan.h"
#include "dough/rendering/SwapChainVulkan.h"

#include <tracy/public/tracy/Tracy.hpp>

namespace DOH {

	std::unique_ptr<LineRenderer> LineRenderer::INSTANCE = nullptr;

	const char* LineRenderer::SCENE_LINE_SHADER_PATH_VERT = "Dough/Dough/res/shaders/spv/LineRenderer3d.vert.spv";
	const char* LineRenderer::SCENE_LINE_SHADER_PATH_FRAG = "Dough/Dough/res/shaders/spv/LineRenderer3d.frag.spv";
	const char* LineRenderer::UI_LINE_SHADER_PATH_VERT = "Dough/Dough/res/shaders/spv/LineRenderer2d.vert.spv";
	const char* LineRenderer::UI_LINE_SHADER_PATH_FRAG = "Dough/Dough/res/shaders/spv/LineRenderer2d.frag.spv";

	LineRenderer::LineRenderer(RenderingContextVulkan& context)
	:	mContext(context)
	{}

	void LineRenderer::initImpl() {
		ZoneScoped;

		std::vector<std::reference_wrapper<DescriptorSetLayoutVulkan>> linePipelinesDescSets = { mContext.getCommonDescriptorSetLayouts().Ubo };
		//NOTE:: Line Renderer currently uses the same descriptor set layouts so the one instance can be used for both SCENE and UI
		std::shared_ptr<ShaderDescriptorSetLayoutsVulkan> lineShadersDescSetLayouts = std::make_shared<ShaderDescriptorSetLayoutsVulkan>(linePipelinesDescSets);
		std::initializer_list<VkDescriptorSet> descSets = { VK_NULL_HANDLE };
		std::shared_ptr<DescriptorSetsInstanceVulkan> descSetInstance = std::make_shared<DescriptorSetsInstanceVulkan>(descSets);
		{
			//Scene
			const StaticVertexInputLayout& sceneVertexLayout = StaticVertexInputLayout::get(SCENE_LINE_VERTEX_TYPE);
			const uint32_t line3dBatchSizeBytes = LINE_BATCH_MAX_LINE_COUNT * sceneVertexLayout.getStride();
			std::shared_ptr<VertexArrayVulkan> vao = mContext.createVertexArray();
			std::shared_ptr<VertexBufferVulkan> vbo = mContext.createVertexBuffer(
				sceneVertexLayout,
				static_cast<VkDeviceSize>(line3dBatchSizeBytes),
				VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
			);
			vao->addVertexBuffer(vbo);
			vao->getVertexBuffers()[0]->map(mContext.getLogicDevice(), line3dBatchSizeBytes);

			//TODO:: Index buffer usage?

			mSceneLineRenderable = std::make_shared<SimpleRenderable>(vao, descSetInstance, false);

			mSceneLineVertexShader = mContext.createShader(EShaderStage::VERTEX, SCENE_LINE_SHADER_PATH_VERT);
			mSceneLineFragmentShader = mContext.createShader(EShaderStage::FRAGMENT, SCENE_LINE_SHADER_PATH_FRAG);
			mSceneLineShaderProgram = mContext.createShaderProgram(
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

			mSceneLineGraphicsPipeline = mContext.createGraphicsPipeline(*mSceneLineGraphicsPipelineInfo);
			mSceneLineGraphicsPipeline->init(
				mContext.getLogicDevice(),
				mContext.getSwapChain().getExtent(),
				mContext.getRenderPass(ERenderPass::APP_SCENE).get()
			);
			mSceneLineBatch = std::make_unique<RenderBatchLineList>(SCENE_LINE_VERTEX_TYPE, LINE_BATCH_MAX_LINE_COUNT);
			mSceneLineGraphicsPipeline->addRenderableToDraw(mSceneLineRenderable);
		}

		{
			//UI
			const StaticVertexInputLayout& uiVertexLayout = StaticVertexInputLayout::get(UI_LINE_VERTEX_TYPE);
			const uint32_t line2dBatchSizeBytes = LINE_BATCH_MAX_LINE_COUNT * uiVertexLayout.getStride();
			std::shared_ptr<VertexArrayVulkan> vao = mContext.createVertexArray();
			std::shared_ptr<VertexBufferVulkan> vbo = mContext.createVertexBuffer(
				uiVertexLayout,
				static_cast<VkDeviceSize>(line2dBatchSizeBytes),
				VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
			);
			vao->addVertexBuffer(vbo);
			vao->getVertexBuffers()[0]->map(mContext.getLogicDevice(), line2dBatchSizeBytes);

			//TODO:: Index buffer usage

			mUiLineRenderable = std::make_shared<SimpleRenderable>(vao, descSetInstance, false);
			mUiLineVertexShader = mContext.createShader(EShaderStage::VERTEX, UI_LINE_SHADER_PATH_VERT);
			mUiLineFragmentShader = mContext.createShader(EShaderStage::FRAGMENT, UI_LINE_SHADER_PATH_FRAG);
			mUiLineShaderProgram = mContext.createShaderProgram(
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

			mUiLineGraphicsPipeline = mContext.createGraphicsPipeline(*mUiLineGraphicsPipelineInfo);
			mUiLineGraphicsPipeline->init(
				mContext.getLogicDevice(),
				mContext.getSwapChain().getExtent(),
				mContext.getRenderPass(ERenderPass::APP_UI).get()
			);
			mUiLineBatch = std::make_unique<RenderBatchLineList>(UI_LINE_VERTEX_TYPE, LINE_BATCH_MAX_LINE_COUNT);
			mUiLineGraphicsPipeline->addRenderableToDraw(mUiLineRenderable);
		}
	}

	void LineRenderer::closeImpl() {
		ZoneScoped;

		mContext.addGpuResourceToClose(mSceneLineRenderable->getVaoPtr());
		mContext.addGpuResourceToClose(mSceneLineVertexShader);
		mContext.addGpuResourceToClose(mSceneLineFragmentShader);
		mContext.addGpuResourceToClose(mSceneLineGraphicsPipeline);

		mContext.addGpuResourceToClose(mUiLineRenderable->getVaoPtr());
		mContext.addGpuResourceToClose(mUiLineVertexShader);
		mContext.addGpuResourceToClose(mUiLineFragmentShader);
		mContext.addGpuResourceToClose(mUiLineGraphicsPipeline);
	}

	void LineRenderer::drawSceneImpl(
		uint32_t imageIndex,
		VkCommandBuffer cmd,
		CurrentBindingsState& currentBindings
	) {
		ZoneScoped;

		if (mSceneCameraData == nullptr) {
			//TODO:: This doesn't account for if the variable is intentionally left null because no Scene is meant to be drawn.
			//if (mWarnOnNullSceneCameraData)
			LOG_WARN("ShapeRenderer::drawSceneImpl mSceneCameraData is null");
			return;
		}

		AppDebugInfo& debugInfo = Application::get().getDebugInfo();
		constexpr uint32_t uboSlot = 0;
		const uint32_t lineCount = mSceneLineBatch->getLineCount();

		if (lineCount > 0) {
			mUiLineRenderable->getDescriptorSetsInstance()->getDescriptorSets()[uboSlot] = mUiCameraData->DescriptorSets[imageIndex];
			if (currentBindings.Pipeline != mSceneLineGraphicsPipeline->get()) {
				mSceneLineGraphicsPipeline->bind(cmd);
				debugInfo.PipelineBinds++;
				currentBindings.Pipeline = mSceneLineGraphicsPipeline->get();
			}

			mSceneLineRenderable->getVao().setDrawCount(mSceneLineBatch->getVertexCount());
			mSceneLineRenderable->getVao().getVertexBuffers()[0]->setDataMapped(
				mContext.getLogicDevice(),
				mSceneLineBatch->getData().data(),
				lineCount * RenderBatchLineList::LINE_3D_SIZE
			);

			mSceneLineGraphicsPipeline->recordDrawCommand_new(cmd, *mSceneLineRenderable, currentBindings, 0);
			debugInfo.SceneDrawCalls++;
		}

		mSceneLineBatch->reset();
	}

	void LineRenderer::drawUiImpl(
		uint32_t imageIndex,
		VkCommandBuffer cmd,
		CurrentBindingsState& currentBindings
	) {
		ZoneScoped;

		if (mUiCameraData == nullptr) {
			//TODO:: This doesn't account for if the variable is intentionally left null because no UI is meant to be drawn.
			//if (mWarnOnNullUiCameraData)
			LOG_WARN("LineRenderer::drawUiImpl mUiCameraData is null");
			return;
		}

		AppDebugInfo& debugInfo = Application::get().getDebugInfo();
		constexpr uint32_t uboSlot = 0;
		const uint32_t lineCount = mUiLineBatch->getLineCount();

		if (lineCount > 0) {
			mUiLineRenderable->getDescriptorSetsInstance()->getDescriptorSets()[uboSlot] = mUiCameraData->DescriptorSets[imageIndex];
			if (currentBindings.Pipeline != mUiLineGraphicsPipeline->get()) {
				mUiLineGraphicsPipeline->bind(cmd);
				debugInfo.PipelineBinds++;
				currentBindings.Pipeline = mUiLineGraphicsPipeline->get();
			}

			mUiLineRenderable->getVao().setDrawCount(mUiLineBatch->getVertexCount());

			mUiLineRenderable->getVao().getVertexBuffers()[0]->setDataMapped(
				mContext.getLogicDevice(),
				mUiLineBatch->getData().data(),
				lineCount * RenderBatchLineList::LINE_2D_SIZE
			);

			mUiLineGraphicsPipeline->recordDrawCommand_new(cmd, *mUiLineRenderable, currentBindings, 0);
			debugInfo.UiDrawCalls++;
		}

		mUiLineBatch->reset();
	}

	void LineRenderer::onSwapChainResizeImpl(SwapChainVulkan& swapChain) {
		ZoneScoped;

		mSceneLineGraphicsPipeline->resize(
			mContext.getLogicDevice(),
			swapChain.getExtent(),
			mContext.getRenderPass(ERenderPass::APP_SCENE).get()
		);
		mUiLineGraphicsPipeline->resize(
			mContext.getLogicDevice(),
			swapChain.getExtent(),
			mContext.getRenderPass(ERenderPass::APP_UI).get()
		);
	}

	void LineRenderer::drawLineSceneImpl(const glm::vec3& start, const glm::vec3& end, const glm::vec4& colour) {
		ZoneScoped;

		if (mSceneLineBatch->hasSpace()) {
			mSceneLineBatch->add3d(start, end, colour);
		} else {
			LOG_WARN("Scene line batch max line count reached: " << mSceneLineBatch->getMaxLineCount());
		}
	}

	void LineRenderer::drawLineUiImpl(const glm::vec2& start, const glm::vec2& end, const glm::vec4& colour) {
		ZoneScoped;

		if (mUiLineBatch->hasSpace()) {
			mUiLineBatch->add2d(start, end, colour);
		} else {
			LOG_WARN("UI line batch max line count reached: " << mUiLineBatch->getMaxLineCount());
		}
	}

	void LineRenderer::drawQuadSceneImpl(const Quad& quad, const glm::vec4& colour) {
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

	void LineRenderer::drawQuadUiImpl(const Quad& quad, const glm::vec4& colour) {
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

	void LineRenderer::init(RenderingContextVulkan& context) {
		if (INSTANCE == nullptr) {
			INSTANCE = std::make_unique<LineRenderer>(context);
			INSTANCE->initImpl();
		} else {
			LOG_WARN("LineRenderer::init called when already initialised.");
		}
	}

	void LineRenderer::close() {
		if (INSTANCE != nullptr) {
			INSTANCE->closeImpl();
			INSTANCE.release();
			INSTANCE = nullptr;
		} else {
			LOG_WARN("LineRenderer::close called when not initialised.");
		}
	}

	void LineRenderer::onSwapChainResize(SwapChainVulkan& swapChain) {
		if (INSTANCE != nullptr) {
			INSTANCE->onSwapChainResizeImpl(swapChain);
		} else {
			LOG_WARN("LineRenderer::onSwapChainResize called when not initialised.");
		}
	}

	void LineRenderer::drawScene(uint32_t imageIndex, VkCommandBuffer cmd, CurrentBindingsState& currentBindings) {
		if (INSTANCE != nullptr) {
			INSTANCE->drawSceneImpl(imageIndex, cmd, currentBindings);
		} else {
			LOG_WARN("LineRenderer::drawScene called when not initialised.");
		}
	}

	void LineRenderer::drawUi(uint32_t imageIndex, VkCommandBuffer cmd, CurrentBindingsState& currentBindings) {
		if (INSTANCE != nullptr) {
			INSTANCE->drawUiImpl(imageIndex, cmd, currentBindings);
		} else {
			LOG_WARN("LineRenderer::drawUi called when not initialised.");
		}
	}
}
