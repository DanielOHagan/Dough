
//TODO:: More comprehensive platform checking that works not just in Logging.h
#if defined (_WIN64)
	#define DOH_PLATFORM_WINDOWS_64
#endif

//TODO:: 
//	Make DOH_LOG_XXX for short term, for namespace clashing risks
//	
//	Make logging. Include error/warn/info integration, timestamp, line of code,
//	more flexible colour and text modifications, "style-blocks" that set the
//	style for a both a pre-defined block of text and "current style" that 
//	affects any none style-specified text, background colours.
//	e.g. Logger::logLine(message, colour, decorators)
//		logBlock = Logger::startLogBlock(colour, decorators)
//		logBlock.logLine(message); logBlock.dump();
#include <iostream>
#define LOG(message) { std::cout << message;}
#define LOGLN(message) { std::cout << message << std::endl; }
#define LNLOG(message) { std::cout << std::endl << message; }
#define LOG_ENDL { std::cout << std::endl; }

#if defined (_DEBUG)

	//Add colour prefixes & suffixes to log messages on windows terminals
	#if defined (DOH_PLATFORM_WINDOWS_64)
		#define TEXT_UNDERLINE(message)			"\033[4m" << message << "\033[0m"
		#define TEXT_BLACK(message)				"\033[30m" << message << "\033[0m"
		#define TEXT_RED(message)				"\033[31m" << message << "\033[0m"
		#define TEXT_GREEN(message)				"\033[32m" << message << "\033[0m"
		#define TEXT_YELLOW(message)			"\033[33m" << message << "\033[0m"
		#define TEXT_BLUE(message)				"\033[34m" << message << "\033[0m"
		#define TEXT_MAGENTA(message)			"\033[35m" << message << "\033[0m"
		#define TEXT_CYAN(message)				"\033[36m" << message << "\033[0m"
		#define TEXT_WHITE(message)				"\033[37m" << message << "\033[0m"
		#define TEXT_BRIGHT_BLACK(message)		"\033[90m" << message << "\033[0m"
		#define TEXT_BRIGHT_RED(message)		"\033[91m" << message << "\033[0m"
		#define TEXT_BRIGHT_GREEN(message)		"\033[92m" << message << "\033[0m"
		#define TEXT_BRIGHT_YELLOW(message)		"\033[93m" << message << "\033[0m"
		#define TEXT_BRIGHT_BLUE(message)		"\033[94m" << message << "\033[0m"
		#define TEXT_BRIGHT_MAGENTA(message)	"\033[95m" << message << "\033[0m"
		#define TEXT_BRIGHT_CYAN(message)		"\033[96m" << message << "\033[0m"
		#define TEXT_BRIGHT_WHITE(message)		"\033[97m" << message << "\033[0m"
	#else 
		#define TEXT_UNDERLINE(message)			message
		#define TEXT_BLACK(message)				message
		#define TEXT_RED(message)				message
		#define TEXT_GREEN(message)				message
		#define TEXT_YELLOW(message)			message
		#define TEXT_BLUE(message)				message
		#define TEXT_MAGENTA(message)			message
		#define TEXT_CYAN(message)				message
		#define TEXT_WHITE(message)				message
		#define TEXT_BRIGHT_BLACK(message)		message
		#define TEXT_BRIGHT_RED(message)		message
		#define TEXT_BRIGHT_GREEN(message)		message
		#define TEXT_BRIGHT_YELLOW(message)		message
		#define TEXT_BRIGHT_BLUE(message)		message
		#define TEXT_BRIGHT_MAGENTA(message)	message
		#define TEXT_BRIGHT_CYAN(message)		message
		#define TEXT_BRIGHT_WHITE(message)		message
	#endif

	#define LOG_UNDERLINED(message) LOG(TEXT_UNDERLINE(message))
	#define LOGLN_UNDERLINED(message) LOGLN(TEXT_UNDERLINE(message))
	
	#define LOG_BLACK(message)			LOG(TEXT_BLACK(message))
	#define LOG_RED(message)			LOG(TEXT_RED(message))
	#define LOG_GREEN(message)			LOG(TEXT_GREEN(message))
	#define LOG_YELLOW(message)			LOG(TEXT_YELLOW(message))
	#define LOG_BLUE(message)			LOG(TEXT_BLUE(message))
	#define LOG_MAGENTA(message)		LOG(TEXT_MAGENTA(message))
	#define LOG_CYAN(message)			LOG(TEXT_CYAN(message))
	#define LOG_WHITE(message)			LOG(TEXT_WHITE(message))
	#define LOG_BRIGHT_BLACK(message)	LOG(TEXT_BRIGHT_BLACK(message))
	#define LOG_BRIGHT_RED(message)		LOG(TEXT_BRIGHT_RED(message))
	#define LOG_BRIGHT_GREEN(message)	LOG(TEXT_BRIGHT_GREEN(message))
	#define LOG_BRIGHT_YELLOW(message)	LOG(TEXT_BRIGHT_YELLOW(message))
	#define LOG_BRIGHT_BLUE(message)	LOG(TEXT_BRIGHT_BLUE(message))
	#define LOG_BRIGHT_MAGENTA(message)	LOG(TEXT_BRIGHT_MAGENTA(message))
	#define LOG_BRIGHT_CYAN(message)	LOG(TEXT_BRIGHT_CYAN(message))
	#define LOG_BRIGHT_WHITE(message)	LOG(TEXT_BRIGHT_WHITE(message))
	
	#define LOGLN_BLACK(message)			LOGLN(TEXT_BLACK(message))
	#define LOGLN_RED(message)				LOGLN(TEXT_RED(message))
	#define LOGLN_GREEN(message)			LOGLN(TEXT_GREEN(message))
	#define LOGLN_YELLOW(message)			LOGLN(TEXT_YELLOW(message))
	#define LOGLN_BLUE(message)				LOGLN(TEXT_BLUE(message))
	#define LOGLN_MAGENTA(message)			LOGLN(TEXT_MAGENTA(message))
	#define LOGLN_CYAN(message)				LOGLN(TEXT_CYAN(message))
	#define LOGLN_WHITE(message)			LOGLN(TEXT_WHITE(message))
	#define LOGLN_BRIGHT_BLACK(message)		LOGLN(TEXT_BRIGHT_BLACK(message))
	#define LOGLN_BRIGHT_RED(message)		LOGLN(TEXT_BRIGHT_RED(message))
	#define LOGLN_BRIGHT_GREEN(message)		LOGLN(TEXT_BRIGHT_GREEN(message))
	#define LOGLN_BRIGHT_YELLOW(message)	LOGLN(TEXT_BRIGHT_YELLOW(message))
	#define LOGLN_BRIGHT_BLUE(message)		LOGLN(TEXT_BRIGHT_BLUE(message))
	#define LOGLN_BRIGHT_MAGENTA(message)	LOGLN(TEXT_BRIGHT_MAGENTA(message))
	#define LOGLN_BRIGHT_CYAN(message)		LOGLN(TEXT_BRIGHT_CYAN(message))
	#define LOGLN_BRIGHT_WHITE(message)		LOGLN(TEXT_BRIGHT_WHITE(message))
