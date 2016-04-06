#ifndef LOGGING_H
#define LOGGING_H

#include <string>

namespace Logging {

	enum LogType{
		LOGGING_LOG=1,
		LOGGING_LUA,
		LOGGING_WARN,
		LOGGING_ERROR
	};

	void Log(std::string msg, LogType msgType = LOGGING_LOG);
}

#endif // LOGGING_H
