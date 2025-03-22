#ifndef __CORE_FUTEX_H
#define __CORE_FUTEX_H

#include <mutex>
#include <condition_variable>
#include <chrono>

namespace core
{
    class futex
    {
        public:
            bool wait(int64_t timeout = 0) // if timeout == 0, then wait always
            {
                try
                {
                    std::unique_lock lock(m_mutex);
                    if (timeout == 0)
                    {
                        m_condvar.wait(lock);
                        return true;
                    }
                    
                    return m_condvar.wait_for(lock, std::chrono::milliseconds(timeout))
                        == std::cv_status::no_timeout;
                }
                catch(...)
                {}
                return false;
            }
            
            void wake()
            {
                m_condvar.notify_one();
            }

        private:
            std::mutex m_mutex;
            std::condition_variable m_condvar;
    };
}

#endif