#else
	#define TEXT_UNDERLINE(message)			message
	#define TEXT_BLACK(message)				message
	#define TEXT_RED(message)				message
	#define TEXT_GREEN(message)				message
	#define TEXT_YELLOW(message)			message
	#define TEXT_BLUE(message)				message
	#define TEXT_MAGENTA(message)			message
	#define TEXT_CYAN(message)				message
	#define TEXT_WHITE(message)				message
	#define TEXT_BRIGHT_BLACK(message)		message
	#define TEXT_BRIGHT_RED(message)		message
	#define TEXT_BRIGHT_GREEN(message)		message
	#define TEXT_BRIGHT_YELLOW(message)		message
	#define TEXT_BRIGHT_BLUE(message)		message
	#define TEXT_BRIGHT_MAGENTA(message)	message
	#define TEXT_BRIGHT_CYAN(message)		message
	#define TEXT_BRIGHT_WHITE(message)		message

	#define LOG_UNDERLINED(message)		LOG(message)
	#define LOGLN_UNDERLINED(message)	LOGLN(message)
	
	#define LOG_BLACK(message)			LOG(message)
	#define LOG_RED(message)			LOG(message)
	#define LOG_GREEN(message)			LOG(message)
	#define LOG_YELLOW(message)			LOG(message)
	#define LOG_BLUE(message)			LOG(message)
	#define LOG_MAGENTA(message)		LOG(message)
	#define LOG_CYAN(message)			LOG(message)
	#define LOG_WHITE(message)			LOG(message)
	#define LOG_BRIGHT_BLACK(message)	LOG(message)
	#define LOG_BRIGHT_RED(message)		LOG(message)
	#define LOG_BRIGHT_GREEN(message)	LOG(message)
	#define LOG_BRIGHT_YELLOW(message)	LOG(message)
	#define LOG_BRIGHT_BLUE(message)	LOG(message)
	#define LOG_BRIGHT_MAGENTA(message)	LOG(message)
	#define LOG_BRIGHT_CYAN(message)	LOG(message)
	#define LOG_BRIGHT_WHITE(message)	LOG(message)
	
	#define LOGLN_BLACK(message)			LOGLN(message)
	#define LOGLN_RED(message)				LOGLN(message)
	#define LOGLN_GREEN(message)			LOGLN(message)
	#define LOGLN_YELLOW(message)			LOGLN(message)
	#define LOGLN_BLUE(message)				LOGLN(message)
	#define LOGLN_MAGENTA(message)			LOGLN(message)
	#define LOGLN_CYAN(message)				LOGLN(message)
	#define LOGLN_WHITE(message)			LOGLN(message)
	#define LOGLN_BRIGHT_BLACK(message)		LOGLN(message)
	#define LOGLN_BRIGHT_RED(message)		LOGLN(message)
	#define LOGLN_BRIGHT_GREEN(message)		LOGLN(message)
	#define LOGLN_BRIGHT_YELLOW(message)	LOGLN(message)
	#define LOGLN_BRIGHT_BLUE(message)		LOGLN(message)
	#define LOGLN_BRIGHT_MAGENTA(message)	LOGLN(message)
	#define LOGLN_BRIGHT_CYAN(message)		LOGLN(message)
	#define LOGLN_BRIGHT_WHITE(message)		LOGLN(message)
