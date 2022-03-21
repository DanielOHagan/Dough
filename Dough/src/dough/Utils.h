#pragma once

#include <stdexcept>
#include <fstream>

#define VAR_NAME(var) #var

#define THROW(message) throw std::runtime_error(message)
#define TRY(result, message) {if (result) THROW(message);}
#define VK_TRY(instructionResult, message) TRY(instructionResult != VK_SUCCESS, message)
