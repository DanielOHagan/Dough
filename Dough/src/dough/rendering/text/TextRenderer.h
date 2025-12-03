#pragma once

#include "dough/Core.h"
#include "dough/rendering/text/FontBitmap.h"
#include "dough/rendering/textures/TextureArray.h"
#include "dough/rendering/pipeline/GraphicsPipelineVulkan.h"
#include "dough/rendering/SwapChainVulkan.h"
#include "dough/rendering/batches/RenderBatchQuad.h"
#include "dough/rendering/renderables/SimpleRenderable.h"
#include "dough/scene/geometry/collections/TextString.h"

namespace DOH {

	class RenderingContextVulkan;
	class CameraGpuData;
	enum class EImGuiContainerType;

	class TextRenderer {

		friend class RenderingContextVulkan;

	private:
		static std::unique_ptr<TextRenderer> INSTANCE;

		RenderingContextVulkan& mContext;

		static const char* SOFT_MASK_SHADER_PATH_VERT;
		static const char* SOFT_MASK_SHADER_PATH_FRAG;
		static const char* MSDF_SHADER_PATH_VERT;
		static const char* MSDF_SHADER_PATH_FRAG;

		static const char* NAME_SHORT_HAND;
		static const char* NAME_LONG_HAND;

		static const uint32_t CAMERA_UBO_SLOT = 0u;

		std::unordered_map<std::string, std::shared_ptr<FontBitmap>> mFontBitmaps;
		std::unique_ptr<TextureArray> mFontBitmapPagesTextureArary;
		std::shared_ptr<IndexBufferVulkan> mQuadIndexBuffer;
		//Whether the index buffer used here is the same as ShapeRenderer::mQuadSharedIndexBuffer.
		bool mQuadIndexBufferShared;
		std::shared_ptr<DescriptorSetsInstanceVulkan> mFontRenderingDescSetsInstanceScene;
		std::shared_ptr<DescriptorSetsInstanceVulkan> mFontRenderingDescSetsInstanceUi;
		VkDescriptorSet mFontBitmapPagesDescSet;

		struct TextRenderingObjects {
			std::shared_ptr<ShaderProgram> SceneShaderProgram;
			std::shared_ptr<ShaderVulkan> SceneVertexShader;
			std::shared_ptr<ShaderVulkan> SceneFragmentShader;
			//Scene
			std::unique_ptr<GraphicsPipelineInstanceInfo> ScenePipelineInstanceInfo;
			std::shared_ptr<GraphicsPipelineVulkan> ScenePipeline;
			std::unique_ptr<RenderBatchQuad> SceneBatch;
			std::shared_ptr<SimpleRenderable> SceneRenderableBatch;
			//Ui
			std::unique_ptr<GraphicsPipelineInstanceInfo> UiPipelineInstanceInfo;
			std::shared_ptr<GraphicsPipelineVulkan> UiPipeline;
			std::unique_ptr<RenderBatchQuad> UiBatch;
			std::shared_ptr<SimpleRenderable> UiRenderableBatch;
		};

		std::unique_ptr<TextRenderingObjects> mSoftMaskRendering;
		std::unique_ptr<TextRenderingObjects> mMsdfRendering;

		std::shared_ptr<CameraGpuData> mSceneCameraData;
		std::shared_ptr<CameraGpuData> mUiCameraData;

		//TODO:: A better way of doing no rendering with TextRenderer would be to "unload" and "load" when needed,
		// including just removing it from the "render order" as needed.
		// 
		//When drawScene/Ui is called, if their respective camera data is null then the function is ended.
		//By default this prints a warning but by setting these values to false that warning is prevented.
		bool mWarnOnNullSceneCameraData;
		bool mWarnOnNullUiCameraData;

		//Local Debug Info
		//Includes both Scene & UI Quads
		uint32_t mDrawnQuadCount;

		void initImpl();
		void closeImpl();
		void onSwapChainResizeImpl(SwapChainVulkan& swapChain);
		bool createFontBitmapImpl(const char* fontName, const char* filePath, const char* imageDir, ETextRenderMethod textRenderMethod);
		void addFontBitmapToTextTextureArrayImpl(const FontBitmap& fontBitmap);
		void updateFontBitmapTextureArrayDescriptorSetImpl();

		void drawSceneImpl(uint32_t imageIndex, VkCommandBuffer cmd, CurrentBindingsState& currentBindings);
		void drawUiImpl(uint32_t imageIndex, VkCommandBuffer cmd, CurrentBindingsState& currentBindings);

		void drawTextFromQuadImpl(const Quad& quad, RenderBatchQuad& renderbatch);
		void drawTextFromQuadsImpl(const std::vector<Quad>& quadArr, const FontBitmap& bitmap, RenderBatchQuad& renderBatch);
		void drawTextSameTextureFromQuadsImpl(const std::vector<Quad>& quadArr, const FontBitmap& bitmap, RenderBatchQuad& renderBatch);
		void drawTextStringImpl(TextString& string, RenderBatchQuad& renderBatch);

		void setSceneCameraDataImpl(std::shared_ptr<CameraGpuData> cameraData);
		void setUiCameraDataImpl(std::shared_ptr<CameraGpuData> cameraData);

