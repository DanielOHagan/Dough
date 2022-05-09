#pragma once

#include "dough/Utils.h"
#include "dough/rendering/pipeline/GraphicsPipelineVulkan.h"
#include "dough/scene/camera/ICamera.h"
#include "dough/ImGuiWrapper.h"
#include "dough/rendering/renderer2d/Renderer2dVulkan.h"

namespace DOH {

	class RenderingContextVulkan {

	private:

		struct UniformBufferObject {
			glm::mat4 ProjectionViewMat;
		} mSceneUbo;

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

		std::unique_ptr<Renderer2dVulkan> mRenderer2d;
		std::unique_ptr<ImGuiWrapper> mImGuiWrapper;

		//NOTE:: in OpenGL space because glm
		glm::mat4x4 mAppUiProjection;

		//Used by Scene and UI pipelines
		//TODO:: when fully transitioned to batch renderer, move this to a renderer3d or something
		//	as 2d rendering should thereby be done solely by batch rendering (except maybe some debug things)
		VkDescriptorPool mDescriptorPool;


		VkCommandPool mCommandPool;

		std::vector<std::shared_ptr<IGPUResourceVulkan>> mToReleaseGpuResources;

		//Sync objects
		const int MAX_FRAMES_IN_FLIGHT = 2;
		size_t mCurrentFrame = 0;
		std::vector<VkSemaphore> mImageAvailableSemaphores;
		std::vector<VkSemaphore> mRenderFinishedSemaphores;
		std::vector<VkFence> mFramesInFlightFences;
		std::vector<VkFence> mImageFencesInFlight;

		bool mUsingScenePipeline;
		bool mUsingUiPipeline;

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
		//Finalise the creation of custom pipelines
		void prepareScenePipeline(ShaderProgramVulkan& shaderProgram, bool createUniformObjects = false);
		void prepareAppUiPipeline(ShaderProgramVulkan& shaderProgram, bool createUniformObjects = false);
		void preparePipeline(
			std::shared_ptr<GraphicsPipelineVulkan> graphicsPipeline,
			ShaderProgramVulkan& shaderProgram,
			VkRenderPass renderPass,
			std::vector<VkVertexInputAttributeDescription>& attribDesc,
			uint32_t vertexStride
		);
		void createPipelineUniformObjects(GraphicsPipelineVulkan& pipeline, VkDescriptorPool descPool);
		void createCustomPipelinesUniformObjects();
		void closeCustomPipelines();
		void closeScenePipeline();
		void closeUiPipeline();
		VkDescriptorPool createDescriptorPool(std::vector<DescriptorTypeInfo>& descTypes, uint32_t pipelineCount);
		bool isReady() const;

		inline void onResize(
			SwapChainSupportDetails& scSupport,
			VkSurfaceKHR surface,
			QueueFamilyIndices& queueFamilyIndices,
			int width,
			int height
		) {
			resizeSwapChain(scSupport, surface, queueFamilyIndices, width, height);
			mImGuiWrapper->onWindowResize(width, height);
		};

		void drawFrame();
		inline void setSceneUniformBufferObject(ICamera& camera) { mSceneUbo.ProjectionViewMat = camera.getProjectionViewMatrix(); }
		inline void setUiProjection(glm::mat4x4& proj) { mAppUiProjection = proj; }
		inline void addVaoToSceneDrawList(VertexArrayVulkan& vao) const { mSceneGraphicsPipeline->addVaoToDraw(vao); }
		inline void addVaoToUiDrawList(VertexArrayVulkan& vao) const { mAppUiGraphicsPipeline->addVaoToDraw(vao); }

		inline void addResourceToCloseAfterUse(std::shared_ptr<IGPUResourceVulkan> res) { mToReleaseGpuResources.push_back(res); }
		void releaseGpuResources();

		//TODO:: Prefer grouping commands together then flushing rather than only single commands
		
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
		inline SwapChainVulkan& getSwapChain() const { return *mSwapChain; }
		inline Renderer2dVulkan& getRenderer2d() const { return *mRenderer2d; }
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
		std::shared_ptr<IndexBufferVulkan> createIndexBuffer(VkDeviceSize size);
		std::shared_ptr<IndexBufferVulkan> createStagedIndexBuffer(void* data, VkDeviceSize size);
		std::shared_ptr<IndexBufferVulkan> createStagedIndexBuffer(const void* data, VkDeviceSize size);
		std::unique_ptr<IndexBufferVulkan> createSharedStagedIndexBuffer(const void* data, VkDeviceSize size);
		std::shared_ptr<BufferVulkan> createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags props);
		std::shared_ptr<BufferVulkan> createStagedBuffer(void* data, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags props);
		std::shared_ptr<BufferVulkan> createStagedBuffer(const void* data, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags props);

		//-----Shader-----
		std::shared_ptr<ShaderProgramVulkan> createShaderProgram(std::shared_ptr<ShaderVulkan> vertShader, std::shared_ptr<ShaderVulkan> fragShader);
		std::shared_ptr<ShaderVulkan> createShader(EShaderType type, const std::string& filePath);

		//-----Texture-----
		std::shared_ptr<TextureVulkan> createTexture(const std::string& filePath);
		std::shared_ptr<TextureVulkan> createTexture(float r, float g, float b, float a, bool colourRgbaNormalised);

	private:
		void createQueues(QueueFamilyIndices& queueFamilyIndices);
		void createCommandPool(QueueFamilyIndices& queueFamilyIndices);
		void createCommandBuffers();
		void createSyncObjects();
		void closeSyncObjects();
		void resizeSwapChain(
			SwapChainSupportDetails& scSupport,
			VkSurfaceKHR surface,
			QueueFamilyIndices& queueFamilyIndices,
			uint32_t width,
			uint32_t height
		);
		void updateSceneUbo(uint32_t currentImage);
		void updateUiProjectionMatrix(uint32_t currentImage);
		void drawScene(uint32_t imageIndex, VkCommandBuffer cmd);
		void drawUi(uint32_t imageIndex, VkCommandBuffer cmd);
		void present(uint32_t imageIndex, VkCommandBuffer cmd);
	};
}
