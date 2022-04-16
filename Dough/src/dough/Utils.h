#pragma once

#include <stdexcept>
#include <fstream>
#include <typeinfo>

#define VAR_NAME(var) #var
#define VAR_SAME_TYPE(var1, var2) typeid(var1) == typeid(var2)

#define THROW(message) throw std::runtime_error(message)
#define TRY(result, message) {if (result) THROW(message);}
#define VK_TRY(instructionResult, message) TRY(instructionResult != VK_SUCCESS, message)
#define VK_TRY_KHR(instructionResult, message) TRY(instructionResult != VK_SUCCESS && instructionResult != VK_SUBOPTIMAL_KHR, message)
