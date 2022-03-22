#pragma once

#include "dough/Utils.h"
#include "dough/rendering/pipeline/GraphicsPipelineVulkan.h"
#include "dough/scene/camera/ICamera.h"
#include "dough/ImGuiWrapper.h"

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
		std::vector<VkCommandBuffer> mCommandBuffers;

		std::shared_ptr<SwapChainVulkan> mSwapChain;

		std::shared_ptr<GraphicsPipelineVulkan> mSceneGraphicsPipeline;
		std::shared_ptr<GraphicsPipelineVulkan> mAppUiGraphicsPipeline;
		
		std::unique_ptr<ImGuiWrapper> mImGuiWrapper;
		
		//TEMP::
		std::shared_ptr<ShaderProgramVulkan> mAppUiShaderProgram;
		std::shared_ptr<VertexArrayVulkan> mAppUiVao;
		std::unique_ptr<std::string> mAppUiShaderVertPath = std::make_unique<std::string>("res/shaders/spv/SimpleUi.vert.spv");
		std::unique_ptr<std::string> mAppUiShaderFragPath = std::make_unique<std::string>("res/shaders/spv/SimpleUi.frag.spv");
		const std::vector<Vertex> mAppUiVertices {
			//	x		y		z		r		g		b
			{{	-1.0f,	-0.90f,	0.0f},	{0.0f,	1.0f,	0.0f}}, //bot-left
			{{	-0.75f,	-0.90f,	0.0f},	{0.0f,	0.5f,	0.5f}}, //bot-right
			{{	-0.75f,	-0.65f,	0.0f},	{0.0f,	0.0f,	1.0f}}, //top-right
			{{	-1.0f,	-0.65f,	0.0f},	{0.0f,	0.5f,	0.5f}}  //top-left
		};
		const std::vector<uint16_t> mAppUiIndices {
			0, 1, 2, 2, 3, 0
		};

		//NOTE:: in OpenGL space because glm
		glm::mat4x4 mAppUiProjection;

		VkDescriptorPool mDescriptorPool;
		VkCommandPool mCommandPool;

		std::vector<std::reference_wrapper<IGPUResourceVulkan>> mToReleaseGpuResources;

		//Sync objects
		const int MAX_FRAMES_IN_FLIGHT = 2;
		size_t mCurrentFrame = 0;
		std::vector<VkSemaphore> mImageAvailableSemaphores;
		std::vector<VkSemaphore> mRenderFinishedSemaphores;
		std::vector<VkFence> mFramesInFlightFences;
		std::vector<VkFence> mImageFencesInFlight;

	public:
		RenderingContextVulkan(VkDevice logicDevice, VkPhysicalDevice physicalDevice);

		RenderingContextVulkan(const RenderingContextVulkan& copy) = delete;
		RenderingContextVulkan operator=(const RenderingContextVulkan& assignment) = delete;

		void init(
			SwapChainSupportDetails& scSupport,
			VkSurfaceKHR surface,
			QueueFamilyIndices& queueFamilyIndices,
			Window& window,
			VkInstance vulkanInstance
		);
		void close();
		void setupPostAppLogicInit();
		void prepareScenePipeline(ShaderProgramVulkan& shaderProgram, bool createUniformObjects = false);
		//TODO:: Currently not taking in shader program as it is using a pre-determined shader program, custom input will be supported later
		void prepareAppUiPipeline(bool createUniformObjects = false);
		void createPipelineUniformObjects(GraphicsPipelineVulkan& pipeline);
		bool isReady() const;

		void resizeSwapChain(
			SwapChainSupportDetails& scSupport,
			VkSurfaceKHR surface,
			QueueFamilyIndices& queueFamilyIndices,
			uint32_t width,
			uint32_t height
		);
		void updateUiProjectionMatrix(float aspectRatio);

		void drawFrame();
		inline void setUniformBufferObject(ICamera& camera) { mUbo.ProjectionViewMat = camera.getProjectionViewMatrix(); }
		void updateUniformBufferObject(uint32_t currentImage);
		void addVaoToDraw(VertexArrayVulkan& vao);

		inline void addResourceToCloseAfterUse(IGPUResourceVulkan& res) { mToReleaseGpuResources.push_back(res); }

		//TODO:: Prefer a grouping commands together then flushing rather than only single commands
		
		//	Creates a command buffer and begins it, the caller MUST call endSingleTimeCommands at some point,
		//	at which point the queue waits upon its exectuion
		VkCommandBuffer beginSingleTimeCommands();
		void endSingleTimeCommands(VkCommandBuffer cmdBuffer);

		//Convenience functions
		//Usage param is optional
		void beginCommandBuffer(VkCommandBuffer cmd, VkCommandBufferUsageFlags usage = 0);
		void endCommandBuffer(VkCommandBuffer cmd);

		//TODO:: These functions call begin and end single time commands individually
		void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
		void transitionImageLayout(
			VkImage image,
			VkImageLayout oldLayout,
			VkImageLayout newLayout,
			VkImageAspectFlags aspectFlags
		);
		inline VkImage createImage(
			uint32_t width,
			uint32_t height,
			VkFormat format,
			VkImageTiling tiling,
			VkImageUsageFlags usage
		) {
			return createImage(mLogicDevice, mPhysicalDevice, width, height, format, tiling, usage);
		};
		inline VkDeviceMemory createImageMemory(VkImage image, VkMemoryPropertyFlags props) {
			return createImageMemory(mLogicDevice, mPhysicalDevice, image, props);
		};
		VkImageView createImageView(VkImage image, VkFormat format);
		VkSampler createSampler();
		inline ImGuiWrapper& getImGuiWrapper() const { return *mImGuiWrapper; }
		inline void setLogicDevice(VkDevice logicDevice) { mLogicDevice = logicDevice; }
		void setPhysicalDevice(VkPhysicalDevice physicalDevice);

		static VkImage createImage(
			VkDevice logicDevice,
			VkPhysicalDevice physicalDevice,
			uint32_t width,
			uint32_t height,
			VkFormat format,
			VkImageTiling tiling,
			VkImageUsageFlags usage
		);
		static VkDeviceMemory createImageMemory(
			VkDevice logicDevice,
			VkPhysicalDevice physicalDevice,
			VkImage image,
			VkMemoryPropertyFlags props
		);

		//-----Rendering Object Initialisation-----
		//-----Pipeline-----
		std::shared_ptr<GraphicsPipelineVulkan> createGraphicsPipeline(
			VkExtent2D extent,
			VkRenderPass renderPass,
			ShaderProgramVulkan& shaderProgram,
			VkVertexInputBindingDescription vertexInputBindingDesc,
			std::vector<VkVertexInputAttributeDescription>& vertexAttributes
		);

		//-----Context-----
		std::shared_ptr<SwapChainVulkan> createSwapChain(SwapChainCreationInfo& swapChainCreate);
		std::shared_ptr<RenderPassVulkan> createRenderPass(
			VkFormat imageFormat,
			bool hasPassBefore,
			bool hasPassAfter,
			bool enableClearColour,
			VkClearValue clearColour
		);

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
		//TODO:: std::shared_ptr<TextureVulkan> createTexture(glm::vec4& colour);

	private:
		void createQueues(QueueFamilyIndices& queueFamilyIndices);
		void createCommandPool(QueueFamilyIndices& queueFamilyIndices);
		void createCommandBuffers();
		void createDescriptorPool(std::vector<VkDescriptorType>& descTypes);
		void createSyncObjects();
		void closeSyncObjects();
		void present(uint32_t imageIndex, VkCommandBuffer cmd);
	};
}
