#pragma once

#include "dough/rendering/textures/TextureAtlas.h"

namespace DOH {

	constexpr static uint32_t MAX_TILE_COUNT_XY = 5000; //Arbitrary number

	/*
	* TileMap stores TextureAtlas inner texture indexes in a std::vector
	* wich represents a 2D Matrix with an X & Y max of MAX_TILE_COUNT_XY.
	* 
	* mTileTextureIndexes is stored in !!!!!!!!!!!TODO:: DECIDE ON THIS AND IMPLEMENT (Row/Column)!!!!!!!!!!!!! -Major order
	* 
	*/
	class TileMap {
	private:
		std::reference_wrapper<IndexedTextureAtlas> mTextureAtlas;
		std::vector<uint32_t> mTileTextureIndexes;
		uint32_t mTileCountX;
		uint32_t mTileCountY;

	public:
		TileMap(IndexedTextureAtlas& textureAtlas, uint32_t tileCountX, uint32_t tileCountY);
		TileMap(IndexedTextureAtlas& textureAtlas, uint32_t tileCountX, uint32_t tileCountY, const std::vector<uint32_t>& tileTextureIndexes);

		void setTileTextureIndex(uint32_t tileIndex, uint32_t textureIndex);

		inline uint32_t getTileTextureIndex(uint32_t tileIndex) const { return tileIndex < static_cast<uint32_t>(mTileTextureIndexes.size()) ? mTileTextureIndexes[tileIndex] : UINT32_MAX; }
		//inline uint32_t getTileTextureIndex(uint32_t x, uint32_t y) const { return getTileTextureIndex(); }
		inline uint32_t getTileCountX() const { return mTileCountX; }
		inline uint32_t getTileCountY() const { return mTileCountY; }
		inline uint32_t getTileCount() const { return mTileCountX * mTileCountY; }

		constexpr static inline bool isValidTileId(uint32_t id) { return id != UINT32_MAX; }
	};
}
