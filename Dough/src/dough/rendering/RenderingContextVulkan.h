#pragma once

#include "dough/Utils.h"
#include "dough/rendering/pipeline/GraphicsPipelineVulkan.h"
#include "dough/scene/camera/ICamera.h"
#include "dough/ImGuiWrapper.h"
#include "dough/rendering/renderer2d/Renderer2dVulkan.h"
#include "dough/rendering/SwapChainVulkan.h"
#include "dough/rendering/CustomRenderState.h"
#include "dough/rendering/LineRenderer.h"

#include <queue>

namespace DOH {


	static std::array<const char*, 2> ERenderPassStrings = {
		"APP_SCENE",
		"APP_UI"
	};
	//TODO:: Make this a class instead of hardcoded enums
	enum class ERenderPass {
		//NONE,

		APP_SCENE = 0,
		APP_UI = 1
	};

	struct CurrentBindingsState {
		VkPipeline Pipeline = VK_NULL_HANDLE;
		//NOTE:: Assumes only one VertexBuffer is bound at a time
		VkBuffer VertexBuffer = VK_NULL_HANDLE;
		VkBuffer IndexBuffer = VK_NULL_HANDLE;
	};

	class RenderingContextVulkan {

	private:
		static constexpr size_t MAX_FRAMES_IN_FLIGHT = 2;
		static constexpr size_t GPU_RESOURCE_CLOSE_DELAY_FRAMES = 1;
		static constexpr size_t GPU_RESOURCE_CLOSE_FRAME_INDEX_COUNT = MAX_FRAMES_IN_FLIGHT + GPU_RESOURCE_CLOSE_DELAY_FRAMES;

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

		std::unique_ptr<CustomRenderState> mCurrentRenderState;

		std::unique_ptr<Renderer2dVulkan> mRenderer2d;
		std::unique_ptr<ImGuiWrapper> mImGuiWrapper;
		std::unique_ptr<LineRenderer> mLineRenderer;

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

		std::queue<std::shared_ptr<IGPUResourceVulkan>> mGpuResourcesToClose;
		//The number of gpu objects to close at frame: array index
		std::array<size_t, GPU_RESOURCE_CLOSE_FRAME_INDEX_COUNT> mGpuResourcesFrameCloseCount;

		size_t mCurrentFrame = 0;
		size_t mGpuResourceCloseFrame = 0;

		//Sync objects
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
		VkDescriptorPool createDescriptorPool(const std::vector<DescriptorTypeInfo>& descTypes);
		bool isReady() const;
		inline VkDevice getLogicDevice() const { return mLogicDevice; }

		//TODO:: Take in a pipeline builder object?
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
		void closePipeline(const ERenderPass renderPass, const char* name);

		inline void onResize(
			SwapChainSupportDetails& scSupport,
			VkSurfaceKHR surface,
			int width,
			int height
		) {
			resizeSwapChain(scSupport, surface, width, height);
			mImGuiWrapper->onWindowResize(width, height);
		};

		void drawFrame();
		inline void setAppSceneUniformBufferObject(ICamera& camera) { mAppSceneUbo.ProjectionViewMat = camera.getProjectionViewMatrix(); }
		inline void setAppUiProjection(glm::mat4x4& proj) { mAppUiProjection = proj; }

		inline CustomRenderState& getCurrentRenderState() const { return *mCurrentRenderState; }
		RenderPassVulkan& getRenderPass(const ERenderPass renderPass) const;

		//Add GPU resource to close queue which is flushed after GPU_OBJECT_RELEASE_FRAME_INDEX_COUNT number of frames
		inline void addGpuResourceToClose(std::shared_ptr<IGPUResourceVulkan> res) {
			mGpuResourcesToClose.emplace(res);
			mGpuResourcesFrameCloseCount[getNextGpuResourceCloseFrameIndex(mGpuResourceCloseFrame + GPU_RESOURCE_CLOSE_DELAY_FRAMES)]++;
		}
		//Immediately closes a GPU resource. Only do this when the object is NOT IN USE!
		inline void closeGpuResourceImmediately(std::shared_ptr<IGPUResourceVulkan> res) const { res->close(mLogicDevice); }
		inline void closeGpuResourceImmediately(IGPUResourceVulkan& res) const { res.close(mLogicDevice); }
		void releaseFrameGpuResources(size_t frameIndex);

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
		VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags) const;
		VkSampler createSampler();

		inline uint32_t getAppFrameBufferCount() const { return static_cast<uint32_t>(mAppSceneFrameBuffers.size() + mAppUiFrameBuffers.size()); }
		inline ImGuiWrapper& getImGuiWrapper() const { return *mImGuiWrapper; }
		inline SwapChainVulkan& getSwapChain() const { return *mSwapChain; }
		inline Renderer2dVulkan& getRenderer2d() const { return *mRenderer2d; }
		inline LineRenderer& getLineRenderer() const { return *mLineRenderer; }
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
		inline std::shared_ptr<GraphicsPipelineVulkan> createGraphicsPipeline(GraphicsPipelineInstanceInfo& instanceInfo, VkExtent2D extent) const { return std::make_shared<GraphicsPipelineVulkan>(mLogicDevice, instanceInfo, getRenderPass(instanceInfo.getRenderPass()).get(), extent); }

