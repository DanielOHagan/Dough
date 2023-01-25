#pragma once

#include "dough/rendering/textures/TextureAtlas.h"
#include "dough/rendering/textures/TextureArray.h"

namespace DOH::EDITOR {

	//TODO:: A way of creating editable fields that don't rely on unique ImGui IDs when sharing scope with
	// other resource viewers with the same field labels.

	class AResourceViewerUi {

	public:
		static constexpr int DEFAULT_WINDOW_PADDING_WIDTH = 5;
		static constexpr int DEFAULT_WINDOW_PADDING_HEIGHT = 5;

		bool Display;

	protected:
		AResourceViewerUi(const bool display = true)
		:	Display(display)
		{}

	public:
		/** 
		* Draw the UI for a given resource including the window.
		*/
		virtual void drawUi() = 0;
		/**
		* Draw the UI for a given resource excluding the window.
		*/
		virtual void drawUiInner() = 0;
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
		) : AResourceViewerUi(display),
			mTexture(texture),
			MatchWindowSize(matchWindowSize),
			Scale(scale)
		{}

		virtual void drawUi() override;
		virtual void drawUiInner() override;
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
		) : AResourceViewerUi(display),
			mTextureAtlas(textureAtlas),
			MatchWindowSize(matchWindowSize),
			Scale(scale)
		{}

		virtual void drawUi() override;
		virtual void drawUiInner() override;
	};

	class ResourceViewerUiTextureArray : public AResourceViewerUi {

	private:
		//TODO:: Is there a way to not require a title for construction, like how the Texture & TextureAtlas use filePath?
		const char* mTitle;
		const TextureArray& mTextureArray;

		//IMPORTANT:: Texture array currently only supports Textures (i.e. NOT texture atlasses) so
		// UI is stored as ResourceViewerUiTexture
		std::vector<ResourceViewerUiTexture> mTextureSlotsUi;
		std::unique_ptr<ResourceViewerUiTexture> mFallbackTextureUi;
		
	
	public:
		ResourceViewerUiTextureArray(
			const char* title,
			const TextureArray& texArray,
			const bool display = true
		);
	
		virtual void drawUi() override;
		virtual void drawUiInner() override;

		inline const char* getTitle() const { return mTitle; }
	};
}
