#include "dough/ResourceHandler.h"

#include "dough/Utils.h"
#include "dough/Logging.h"
#include "dough/rendering/buffer/BufferElement.h"
#include "dough/readers/FntFileReader.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define TINYOBJLOADER_IMPLEMENTATION
#include <tinyobjloader/tiny_obj_loader.h>
//Required for hash function
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

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
				) >> 1) ^
					(hash<float>()(vertex.TexIndex) << 1)
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

	TextureCreationData ResourceHandler::loadTexture(const char* filepath) {
		return ResourceHandler::INSTANCE.loadTextureImpl(filepath);
	}

	std::shared_ptr<Model3dCreationData> ResourceHandler::loadObjModel(const std::string& filepath) {
		return ResourceHandler::INSTANCE.loadObjModelImpl(filepath);
	}

	std::shared_ptr<FntFileData> ResourceHandler::loadFntFile(const char* filepath) {
		return ResourceHandler::INSTANCE.loadFntFileImpl(filepath);
	}

	void ResourceHandler::freeImage(void* imageData) {
		ResourceHandler::INSTANCE.freeImageImpl(imageData);
	}

	uint32_t ResourceHandler::getNextUniqueTextureId() {
		return ResourceHandler::INSTANCE.mNextAvailableTextureId++;
	}

	std::vector<char> ResourceHandler::readFile(const std::string& filepath) {
		std::ifstream file(filepath, std::ios::ate | std::ios::binary);

		TRY(!file.is_open(), "Failed to open file.");

		size_t fileSize = (size_t) file.tellg();
		std::vector<char> buffer(fileSize);

		file.seekg(0);
		file.read(buffer.data(), fileSize);

		file.close();

		return buffer;
	}

	TextureCreationData ResourceHandler::loadTextureImpl(const char* filepath) {
		int width = -1;
		int height = -1;
		int channels = -1;
		stbi_uc* pixels = stbi_load(filepath, &width, &height, &channels, STBI_rgb_alpha);

		TRY(pixels == nullptr || width < 0 || height < 0 || channels < 0, "Failed to load image data");

		TextureCreationData textureData{};
		textureData.Width = static_cast<uint32_t>(width);
		textureData.Height = static_cast<uint32_t>(height);
		textureData.Channels = static_cast<uint32_t>(channels);
		textureData.Data = static_cast<void*>(pixels);
		return textureData;
	}

	void ResourceHandler::freeImageImpl(void* imageData) {
		stbi_image_free(imageData);
	}

	std::shared_ptr<Model3dCreationData> ResourceHandler::loadObjModelImpl(const std::string& filepath) {
		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string warn;
		std::string err;

		if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filepath.c_str())) {
			THROW("OBJ load fail: " + warn + err);
		}

		std::shared_ptr<Model3dCreationData> model3d = std::make_shared<Model3dCreationData>();

		//TODO:: This assumes Vertex3d
		model3d->BufferElements.push_back(EDataType::FLOAT3);
		model3d->BufferElements.push_back(EDataType::FLOAT4);

		std::unordered_map<Vertex3d, uint32_t> uniqueVertices = {};
		for (const tinyobj::shape_t& shape : shapes) {
			for (const tinyobj::index_t& index : shape.mesh.indices) {
				Vertex3d vertex = {};

				if (index.vertex_index >= 0) {
					vertex.Pos = {
						attrib.vertices[3 * index.vertex_index + 0],
						attrib.vertices[3 * index.vertex_index + 1],
						attrib.vertices[3 * index.vertex_index + 2]
					};

					//TEMP::Set colour based off pos
					vertex.Colour = {
						attrib.vertices[3 * index.vertex_index + 0],
						attrib.vertices[3 * index.vertex_index + 1],
						attrib.vertices[3 * index.vertex_index + 2],
						1.0f
					};
				}

				//if (index.normal_index >= 0) {
				//	vertex.Normal = {
				//		attrib.normals[3 * index.normal_index + 0],
				//		attrib.normals[3 * index.normal_index + 1],
				//		attrib.normals[3 * index.normal_index + 2]
				//	};
				//}

				if (uniqueVertices.count(vertex) == 0) {
					uniqueVertices[vertex] = static_cast<uint32_t>(model3d->Vertices.size());
					model3d->Vertices.push_back(vertex);
				}

				model3d->Indices.push_back(uniqueVertices[vertex]);
			}
		}

		model3d->VertexBufferSize = model3d->Vertices.size() * sizeof(Vertex3d);
		model3d->IndexBufferSize = model3d->Indices.size() * sizeof(uint32_t);

		return model3d;
	}

	std::shared_ptr<FntFileData> ResourceHandler::loadFntFileImpl(const char* filepath) {
		FntFileReader fntReader(filepath);
		if (!fntReader.isOpen()) {
			LOG_ERR("Failed to open file: " << filepath);
			return nullptr;
		}
		return fntReader.read();
	}

	const std::string ResourceHandler::getCurrentLineAsBuffer(const std::vector<char>& chars, const size_t startIndex) {
		const size_t lineLength = ResourceHandler::getLengthOfCurrentLine(chars, startIndex);
		return std::string(chars.begin() + startIndex, chars.begin() + startIndex + lineLength) + "\0";
	}

	const size_t ResourceHandler::getLengthTillNextTargetChar(
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
		return ResourceHandler::getLengthTillNextTargetChar(chars, '\n', currentLineStartIndex) + 1; //+1 to include '\n'
	}

	const bool ResourceHandler::isFileOfType(const char* filepath, const char* type) {
		size_t typeLength = strlen(type);
		size_t filepathLength = strlen(filepath);

		if (typeLength == 0) {
			LOG_ERR("isFileOfType typeLength is 0");
			return false;
		} else if (typeLength == filepathLength) {
			LOG_ERR("isFileOfType typeLength same as filepathLength");
			return false;
		}

		for (size_t i = 0; i < typeLength; i++) {
			if (type[i] != filepath[(filepathLength - typeLength) + i]) {
				return false;
			}
		}

		return true;
	}
}
