#include <blt/event.hh>
#include <blt/http.hh>
#include <curl/curl.h>
#include <openssl/crypto.h>
#include <cstddef>
#include <string>
#include <list>
#include <thread>
#include <mutex>

namespace blt {

    using std::string;
    using std::list;
    using std::thread;
    using std::mutex;

    static list<thread*> threadList;

    struct HTTPProgressNotification
    {
        HTTPItem*   item;
        long        progressCounter;
        long        dataCounter;
    };

    /**
     * OpenSSL Callback
     */
    static void 
    lock_callback(int mode, int type, const char* file, int line)
    {
        if (mode & CRYPTO_LOCK)
        {
            HTTPManager::get_instance()->SSL_Lock(type);
        }
        else
        {
            HTTPManager::get_instance()->SSL_Unlock(type);
        }
    }

    /**
     * CURLOPT_WRITEFUNCTION value
     */
    static size_t
    write_http_data(char* ptr, size_t len, size_t nmemb, void* data)
    {
        string newData = string(ptr, len * nmemb);
        HTTPItem* item = (HTTPItem*) data; // what
        item->body += newData;
        return len * nmemb;
    }
   
    /*
     * CURLOPT_XFERINFOFUNCTION & Related
     */

    static void
    curl_transfer_event_cb(void* data)
    {
        HTTPProgressNotification* notification = (HTTPProgressNotification*) data;
        HTTPItem* item = notification->item;
        item->progressCallback(item->data, notification->progressCounter, notification->dataCounter);
        delete notification;
    }

    /**
     * CURLOPT_XFERINFOFUNCTION implementation
     */
    static int
    curl_transfer_cb(void* data, curl_off_t dlTotal, curl_off_t dlNow,
                     curl_off_t ulTotal, curl_off_t ulNow)
    {
        HTTPItem* item = (HTTPItem*) data;

        if (!item->progressCallback
            || dlTotal == dlNow 
            || dlTotal == 0 
            || dlNow == 0 
            || item->dataCounter >= dlNow)
        {
            return 0;
        }

        item->progressCounter = dlNow;
        item->dataCounter = dlTotal;

        HTTPProgressNotification* notification = new HTTPProgressNotification();
        {
            notification->item = item;
            notification->progressCounter = dlNow;
            notification->dataCounter = dlTotal;
        }

        event::EventQueue::get_instance()->enqueue(curl_transfer_event_cb, notification);
        return 0;
    }

    /*
     * HTTP Start/Finish
     */

    /*
     * EventQueue callback for http thread completion
     * Binder method that calls the callback a transfer specified when the transfer completes
     * This is done in order to run the callback out-of-thread
     */
    static void
    http_complete_event_cb(void* data)
    {
        HTTPItem* item = (HTTPItem*) data;
        item->callback(item->data, item->body);
    }

    /**
     * HTTP transfer thread entry point
     * Initialises and configures CURL and then begins the transfer
     * Defferred callee of HTTPManager::launch_request(HTTPItem) via std::thread::thread(...)
     */
    static void
    launch_http_thread(HTTPItem* item)
    {
        CURL* curl;
        {
            curl = curl_easy_init();
            curl_easy_setopt(curl, CURLOPT_URL,             item->url.c_str());
            curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION,  1L);
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER,  0L);
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST,  0L);
            curl_easy_setopt(curl, CURLOPT_TIMEOUT,         60);

            // Write callback
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION,   write_http_data);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA,       item);

            // TXRX callback
            if (item->progressCallback)
            {
#               if LIBCURL_VERSION_NUM >= 0x072000
                    curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION,    curl_transfer_cb);
                    curl_easy_setopt(curl, CURLOPT_XFERINFODATA,        item);
#               else 
                    curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION,    curl_transfer_cb);
                    curl_easy_setopt(curl, CURLOPT_PROGRESSDATA,        item);
#               endif
                curl_easy_setopt(curl, CURLOPT_NOPROGRESS,          0);
            }

        }

        curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        event::EventQueue::get_instance()->enqueue(http_complete_event_cb, item);
    }

    /*
     * HTTPManager Implementation
     */

    /* static */ HTTPManager* HTTPManager::instance = NULL;

    /* constructor */
    HTTPManager::HTTPManager() 
    {
        numLocks = 0;
        sslLocks = NULL;
        lockInitDone = false;

        curl_global_init(CURL_GLOBAL_ALL);

        if (instance)
        {
            delete instance;
        }

        instance = this;
    }

    /* destructor */
    HTTPManager::~HTTPManager()
    {
        CRYPTO_set_locking_callback(NULL);
        curl_global_cleanup();
        delete[] sslLocks;

        // Clean threads
        {
            list<thread*>::iterator it;
            for (it = threads.begin();
                 it != threads.end();
                 ++it)
            {
                (*it)->join();
                delete *it;
            }
        }
    }


    bool
    HTTPManager::locks_initd()
    {
        return lockInitDone;
    }

    void
    HTTPManager::init_locks()
    {
        numLocks = CRYPTO_num_locks();
        sslLocks = new mutex[numLocks];
        CRYPTO_set_locking_callback(lock_callback);

        lockInitDone = true;
    }

    void
    HTTPManager::SSL_Lock(int lockId)
    {
        sslLocks[lockId].lock();
    }

    void
    HTTPManager::SSL_Unlock(int lockId)
    {
        sslLocks[lockId].unlock();
    }

    void
    HTTPManager::launch_request(HTTPItem* xfData)
    {
        threadList.push_back(new thread(launch_http_thread, xfData));
    }


    /* static */ HTTPManager*
    HTTPManager::get_instance() 
    {
        if (!instance)
        {
            instance = new HTTPManager();
        }

        return instance;
    }

    /*
     * HTTPItem Implementation
     */

    HTTPItem::HTTPItem()
    {
        callback            = NULL;
        progressCallback    = NULL;
        progressCounter     = 0;
        dataCounter         = 0;
        data                = NULL;
    }

}

/* vim: set ts=4 softtabstop=0 sw=4 expandtab: */

