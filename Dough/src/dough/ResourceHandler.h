#include "dough/Utils.h"

namespace DOH {

	struct TextureCreationData {
		void* data;
		int width;
		int height;
		int channels;
	};

	class ResourceHandler {
	private:
		static ResourceHandler INSTANCE;

	private:
		ResourceHandler() {};

		TextureCreationData loadTextureImpl(const char* filepath);
		void freeImageImpl(void* imageData);

	public:
		ResourceHandler(const ResourceHandler& copy) = delete;

		static ResourceHandler& get() { return INSTANCE; }

	public:
		static TextureCreationData loadTexture(const char* filepath);
		static void freeImage(void* imageData);
		static std::vector<char> readFile(const std::string& filename);
	};
}
