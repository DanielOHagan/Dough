#pragma once

#include "dough/Core.h"
#include "dough/rendering/text/FontBitmap.h"
#include "dough/rendering/textures/TextureArray.h"
#include "dough/rendering/pipeline/GraphicsPipelineVulkan.h"
#include "dough/rendering/SwapChainVulkan.h"
#include "dough/rendering/batches/RenderBatchQuad.h"
#include "dough/rendering/renderables/SimpleRenderable.h"
#include "dough/rendering/text/FontBitmap.h"
#include "dough/scene/geometry/collections/TextString.h"

namespace DOH {

	class RenderingContextVulkan;

	class TextRenderer {

		friend class RenderingContextVulkan;

	private:
		static std::unique_ptr<TextRenderer> INSTANCE;

		RenderingContextVulkan& mContext;

		static const char* SOFT_MASK_SHADER_PATH_VERT;
		static const char* SOFT_MASK_SHADER_PATH_FRAG;
		static const char* MSDF_SHADER_PATH_VERT;
		static const char* MSDF_SHADER_PATH_FRAG;

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

		//std::unique_ptr<TextRenderingObjects> mSoftMaskRendering;
		//std::unique_ptr<TextRenderingObjects> mMsdfRendering;
		TextRenderingObjects mSoftMaskRendering;
		TextRenderingObjects mMsdfRendering;

		//Local Debug Info
		//Includes both Scene & UI Quads
		uint32_t mDrawnQuadCount;

		void initImpl();
		void closeImpl();
		void onSwapChainResizeImpl(SwapChainVulkan& swapChain);
		bool createFontBitmapImpl(const char* fontName, const char* filePath, const char* imageDir, ETextRenderMethod textRenderMethod);
		void addFontBitmapToTextTextureArrayImpl(const FontBitmap& fontBitmap);

		void drawSceneImpl(uint32_t imageIndex, VkCommandBuffer cmd, CurrentBindingsState& currentBindings);
		void drawUiImpl(uint32_t imageIndex, VkCommandBuffer cmd, CurrentBindingsState& currentBindings);

		void drawTextFromQuadsImpl(const std::vector<Quad>& quadArr, const FontBitmap& bitmap, RenderBatchQuad& renderBatch);
		void drawTextSameTextureFromQuadsImpl(const std::vector<Quad>& quadArr, const FontBitmap& bitmap, RenderBatchQuad& renderBatch);
		void drawTextStringImpl(TextString& string, RenderBatchQuad& renderBatch);

		RenderBatchQuad& getSuitableTextBatchScene(const FontBitmap& bitmap);
		RenderBatchQuad& getSuitableTextBatchUi(const FontBitmap& bitmap);
		static inline RenderBatchQuad& getSuitableTextBatchScene(TextString& string) { return INSTANCE->getSuitableTextBatchScene(string.getCurrentFontBitmap()); }
		static inline RenderBatchQuad& getSuitableTextBatchUi(TextString& string) { return INSTANCE->getSuitableTextBatchUi(string.getCurrentFontBitmap()); }

	public:

		static constexpr const char* ARIAL_SOFT_MASK_NAME = "Arial-SoftMask";
		static constexpr const char* ARIAL_MSDF_NAME = "Arial-MSDF";

		TextRenderer(RenderingContextVulkan& context);

		static void init(RenderingContextVulkan& context);
		static void close();
		static void onSwapChainResize(SwapChainVulkan& swapChain);
		//TODO:: Currently only private impl function is usable because creating font bitmaps post-init is not available.
		//static void createFontBitmap(const char* fontName, std::shared_ptr<FontBitmap> fontBitmap);
		static void addFontBitmapToTextTextureArray(const FontBitmap& fontBitmap);

		static inline void drawScene(uint32_t imageIndex, VkCommandBuffer cmd, CurrentBindingsState& currentBindings) { INSTANCE->drawSceneImpl(imageIndex, cmd, currentBindings); }
		static inline void drawUi(uint32_t imageIndex, VkCommandBuffer cmd, CurrentBindingsState& currentBindings) { INSTANCE->drawUiImpl(imageIndex, cmd, currentBindings); }

		static inline uint32_t getDrawnQuadCount() { return INSTANCE->mDrawnQuadCount; }
		static inline void resetLocalDebugInfo() { INSTANCE->mDrawnQuadCount = 0; }

		static bool hasFont(const char* fontName);
		static FontBitmap& getFontBitmap(const char* fontName);
		static std::vector<DescriptorTypeInfo> getEngineDescriptorTypeInfos();


		//-----Primitives-----
		static inline void drawTextFromQuadsScene(const std::vector<Quad>& quadArr, const FontBitmap& bitmap) { INSTANCE->drawTextFromQuadsImpl(quadArr, bitmap, INSTANCE->getSuitableTextBatchScene(bitmap));
		}
		static inline void drawTextSameTextureFromQuadsScene(const std::vector<Quad>& quadArr, const FontBitmap& bitmap) { INSTANCE->drawTextSameTextureFromQuadsImpl(quadArr, bitmap, INSTANCE->getSuitableTextBatchScene(bitmap)); }
		static inline void drawTextFromQuadsUi(const std::vector<Quad>& quadArr, const FontBitmap& bitmap) { INSTANCE->drawTextFromQuadsImpl(quadArr, bitmap, INSTANCE->getSuitableTextBatchUi(bitmap)); }
		static inline void drawTextSameTextureFromQuadsUi(const std::vector<Quad>& quadArr, const FontBitmap& bitmap) { INSTANCE->drawTextSameTextureFromQuadsImpl(quadArr, bitmap, INSTANCE->getSuitableTextBatchUi(bitmap)); }

		//-----Collection Objects-----
		static inline void drawTextStringScene(TextString& string) { INSTANCE->drawTextStringImpl(string, INSTANCE->getSuitableTextBatchScene(string.getCurrentFontBitmap())); }
		static inline void drawTextStringUi(TextString& string) { INSTANCE->drawTextStringImpl(string, INSTANCE->getSuitableTextBatchUi(string.getCurrentFontBitmap())); }
	};
}
