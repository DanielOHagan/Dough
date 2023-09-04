#pragma once

#include "dough/files/AFileData.h"
#include "dough/Core.h"
#include "dough/rendering/textures/TextureAtlas.h"
#include "dough/rendering/animations/TextureAtlasAnimation.h"

namespace DOH {

	struct IndexedAtlasInfoFileData : public AFileData {
		IndexedAtlasInfoFileData()
		:	AFileData(),
			Name(),
			TextureFileName(),
			InnerTextures({}),
			Animations({}),
			InnerTextureCount(0),
			AnimationCount(0)
		{}

		IndexedAtlasInfoFileData(std::string& name, std::string& textureFileName)
		:	AFileData(),
			Name(name),
			TextureFileName(textureFileName),
			InnerTextureCount(0),
			AnimationCount(0)
		{}

		std::string Name;
		std::string TextureFileName;
		std::unordered_map<std::string, InnerTexture> InnerTextures;
		std::unordered_map<std::string, TextureAtlasAnimation> Animations;
		uint32_t InnerTextureCount;
		uint32_t AnimationCount;
	};
}
