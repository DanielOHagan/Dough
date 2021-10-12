#pragma once

#include <stdexcept>
#include <vector>
#include <fstream>

#define THROW(message) {throw std::runtime_error(message);}
#define TRY(result, message) {if (result) THROW(message);}
