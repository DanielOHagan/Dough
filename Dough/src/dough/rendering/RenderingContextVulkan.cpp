#include "dough/rendering/RenderingContextVulkan.h"

#include "dough/rendering/ObjInit.h"

#include <chrono>
#include <glm/gtc/matrix_transform.hpp>

namespace DOH {

	RenderingContextVulkan::RenderingContextVulkan(VkDevice logicDevice, VkPhysicalDevice physicalDevice)
	:	mLogicDevice(logicDevice),
		mPhysicalDevice(physicalDevice),
		mGraphicsQueue(VK_NULL_HANDLE),
		mPresentQueue(VK_NULL_HANDLE),
		mDescriptorPool(VK_NULL_HANDLE),
		mCommandPool(VK_NULL_HANDLE),
		mUbo({ glm::mat4x4(1.0f) })
	{}

	void RenderingContextVulkan::init(
		SwapChainSupportDetails& scSupport,
		VkSurfaceKHR surface,
		QueueFamilyIndices& queueFamilyIndices,
		uint32_t width,
		uint32_t height
	) {
		mPhysicalDeviceProperties = std::make_unique<VkPhysicalDeviceProperties>();
		vkGetPhysicalDeviceProperties(mPhysicalDevice, mPhysicalDeviceProperties.get());

		createQueues(queueFamilyIndices);

		createCommandPool(queueFamilyIndices);

		mSwapChainCreationInfo = std::make_unique<SwapChainCreationInfo>(scSupport, surface, queueFamilyIndices);
	}

	void RenderingContextVulkan::close() {
		//Remove any app resources added after last frame was presented
		for (IGPUResourceVulkan& res : mToReleaseGpuResources) {
			res.close(mLogicDevice);
		}

		mGraphicsPipeline->close(mLogicDevice);

		vkDestroyCommandPool(mLogicDevice, mCommandPool, nullptr);
		vkDestroyDescriptorPool(mLogicDevice, mDescriptorPool, nullptr);
	}

	void RenderingContextVulkan::setupPipeline(ShaderProgramVulkan& shaderProgram) {
		std::vector<VkDescriptorType> descTypes = std::move(shaderProgram.getUniformLayout().asDescriptorTypes());
		createDescriptorPool(descTypes);

		mGraphicsPipeline->setDescriptorPool(mDescriptorPool);
		mGraphicsPipeline->uploadShaderUniforms(mLogicDevice, mPhysicalDevice);
	}

	void RenderingContextVulkan::resizeSwapChain(
		SwapChainSupportDetails& scSupport,
		VkSurfaceKHR surface,
		QueueFamilyIndices& queueFamilyIndices,
		uint32_t width,
		uint32_t height
	) {
		if (mGraphicsPipeline->getSwapChain().isResizable()) {
			vkDeviceWaitIdle(mLogicDevice);

			ShaderProgramVulkan& shaderProgram = mGraphicsPipeline->getShaderProgram();
			shaderProgram.closePipelineSpecificObjects(mLogicDevice);

			mGraphicsPipeline->close(mLogicDevice);
			
			mSwapChainCreationInfo->setWidth(width);
			mSwapChainCreationInfo->setHeight(height);
			mGraphicsPipeline = ObjInit::graphicsPipeline(*mSwapChainCreationInfo, shaderProgram);

			vkDestroyDescriptorPool(mLogicDevice, mDescriptorPool, nullptr);

			setupPipeline(shaderProgram);
		}
	}

	//Draw then Present rendered frame
	void RenderingContextVulkan::drawFrame() {
		uint32_t test_imageIndex = mGraphicsPipeline->aquireNextImageIndex(mLogicDevice);
		mGraphicsPipeline->recordDrawCommands(test_imageIndex);

		//TODO:: Check result of vkAcquireNextImageKHR here

		updateUniformBufferObject(test_imageIndex);

		mGraphicsPipeline->present(mLogicDevice, mGraphicsQueue, mPresentQueue, test_imageIndex);

		mGraphicsPipeline->clearVaoToDraw();
		
		//TODO::Make this so it is only removing resources from frames that are NO LONGER in use
		//Release GPU resources of no longer in use app data
		for (IGPUResourceVulkan& res : mToReleaseGpuResources) {
			res.close(mLogicDevice);
		}
		mToReleaseGpuResources.clear();
	}

