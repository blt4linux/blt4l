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

/* vim: set ts=4 softtabstop=0 sw=4 expandtab: */

