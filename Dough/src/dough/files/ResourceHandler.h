#pragma once

#include "dough/rendering/Config.h"
#include "dough/rendering/VertexInputLayout.h"

namespace tinyobj {
	struct attrib_t;
	struct shape_t;
	struct material_t;
}

namespace DOH {

	struct FntFileData;
	struct JsonFileData;
	struct ObjFileData;
	struct IndexedAtlasInfoFileData;
	struct ApplicationInitSettings;

	struct TextureCreationData {
		void* Data;
		uint32_t Width = 0;
		uint32_t Height = 0;
		uint32_t Channels = 0;

		bool Failed; //NOTE:: Other "CreationData" or "FileData" types are stored in a shard_ptr, this allows for an equivalent to "!= nullptr" check
	};

	struct Model3dCreationData {
		Model3dCreationData(const AVertexInputLayout& vertexInputLayout)
		:	VertexInputLayout(vertexInputLayout)
		{};
		Model3dCreationData(
			const AVertexInputLayout& vertexInputLayout,
			const std::vector<float>& vertices,
			const std::vector<uint32_t>& indices
		) : VertexInputLayout(vertexInputLayout),
			Vertices(vertices),
			Indices(indices)
		{}

		std::vector<float> Vertices;
		std::vector<uint32_t> Indices;

		//IMPORTANT:: Since model file reading only supports statically defined types the VertexInputLayout is defined by EVertexType
		const AVertexInputLayout& VertexInputLayout;
	};

	class ResourceHandler {
	private:
		static ResourceHandler INSTANCE;

		ResourceHandler()
		:	mNextAvailableTextureId(0)
		{}

		uint32_t mNextAvailableTextureId;

		TextureCreationData loadTextureImpl(const char* filePath);
		void freeImageImpl(void* imageData);
		std::shared_ptr<IndexedAtlasInfoFileData> loadIndexedTextureAtlasImpl(const char* atlasInfoFilePath);
		std::shared_ptr<FntFileData> loadFntFileImpl(const char* filePath);
		std::shared_ptr<JsonFileData> loadJsonFileImpl(const char* filePath);
		bool writeJsonFileImpl(const char* filePath, std::shared_ptr<JsonFileData> fileData);
		std::shared_ptr<Model3dCreationData> loadObjModelImpl(const std::string& filePath, const AVertexInputLayout& vertexInputLayout);

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
		constexpr static uint32_t INVALID_TEXTURE_ID = UINT32_MAX;

		ResourceHandler(const ResourceHandler& copy) = delete;
		ResourceHandler operator=(const ResourceHandler& assignment) = delete;

		static ResourceHandler& get() { return INSTANCE; }

		static TextureCreationData loadTexture(const char* filePath);
		static void freeImage(void* imageData);
		static std::shared_ptr<IndexedAtlasInfoFileData> loadIndexedTextureAtlas(const char* atlasInfoFilePath);
		static std::vector<char> readFile(const std::string& filePath);
		static uint32_t getNextUniqueTextureId();
		constexpr static inline bool isValidTextureId(uint32_t id) { return id != ResourceHandler::INVALID_TEXTURE_ID; }
		//TODO:: allow for VertexType to be specified when loading
		static std::shared_ptr<Model3dCreationData> loadObjModel(const std::string& filePath);
		static std::shared_ptr<FntFileData> loadFntFile(const char* filePath);
		static std::shared_ptr<JsonFileData> loadJsonFile(const char* filePath);
		static bool writeJsonFile(const char* filePath, std::shared_ptr<JsonFileData> fileData);


		//------String parsing helpers-----
		static const std::string getCurrentLineAsBuffer(const std::vector<char>& chars, const size_t startIndex);
		static const size_t getLengthTillNextTargetChar(const std::vector<char>& chars, const char targetChar, const size_t startIndex);
		static const size_t getLengthOfCurrentLine(const std::vector<char>& chars, const size_t currentLineStartIndex);

		//-----File type helpers-----
		static const bool isFileOfType(const char* filePath, const char* type);

		//-----File helpers-----
		static bool doesFileExist(const char* filePath);

		//-----Settings loading-----
		static std::shared_ptr<ApplicationInitSettings> loadAppInitSettings(const char* fileName);
		static void wrtieAppInitSettings(const char* fileName, std::shared_ptr<ApplicationInitSettings> initSettings);

		static std::shared_ptr<Model3dCreationData> loadObjModel(const std::string& filePath, const AVertexInputLayout& vertexInputLayout);
	};
}