	void RenderingContextVulkan::updateUniformBufferObject(uint32_t currentImage) {

		//NOTE:: Current the UBO only uses the camera's projection view matrix so updating here works,

		const uint32_t uboBinding = 0;
		mGraphicsPipeline->getShaderDescriptor().getBuffersFromBinding(uboBinding)[currentImage]->
			setData(mLogicDevice, &mUbo.ProjectionViewMat, sizeof(mUbo.ProjectionViewMat));
	}

	void RenderingContextVulkan::createQueues(QueueFamilyIndices& queueFamilyIndices) {
		vkGetDeviceQueue(mLogicDevice, queueFamilyIndices.graphicsFamily.value(), 0, &mGraphicsQueue);
		vkGetDeviceQueue(mLogicDevice, queueFamilyIndices.presentFamily.value(), 0, &mPresentQueue);
	}

	void RenderingContextVulkan::createCommandPool(QueueFamilyIndices& queueFamilyIndices) {
		VkCommandPoolCreateInfo cmdPoolCreateInfo = {};
		cmdPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		cmdPoolCreateInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
		cmdPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

		TRY(
			vkCreateCommandPool(mLogicDevice, &cmdPoolCreateInfo, nullptr, &mCommandPool) != VK_SUCCESS,
			"Failed to create Command Pool."
		);
	}

	void RenderingContextVulkan::createDescriptorPool(std::vector<VkDescriptorType>& descTypes) {
		uint32_t imageCount = static_cast<uint32_t>(mGraphicsPipeline->getSwapChain().getImageCount());

		std::vector<VkDescriptorPoolSize> poolSizes;
		for (VkDescriptorType descType : descTypes) {
			poolSizes.push_back({ descType, imageCount });
		}

		VkDescriptorPoolCreateInfo poolCreateInfo = {};
		poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolCreateInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		poolCreateInfo.pPoolSizes = poolSizes.data();
		poolCreateInfo.maxSets = imageCount;

		TRY(
			vkCreateDescriptorPool(mLogicDevice, &poolCreateInfo, nullptr, &mDescriptorPool) != VK_SUCCESS,
			"Failed to create descriptor pool."
		);
	}

	void RenderingContextVulkan::openPipeline(ShaderProgramVulkan& shaderProgram) {
		mGraphicsPipeline = ObjInit::graphicsPipeline(*mSwapChainCreationInfo, shaderProgram);
		setupPipeline(shaderProgram);
	}

	void RenderingContextVulkan::addVaoToDraw(VertexArrayVulkan& vao) {
		mGraphicsPipeline->addVaoToDraw(vao);
	}

	VkCommandBuffer RenderingContextVulkan::beginSingleTimeCommands() {
		VkCommandBufferAllocateInfo allocation{};
		allocation.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocation.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocation.commandPool = mCommandPool;
		allocation.commandBufferCount = 1;

		VkCommandBuffer cmdBuffer;
		vkAllocateCommandBuffers(mLogicDevice, &allocation, &cmdBuffer);

		VkCommandBufferBeginInfo begin{};
		begin.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		begin.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(cmdBuffer, &begin);

		return cmdBuffer;
	}

	void RenderingContextVulkan::endSingleTimeCommands(VkCommandBuffer cmdBuffer) {
		vkEndCommandBuffer(cmdBuffer);

		VkSubmitInfo submit{};
		submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit.commandBufferCount = 1;
		submit.pCommandBuffers = &cmdBuffer;

		vkQueueSubmit(mGraphicsQueue, 1, &submit, VK_NULL_HANDLE);
		vkQueueWaitIdle(mGraphicsQueue);

		vkFreeCommandBuffers(mLogicDevice, mCommandPool, 1, &cmdBuffer);
	}

