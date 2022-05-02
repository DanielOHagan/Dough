#include "dough/rendering/renderer2d/Renderer2dStorageVulkan.h"

#include "dough/rendering/RenderingContextVulkan.h"
#include "dough/rendering/ObjInit.h"
#include "dough/Logging.h"

namespace DOH {

	const std::string Renderer2dStorageVulkan::QUAD_SHADER_PATH_VERT = "res/shaders/spv/QuadBatch.vert.spv";
	const std::string Renderer2dStorageVulkan::QUAD_SHADER_PATH_FRAG = "res/shaders/spv/QuadBatch.frag.spv";

	Renderer2dStorageVulkan::Renderer2dStorageVulkan(RenderingContextVulkan& context)
	:	mContext(context),
		mDescriptorPool(VK_NULL_HANDLE)
	{}

	void Renderer2dStorageVulkan::init(VkDevice logicDevice) {

		mWhiteTexture = ObjInit::texture(255.0f, 255.0f, 255.0f, 255.0f, false);

		initForQuads(logicDevice);

		std::vector<DescriptorTypeInfo> descTypes = mQuadGraphicsPipeline->getShaderProgram().getUniformLayout().asDescriptorTypes();
		mDescriptorPool = mContext.createDescriptorPool(descTypes, 1);

		mContext.createPipelineUniformObjects(*mQuadGraphicsPipeline, mDescriptorPool);
	}

	void Renderer2dStorageVulkan::close(VkDevice logicDevice) {
		mQuadShaderProgram->close(logicDevice);
		mQuadGraphicsPipeline->close(logicDevice);

		mWhiteTexture->close(logicDevice);

		for (std::shared_ptr<TextureVulkan> texture : mTestTextures) {
			texture->close(logicDevice);
		}

		for (std::shared_ptr<VertexArrayVulkan> vao : mQuadBatchVaos) {
			vao->close(logicDevice);
		}

		mQuadSharedIndexBuffer->close(logicDevice);

		vkDestroyDescriptorPool(logicDevice, mDescriptorPool, nullptr);
	}

	void Renderer2dStorageVulkan::closeSwapChainSpecificObjects(VkDevice logicDevice) {
		mQuadShaderProgram->closePipelineSpecificObjects(logicDevice);

		if (mQuadGraphicsPipeline != nullptr) {
			mQuadGraphicsPipeline->close(logicDevice);
		}

		vkDestroyDescriptorPool(logicDevice, mDescriptorPool, nullptr);
	}

	void Renderer2dStorageVulkan::recreateSwapChainSpecificObjects() {
		std::vector<DescriptorTypeInfo> descTypes = mQuadGraphicsPipeline->getShaderProgram().getUniformLayout().asDescriptorTypes();
		mDescriptorPool = mContext.createDescriptorPool(descTypes, 1);

		const uint32_t binding = 0;
		std::vector<VkVertexInputAttributeDescription> attribDesc = std::move(Vertex3dTextured::asAttributeDescriptions(binding));
		mQuadGraphicsPipeline = ObjInit::graphicsPipeline(
			mContext.getSwapChain().getExtent(),
			mContext.getSwapChain().getRenderPass(SwapChainVulkan::ERenderPassType::SCENE).get(),
			*mQuadShaderProgram,
			createBindingDescription(binding, sizeof(Vertex3dTextured), VK_VERTEX_INPUT_RATE_VERTEX),
			attribDesc
		);

		mContext.createPipelineUniformObjects(*mQuadGraphicsPipeline, mDescriptorPool);
	}

