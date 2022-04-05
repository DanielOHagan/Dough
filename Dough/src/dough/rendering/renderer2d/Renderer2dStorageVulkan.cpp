#include "dough/rendering/renderer2d/Renderer2dStorageVulkan.h"

#include "dough/rendering/RenderingContextVulkan.h"
#include "dough/rendering/ObjInit.h"
#include "dough/Logging.h"

namespace DOH {

	const std::string Renderer2dStorageVulkan::QUAD_SHADER_PATH_VERT = "res/shaders/spv/QuadBatch.vert.spv";
	const std::string Renderer2dStorageVulkan::QUAD_SHADER_PATH_FRAG = "res/shaders/spv/QuadBatch.frag.spv";

	Renderer2dStorageVulkan::Renderer2dStorageVulkan(RenderingContextVulkan& context)
	:	mContext(context),
		mSceneQuadDataIndex(0),
		mSceneQuadCount(0),
		mDescriptorPool(VK_NULL_HANDLE)
	{}

	void Renderer2dStorageVulkan::init(VkDevice logicDevice) {

		mWhiteTexture = ObjInit::texture(255.0f, 255.0f, 255.0f, 255.0f, false);

		for (int i = 0; i < 8; i++) {
			std::string path = testTexturesPath + "texture" + std::to_string(i) + ".png";
			std::shared_ptr<TextureVulkan> testTexture = ObjInit::texture(path);
			mTestTextures.push_back(testTexture);
		}

		initForQuads(logicDevice);

		std::vector<DescriptorTypeInfo> descTypes = mQuadGraphicsPipeline->getShaderProgram().getUniformLayout().asDescriptorTypes();
		mDescriptorPool = mContext.createDescriptorPool(descTypes, 1);

		mContext.createPipelineUniformObjects(*mQuadGraphicsPipeline, mDescriptorPool);
	}