	void RenderingContextVulkan::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
		VkCommandBuffer cmdBuffer = beginSingleTimeCommands();

		VkBufferImageCopy region{};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;
		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;
		region.imageOffset = {0, 0, 0};
		region.imageExtent = {
			width,
			height,
			1
		};

		vkCmdCopyBufferToImage(
			cmdBuffer,
			buffer,
			image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1,
			&region
		);

		endSingleTimeCommands(cmdBuffer);
	}

	VkImageView RenderingContextVulkan::createImageView(VkImage image, VkFormat format) {
		VkImageViewCreateInfo view{};
		view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		view.image = image;
		view.viewType = VK_IMAGE_VIEW_TYPE_2D;
		view.format = format;
		view.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		view.subresourceRange.baseMipLevel = 0;
		view.subresourceRange.levelCount = 1;
		view.subresourceRange.baseArrayLayer = 0;
		view.subresourceRange.layerCount = 1;

		VkImageView imageView;
		TRY(
			vkCreateImageView(mLogicDevice, &view, nullptr, &imageView) != VK_SUCCESS,
			"Failed to create image view"
		);
		return imageView;
	}

	VkSampler RenderingContextVulkan::createSampler() {
		VkSamplerCreateInfo samplerCreate{};
		samplerCreate.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerCreate.magFilter = VK_FILTER_LINEAR;
		samplerCreate.minFilter = VK_FILTER_LINEAR;
		samplerCreate.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerCreate.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerCreate.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerCreate.anisotropyEnable = VK_TRUE;
		samplerCreate.maxAnisotropy = mPhysicalDeviceProperties->limits.maxSamplerAnisotropy; //TODO:: Make this a variable instead of always max possible value device allows
		samplerCreate.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerCreate.unnormalizedCoordinates = VK_FALSE;
		samplerCreate.compareEnable = VK_FALSE;
		samplerCreate.compareOp = VK_COMPARE_OP_ALWAYS;
		samplerCreate.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerCreate.mipLodBias = 0.0f;
		samplerCreate.minLod = 0.0f;
		samplerCreate.maxLod = 0.0f;

		VkSampler sampler;
		TRY(
			vkCreateSampler(mLogicDevice, &samplerCreate, nullptr, &sampler),
			"Failed to create sampler"
		);
		return sampler;
	}

	void RenderingContextVulkan::setPhysicalDevice(VkPhysicalDevice physicalDevice) {
		mPhysicalDevice = physicalDevice;
		vkGetPhysicalDeviceProperties(mPhysicalDevice, mPhysicalDeviceProperties.get());
	}

	std::shared_ptr<GraphicsPipelineVulkan> RenderingContextVulkan::createGraphicsPipeline(SwapChainCreationInfo& swapChainCreate, ShaderProgramVulkan& shaderProgram) {
		return std::make_shared<GraphicsPipelineVulkan>(mLogicDevice, mCommandPool, swapChainCreate, shaderProgram);
	}

	std::shared_ptr<SwapChainVulkan> RenderingContextVulkan::createSwapChain(SwapChainCreationInfo& swapChainCreate) {
		return std::make_shared<SwapChainVulkan>(mLogicDevice, swapChainCreate);
	}

	std::shared_ptr<RenderPassVulkan> RenderingContextVulkan::createRenderPass(VkFormat imageFormat) {
		return std::make_shared<RenderPassVulkan>(mLogicDevice, imageFormat);
	}

	std::shared_ptr<VertexArrayVulkan> RenderingContextVulkan::createVertexArray() {
		return std::make_shared<VertexArrayVulkan>();
	}

	std::shared_ptr<VertexBufferVulkan> RenderingContextVulkan::createVertexBuffer(
		const std::initializer_list<BufferElement>& elements,
		VkDeviceSize size,
		VkBufferUsageFlags usage,
		VkMemoryPropertyFlags props
	) {
		return std::make_shared<VertexBufferVulkan>(elements, mLogicDevice, mPhysicalDevice, size, usage, props);
	}

	std::shared_ptr<VertexBufferVulkan> RenderingContextVulkan::createStagedVertexBuffer(
		const std::initializer_list<BufferElement>& elements,
		void* data,
		VkDeviceSize size,
		VkBufferUsageFlags usage,
		VkMemoryPropertyFlags props
	) {
		return std::make_shared<VertexBufferVulkan>(elements, mLogicDevice, mPhysicalDevice, mCommandPool, mGraphicsQueue, (const void*) data, size, usage, props);
	}

	std::shared_ptr<VertexBufferVulkan> RenderingContextVulkan::createStagedVertexBuffer(
		const std::initializer_list<BufferElement>& elements,
		const void* data,
		VkDeviceSize size,
		VkBufferUsageFlags usage,
		VkMemoryPropertyFlags props
	) {
		return std::make_shared<VertexBufferVulkan>(elements, mLogicDevice, mPhysicalDevice, mCommandPool, mGraphicsQueue, data, size, usage, props);
	}

	std::shared_ptr<IndexBufferVulkan> RenderingContextVulkan::createIndexBuffer(VkDeviceSize size, uint32_t count) {
		return std::make_shared<IndexBufferVulkan>(mLogicDevice, mPhysicalDevice, size, count);
	}

	std::shared_ptr<IndexBufferVulkan> RenderingContextVulkan::createStagedIndexBuffer(void* data, VkDeviceSize size, uint32_t count) {
		return std::make_shared<IndexBufferVulkan>(mLogicDevice, mPhysicalDevice, mCommandPool, mGraphicsQueue, (const void*) data, size, count);
	}

	std::shared_ptr<IndexBufferVulkan> RenderingContextVulkan::createStagedIndexBuffer(const void* data, VkDeviceSize size, uint32_t count) {
		return std::make_shared<IndexBufferVulkan>(mLogicDevice, mPhysicalDevice, mCommandPool, mGraphicsQueue, data, size, count);
	}

	std::shared_ptr<BufferVulkan> RenderingContextVulkan::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags props) {
		return std::make_shared<BufferVulkan>(mLogicDevice, mPhysicalDevice, size, usage, props);
	}

	std::shared_ptr<BufferVulkan> RenderingContextVulkan::createStagedBuffer(void* data, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags props) {
		return std::make_shared<BufferVulkan>(mLogicDevice, mPhysicalDevice, mCommandPool, mGraphicsQueue, (const void*) data, size, usage, props);
	}

	std::shared_ptr<BufferVulkan> RenderingContextVulkan::createStagedBuffer(const void* data, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags props) {
		return std::make_shared<BufferVulkan>(mLogicDevice, mPhysicalDevice, mCommandPool, mGraphicsQueue, data, size, usage, props);
	}

	std::shared_ptr<ShaderProgramVulkan> RenderingContextVulkan::createShaderProgram(std::shared_ptr<ShaderVulkan> vertShader, std::shared_ptr<ShaderVulkan> fragShader) {
		return std::make_shared<ShaderProgramVulkan>(vertShader, fragShader);
	}

	std::shared_ptr<ShaderVulkan> RenderingContextVulkan::createShader(EShaderType type, std::string& filePath) {
		return std::make_shared<ShaderVulkan>(type, filePath);
	}

	std::shared_ptr<TextureVulkan> RenderingContextVulkan::createTexture(std::string& filePath) {
		return std::make_shared<TextureVulkan>(mLogicDevice, mPhysicalDevice, mCommandPool, mGraphicsQueue, filePath);
	}
}
