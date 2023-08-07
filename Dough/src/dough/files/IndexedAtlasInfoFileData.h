#pragma once

#include "dough/files/AFileData.h"
#include "dough/Core.h"
#include "dough/rendering/textures/TextureAtlas.h"

namespace DOH {

	struct IndexedAtlasInfoFileData : public AFileData {
		IndexedAtlasInfoFileData()
		:	AFileData(),
			Name(),
			TextureFileName(),
			InnerTextures({}),
			InnerTextureCount(0)
		{}

		IndexedAtlasInfoFileData(std::string& name, std::string& textureFileName)
		:	AFileData(),
			Name(name),
			TextureFileName(textureFileName),
			InnerTextureCount(0)
		{}

		std::string Name;
		std::string TextureFileName;
		std::unordered_map<std::string, InnerTexture> InnerTextures;
		uint32_t InnerTextureCount;
	};
}
