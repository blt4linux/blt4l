#include <blt/event.hh>
#include <deque>
#include <mutex>

namespace blt {
    namespace event {
        using std::deque;
        using std::mutex;

        /*
         * EventItem Implementation 
         */
        EventItem::EventItem(EventFunction cbFun, void* cbData)
            : evFunc(cbFun)
            , evData(cbData)
        {
        }

        void
        EventItem::run_function()
        {
            evFunc(evData);
        }

        /*
         * EventQueue Implementation
         */

        EventQueue::EventQueue()
        {
        }

        EventQueue*
        EventQueue::get_instance()
        {
            if (!instance)
            {
                instance = new EventQueue();
            }

            return instance;
        }

        void
        EventQueue::process_events()
        {
            deque<EventItem*> queueClone;
            critLock.lock();

            if (queueImpl.size() <= 0)
            {
                critLock.unlock();
                return;
            }

            queueClone = queueImpl;
            queueImpl.clear();

            critLock.unlock();

            deque<EventItem*>::iterator it;
            for(it = queueClone.begin();
                it != queueClone.end();
                ++it)
            {
                (*it)->run_function();
                delete *it;
            }
        }

        void
        EventQueue::enqueue(EventItem* item)
        {
            critLock.lock();
            queueImpl.push_back(item);
            critLock.unlock();
        }

        void
        EventQueue::enqueue(EventFunction func, void* data)
        {
            EventItem* item = new EventItem(func, data);
            enqueue(item);
        }
    }
}
