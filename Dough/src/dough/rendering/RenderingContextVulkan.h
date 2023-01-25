#pragma once

#include "dough/Utils.h"
#include "dough/rendering/pipeline/GraphicsPipelineVulkan.h"
#include "dough/scene/camera/ICamera.h"
#include "dough/ImGuiWrapper.h"
#include "dough/rendering/renderer2d/Renderer2dVulkan.h"
#include "dough/rendering/SwapChainVulkan.h"

namespace DOH {

	enum class ERenderPass {
		APP_SCENE,
		APP_UI
	};

	class RenderingContextVulkan {

	private:

		struct UniformBufferObject {
			glm::mat4 ProjectionViewMat;
		} mAppSceneUbo;

		struct RenderingDeviceInfo {
			const std::string ApiVersion;
			const std::string DeviceName;
			const std::string DeviceDriverVersion;
			const bool ValidationLayersEnabled;

			RenderingDeviceInfo(
				const std::string& apiVersion,
				const std::string& deviceName,
				const std::string& deviceDriverVersion,
				const bool validationLayersEnabled
			) : ApiVersion(apiVersion),
				DeviceName(deviceName),
				DeviceDriverVersion(deviceDriverVersion),
				ValidationLayersEnabled(validationLayersEnabled)
			{}
		};

		//NOTE:: in OpenGL space because glm
		glm::mat4x4 mAppUiProjection;

		//Shared device handles for convenience
		VkDevice mLogicDevice;
		VkPhysicalDevice mPhysicalDevice;
		std::unique_ptr<RenderingDeviceInfo> mRenderingDeviceInfo;

		std::unique_ptr<VkPhysicalDeviceProperties> mPhysicalDeviceProperties;
		std::unique_ptr<SwapChainCreationInfo> mSwapChainCreationInfo;

		VkQueue mGraphicsQueue;
		VkQueue mPresentQueue;
		std::vector<VkCommandBuffer> mCommandBuffers;

		std::shared_ptr<SwapChainVulkan> mSwapChain;

		std::unordered_map<std::string, std::shared_ptr<GraphicsPipelineVulkan>> mAppSceneGraphicsPipelines;
		std::unordered_map<std::string, std::shared_ptr<GraphicsPipelineVulkan>> mAppUiGraphicsPipelines;

		std::unique_ptr<Renderer2dVulkan> mRenderer2d;
		std::unique_ptr<ImGuiWrapper> mImGuiWrapper;

		std::vector<VkFramebuffer> mAppSceneFrameBuffers;
		std::vector<VkFramebuffer> mAppUiFrameBuffers;

		std::shared_ptr<RenderPassVulkan> mAppSceneRenderPass;
		std::shared_ptr<RenderPassVulkan> mAppUiRenderPass;

		std::vector<ImageVulkan> mAppSceneDepthImages;

		//Used by Scene and UI pipelines
		//TODO::
		// Separate the custom Scene and UI resources (descriptors, pipelines, buffers, etc...)
		//	so they have their own.
		VkDescriptorPool mDescriptorPool;

		VkCommandPool mCommandPool;

		std::vector<std::shared_ptr<IGPUResourceVulkan>> mToReleaseGpuResources;

		//Sync objects
		const int MAX_FRAMES_IN_FLIGHT = 2;
		size_t mCurrentFrame = 0;
		std::vector<VkSemaphore> mImageAvailableSemaphores;
		std::vector<VkSemaphore> mRenderFinishedSemaphores;
		std::vector<VkFence> mFramesInFlightFences;

		VkFormat mDepthFormat;

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
		void createPipelineUniformObjects(GraphicsPipelineVulkan& pipeline, VkDescriptorPool descPool);
		void createPipelineUniformObjects();
		void closeAppPipelines();
		void closeAppScenePipelines();
		void closeAppUiPipelines();
		VkDescriptorPool createDescriptorPool(std::vector<DescriptorTypeInfo>& descTypes);
		bool isReady() const;

		//TODO:: return PipelineVaoConveyor for easier and faster vao adding?
		//	Take in a pipeline builder object?
		PipelineRenderableConveyor createPipeline(
			const std::string& name,
			GraphicsPipelineInstanceInfo& instanceInfo,
			const bool enabled = true
		);
		PipelineRenderableConveyor createPipelineConveyor(
			const ERenderPass renderPass,
			const std::string& name
		);
		void enablePipeline(const ERenderPass renderPass, const std::string& name, bool enable);
		void closePipeline(const ERenderPass renderPass, const std::string& name);

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
		inline void setAppSceneUniformBufferObject(ICamera& camera) { mAppSceneUbo.ProjectionViewMat = camera.getProjectionViewMatrix(); }
		inline void setAppUiProjection(glm::mat4x4& proj) { mAppUiProjection = proj; }

