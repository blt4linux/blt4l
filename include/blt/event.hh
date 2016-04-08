#pragma once 

#include <deque>
#include <mutex>

namespace blt {
    namespace event {

        typedef void (*EventFunction)(void*);

        class EventItem {
            public:
                EventItem(EventFunction, void*);

                void run_function();
            private:
                EventFunction evFunc;
                void* evData;
        };

        class EventQueue {
            public:
                EventQueue();

                void process_events();
                void enqueue(EventItem*);
                void enqueue(EventFunction, void*);

                static EventQueue* get_instance();
            private:
                static EventQueue* instance;

                std::mutex critLock;
                std::deque<EventItem*> queueImpl;
        };

    }
}
