#pragma once

#include "dough/rendering/Config.h"

namespace tinyobj {
	struct attrib_t;
	struct shape_t;
	struct material_t;
}

namespace DOH {

	struct BufferElement;
	struct FntFileData;
	struct ObjFileData;

	struct TextureCreationData {
		void* Data;
		uint32_t Width = 0;
		uint32_t Height = 0;
		uint32_t Channels = 0;
	};

	struct Model3dCreationData {
		Model3dCreationData(const EVertexType vertexType)
		:	VertexType(vertexType)
		{};

		std::vector<BufferElement> BufferElements;
		std::vector<float> Vertices;
		std::vector<uint32_t> Indices;
		const EVertexType VertexType;
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
		std::shared_ptr<FntFileData> loadFntFileImpl(const char* filepath);
		std::shared_ptr<Model3dCreationData> loadObjModelImpl(const std::string& filePath, const EVertexType vertexType);

		static std::pair<std::vector<float>, std::vector<uint32_t>> extractObjFileDataAsVertex3d(
			const tinyobj::attrib_t& attrib,
			const std::vector<tinyobj::shape_t>& shapes,
			const std::vector<tinyobj::material_t>& materials
		);
		static std::pair<std::vector<float>, std::vector<uint32_t>> extractObjFileDataAsVertex3dTextured(
			const tinyobj::attrib_t& attrib,
			const std::vector<tinyobj::shape_t>& shapes,
			const std::vector<tinyobj::material_t>& materials
		);
		static std::pair<std::vector<float>, std::vector<uint32_t>> extractObjFileDataAsVertex3dLitTextured(
			const tinyobj::attrib_t& attrib,
			const std::vector<tinyobj::shape_t>& shapes,
			const std::vector<tinyobj::material_t>& materials
		);

	public:
		ResourceHandler(const ResourceHandler& copy) = delete;
		ResourceHandler operator=(const ResourceHandler& assignment) = delete;

		static ResourceHandler& get() { return INSTANCE; }

		static TextureCreationData loadTexture(const char* filepath);
		static void freeImage(void* imageData);
		static std::vector<char> readFile(const std::string& filepath);
		static uint32_t getNextUniqueTextureId();
		//TODO:: allow for VertexType to be specified when loading
		static std::shared_ptr<Model3dCreationData> loadObjModel(const std::string& filepath);
		static std::shared_ptr<FntFileData> loadFntFile(const char* filepath);


		//------String parsing helpers-----
		static const std::string getCurrentLineAsBuffer(const std::vector<char>& chars, const size_t startIndex);
		static const size_t getLengthTillNextTargetChar(const std::vector<char>& chars, const char targetChar, const size_t startIndex);
		static const size_t getLengthOfCurrentLine(const std::vector<char>& chars, const size_t currentLineStartIndex);

		//-----File type helpers-----
		static const bool isFileOfType(const char* filepath, const char* type);

		static std::shared_ptr<Model3dCreationData> loadObjModel(const std::string& filePath, const EVertexType vertexType);
	};
}
