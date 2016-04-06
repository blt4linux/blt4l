#include <blt/logging.h>
#include <stdio.h>
#include <time.h>
#include <fstream>
#include <iostream>

void Logging::Log(std::string msg, LogType mType){

	time_t currentTime = time(0);
	struct tm now = *localtime(&currentTime);

	char datestring[100];
	strftime(datestring, sizeof(datestring), "%Y-%m-%d.%X", &now);

	std::string fPath = "mods/logs/" + std::string(datestring) + "_log.txt";
	std::ofstream mFile;
	mFile.open(fPath.c_str(), std::ofstream::out | std::ofstream::app);

	strftime(datestring, sizeof(datestring), "%Y-%m-%d.%X", &now);

	mFile << datestring;
	mFile << " ";

	switch (mType){
	case LOGGING_LOG:
		mFile << "Log: ";
		break;
	case LOGGING_LUA:
		mFile << "Lua: ";
		break;
	case LOGGING_WARN:
		mFile << "WARNING: ";
		break;
	case LOGGING_ERROR:
		mFile << "FATAL ERROR: ";
		break;
	default:
		mFile << "Message: ";
		break;
	}

	mFile << msg.c_str();
	mFile << "\n";
	mFile.close();
	printf("%s", msg.c_str());
	printf("\n");
}