		RenderBatchQuad& getSuitableTextBatchSceneImpl(const FontBitmap& bitmap);
		RenderBatchQuad& getSuitableTextBatchUiImpl(const FontBitmap& bitmap);
		static inline RenderBatchQuad& getSuitableTextBatchScene(TextString& string) { return INSTANCE->getSuitableTextBatchSceneImpl(string.getCurrentFontBitmap()); }
		static inline RenderBatchQuad& getSuitableTextBatchUi(TextString& string) { return INSTANCE->getSuitableTextBatchUiImpl(string.getCurrentFontBitmap()); }

		void drawImGuiImpl(EImGuiContainerType type);

	public:
		static constexpr const char* ARIAL_SOFT_MASK_NAME = "Arial-SoftMask";
		static constexpr const char* ARIAL_MSDF_NAME = "Arial-MSDF";

		TextRenderer(RenderingContextVulkan& context);
		TextRenderer(const TextRenderer& copy) = delete;
		void operator=(const TextRenderer& assignment) = delete;

		static void init(RenderingContextVulkan& context);
		static void close();
		static void onSwapChainResize(SwapChainVulkan& swapChain);
		//TODO:: Currently only private impl function is usable because creating font bitmaps post-init is not available.
		//static void createFontBitmap(const char* fontName, std::shared_ptr<FontBitmap> fontBitmap);
		static void addFontBitmapToTextTextureArray(const FontBitmap& fontBitmap);
		//TEMP:: Updates mFontBitmapPagesDescSet to point to textures currently in mFontBitmapPagesTextureArary.
		//TODO:: Rework this system to allow for more textures and not rely on the app logic to call this function if more font bitmaps are added.
		static void updateFontBitmapTextureArrayDescriptorSet();

		static inline void drawScene(uint32_t imageIndex, VkCommandBuffer cmd, CurrentBindingsState& currentBindings) { INSTANCE->drawSceneImpl(imageIndex, cmd, currentBindings); }
		static inline void drawUi(uint32_t imageIndex, VkCommandBuffer cmd, CurrentBindingsState& currentBindings) { INSTANCE->drawUiImpl(imageIndex, cmd, currentBindings); }

		static inline uint32_t getDrawnQuadCount() { return INSTANCE->mDrawnQuadCount; }
		static inline void resetLocalDebugInfo() { INSTANCE->mDrawnQuadCount = 0; }

		static bool hasFont(const char* fontName);
		static FontBitmap& getFontBitmap(const char* fontName);
		static std::vector<DescriptorTypeInfo> getEngineDescriptorTypeInfos();

		//-----Primitives-----
		static inline void drawTextFromQuadScene(const Quad& quad, const FontBitmap& bitmap) { INSTANCE->drawTextFromQuadImpl(quad, INSTANCE->getSuitableTextBatchSceneImpl(bitmap)); }
		static inline void drawTextFromQuadsScene(const std::vector<Quad>& quadArr, const FontBitmap& bitmap) { INSTANCE->drawTextFromQuadsImpl(quadArr, bitmap, INSTANCE->getSuitableTextBatchSceneImpl(bitmap)); }
		static inline void drawTextSameTextureFromQuadsScene(const std::vector<Quad>& quadArr, const FontBitmap& bitmap) { INSTANCE->drawTextSameTextureFromQuadsImpl(quadArr, bitmap, INSTANCE->getSuitableTextBatchSceneImpl(bitmap)); }
		static inline void drawTextFromQuadUi(const Quad& quad, const FontBitmap& bitmap) { INSTANCE->drawTextFromQuadImpl(quad, INSTANCE->getSuitableTextBatchUiImpl(bitmap)); }
		static inline void drawTextFromQuadsUi(const std::vector<Quad>& quadArr, const FontBitmap& bitmap) { INSTANCE->drawTextFromQuadsImpl(quadArr, bitmap, INSTANCE->getSuitableTextBatchUiImpl(bitmap)); }
		static inline void drawTextSameTextureFromQuadsUi(const std::vector<Quad>& quadArr, const FontBitmap& bitmap) { INSTANCE->drawTextSameTextureFromQuadsImpl(quadArr, bitmap, INSTANCE->getSuitableTextBatchUiImpl(bitmap)); }

		//-----Collection Objects-----
		static inline void drawTextStringScene(TextString& string) { INSTANCE->drawTextStringImpl(string, INSTANCE->getSuitableTextBatchSceneImpl(string.getCurrentFontBitmap())); }
		static inline void drawTextStringUi(TextString& string) { INSTANCE->drawTextStringImpl(string, INSTANCE->getSuitableTextBatchUiImpl(string.getCurrentFontBitmap())); }

		static inline void setSceneCameraData(std::shared_ptr<CameraGpuData> cameraData) { INSTANCE->setSceneCameraDataImpl(cameraData); }
		static inline void setUiCameraData(std::shared_ptr<CameraGpuData> cameraData) { INSTANCE->setUiCameraDataImpl(cameraData); }
		static inline void setWarnOnNullSceneCameraData(bool enabled) { INSTANCE->mWarnOnNullSceneCameraData = enabled; }
		static inline void setWarnOnNullUiCameraData(bool enabled) { INSTANCE->mWarnOnNullUiCameraData = enabled; }

		static inline void drawImGui(EImGuiContainerType type) { INSTANCE->drawImGuiImpl(type); }
	};
}
