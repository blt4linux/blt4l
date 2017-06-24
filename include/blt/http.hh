#include <list>
#include <mutex>
#include <thread>
#include <string>

namespace blt {

    typedef void(*HTTPCallback)(void*, std::string);
    typedef void(*HTTPProgress)(void*, long, long);

    struct HTTPItem {
        HTTPItem();
        HTTPCallback callback;
        HTTPProgress progressCallback;

        std::string url;
        std::string body;

        void* data;

        long progressCounter;
        long dataCounter;
    };

    class HTTPManager {
        public: 
            HTTPManager();
            ~HTTPManager();

            // instance

            bool locks_initd();
            void init_locks();

            void SSL_Lock(int);
            void SSL_Unlock(int);

            void launch_request(HTTPItem*);

            // static

            static HTTPManager* get_instance();
        private:
            
            // instance

            bool lockInitDone;
            std::mutex* sslLocks;
            int numLocks;
            std::list<std::thread*> threads;

            // static

            static HTTPManager* instance;
    };


}

/* vim: set ts=4 softtabstop=0 sw=4 expandtab: */

