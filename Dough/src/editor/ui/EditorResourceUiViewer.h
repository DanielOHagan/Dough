#pragma once

#include "dough/rendering/textures/TextureAtlas.h"
#include "dough/rendering/textures/TextureArray.h"

namespace DOH::EDITOR {

	//TODO:: A way of creating editable fields that don't rely on unique ImGui IDs when sharing scope with
	// other resource viewers with the same field labels.

	enum class EResourceViewerUiType {
		NONE = 0,

		TEXTURE,
		MONO_SPACE_TEXTURE_ATLAS,
		INDEXED_TEXTURE_ATLAS,

		TEXTURE_ARRAY
	};

	static const char* EResourceViewerUiTypeStrings[] = {
		"NONE",

		"TEXTURE",
		"MONO_SPACE_TEXTURE_ATLAS",
		"INDEXED_TEXTURE_ATLAS",

		"TEXTURE_ARRAY"
	};

	class AResourceViewerUi {
	private:
		const EResourceViewerUiType mResourceType;

	protected:
		AResourceViewerUi(const EResourceViewerUiType resourceType, const bool display = true)
		:	mResourceType(resourceType),
			Display(display)
		{}

	public:
		static constexpr int DEFAULT_WINDOW_PADDING_WIDTH = 5;
		static constexpr int DEFAULT_WINDOW_PADDING_HEIGHT = 5;

		bool Display;

		virtual void draw(bool openAsWindow = false) = 0;

		inline EResourceViewerUiType getResourceUiType() const { return mResourceType; }
	};

	class ResourceViewerUiTexture : public AResourceViewerUi {
	private:
		const TextureVulkan& mTexture;

	public:
		bool MatchWindowSize;
		float Scale;

		ResourceViewerUiTexture(
			const TextureVulkan& texture,
			const bool display = true,
			const bool matchWindowSize = false,
			const float scale = 1.0f
		) : AResourceViewerUi(EResourceViewerUiType::TEXTURE, display),
			mTexture(texture),
			MatchWindowSize(matchWindowSize),
			Scale(scale)
		{}

		virtual void draw(bool openAsWindow = false) override;

		inline const TextureVulkan& getTexture() const { return mTexture; }
	};

	class ResourceViewerUiMonoSpaceTextureAtlas : public AResourceViewerUi {
	private:
		const MonoSpaceTextureAtlas& mTextureAtlas;

	public:
		bool MatchWindowSize;
		float Scale;

		ResourceViewerUiMonoSpaceTextureAtlas(
			const MonoSpaceTextureAtlas& textureAtlas,
			const bool display = true,
			const bool matchWindowSize = false,
			const float scale = 1.0f
		) : AResourceViewerUi(EResourceViewerUiType::MONO_SPACE_TEXTURE_ATLAS, display),
			mTextureAtlas(textureAtlas),
			MatchWindowSize(matchWindowSize),
			Scale(scale)
		{}

		virtual void draw(bool openAsWindow = false) override;

		inline const MonoSpaceTextureAtlas& getTextureAtlas() const { return mTextureAtlas; }
	};

	class ResourceViewerUiIndexedTextureAtlas : public AResourceViewerUi {
	private:
		const IndexedTextureAtlas& mTextureAtlas;
		bool mMatchWindowSize;
		float mScale;
		float mPreviewScale;
		const char* mPreviewedInnerTexture;
		std::array<float, 4> mPreviewTexelCoords;
		std::array<float, 4> mPreviewTextureCoords;

	public:
		ResourceViewerUiIndexedTextureAtlas(
			const IndexedTextureAtlas& textureAtlas,
			const bool display = true,
			const bool matchWindowSize = false,
			const float scale = 1.0f
		) : AResourceViewerUi(EResourceViewerUiType::INDEXED_TEXTURE_ATLAS, display),
			mTextureAtlas(textureAtlas),
			mMatchWindowSize(matchWindowSize),
			mScale(scale),
			mPreviewScale(1.0f),
			mPreviewedInnerTexture(
				textureAtlas.getInnerTextures().begin() != textureAtlas.getInnerTextures().end() ? 
					textureAtlas.getInnerTextures().begin()->first.c_str() : "No Preview"
			),
			mPreviewTexelCoords(),
			mPreviewTextureCoords()
		{}

		virtual void draw(bool openAsWindow = false) override;

		inline const IndexedTextureAtlas& getTextureAtlas() const { return mTextureAtlas; }
	};

	class ResourceViewerUiTextureArray : public AResourceViewerUi {
	private:
		const char* mTitle;
		const TextureArray& mTextureArray;

		//IMPORTANT:: Texture array currently only supports Textures (i.e. NOT texture atlasses) so
		// UI is stored as ResourceViewerUiTexture
		std::vector<ResourceViewerUiTexture> mTextureSlotsUi;
		std::shared_ptr<ResourceViewerUiTexture> mFallbackTextureUi;

	public:
		ResourceViewerUiTextureArray(
			const char* title,
			const TextureArray& texArray,
			const bool display = true
		);

		virtual void draw(bool openAsWindow = false) override;

		inline const char* getTitle() const { return mTitle; }
		inline const TextureArray& getTextureArray() const { return mTextureArray; }
		inline std::vector<ResourceViewerUiTexture>& getTextureSlotsUiViewers() { return mTextureSlotsUi; }
		inline ResourceViewerUiTexture& getFallbackTextureUiViewer() { return *mFallbackTextureUi; }
	};
}
