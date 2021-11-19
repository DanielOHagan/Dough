#pragma once

#include <stdexcept>
#include <fstream>

#define THROW(message) {throw std::runtime_error(message);}
#define TRY(result, message) {if (result) THROW(message);}

//TODO:: Make logging
#include <iostream>
#define LOG(message) { std::cout << message;}
#define LOGLN(message) { std::cout << message << std::endl; }