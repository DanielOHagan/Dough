#include "dough/ResourceHandler.h"

#include "dough/Utils.h"
#include "dough/Logging.h"
#include "dough/rendering/buffer/BufferElement.h"

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
}

namespace DOH {

	ResourceHandler ResourceHandler::INSTANCE = ResourceHandler();

	TextureCreationData ResourceHandler::loadTexture(const char* filepath) {
		return ResourceHandler::INSTANCE.loadTextureImpl(filepath);
	}

	Model3dCreationData ResourceHandler::loadObjModel(const std::string& filepath) {
		return ResourceHandler::INSTANCE.loadObjModelImpl(filepath);
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
		TextureCreationData textureData{};

		stbi_uc* pixels = stbi_load(filepath, &textureData.width, &textureData.height, &textureData.channels, STBI_rgb_alpha);

		TRY(pixels == nullptr || textureData.width < 0 || textureData.height < 0 || textureData.channels < 0, "Failed to load image data");

		textureData.data = static_cast<void*>(pixels);
		return textureData;
	}

	void ResourceHandler::freeImageImpl(void* imageData) {
		stbi_image_free(imageData);
	}

	Model3dCreationData ResourceHandler::loadObjModelImpl(const std::string& filepath) {
		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string warn;
		std::string err;

		if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filepath.c_str())) {
			THROW("OBJ load fail: " + warn + err);
		}

		Model3dCreationData model3d = {};

		//TODO:: This assumes Vertex3d
		model3d.BufferElements.push_back(EDataType::FLOAT3);
		model3d.BufferElements.push_back(EDataType::FLOAT4);

		std::unordered_map<Vertex3d, uint16_t> uniqueVertices = {};
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

				if (uniqueVertices.count(vertex) == 0) {
					uniqueVertices[vertex] = static_cast<uint16_t>(model3d.Vertices.size());
					model3d.Vertices.push_back(vertex);
				}

				model3d.Indices.push_back(uniqueVertices[vertex]);
			}
		}

		model3d.VertexBufferSize = model3d.Vertices.size() * sizeof(Vertex3d);
		model3d.IndexBufferSize = model3d.Indices.size() * sizeof(uint16_t);

		return model3d;
	}
}
