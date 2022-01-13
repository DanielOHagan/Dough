#pragma once

#include "dough/Utils.h"
#include "dough/rendering/pipeline/GraphicsPipelineVulkan.h"
#include "dough/scene/camera/ICamera.h"

namespace DOH {

	class RenderingContextVulkan {

	private:

		struct UniformBufferObject {
			glm::mat4 ProjectionViewMat;
		} mUbo;
		
		//Shared device handles for convenience
		VkDevice mLogicDevice;
		VkPhysicalDevice mPhysicalDevice;

		std::unique_ptr<VkPhysicalDeviceProperties> mPhysicalDeviceProperties;
		std::unique_ptr<SwapChainCreationInfo> mSwapChainCreationInfo;
		
		VkQueue mGraphicsQueue;
		VkQueue mPresentQueue;

		std::shared_ptr<GraphicsPipelineVulkan> mGraphicsPipeline;

		//Allocators
		VkDescriptorPool mDescriptorPool;
		VkCommandPool mCommandPool;

		std::vector<std::reference_wrapper<IGPUResourceVulkan>> mToReleaseGpuResources;

	public:
		RenderingContextVulkan(VkDevice logicDevice, VkPhysicalDevice physicalDevice);

		RenderingContextVulkan(const RenderingContextVulkan& copy) = delete;
		RenderingContextVulkan operator=(const RenderingContextVulkan& assignment) = delete;

		void init(
			SwapChainSupportDetails& scSupport,
			VkSurfaceKHR surface,
			QueueFamilyIndices& queueFamilyIndices,
			uint32_t width,
			uint32_t height
		);
		void close();
		void openPipeline(ShaderProgramVulkan& shaderProgram);
		void setupPipeline(ShaderProgramVulkan& shaderProgram);

		void resizeSwapChain(
			SwapChainSupportDetails& scSupport,
			VkSurfaceKHR surface,
			QueueFamilyIndices& queueFamilyIndices,
			uint32_t width,
			uint32_t height
		);

		void drawFrame();
		inline void setUniformBufferObject(glm::mat4& projView) { mUbo.ProjectionViewMat = projView; };
		void updateUniformBufferObject(uint32_t currentImage);
		void addVaoToDraw(VertexArrayVulkan& vao);

		inline void addResourceToCloseAfterUse(IGPUResourceVulkan& res) { mToReleaseGpuResources.push_back(res); }

		//TODO:: This
		VkCommandBuffer beginSingleTimeCommands();
		void endSingleTimeCommands(VkCommandBuffer cmdBuffer);

		void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

		VkImageView createImageView(VkImage image, VkFormat format);
		VkSampler createSampler();

		void setLogicDevice(VkDevice logicDevice) { mLogicDevice = logicDevice; }
		void setPhysicalDevice(VkPhysicalDevice physicalDevice);

		//-----Rendering Object Initialisation-----
		//-----Pipeline-----
		std::shared_ptr<GraphicsPipelineVulkan> createGraphicsPipeline(SwapChainCreationInfo& swapChainCreate, ShaderProgramVulkan& shaderProgram);
		std::shared_ptr<SwapChainVulkan> createSwapChain(SwapChainCreationInfo& swapChainCreate);
		std::shared_ptr<RenderPassVulkan> createRenderPass(VkFormat imageFormat);

		//-----VAO & Buffers-----
		std::shared_ptr<VertexArrayVulkan> createVertexArray();
		std::shared_ptr<VertexBufferVulkan> createVertexBuffer(
			const std::initializer_list<BufferElement>& elements,
			VkDeviceSize size,
			VkBufferUsageFlags usage,
			VkMemoryPropertyFlags props
		);
		std::shared_ptr<VertexBufferVulkan> createStagedVertexBuffer(
			const std::initializer_list<BufferElement>& elements,
			void* data,
			VkDeviceSize size,
			VkBufferUsageFlags usage,
			VkMemoryPropertyFlags props
		);
		std::shared_ptr<VertexBufferVulkan> createStagedVertexBuffer(
			const std::initializer_list<BufferElement>& elements,
			const void* data,
			VkDeviceSize size,
			VkBufferUsageFlags usage,
			VkMemoryPropertyFlags props
		);
		std::shared_ptr<IndexBufferVulkan> createIndexBuffer(VkDeviceSize size, uint32_t count);
		std::shared_ptr<IndexBufferVulkan> createStagedIndexBuffer(void* data, VkDeviceSize size, uint32_t count);
		std::shared_ptr<IndexBufferVulkan> createStagedIndexBuffer(const void* data, VkDeviceSize size, uint32_t count);
		std::shared_ptr<BufferVulkan> createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags props);
		std::shared_ptr<BufferVulkan> createStagedBuffer(void* data, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags props);
		std::shared_ptr<BufferVulkan> createStagedBuffer(const void* data, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags props);

		//-----Shader-----
		std::shared_ptr<ShaderProgramVulkan> createShaderProgram(std::shared_ptr<ShaderVulkan> vertShader, std::shared_ptr<ShaderVulkan> fragShader);
		std::shared_ptr<ShaderVulkan> createShader(EShaderType type, std::string& filePath);

		//-----Texture-----
		std::shared_ptr<TextureVulkan> createTexture(std::string& filePath);

	private:
		void createQueues(QueueFamilyIndices& queueFamilyIndices);

		void createCommandPool(QueueFamilyIndices& queueFamilyIndices);
		void createDescriptorPool(std::vector<VkDescriptorType>& descTypes);
	};
}
