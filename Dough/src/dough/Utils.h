#pragma once

#include <stdexcept>
#include <vector>
#include <fstream>

#define TRY(result, message) {if (result) throw std::runtime_error(message);}

static std::vector<char> readFile(const std::string& filename) {
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	TRY(!file.is_open(), "Failed to open file.");

	size_t fileSize = (size_t) file.tellg();
	std::vector<char> buffer(fileSize);

	file.seekg(0);
	file.read(buffer.data(), fileSize);

	file.close();

	return buffer;
}