		//-----Context-----
		inline std::shared_ptr<SwapChainVulkan> createSwapChain(SwapChainCreationInfo& swapChainCreate) const { return std::make_shared<SwapChainVulkan>(mLogicDevice, swapChainCreate); }

		//-----VAO & Buffers-----
		inline std::shared_ptr<VertexArrayVulkan> createVertexArray() const { return std::make_shared<VertexArrayVulkan>(); }
		inline std::shared_ptr<VertexBufferVulkan> createVertexBuffer(const AVertexInputLayout& vertexInputLayout, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags props) const { return std::make_shared<VertexBufferVulkan>(vertexInputLayout, mLogicDevice, mPhysicalDevice, size, usage, props); }
		inline std::shared_ptr<VertexBufferVulkan> createStagedVertexBuffer(const AVertexInputLayout& vertexInputLayout, const void* data, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags props) const { return std::make_shared<VertexBufferVulkan>(vertexInputLayout, mLogicDevice, mPhysicalDevice, mCommandPool, mGraphicsQueue, data, size, usage, props); }
		inline std::shared_ptr<IndexBufferVulkan> createIndexBuffer(VkDeviceSize size) const { return std::make_shared<IndexBufferVulkan>(mLogicDevice, mPhysicalDevice, size); }
		inline std::shared_ptr<IndexBufferVulkan> createStagedIndexBuffer(void* data, VkDeviceSize size) const { return std::make_shared<IndexBufferVulkan>(mLogicDevice, mPhysicalDevice, mCommandPool, mGraphicsQueue, (const void*) data, size); }
		inline std::shared_ptr<IndexBufferVulkan> createStagedIndexBuffer(const void* data, VkDeviceSize size) const { return std::make_shared<IndexBufferVulkan>(mLogicDevice, mPhysicalDevice, mCommandPool, mGraphicsQueue, data, size); }
		inline std::unique_ptr<IndexBufferVulkan> createSharedStagedIndexBuffer(const void* data, VkDeviceSize size) const { return std::make_unique<IndexBufferVulkan>(mLogicDevice, mPhysicalDevice, mCommandPool, mGraphicsQueue, data, size); }
		inline std::shared_ptr<BufferVulkan> createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags props) const { return std::make_shared<BufferVulkan>(mLogicDevice, mPhysicalDevice, size, usage, props); }
		inline std::shared_ptr<BufferVulkan> createStagedBuffer(void* data, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags props) const { return std::make_shared<BufferVulkan>(mLogicDevice, mPhysicalDevice, mCommandPool, mGraphicsQueue, (const void*) data, size, usage, props); }
		inline std::shared_ptr<BufferVulkan> createStagedBuffer(const void* data, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags props) const { return std::make_shared<BufferVulkan>(mLogicDevice, mPhysicalDevice, mCommandPool, mGraphicsQueue, data, size, usage, props); }

		//-----Shader-----
		inline std::shared_ptr<ShaderProgramVulkan> createShaderProgram(std::shared_ptr<ShaderVulkan> vertShader, std::shared_ptr<ShaderVulkan> fragShader) const { return std::make_shared<ShaderProgramVulkan>(vertShader, fragShader); }
		inline std::shared_ptr<ShaderVulkan> createShader(EShaderType type, const std::string& filePath) const { return std::make_shared<ShaderVulkan>(type, filePath); }

		//-----Texture-----
		inline std::shared_ptr<TextureVulkan> createTexture(const std::string& filePath) const { return std::make_shared<TextureVulkan>(mLogicDevice, mPhysicalDevice, filePath); }
		inline std::shared_ptr<TextureVulkan> createTexture(float r, float g, float b, float a, bool colourRgbaNormalised = false, const char* name = "Un-named Texture") const { return std::make_shared<TextureVulkan>(mLogicDevice, mPhysicalDevice, r, g, b, a, colourRgbaNormalised, name); }
		inline std::shared_ptr<MonoSpaceTextureAtlas> createMonoSpaceTextureAtlas(
			const std::string& filePath,
			const uint32_t rowCount,
			const uint32_t columnCount
		) const { return std::make_shared<MonoSpaceTextureAtlas>(mLogicDevice, mPhysicalDevice, filePath, rowCount, columnCount); }

		//-----Font-----
		inline std::shared_ptr<FontBitmap> createFontBitmap(const char* filepath, const char* imageDir) const { return std::make_shared<FontBitmap>(filepath, imageDir); }

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
			uint32_t width,
			uint32_t height
		);
		void drawScene(uint32_t imageIndex, VkCommandBuffer cmd, CurrentBindingsState& currentBindings);
		void drawUi(uint32_t imageIndex, VkCommandBuffer cmd, CurrentBindingsState& currentBindings);
		void present(uint32_t imageIndex, VkCommandBuffer cmd);

		inline size_t getNextFrameIndex(size_t currentFrameIndex) const { return ((currentFrameIndex + 1) % MAX_FRAMES_IN_FLIGHT); }
		inline size_t getNextGpuResourceCloseFrameIndex(size_t currentReleaseFrameIndex) const { return ((currentReleaseFrameIndex + 1) % GPU_RESOURCE_CLOSE_FRAME_INDEX_COUNT); }
	};
}
