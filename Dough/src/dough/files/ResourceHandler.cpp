#include "dough/files/ResourceHandler.h"

#include "dough/Utils.h"
#include "dough/Logging.h"
#include "dough/files/readers/FntFileReader.h"
#include "dough/files/readers/JsonFileReader.h"
#include "dough/files/readers/IndexedAtlasInfoFileReader.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
#define TINYOBJLOADER_IMPLEMENTATION
#include <tinyobjloader/tiny_obj_loader.h>
//Required for hash function
//#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include <tracy/public/tracy/Tracy.hpp>

//Hash function based off of example from https://vulkan-tutorial.com/Loading_models
namespace std {
	template<>
	struct hash<DOH::Vertex3d> {
		size_t operator()(DOH::Vertex3d const& vertex) const {
			return (
				(
					hash<glm::vec3>()(vertex.Pos) ^
					(hash<glm::vec3>()(vertex.Colour) << 1)
				)
				>> 1
			);
		}
	};
	template<>
	struct hash<DOH::Vertex3dTextured> {
		size_t operator()(DOH::Vertex3dTextured const& vertex) const {
			return ((((((
				hash<glm::vec3>()(vertex.Pos) ^
					(hash<glm::vec4>()(vertex.Colour) << 1)
				) >> 1)) ^
					(hash<glm::vec2>()(vertex.TexCoord) << 1)
				) >> 1)
			);
		}
	};
	template<>
	struct hash<DOH::Vertex3dTexturedIndexed> {
		size_t operator()(DOH::Vertex3dTexturedIndexed const& vertex) const {
			return (((((((((
				hash<glm::vec3>()(vertex.Pos) ^
					(hash<glm::vec4>()(vertex.Colour) << 1)
				) >> 1)) ^
					(hash<glm::vec2>()(vertex.TexCoord) << 1)
				) >> 1)) ^
					(hash<float>()(vertex.TexIndex) << 1)
				) >> 1)
			);
		}
	};
	template<>
	struct hash<DOH::Vertex3dLitTextured> {
		size_t operator()(DOH::Vertex3dLitTextured const& vertex) const {
			return ((((((
					hash<glm::vec3>()(vertex.Pos) ^
					(hash<glm::vec4>()(vertex.Colour) << 1)
				) >> 1)) ^
					(hash<glm::vec3>()(vertex.Normal) << 1)
				) >> 1) ^
					(hash<glm::vec2>()(vertex.TexCoord) << 1)
			);
		}
	};
}

namespace DOH {

	ResourceHandler ResourceHandler::INSTANCE = ResourceHandler();

	TextureCreationData ResourceHandler::loadTexture(const char* filePath) {
		return ResourceHandler::INSTANCE.loadTextureImpl(filePath);
	}

	std::shared_ptr<IndexedAtlasInfoFileData> ResourceHandler::loadIndexedTextureAtlas(const char* atlasInfoFilePath) {
		return INSTANCE.loadIndexedTextureAtlasImpl(atlasInfoFilePath);
	}

	std::shared_ptr<Model3dCreationData> ResourceHandler::loadObjModel(const std::string& filePath, const AVertexInputLayout& vertexInputLayout) {
		return ResourceHandler::INSTANCE.loadObjModelImpl(filePath, vertexInputLayout);
	}

	std::shared_ptr<FntFileData> ResourceHandler::loadFntFile(const char* filePath) {
		return ResourceHandler::INSTANCE.loadFntFileImpl(filePath);
	}

	std::shared_ptr<JsonFileData> ResourceHandler::loadJsonFile(const char* filePath) {
		return ResourceHandler::INSTANCE.loadJsonFileImpl(filePath);
	}

	void ResourceHandler::freeImage(void* imageData) {
		ResourceHandler::INSTANCE.freeImageImpl(imageData);
	}

	uint32_t ResourceHandler::getNextUniqueTextureId() {
		return ResourceHandler::INSTANCE.mNextAvailableTextureId++;
	}

	std::vector<char> ResourceHandler::readFile(const std::string& filePath) {
		ZoneScoped;

		std::ifstream file(filePath, std::ios::ate | std::ios::binary);

		TRY(!file.is_open(), "Failed to open file.");

		size_t fileSize = (size_t) file.tellg();
		std::vector<char> buffer(fileSize);

		file.seekg(0);
		file.read(buffer.data(), fileSize);

		file.close();

		return buffer;
	}