		inline std::unordered_map<std::string, std::shared_ptr<GraphicsPipelineVulkan>> getAppSceneGraphicsPipelines() const { return mAppSceneGraphicsPipelines; }
		inline std::unordered_map<std::string, std::shared_ptr<GraphicsPipelineVulkan>> getAppUiGraphicsPipelines() const { return mAppUiGraphicsPipelines; }
		RenderPassVulkan& getRenderPass(const ERenderPass renderPass) const;

		//TODO:: Ability for better control over when GPU resources can be released (e.g. after certain program stages or as soon as possible)
		inline void addGpuResourceToCloseAfterUse(std::shared_ptr<IGPUResourceVulkan> res) { mToReleaseGpuResources.emplace_back(res); }
		void closeGpuResourceImmediately(std::shared_ptr<IGPUResourceVulkan> res);
		void closeGpuResourceImmediately(IGPUResourceVulkan& res);
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
		void transitionStagedImageLayout(
			VkImage image,
			VkImageLayout oldLayout,
			VkImageLayout newLayout,
			VkImageAspectFlags aspectFlags
		);
		void transitionImageLayout(
			VkImage image,
			VkImageLayout oldLayout,
			VkImageLayout newLayout,
			VkImageAspectFlags aspectFlags,
			VkPipelineStageFlags srcStage,
			VkPipelineStageFlags dstStage,
			VkAccessFlags srcAccessMask,
			VkAccessFlags dstAccessMask
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
		VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
		VkSampler createSampler();

		inline uint32_t getAppFrameBufferCount() const { return static_cast<uint32_t>(mAppSceneFrameBuffers.size() + mAppUiFrameBuffers.size()); }
		inline ImGuiWrapper& getImGuiWrapper() const { return *mImGuiWrapper; }
		inline SwapChainVulkan& getSwapChain() const { return *mSwapChain; }
		inline Renderer2dVulkan& getRenderer2d() const { return *mRenderer2d; }
		inline void setLogicDevice(VkDevice logicDevice) { mLogicDevice = logicDevice; }
		void setPhysicalDevice(VkPhysicalDevice physicalDevice);
		inline RenderingDeviceInfo& getRenderingDeviceInfo() const { return *mRenderingDeviceInfo; }
		inline size_t getCurrentFrame() const { return mCurrentFrame; }

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
		//TODO:: having a "createPipeline" and "createGraphicsPipeline" is confusing
		std::shared_ptr<GraphicsPipelineVulkan> createGraphicsPipeline(
			GraphicsPipelineInstanceInfo& instanceInfo,
			VkExtent2D extent
		);

		//-----Context-----
		std::shared_ptr<SwapChainVulkan> createSwapChain(SwapChainCreationInfo& swapChainCreate);

		//-----VAO & Buffers-----
		std::shared_ptr<VertexArrayVulkan> createVertexArray();
		std::shared_ptr<VertexBufferVulkan> createVertexBuffer(
			const std::initializer_list<BufferElement>& elements,
			VkDeviceSize size,
			VkBufferUsageFlags usage,
			VkMemoryPropertyFlags props
		);
		std::shared_ptr<VertexBufferVulkan> createVertexBuffer(
			const std::vector<BufferElement>& elements,
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
		std::shared_ptr<VertexBufferVulkan> createStagedVertexBuffer(
			const std::vector<BufferElement>& elements,
			const void* data,
			VkDeviceSize size,
			VkBufferUsageFlags usage,
			VkMemoryPropertyFlags props
		);
		std::shared_ptr<VertexBufferVulkan> createStagedVertexBuffer(
			const EVertexType vertexType,
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
		std::shared_ptr<TextureVulkan> createTexture(float r, float g, float b, float a, bool colourRgbaNormalised = false, const char* name = "Un-named Texture");
		std::shared_ptr<MonoSpaceTextureAtlas> createMonoSpaceTextureAtlas(
			const std::string& filePath,
			const uint32_t rowCount,
			const uint32_t columnCount
		);

		//-----Font-----
		std::shared_ptr<FontBitmap> createFontBitmap(const char* filepath, const char* imageDir);

	private:
		void createQueues(QueueFamilyIndices& queueFamilyIndices);
		void createCommandPool(QueueFamilyIndices& queueFamilyIndices);
		void createCommandBuffers();
		void createSyncObjects();
		void closeSyncObjects();
		void createAppSceneDepthResources();
		void closeAppSceneDepthResources();
		void createRenderPasses();
		void closeRenderPasses();
		void createFrameBuffers();
		void closeFrameBuffers();
		void resizeSwapChain(
			SwapChainSupportDetails& scSupport,
			VkSurfaceKHR surface,
			QueueFamilyIndices& queueFamilyIndices,
			uint32_t width,
			uint32_t height
		);
		void drawScene(uint32_t imageIndex, VkCommandBuffer cmd);
		void drawUi(uint32_t imageIndex, VkCommandBuffer cmd);
		void present(uint32_t imageIndex, VkCommandBuffer cmd);
	};
}
