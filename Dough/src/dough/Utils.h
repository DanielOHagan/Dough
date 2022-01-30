#pragma once

#include <stdexcept>
#include <fstream>

#define THROW(message) {throw std::runtime_error(message);}
#define TRY(result, message) {if (result) THROW(message);}

//TODO:: Make logging. Include error/warn/info integration, timestamp, line of code,
//	more flexible colour and text modifications, "style-blocks" that set the
//	style for a both a pre-defined block of text and "current style" that 
//	affects any none style-specified text, background colours.
#include <iostream>
#define LOG(message) { std::cout << message;}
#define LOGLN(message) { std::cout << message << std::endl; }

#define LOGLN_UNDERLINED(message) { LOGLN("\033[4m" << message << "\033[0m") }

#define LOGLN_BLACK(message) { LOGLN("\033[30m" << message << "\033[0m"); }
#define LOGLN_RED(message) { LOGLN("\033[31m" << message << "\033[0m"); }
#define LOGLN_GREEN(message) { LOGLN("\033[32m" << message << "\033[0m"); }
#define LOGLN_YELLOW(message) { LOGLN("\033[33m" << message << "\033[0m"); }
#define LOGLN_BLUE(message) { LOGLN("\033[34m" << message << "\033[0m"); }
#define LOGLN_MAGENTA(message) { LOGLN("\033[35m" << message << "\033[0m"); }
#define LOGLN_CYAN(message) { LOGLN("\033[36m" << message << "\033[0m"); }
#define LOGLN_WHITE(message) { LOGLN("\033[37m" << message << "\033[0m"); }
#define LOGLN_BRIGHT_BLACK(message) { LOGLN("\033[90m" << message << "\033[0m"); }
#define LOGLN_BRIGHT_RED(message) { LOGLN("\033[91m" << message << "\033[0m"); }
#define LOGLN_BRIGHT_GREEN(message) { LOGLN("\033[92m" << message << "\033[0m"); }
#define LOGLN_BRIGHT_YELLOW(message) { LOGLN("\033[93m" << message << "\033[0m"); }
#define LOGLN_BRIGHT_BLUE(message) { LOGLN("\033[94m" << message << "\033[0m"); }
#define LOGLN_BRIGHT_MAGENTA(message) { LOGLN("\033[95m" << message << "\033[0m"); }
#define LOGLN_BRIGHT_CYAN(message) { LOGLN("\033[96m" << message << "\033[0m"); }
#define LOGLN_BRIGHT_WHITE(message) { LOGLN("\033[97m" << message << "\033[0m"); }