	size_t Renderer2dStorageVulkan::createNewBatchQuad() {
		if (mQuadRenderBatches.size() < BatchSizeLimits::MAX_BATCH_COUNT_QUAD) {
			RenderBatchQuad& batch = mQuadRenderBatches.emplace_back(
				BatchSizeLimits::BATCH_MAX_GEO_COUNT_QUAD,
				BatchSizeLimits::BATCH_MAX_COUNT_TEXTURE
			);

			std::shared_ptr<VertexArrayVulkan> vao = ObjInit::vertexArray();
			std::shared_ptr<VertexBufferVulkan> vbo = ObjInit::vertexBuffer(
				{
					{ EDataType::FLOAT3 },
					{ EDataType::FLOAT4 },
					{ EDataType::FLOAT2 },
					{ EDataType::FLOAT }
				},
				BatchSizeLimits::BATCH_QUAD_BYTE_SIZE,
				VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
			);
			vao->addVertexBuffer(vbo);			
			vao->setIndexBuffer(mQuadSharedIndexBuffer, true);

			mQuadBatchVaos.push_back(vao);

			return mQuadRenderBatches.size() - 1;
		} else {
			LOG_WARN("Max Quad Batch Count reached");
			return -1;
		}
	}

	void Renderer2dStorageVulkan::initForQuads(VkDevice logicDevice) {
		mQuadShaderProgram = ObjInit::shaderProgram(
			ObjInit::shader(EShaderType::VERTEX, Renderer2dStorageVulkan::QUAD_SHADER_PATH_VERT),
			ObjInit::shader(EShaderType::FRAGMENT, Renderer2dStorageVulkan::QUAD_SHADER_PATH_FRAG)
		);

		mQuadBatchTextureArray = std::make_unique<TextureArray>(
			BatchSizeLimits::BATCH_MAX_COUNT_TEXTURE,
			*mWhiteTexture
		);

		for (int i = 0; i < 8; i++) {
			std::string path = testTexturesPath + "texture" + std::to_string(i) + ".png";
			std::shared_ptr<TextureVulkan> testTexture = ObjInit::texture(path);
			mTestTextures.push_back(testTexture);
			
			//TODO:: dynamic texture arrays, instead of assigning at initialisation
			mQuadBatchTextureArray->addNewTexture(*testTexture);
		}

		ShaderUniformLayout& layout = mQuadShaderProgram->getUniformLayout();
		layout.setValue(0, sizeof(glm::mat4x4));
		layout.setTextureArray(1, *mQuadBatchTextureArray);

		const uint32_t binding = 0;
		std::vector<VkVertexInputAttributeDescription> attribDescs = std::move(Vertex3dTextured::asAttributeDescriptions(binding));

		mQuadGraphicsPipeline = ObjInit::graphicsPipeline(
			mContext.getSwapChain().getExtent(),
			mContext.getSwapChain().getRenderPass(SwapChainVulkan::ERenderPassType::SCENE).get(),
			*mQuadShaderProgram,
			createBindingDescription(binding, sizeof(Vertex3dTextured), VK_VERTEX_INPUT_RATE_VERTEX),
			attribDescs
		);

		//Quad Index Buffer
		std::vector<uint16_t> quadIndices;
		quadIndices.resize(BatchSizeLimits::BATCH_QUAD_INDEX_COUNT);
		uint16_t vertexOffset = 0;
		for (uint16_t i = 0; i < BatchSizeLimits::BATCH_QUAD_INDEX_COUNT; i += BatchSizeLimits::SINGLE_QUAD_INDEX_COUNT) {
			quadIndices[i + 0] = vertexOffset + 0;
			quadIndices[i + 1] = vertexOffset + 1;
			quadIndices[i + 2] = vertexOffset + 2;

			quadIndices[i + 3] = vertexOffset + 2;
			quadIndices[i + 4] = vertexOffset + 3;
			quadIndices[i + 5] = vertexOffset + 0;

			vertexOffset += BatchSizeLimits::SINGLE_QUAD_VERTEX_COUNT;
		}
		mQuadSharedIndexBuffer = ObjInit::stagedIndexBuffer(
			quadIndices.data(),
			sizeof(uint16_t) * BatchSizeLimits::BATCH_QUAD_INDEX_COUNT
		);
	}
}
