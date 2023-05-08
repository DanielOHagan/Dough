#pragma once

#include "dough/Core.h"
#include "dough/rendering/buffer/DataType.h"
#include "dough/rendering/Config.h"

namespace DOH {

	enum class EVertexInputLayoutType {
		STATIC,
		CUSTOM
	};

	class AVertexInputLayout {
	protected:
		const EVertexInputLayoutType mVertexInputLayoutType;
		const uint32_t mStride;

		AVertexInputLayout(const EVertexInputLayoutType vertexInputLayoutType, uint32_t stride);

		AVertexInputLayout(const AVertexInputLayout& copy) = delete;
		AVertexInputLayout operator=(const AVertexInputLayout& assignment) = delete;

	public:
		inline EVertexInputLayoutType getVertexInputLayoutType() const { return mVertexInputLayoutType; }
		inline uint32_t getStride() const { return mStride; }

		virtual std::vector<VkVertexInputAttributeDescription> asAttribDesc(uint32_t binding) const = 0;
		virtual bool hasSameLayout(const AVertexInputLayout& other) const = 0;
	};

	/**
	* StaticVertexInputLayout is a wrapper for a EVertexType that exists statically in the engine.
	* The wrapper class is used for a consolidated API between statically existing VertexTypes and custom ones.
	*/
	class StaticVertexInputLayout : public AVertexInputLayout {

	//Pre-defined engine default StaticVertexInputLayouts
	private:
		static const std::unique_ptr<StaticVertexInputLayout> VERTEX_2D;
		static const std::unique_ptr<StaticVertexInputLayout> VERTEX_3D;
		static const std::unique_ptr<StaticVertexInputLayout> VERTEX_3D_TEXTURED;
		static const std::unique_ptr<StaticVertexInputLayout> VERTEX_3D_TEXTURED_INDEXED;
		static const std::unique_ptr<StaticVertexInputLayout> VERTEX_3D_LIT_TEXTURED;

	private:
		const EVertexType mVertexType;

	public:
		StaticVertexInputLayout(const EVertexType vertexType);

		virtual std::vector<VkVertexInputAttributeDescription> asAttribDesc(uint32_t binding) const override;
		virtual bool hasSameLayout(const AVertexInputLayout& other) const override;

		inline const EVertexType getVertexType() const { return mVertexType; }

		static const StaticVertexInputLayout& get(const EVertexType vertexType);
	};

	class CustomVertexInputLayout : public AVertexInputLayout {
	private:
		const std::vector<EDataType> mVertexLayout;
		const uint32_t mComponentCount;

	public:
		CustomVertexInputLayout(const std::initializer_list<EDataType> vertexLayout);

		virtual std::vector<VkVertexInputAttributeDescription> asAttribDesc(uint32_t binding) const override;
		virtual bool hasSameLayout(const AVertexInputLayout& other) const override;

		inline const std::vector<EDataType>& getVertexLayout() const { return mVertexLayout; }
		inline uint32_t getComponentCount() const { return mComponentCount; }

	private:
		constexpr static uint32_t getComponentCount(const std::initializer_list<EDataType>& vertexLayout) {
			uint32_t count = 0;
			for (EDataType dataType : vertexLayout) {
				count += DataType::getComponentCount(dataType);
			}

			return count;
		}
		constexpr static uint32_t getByteSize(const std::initializer_list<EDataType>& vertexLayout) {
			uint32_t size = 0;
			for (EDataType dataType : vertexLayout) {
				size += DataType::getDataTypeSize(dataType);
			}

			return size;
		}
	};
}
