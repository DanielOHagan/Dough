#pragma once

#include "dough/Core.h"
#include "dough/rendering/text/FontBitmap.h"
#include "dough/rendering/textures/TextureArray.h"
#include "dough/rendering/pipeline/GraphicsPipelineVulkan.h"
#include "dough/rendering/SwapChainVulkan.h"
#include "dough/rendering/renderer2d/RenderBatchQuad.h"
#include "dough/rendering/renderables/SimpleRenderable.h"
#include "dough/rendering/text/FontBitmap.h"
#include "dough/scene/geometry/collections/TextString.h"

namespace DOH {

	class RenderingContextVulkan;
	struct AppDebugInfo;

	class TextRenderer {

	private:

		static std::unique_ptr<TextRenderer> INSTANCE;

		RenderingContextVulkan& mContext;

		static const std::string SOFT_MASK_SCENE_SHADER_PATH_VERT;
		static const std::string SOFT_MASK_SCENE_SHADER_PATH_FRAG;
		static const std::string MSDF_SCENE_SHADER_PATH_VERT;
		static const std::string MSDF_SCENE_SHADER_PATH_FRAG;

		std::unordered_map<std::string, std::shared_ptr<FontBitmap>> mFontBitmaps;
		std::unique_ptr<TextureArray> mFontBitmapPagesTextureArary;
		std::shared_ptr<IndexBufferVulkan> mQuadSharedIndexBuffer;

		//Soft Mask
		std::shared_ptr<ShaderProgramVulkan> mSoftMaskSceneShaderProgram;
		std::unique_ptr<GraphicsPipelineInstanceInfo> mSoftMaskScenePipelineInstanceInfo;
		std::shared_ptr<GraphicsPipelineVulkan> mSoftMaskScenePipeline;
		std::unique_ptr<RenderBatchQuad> mSoftMaskSceneBatch;
		std::shared_ptr<SimpleRenderable> mSoftMaskSceneRenderableBatch;

		//MSDF
		std::shared_ptr<ShaderProgramVulkan> mMsdfSceneShaderProgram;
		std::unique_ptr<GraphicsPipelineInstanceInfo> mMsdfScenePipelineInstanceInfo;
		std::shared_ptr<GraphicsPipelineVulkan> mMsdfScenePipeline;
		std::unique_ptr<RenderBatchQuad> mMsdfSceneBatch;
		std::shared_ptr<SimpleRenderable> mMsdfSceneRenderableBatch;

		//Local Debug Info
		//Includes both Scene & UI Quads
		uint32_t mDrawnQuadCount;

		void initImpl(TextureVulkan& fallbackTexture, std::shared_ptr<IndexBufferVulkan> quadSharedIndexBuffer);
		void closeImpl();
		void onSwapChainResizeImpl(SwapChainVulkan& swapChain);
		void createPipelineUniformObjectsImpl(VkDescriptorPool descPool);
		bool createFontBitmapImpl(const char* fontName, const char* filePath, const char* imageDir, ETextRenderMethod textRenderMethod);
		void addFontBitmapToTextTextureArrayImpl(const FontBitmap& fontBitmap);

		void setUniformDataImpl(uint32_t currentImage, uint32_t uboBinding, glm::mat4x4& sceneProjView, glm::mat4x4& uiProjView);
		void drawSceneImpl(const uint32_t imageIndex, VkCommandBuffer cmd, CurrentBindingsState& currentBindings, AppDebugInfo& debugInfo);
		void drawUiImpl(const uint32_t imageIndex, VkCommandBuffer cmd, CurrentBindingsState& currentBindings, AppDebugInfo& debugInfo);

		void drawTextFromQuadsImpl(const std::vector<Quad>& quadArr, const FontBitmap& bitmap);
		void drawTextSameTextureFromQuadsImpl(const std::vector<Quad>& quadArr, const FontBitmap& bitmap);
		//TODO:: drawTextFromQuads Scene & UI
		//TODO:: drawTextSameTextureFromQuads Scene & UI
		void drawTextStringImpl(TextString& string);
		//TODO:: drawTextString Scene & UI

	public:

		static constexpr const char* ARIAL_SOFT_MASK_NAME = "Arial-SoftMask";
		static constexpr const char* ARIAL_MSDF_NAME = "Arial-MSDF";

		TextRenderer(RenderingContextVulkan& context);

		static void init(RenderingContextVulkan& context, TextureVulkan& fallbackTexture, std::shared_ptr<IndexBufferVulkan> quadSharedIndexBuffer);
		static void close();
		static void onSwapChainResize(SwapChainVulkan& swapChain);
		static void createPipelineUniformObjects(VkDescriptorPool descPool);
		//TODO:: Currently only private impl function is usable because creating font bitmaps post-init is not available.
		//static void createFontBitmap(const char* fontName, std::shared_ptr<FontBitmap> fontBitmap);
		static void addFontBitmapToTextTextureArray(const FontBitmap& fontBitmap);

		static void setUniformData(uint32_t currentImage, uint32_t uboBinding, glm::mat4x4& sceneProjView, glm::mat4x4& uiProjView);
		static void drawScene(const uint32_t imageIndex, VkCommandBuffer cmd, CurrentBindingsState& currentBindings, AppDebugInfo& debugInfo);
		static void drawUi(const uint32_t imageIndex, VkCommandBuffer cmd, CurrentBindingsState& currentBindings, AppDebugInfo& debugInfo);

		static uint32_t getDrawnQuadCount();
		static void resetLocalDebugInfo();

		static bool hasFont(const char* fontName);
		static FontBitmap& getFontBitmap(const char* fontName);
		static std::vector<DescriptorTypeInfo> getDescriptorTypeInfos();


		//-----Primitives-----
		static void drawTextFromQuads(const std::vector<Quad>& quadArr, const FontBitmap& bitmap);
		static void drawTextSameTextureFromQuads(const std::vector<Quad>& quadArr, const FontBitmap& bitmap);
		//TODO:: drawTextFromQuads Scene & UI
		//TODO:: drawTextSameTextureFromQuads Scene & UI

		//-----Collection Objects-----
		static void drawTextString(TextString& string);
		//TODO:: drawTextString Scene & UI

	};
}