	void Renderer2dStorageVulkan::close(VkDevice logicDevice) {
		mQuadShaderProgram->close(logicDevice);
		mQuadGraphicsPipeline->close(logicDevice);
		mQuadVao->close(logicDevice);
		mQuadVbo->close(logicDevice);

		mSceneQuadData.clear();

		mWhiteTexture->close(logicDevice);

		for (std::shared_ptr<TextureVulkan> texture : mTestTextures) {
			texture->close(logicDevice);
		}

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

	void Renderer2dStorageVulkan::initForQuads(VkDevice logicDevice) {
		mQuadShaderProgram = ObjInit::shaderProgram(
			ObjInit::shader(EShaderType::VERTEX, Renderer2dStorageVulkan::QUAD_SHADER_PATH_VERT),
			ObjInit::shader(EShaderType::FRAGMENT, Renderer2dStorageVulkan::QUAD_SHADER_PATH_FRAG)
		);

		mSceneQuadData.resize(BatchSizeLimits::QUAD_COMPONENT_COUNT);

		ShaderUniformLayout& layout = mQuadShaderProgram->getUniformLayout();
		layout.setValue(0, sizeof(glm::mat4x4));
		layout.setTextureArray(
			1, 
			{
				{ mTestTextures[0]->getImageView(), mTestTextures[0]->getSampler() },
				{ mTestTextures[1]->getImageView(), mTestTextures[1]->getSampler() },
				{ mTestTextures[2]->getImageView(), mTestTextures[2]->getSampler() },
				{ mTestTextures[3]->getImageView(), mTestTextures[3]->getSampler() },
				{ mTestTextures[4]->getImageView(), mTestTextures[4]->getSampler() },
				{ mTestTextures[5]->getImageView(), mTestTextures[5]->getSampler() },
				{ mTestTextures[6]->getImageView(), mTestTextures[6]->getSampler() },
				{ mTestTextures[7]->getImageView(), mTestTextures[7]->getSampler() }
			},
			8,
			{ mWhiteTexture->getImageView(), mWhiteTexture->getSampler() }
		);

		glm::vec2 quadSize = { 0.1f, 0.1f };
		const float quadGapDistanceX = quadSize.x * 1.5f;
		const float quadGapDistanceY = quadSize.y * 1.5f;

		for (float x = 0.0f; x < 10.0f; x++) {
			for (float y = 0.0f; y < 10.0f; y++) {
				Quad quad = {
					{x * quadGapDistanceX, y * quadGapDistanceY, 1.0f},
					quadSize,
					{0.0f, 1.0f, 1.0f, 1.0f}
				};
				uint32_t textureSlot = static_cast<uint32_t>(x) % 8;
				addSceneQuad(quad, (uint32_t) (x + y) % 8);
			}
		}

		mQuadVao = ObjInit::vertexArray();
		mQuadVbo = ObjInit::stagedVertexBuffer(
			{
				{ EDataType::FLOAT3 },
				{ EDataType::FLOAT4 },
				{ EDataType::FLOAT2 },
				{ EDataType::FLOAT }
			},
			mSceneQuadData.data(),
			sizeof(float) * mSceneQuadData.size(),
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		);
		mQuadVao->addVertexBuffer(mQuadVbo);

		//Quad index buffer
		std::vector<uint16_t> quadIndices;
		quadIndices.resize(BatchSizeLimits::QUAD_INDEX_COUNT);
		uint16_t vertexOffset = 0;
		for (uint16_t i = 0; i < BatchSizeLimits::QUAD_INDEX_COUNT; i += BatchSizeLimits::SINGLE_QUAD_INDEX_COUNT) {
			quadIndices[i + 0] = vertexOffset + 0;
			quadIndices[i + 1] = vertexOffset + 1;
			quadIndices[i + 2] = vertexOffset + 2;
		
			quadIndices[i + 3] = vertexOffset + 2;
			quadIndices[i + 4] = vertexOffset + 3;
			quadIndices[i + 5] = vertexOffset + 0;
		
			vertexOffset += BatchSizeLimits::SINGLE_QUAD_VERTEX_COUNT;
		}
		std::shared_ptr<IndexBufferVulkan> quadIb = ObjInit::stagedIndexBuffer(
			quadIndices.data(),
			sizeof(uint16_t) * BatchSizeLimits::QUAD_INDEX_COUNT,
			BatchSizeLimits::QUAD_INDEX_COUNT
		);
		mQuadVao->setIndexBuffer(quadIb);

		const uint32_t binding = 0;
		std::vector<VkVertexInputAttributeDescription> attribDescs = std::move(Vertex3dTextured::asAttributeDescriptions(binding));


		mQuadGraphicsPipeline = ObjInit::graphicsPipeline(
			mContext.getSwapChain().getExtent(),
			mContext.getSwapChain().getRenderPass(SwapChainVulkan::ERenderPassType::SCENE).get(),
			*mQuadShaderProgram,
			createBindingDescription(binding, sizeof(Vertex3dTextured), VK_VERTEX_INPUT_RATE_VERTEX),
			attribDescs
		);
	}

	void Renderer2dStorageVulkan::addSceneQuad(Quad& quad) {
		int texCoordsIndex = 0;
		const uint32_t textureSlotIndex = 0;

		addSceneQuadVertexToBatch(
			quad.getPosition().x,
			quad.getPosition().y,
			quad.getPosition().z,
			quad.getColour().x,
			quad.getColour().y,
			quad.getColour().z,
			quad.getColour().w,
			//quad.mTextureCoordsU[texCoordsIndex],
			//quad.mTextureCoordsV[texCoordsIndex],
			0.0f,
			1.0f,
			textureSlotIndex
		);
		texCoordsIndex++;

		addSceneQuadVertexToBatch(
			quad.getPosition().x + quad.getSize().x,
			quad.getPosition().y,
			quad.getPosition().z,
			quad.getColour().x,
			quad.getColour().y,
			quad.getColour().z,
			quad.getColour().w,
			//quad.mTextureCoordsU[texCoordsIndex],
			//quad.mTextureCoordsV[texCoordsIndex],
			1.0f,
			1.0f,
			textureSlotIndex
		);
		texCoordsIndex++;

		addSceneQuadVertexToBatch(
			quad.getPosition().x + quad.getSize().x,
			quad.getPosition().y + quad.getSize().y,
			quad.getPosition().z,
			quad.getColour().x,
			quad.getColour().y,
			quad.getColour().z,
			quad.getColour().w,
			//quad.mTextureCoordsU[texCoordsIndex],
			//quad.mTextureCoordsV[texCoordsIndex],
			1.0f,
			0.0f,
			textureSlotIndex
		);
		texCoordsIndex++;

		addSceneQuadVertexToBatch(
			quad.getPosition().x,
			quad.getPosition().y + quad.getSize().y,
			quad.getPosition().z,
			quad.getColour().x,
			quad.getColour().y,
			quad.getColour().z,
			quad.getColour().w,
			//quad.mTextureCoordsU[texCoordsIndex],
			//quad.mTextureCoordsV[texCoordsIndex],
			0.0f,
			0.0f,
			textureSlotIndex
		);

		mSceneQuadCount++;
	}

	void Renderer2dStorageVulkan::addSceneQuad(Quad& quad, uint32_t textureSlotIndex) {
		int texCoordsIndex = 0;

		addSceneQuadVertexToBatch(
			quad.getPosition().x,
			quad.getPosition().y,
			quad.getPosition().z,
			quad.getColour().x,
			quad.getColour().y,
			quad.getColour().z,
			quad.getColour().w,
			//quad.mTextureCoordsU[texCoordsIndex],
			//quad.mTextureCoordsV[texCoordsIndex],
			0.0f,
			1.0f,
			static_cast<float>(textureSlotIndex)
		);
		texCoordsIndex++;

		addSceneQuadVertexToBatch(
			quad.getPosition().x + quad.getSize().x,
			quad.getPosition().y,
			quad.getPosition().z,
			quad.getColour().x,
			quad.getColour().y,
			quad.getColour().z,
			quad.getColour().w,
			//quad.mTextureCoordsU[texCoordsIndex],
			//quad.mTextureCoordsV[texCoordsIndex],
			1.0f,
			1.0f,
			static_cast<float>(textureSlotIndex)
		);
		texCoordsIndex++;

		addSceneQuadVertexToBatch(
			quad.getPosition().x + quad.getSize().x,
			quad.getPosition().y + quad.getSize().y,
			quad.getPosition().z,
			quad.getColour().x,
			quad.getColour().y,
			quad.getColour().z,
			quad.getColour().w,
			//quad.mTextureCoordsU[texCoordsIndex],
			//quad.mTextureCoordsV[texCoordsIndex],
			1.0f,
			0.0f,
			static_cast<float>(textureSlotIndex)
		);
		texCoordsIndex++;

		addSceneQuadVertexToBatch(
			quad.getPosition().x,
			quad.getPosition().y + quad.getSize().y,
			quad.getPosition().z,
			quad.getColour().x,
			quad.getColour().y,
			quad.getColour().z,
			quad.getColour().w,
			//quad.mTextureCoordsU[texCoordsIndex],
			//quad.mTextureCoordsV[texCoordsIndex],
			0.0f,
			0.0f,
			static_cast<float>(textureSlotIndex)
		);

		mSceneQuadCount++;
	}

	void Renderer2dStorageVulkan::resetBatch() {
		mSceneQuadCount = 0;
		mSceneQuadDataIndex = 0;
		//mQuadTextureSlots.clear();
	}

	void Renderer2dStorageVulkan::addSceneQuadVertexToBatch(
		float posX,
		float posY,
		float posZ,
		float colourR,
		float colourG,
		float colourB,
		float colourA,
		float texCoordU,
		float texCoordV,
		float texIndex 
	) {
		mSceneQuadData[mSceneQuadDataIndex] = posX;
		mSceneQuadDataIndex++;
		mSceneQuadData[mSceneQuadDataIndex]  = posY;
		mSceneQuadDataIndex++;
		mSceneQuadData[mSceneQuadDataIndex]  = posZ;
		mSceneQuadDataIndex++;
		mSceneQuadData[mSceneQuadDataIndex]  = colourR;
		mSceneQuadDataIndex++;
		mSceneQuadData[mSceneQuadDataIndex]  = colourG;
		mSceneQuadDataIndex++;
		mSceneQuadData[mSceneQuadDataIndex]  = colourB;
		mSceneQuadDataIndex++;
		mSceneQuadData[mSceneQuadDataIndex]  = colourA;
		mSceneQuadDataIndex++;

		mSceneQuadData[mSceneQuadDataIndex] = texCoordU;
		mSceneQuadDataIndex++;
		mSceneQuadData[mSceneQuadDataIndex] = texCoordV;
		mSceneQuadDataIndex++;
		mSceneQuadData[mSceneQuadDataIndex] = texIndex;
		mSceneQuadDataIndex++;
	}
}