	TextureCreationData ResourceHandler::loadTextureImpl(const char* filePath) {
		ZoneScoped;

		int width = -1;
		int height = -1;
		int channels = -1;
		stbi_uc* pixels = stbi_load(filePath, &width, &height, &channels, STBI_rgb_alpha);

		bool failed = pixels == nullptr || width < 0 || height < 0 || channels < 0;

		TextureCreationData textureData = {};
		textureData.Width = static_cast<uint32_t>(width);
		textureData.Height = static_cast<uint32_t>(height);
		textureData.Channels = static_cast<uint32_t>(channels);
		textureData.Data = static_cast<void*>(pixels);
		textureData.Failed = failed;

		if (failed) {
			LOG_ERR("Failed to load image data: " << filePath);
		}

		return textureData;
	}

	void ResourceHandler::freeImageImpl(void* imageData) {
		ZoneScoped;

		stbi_image_free(imageData);
	}

	std::shared_ptr<IndexedAtlasInfoFileData> ResourceHandler::loadIndexedTextureAtlasImpl(const char* atlasInfoFilePath) {
		ZoneScoped;

		//Load info file
		std::shared_ptr<IndexedAtlasInfoFileData> fileData = std::make_shared<IndexedAtlasInfoFileData>();

		IndexedAtlasInfoFileReader reader(atlasInfoFilePath);
		if (!reader.isOpen()) {
			LOG_ERR("Failed to open reader of IndexedAtlasInfoFile: " << atlasInfoFilePath);
			return nullptr;
		}
		return reader.read();
	}

