#include <string>

namespace blt {
    namespace log {
        enum MessageType {
            LOG_INFO = 1,
            LOG_LUA,
            LOG_WARN,
            LOG_ERROR
        };

        void log(std::string, MessageType = LOG_INFO);

        void finalize();
    }
}
