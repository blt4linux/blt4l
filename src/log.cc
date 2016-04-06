#include <blt/log.hh>
#include <ctime>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

namespace blt {
    namespace log {

        using std::cerr;
        using std::string;
        using std::ofstream;
        using std::stringstream;
        using blt::log::MessageType;

        ofstream* ostream = NULL;

        void 
        log(string msg, MessageType mt)
        {
            std::tm* now;
            char dateString[100];
            {
                std::time_t currentTime = time(0);
                now = localtime(&currentTime);
            }

            /*
             * In BLT4WIN, the output file is opened & closed every time log() is called
             * This has the advantage of adding log rotation without doing any extra work,
             * but has the downside of being in poor taste, IMO.
             *
             * I would rather sacrifice log rotation for more efficient file IO
             */
            if (!ostream)
            {
                std::strftime(dateString, sizeof(dateString), "%Y_%m_%d", now);
                string filePath = "/mods/logs/" + string(dateString) + "_log.txt";
                ostream = new ofstream();
                ostream->open(filePath, std::ios::out | std::ios::app);
            }

            
            std::strftime(dateString, sizeof(dateString), "%I:%M:%S %p", now);

            stringstream logMessage;
            {
                logMessage << dateString << " ";

                switch (mt)
                {
                    case LOG_INFO:
                        logMessage << "Info: ";
                        break;
                    case LOG_LUA:
                        logMessage << "Lua: ";
                        break;
                    case LOG_WARN:
                        logMessage << "Warn: ";
                        break;
                    case LOG_ERROR:
                        logMessage << "Error: ";
                        break;
                    default:
                        logMessage << ": ";
                        break;
                }
            }

            (*ostream) << logMessage.str() << "\n"; // lol this language i dont even
            cerr << logMessage.str() << "\n";
        }

        void
        finalize()
        {
            if (ostream)
            {
                (*ostream) << "Closing log\n";
                ostream->flush();
                ostream->close();
                ostream = NULL;
            }
        }
    }
}
