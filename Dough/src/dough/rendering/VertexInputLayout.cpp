#include "dough/rendering/VertexInputLayout.h"

#include "dough/Logging.h"

#include <tracy/public/tracy/Tracy.hpp>

namespace DOH {

	AVertexInputLayout::AVertexInputLayout(const EVertexInputLayoutType vertexInputLayoutType, uint32_t stride)
	:	mVertexInputLayoutType(vertexInputLayoutType),
		mStride(stride)
	{}


	StaticVertexInputLayout::StaticVertexInputLayout(const EVertexType vertexType)
	:	AVertexInputLayout(EVertexInputLayoutType::STATIC, static_cast<uint32_t>(vertexType)),
		mVertexType(vertexType)
	{}


	std::vector<VkVertexInputAttributeDescription> StaticVertexInputLayout::asAttribDesc(uint32_t binding) const {
		return getVertexTypeAsAttribDesc(mVertexType, binding);
	}

	bool StaticVertexInputLayout::hasSameLayout(const AVertexInputLayout& other) const {
		ZoneScoped;

		if (other.getVertexInputLayoutType() == EVertexInputLayoutType::STATIC) {
			const StaticVertexInputLayout& staticVertexInputLayout = (const StaticVertexInputLayout&) other;
			return mVertexType == staticVertexInputLayout.mVertexType;
		} else if (other.getVertexInputLayoutType() == EVertexInputLayoutType::CUSTOM) {
			LOG_ERR("Currently StaticVertexInputLayouts are NOT compatible with CustomVertexInputLayouts.");
			return false;

			//const CustomVertexInputLayout& customVertexInputLayout = (const CustomVertexInputLayout&)other;
			//const size_t dataTypeCount = customVertexInputLayout.getVertexLayout().size();
			//for (uint32_t i = 0; i < dataTypeCount; i++) {
			//	if ( 'this vertex input layout as EDataType array' != customVertexInputLayout.getVertexLayout()[i]) {
			//		return false;
			//	}
			//}
		}

		return false;
	}

	CustomVertexInputLayout::CustomVertexInputLayout(const std::initializer_list<EDataType> vertexLayout)
	:	AVertexInputLayout(EVertexInputLayoutType::CUSTOM, CustomVertexInputLayout::getByteSize(vertexLayout)),
		mVertexLayout(vertexLayout),
		mComponentCount(CustomVertexInputLayout::getComponentCount(vertexLayout))
	{}

	std::vector<VkVertexInputAttributeDescription> CustomVertexInputLayout::asAttribDesc(uint32_t binding) const {
		ZoneScoped;

		std::vector<VkVertexInputAttributeDescription> attribDescs = {};
		attribDescs.reserve(mVertexLayout.size());

		uint32_t i = 0;
		uint32_t offset = 0;
		for (EDataType dataType : mVertexLayout) {
			VkVertexInputAttributeDescription attribDesc = { i, binding, DataType::getDataTypeFormat(dataType), offset };
			attribDescs.emplace_back(attribDesc);

			i++;
			offset += DataType::getDataTypeSize(dataType);
		}

		return attribDescs;
	}

	bool CustomVertexInputLayout::hasSameLayout(const AVertexInputLayout& other) const {
		ZoneScoped;

		if (other.getVertexInputLayoutType() == EVertexInputLayoutType::CUSTOM) {
			const CustomVertexInputLayout& customVertexInputLayout = (const CustomVertexInputLayout&) other;
			const size_t dataTypeCount = customVertexInputLayout.getVertexLayout().size();
			for (uint32_t i = 0; i < dataTypeCount; i++) {
				if (mVertexLayout[i] != customVertexInputLayout.getVertexLayout()[i]) {
					return false;
				}
			}
		} else if (other.getVertexInputLayoutType() == EVertexInputLayoutType::STATIC) {
			LOG_ERR("Currently CustomVertexInputLayouts are NOT compatible with StaticVertexInputLayouts.");
			return false;
		}

		return false;
	}

	const std::unique_ptr<StaticVertexInputLayout> StaticVertexInputLayout::VERTEX_2D = std::make_unique<StaticVertexInputLayout>(EVertexType::VERTEX_2D);
	const std::unique_ptr<StaticVertexInputLayout> StaticVertexInputLayout::VERTEX_3D = std::make_unique<StaticVertexInputLayout>(EVertexType::VERTEX_3D);
	const std::unique_ptr<StaticVertexInputLayout> StaticVertexInputLayout::VERTEX_3D_TEXTURED = std::make_unique<StaticVertexInputLayout>(EVertexType::VERTEX_3D_TEXTURED);
	const std::unique_ptr<StaticVertexInputLayout> StaticVertexInputLayout::VERTEX_3D_TEXTURED_INDEXED = std::make_unique<StaticVertexInputLayout>(EVertexType::VERTEX_3D_TEXTURED_INDEXED);
	const std::unique_ptr<StaticVertexInputLayout> StaticVertexInputLayout::VERTEX_3D_LIT_TEXTURED = std::make_unique<StaticVertexInputLayout>(EVertexType::VERTEX_3D_LIT_TEXTURED);

	const StaticVertexInputLayout& StaticVertexInputLayout::get(const EVertexType vertexType) {
		switch (vertexType) {
			case EVertexType::VERTEX_2D:
				return *VERTEX_2D;
			case EVertexType::VERTEX_3D:
				return *VERTEX_3D;
			case EVertexType::VERTEX_3D_TEXTURED:
				return *VERTEX_3D_TEXTURED;
			case EVertexType::VERTEX_3D_TEXTURED_INDEXED:
				return *VERTEX_3D_TEXTURED_INDEXED;
			case EVertexType::VERTEX_3D_LIT_TEXTURED:
				return *VERTEX_3D_LIT_TEXTURED;

			case EVertexType::NONE:
			default:
				LOG_ERR("Unknown vertexType or NONE given. Returning DOH_STATIC_VERTEX_INPUT_LAYOUT_VERTEX_3D");
				return *VERTEX_3D;
		}
	}
}
