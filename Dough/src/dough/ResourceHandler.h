#pragma once

#include "dough/rendering/Config.h"

namespace DOH {

	struct BufferElement;

	struct TextureCreationData {
		void* Data;
		uint32_t Width;
		uint32_t Height;
		uint32_t Channels;
	};

	struct Model3dCreationData {
		std::vector<BufferElement> BufferElements;
		std::vector<Vertex3d> Vertices;
		std::vector<uint32_t> Indices;
		size_t VertexBufferSize;
		size_t IndexBufferSize;
	};

	class ResourceHandler {
	private:
		static ResourceHandler INSTANCE;

		ResourceHandler()
		:	mNextAvailableTextureId(0)
		{}

		uint32_t mNextAvailableTextureId;

		TextureCreationData loadTextureImpl(const char* filepath);
		void freeImageImpl(void* imageData);
		Model3dCreationData loadObjModelImpl(const std::string& filepath);

	public:
		ResourceHandler(const ResourceHandler& copy) = delete;
		ResourceHandler operator=(const ResourceHandler& assignment) = delete;

		static ResourceHandler& get() { return INSTANCE; }

		static TextureCreationData loadTexture(const char* filepath);
		static void freeImage(void* imageData);
		static std::vector<char> readFile(const std::string& filepath);
		static uint32_t getNextUniqueTextureId();
		//TODO:: allow for VertexType to be specified when loading
		static Model3dCreationData loadObjModel(const std::string& filepath);
	};
}
