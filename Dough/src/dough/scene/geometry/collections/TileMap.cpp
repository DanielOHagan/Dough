#include "dough/scene/geometry/collections/TileMap.h"

#include "dough/Logging.h"

namespace DOH {

	TileMap::TileMap(IndexedTextureAtlas& textureAtlas, uint32_t tileCountX, uint32_t tileCountY)
	:	mTextureAtlas(textureAtlas),
		mTileCountX(std::min(tileCountX, MAX_TILE_COUNT_XY)),
		mTileCountY(std::min(tileCountY, MAX_TILE_COUNT_XY))
	{
		if (tileCountX > MAX_TILE_COUNT_XY) {
			LOG_WARN("tileCountX given: " << tileCountX << " exceeds max: " << MAX_TILE_COUNT_XY);
		} else if (tileCountX == 0) {
			LOG_WARN("tileCountX given was 0, mTileCountX set to 1");
			mTileCountX = 1;
		}
		if (tileCountY > MAX_TILE_COUNT_XY) {
			LOG_WARN("tileCountY given: " << tileCountY << " exceeds max: " << MAX_TILE_COUNT_XY);
		} else if (tileCountY == 0) {
			LOG_WARN("tileCountY given was 0, mTileCountY set to 1");
			mTileCountX = 1;
		}

		mTileTextureIndexes.reserve(mTileCountX * mTileCountY);
	}

	TileMap::TileMap(IndexedTextureAtlas& textureAtlas, uint32_t tileCountX, uint32_t tileCountY, const std::vector<uint32_t>& tileTextureIndexes)
	:	mTextureAtlas(textureAtlas),
		mTileTextureIndexes(tileTextureIndexes),
		mTileCountX(std::min(tileCountX, MAX_TILE_COUNT_XY)),
		mTileCountY(std::min(tileCountY, MAX_TILE_COUNT_XY))
	{
		if (tileCountX > MAX_TILE_COUNT_XY) {
			LOG_WARN("tileCountX given: " << tileCountX << " exceeds max: " << MAX_TILE_COUNT_XY);
		} else if (tileCountX == 0) {
			LOG_WARN("tileCountX given was 0, mTileCountX set to 1");
			mTileCountX = 1;
		}
		if (tileCountY > MAX_TILE_COUNT_XY) {
			LOG_WARN("tileCountY given: " << tileCountY << " exceeds max: " << MAX_TILE_COUNT_XY);
		} else if (tileCountY == 0) {
			LOG_WARN("tileCountY given was 0, mTileCountY set to 1");
			mTileCountX = 1;
		}

		//Check given tileIds size matches tileCounts and is within count limits.
		const uint32_t tileTextureIndexesSize = static_cast<uint32_t>(tileTextureIndexes.size());
		const uint32_t tileCountXY = mTileCountX * mTileCountY;
		
		if (tileTextureIndexesSize != tileCountXY) {
			LOG_ERR("Given textureIndexesSize: " << tileTextureIndexesSize << " does NOT match given tileCounts: " << tileCountXY);
			
			//NOTE:: Since this tile map is designed with variable x/y counts in mind, treat that as the higher priority and make the textureIndexes fit the tileCounts.
			//	The problem with this is that it will almost definitely result in tiles being dispalyed in the wrong order.

			mTileTextureIndexes.resize(tileCountXY);
		}
	}

	void TileMap::setTileTextureIndex(uint32_t tileIndex, uint32_t textureIndex) {
		if (tileIndex < static_cast<uint32_t>(mTileTextureIndexes.size())) {
			mTileTextureIndexes[tileIndex] = textureIndex;
		} else {
			LOG_WARN("TileMap setTileId does not contain tileIndex: " << tileIndex);
		}
	}
}