	std::shared_ptr<Model3dCreationData> ResourceHandler::loadObjModelImpl(const std::string& filePath, const AVertexInputLayout& vertexInputLayout) {
		ZoneScoped;

		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string warn;
		std::string err;

		if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filePath.c_str())) {
			THROW("OBJ load fail: " + warn + err);
		}

		if (vertexInputLayout.getVertexInputLayoutType() != EVertexInputLayoutType::STATIC) {
			THROW("Currently Models only support StaticVertexInputLayouts");
			return nullptr;
		}

		std::shared_ptr<Model3dCreationData> fileData = std::make_shared<Model3dCreationData>(vertexInputLayout);

		const StaticVertexInputLayout& staticVertexInputLayout = (const StaticVertexInputLayout&) vertexInputLayout;
		switch (staticVertexInputLayout.getVertexType()) {
			case EVertexType::VERTEX_3D:
			{
				const auto& verticesAndIndices = extractObjFileDataAsVertex3d(attrib, shapes, materials);
				fileData->Vertices = verticesAndIndices.first;
				fileData->Indices = verticesAndIndices.second;
				break;
			}

			case EVertexType::VERTEX_3D_TEXTURED:
			{
				const auto& verticesAndIndices = extractObjFileDataAsVertex3dTextured(attrib, shapes, materials);
				fileData->Vertices = verticesAndIndices.first;
				fileData->Indices = verticesAndIndices.second;
				break;
			}

			case EVertexType::VERTEX_3D_LIT_TEXTURED:
			{
				const auto& verticesAndIndices = extractObjFileDataAsVertex3dTextured(attrib, shapes, materials);
				fileData->Vertices = verticesAndIndices.first;
				fileData->Indices = verticesAndIndices.second;
				break;
			}

			default:
				LOG_ERR("Can't read vertex type from .obj files. VertexType: " << EVertexTypeStrings[static_cast<uint32_t>(staticVertexInputLayout.getVertexType())]);
				break;
		}

		return fileData;
	}

	std::shared_ptr<FntFileData> ResourceHandler::loadFntFileImpl(const char* filePath) {
		ZoneScoped;

		FntFileReader fntReader(filePath);
		if (!fntReader.isOpen()) {
			LOG_ERR("Failed to open file: " << filePath);
			return nullptr;
		}
		return fntReader.read();
	}

	std::shared_ptr<JsonFileData> ResourceHandler::loadJsonFileImpl(const char* filePath) {
		ZoneScoped;

		JsonFileReader jsonReader(filePath);
		if (!jsonReader.isOpen()) {
			LOG_ERR("Failed to open file: " << filePath);
			return nullptr;
		}
		return jsonReader.read();
	}

	const std::string ResourceHandler::getCurrentLineAsBuffer(const std::vector<char>& chars, const size_t startIndex) {
		//ZoneScoped;

		const size_t lineLength = ResourceHandler::getLengthOfCurrentLine(chars, startIndex);
		return std::string(chars.begin() + startIndex, chars.begin() + startIndex + lineLength) + "\0";
	}

	const size_t ResourceHandler::getLengthTillNextTargetChar(
		//ZoneScoped;

		const std::vector<char>& chars,
		const char targetChar,
		const size_t startIndex
	) {
		if (startIndex >= chars.size()) {
			LOG_WARN("getLengthTillNextTargetChar() startIndex greater than chars.size()");
			return 0;
		}

		for (size_t i = 0; i < chars.size(); i++) {
			if (chars[startIndex + i] == targetChar) {
				return i;
			}
		}

		return 0;
	}

	const size_t ResourceHandler::getLengthOfCurrentLine(const std::vector<char>& chars, const size_t currentLineStartIndex) {
		//ZoneScoped;

		return ResourceHandler::getLengthTillNextTargetChar(chars, '\n', currentLineStartIndex) + 1; //+1 to include '\n'
	}

	const bool ResourceHandler::isFileOfType(const char* filePath, const char* type) {
		ZoneScoped;

		size_t typeLength = strlen(type);
		size_t filePathLength = strlen(filePath);

		if (typeLength == 0) {
			LOG_ERR("isFileOfType typeLength is 0");
			return false;
		} else if (typeLength == filePathLength) {
			LOG_ERR("isFileOfType typeLength same as filePathLength");
			return false;
		}

		for (size_t i = 0; i < typeLength; i++) {
			if (type[i] != filePath[(filePathLength - typeLength) + i]) {
				return false;
			}
		}

		return true;
	}
	
	std::pair<std::vector<float>, std::vector<uint32_t>> ResourceHandler::extractObjFileDataAsVertex3d(
		const tinyobj::attrib_t& attrib,
		const std::vector<tinyobj::shape_t>& shapes,
		const std::vector<tinyobj::material_t>& materials
	) {
		ZoneScoped;

		std::unordered_map<Vertex3d, uint32_t> uniqueVertices = {};
		std::vector<Vertex3d> vertices;
		std::vector<uint32_t> indices;

		uint32_t uniqueVertexIndex = 0;
		for (const tinyobj::shape_t& shape : shapes) {
			for (const tinyobj::index_t& index : shape.mesh.indices) {
				Vertex3d vertex = {};

				if (index.vertex_index >= 0) {
					vertex.Pos = {
						attrib.vertices[3 * index.vertex_index + 0],
						attrib.vertices[3 * index.vertex_index + 1],
						attrib.vertices[3 * index.vertex_index + 2]
					};

					//if (does not have a texture) {
						//TEMP::Set colour based off pos
					vertex.Colour = {
						attrib.vertices[3 * index.vertex_index + 0],
						attrib.vertices[3 * index.vertex_index + 1],
						attrib.vertices[3 * index.vertex_index + 2],
						1.0f
					};
					//} else {
						//Set to white
						//vertex.Colour = { 1.0f, 1.0f, 1.0f, 1.0f };
					//}
				}

				if (uniqueVertices.count(vertex) == 0) {
					uniqueVertices[vertex] = uniqueVertexIndex;
					vertices.push_back(vertex);
					uniqueVertexIndex++;
				}

				indices.push_back(uniqueVertices[vertex]);
			}
		}

		std::vector<float> verticesAsFloats = {};
		verticesAsFloats.reserve(vertices.size() * Vertex3d::COMPONENT_COUNT);

		for (const Vertex3d& vertex : vertices) {
			verticesAsFloats.emplace_back(vertex.Pos.x);
			verticesAsFloats.emplace_back(vertex.Pos.y);
			verticesAsFloats.emplace_back(vertex.Pos.z);
			verticesAsFloats.emplace_back(vertex.Colour.r);
			verticesAsFloats.emplace_back(vertex.Colour.g);
			verticesAsFloats.emplace_back(vertex.Colour.b);
			verticesAsFloats.emplace_back(vertex.Colour.a);
		}

		return { verticesAsFloats, indices };
	}

	std::pair<std::vector<float>, std::vector<uint32_t>> ResourceHandler::extractObjFileDataAsVertex3dTextured(
		const tinyobj::attrib_t& attrib,
		const std::vector<tinyobj::shape_t>& shapes,
		const std::vector<tinyobj::material_t>& materials
	) {
		ZoneScoped;

		std::unordered_map<Vertex3dTextured, uint32_t> uniqueVertices = {};
		std::vector<Vertex3dTextured> vertices;
		std::vector<uint32_t> indices;

		uint32_t uniqueVertexIndex = 0;
		for (const tinyobj::shape_t& shape : shapes) {
			for (const tinyobj::index_t& index : shape.mesh.indices) {
				Vertex3dTextured vertex = {};

				if (index.vertex_index >= 0) {
					vertex.Pos = {
						attrib.vertices[3 * index.vertex_index + 0],
						attrib.vertices[3 * index.vertex_index + 1],
						attrib.vertices[3 * index.vertex_index + 2]
					};
					vertex.Colour = { 1.0f, 1.0f, 1.0f, 1.0f };
					vertex.TexCoord = {
						attrib.texcoords[2 * index.texcoord_index + 0],

						//IMPORTANT:: 1 - texCoord because of Vulkan's 0-top coordinate space whereas OBJ is 0-bottom
						1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
					};
				}

				if (uniqueVertices.count(vertex) == 0) {
					uniqueVertices[vertex] = uniqueVertexIndex;
					vertices.push_back(vertex);
					uniqueVertexIndex++;
				}

				indices.push_back(uniqueVertices[vertex]);
			}
		}

		std::vector<float> verticesAsFloats = {};
		verticesAsFloats.reserve(vertices.size() * Vertex3dTextured::COMPONENT_COUNT);

		for (const Vertex3dTextured& vertex : vertices) {
			verticesAsFloats.emplace_back(vertex.Pos.x);
			verticesAsFloats.emplace_back(vertex.Pos.y);
			verticesAsFloats.emplace_back(vertex.Pos.z);
			verticesAsFloats.emplace_back(vertex.Colour.r);
			verticesAsFloats.emplace_back(vertex.Colour.g);
			verticesAsFloats.emplace_back(vertex.Colour.b);
			verticesAsFloats.emplace_back(vertex.Colour.a);
			verticesAsFloats.emplace_back(vertex.TexCoord.x);
			verticesAsFloats.emplace_back(vertex.TexCoord.y);
		}

		return { verticesAsFloats, indices };
	}

	std::pair<std::vector<float>, std::vector<uint32_t>> extractObjFileDataAsVertex3dLitTextured(
		const tinyobj::attrib_t& attrib,
		const std::vector<tinyobj::shape_t>& shapes,
		const std::vector<tinyobj::material_t>& materials
	) {
		ZoneScoped;

		std::unordered_map<Vertex3dLitTextured, uint32_t> uniqueVertices = {};
		std::vector<Vertex3dLitTextured> vertices;
		std::vector<uint32_t> indices;

		uint32_t uniqueVertexIndex = 0;
		for (const tinyobj::shape_t& shape : shapes) {
			for (const tinyobj::index_t& index : shape.mesh.indices) {
				Vertex3dLitTextured vertex = {};

				if (index.vertex_index >= 0) {
					vertex.Pos = {
						attrib.vertices[3 * index.vertex_index + 0],
						attrib.vertices[3 * index.vertex_index + 1],
						attrib.vertices[3 * index.vertex_index + 2]
					};

					//if (does not have a texture) {
						//TEMP::Set colour based off pos
					vertex.Colour = {
						attrib.vertices[3 * index.vertex_index + 0],
						attrib.vertices[3 * index.vertex_index + 1],
						attrib.vertices[3 * index.vertex_index + 2],
						1.0f
					};
					//} else {
						//Set to white
						//vertex.Colour = { 1.0f, 1.0f, 1.0f, 1.0f };
					//}

					vertex.Normal = {
						attrib.vertices[3 * index.normal_index + 0],
						attrib.vertices[3 * index.normal_index + 1],
						attrib.vertices[3 * index.normal_index + 2]
					};

					vertex.TexCoord = {
						attrib.vertices[3 * index.texcoord_index + 0],
						attrib.vertices[3 * index.texcoord_index + 1]
					};
				}

				if (uniqueVertices.count(vertex) == 0) {
					uniqueVertices[vertex] = uniqueVertexIndex;
					vertices.push_back(vertex);
					uniqueVertexIndex++;
				}

				indices.push_back(uniqueVertices[vertex]);
			}
		}

		std::vector<float> verticesAsFloats = {};
		verticesAsFloats.reserve(vertices.size() * Vertex3dLitTextured::COMPONENT_COUNT);

		for (const Vertex3dLitTextured& vertex : vertices) {
			verticesAsFloats.emplace_back(vertex.Pos.x);
			verticesAsFloats.emplace_back(vertex.Pos.y);
			verticesAsFloats.emplace_back(vertex.Pos.z);
			verticesAsFloats.emplace_back(vertex.Colour.r);
			verticesAsFloats.emplace_back(vertex.Colour.g);
			verticesAsFloats.emplace_back(vertex.Colour.b);
			verticesAsFloats.emplace_back(vertex.Colour.a);
			verticesAsFloats.emplace_back(vertex.Normal.x);
			verticesAsFloats.emplace_back(vertex.Normal.y);
			verticesAsFloats.emplace_back(vertex.Normal.z);
			verticesAsFloats.emplace_back(vertex.TexCoord.x);
			verticesAsFloats.emplace_back(vertex.TexCoord.y);
		}

		return { verticesAsFloats, indices };
	}
}