#endif

#define LOG_ERR(message)	LOGLN_RED(message)
#define LOG_WARN(message)	LOGLN_BRIGHT_YELLOW(message)
#define LOG_INFO(message)	LOGLN_BRIGHT_BLACK(message)

namespace DOH {
	static void displaySampleLogText() {
		LOG_ENDL;
		LOGLN_BLACK("Black");
		LOGLN_RED("Red");
		LOGLN_GREEN("Green");
		LOGLN_YELLOW("Yellow");
		LOGLN_BLUE("Blue");
		LOGLN_MAGENTA("Magenta");
		LOGLN_CYAN("Cyan");
		LOGLN_WHITE("White");
		LOGLN_BRIGHT_BLACK("Bright Black");
		LOGLN_BRIGHT_RED("Bright Red");
		LOGLN_BRIGHT_GREEN("Bright Green");
		LOGLN_BRIGHT_YELLOW("Bright Yellow");
		LOGLN_BRIGHT_BLUE("Bright Blue");
		LOGLN_BRIGHT_MAGENTA("Bright Magenta");
		LOGLN_BRIGHT_CYAN("Bright Cyan");
		LOGLN_BRIGHT_WHITE("Bright White");
		LOGLN_UNDERLINED("underlined");
		LOG_ENDL;
		LOG_BLACK("Black");
		LOG_RED("Red");
		LOG_GREEN("Green");
		LOG_YELLOW("Yellow");
		LOG_BLUE("Blue");
		LOG_MAGENTA("Magenta");
		LOG_CYAN("Cyan");
		LOG_WHITE("White");
		LOG_BRIGHT_BLACK("Bright Black");
		LOG_BRIGHT_RED("Bright Red");
		LOG_BRIGHT_GREEN("Bright Green");
		LOG_BRIGHT_YELLOW("Bright Yellow");
		LOG_BRIGHT_BLUE("Bright Blue");
		LOG_BRIGHT_MAGENTA("Bright Magenta");
		LOG_BRIGHT_CYAN("Bright Cyan");
		LOG_BRIGHT_WHITE("Bright White");
		LOG_UNDERLINED("underlined");
		LOG_ENDL;
		LOG_ERR("This is an Error");
		LOG_WARN("This is a Warning");
		LOG_INFO("This is information");
		LOG_ENDL;
	}
}

#if defined (_DEBUG)
	#define DISPLAY_SAMPLE_LOG_TEXT DOH::displaySampleLogText();
#else
	#define DISPLAY_SAMPLE_LOG_TEXT ()
#endif
