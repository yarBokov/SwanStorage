#ifndef __COMMON_SYSTEMD_UNIT_BASE_H
#define __COMMON_SYSTEMD_UNIT_BASE_H

#include "detail/system_event_listener.h"

#include "libs/core/futex.h"

#include <array>

namespace common
{
    class microservice_unit : public system_event_listener
    {
        private:
            const unsigned int m_check_interval_ms{1000};

            core::futex m_futex;
            std::atomic_bool m_running{false};

            void stop_event_processor()
            {
                m_running = false;
                m_futex.wake();
            }

            template <typename ProcessorType>
            bool register_system_event_processor(event_t event, ProcessorType processor)
            {
                return system_event_listener::register_processor< ProcessorType >(event, processor);
            }

            void throw_on_event_fail(event_t event)
            {
                throw std::runtime_error("Failed to register system event, sig no: " + event);
            }

        public:
            microservice_unit()
            {
                constexpr std::array stop_signals{ SIGTERM, SIGFPE, SIGILL, SIGBUS, SIGQUIT, SIGINT, SIGTSTP };

                for (auto& signal: stop_signals)
                {
                    if (!register_system_event_processor(signal, [this]() { stop_event_processor(); } ))
                        throw_on_event_fail(signal);
                }

                if (!register_system_event_processor(SIGSEGV, [this]() { abort(); } ))
                    throw_on_event_fail(SIGSEGV);
            }

            void run()
            {
                m_running = true;

                if (initialize())
                {
                    while(m_running)
                        m_futex.wait(m_check_interval_ms);

                    finalize();
                }
            }

            bool is_running() const
            {
                return m_running;
            }

        private:
            virtual bool initialize() = 0;
            virtual void finalize() = 0;
    };
}

#endif