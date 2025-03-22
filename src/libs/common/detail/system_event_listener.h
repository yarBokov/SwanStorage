#ifndef __COMMON_DETAIL_SYSTEM_EVENT_LISTENER_H
#define __COMMON_DETAIL_SYSTEM_EVENT_LISTENER_H

#include <vector>
#include <memory>
#include <unordered_map>
#include <functional>
#include <string>
#include <atomic>
#include <algorithm>
#include <csignal>
#include <stdexcept>

namespace common
{
    class system_event_listener
    {
        protected:
            using event_t = int;
        private:
            struct processor_base;
            using processor_vec_t = std::vector< std::unique_ptr < processor_base > >;
            using event_processor_dict_t = std::unordered_map < event_t , processor_vec_t >;
            
            struct processor_base
            {
                virtual ~processor_base() = default;
                virtual void process_execute() = 0;
            };

            template <typename ProcessType>
            struct processor final : public processor_base
            {
                using msg_tracer_t = std::function<void(const std::string&)>;

                explicit processor(ProcessType process, event_t event, msg_tracer_t msg_tracer_cb)
                    : m_process(process)
                    , m_event(event)
                    , m_msg_tracer_cb(msg_tracer_cb)
                {}

                void process_execute() override
                {
                    try
                    {
                        m_process();
                    }
                    catch(const std::exception& ex)
                    {
                        m_msg_tracer_cb("system event processor failed on event: " +  m_event + ", info: " + ex.what());
                    }
                    catch(...)
                    {
                        m_msg_tracer_cb("system event processor failed on event: " + m_event);
                    }
                }

                ProcessType m_process;
                event_t m_event{};
                msg_tracer_t m_msg_tracer_cb;
            };

            event_processor_dict_t processors;

            static inline std::atomic<system_event_listener*> atomic_this{};

            system_event_listener(const system_event_listener&) = delete;
            system_event_listener& operator=(const system_event_listener&) = delete;

            void process_signal(event_t signal_event)
            {
                const auto process_iter = processors.find(signal_event);
                if (process_iter != processors.cend())
                {
                    std::for_each(
                        std::cbegin(process_iter->second),
                        std::cend(process_iter->second),
                        [](const processor_vec_t::value_type & processor) { processor->process_execute(); }
                    );
                }
            }

            static void system_signal_processor(int signal, siginfo_t* siginfo, void* ptr)
            {
                auto* this_listener = std::atomic_load(&atomic_this);
                if(this_listener)
                    this_listener->process_signal(signal);
            }

            virtual void system_signal_error_cb(const std::string&) {}
        protected:
            system_event_listener()
            {
                if (std::atomic_exchange(&atomic_this, this))
                    throw std::runtime_error("Repeated attempt to create service! Service is already running");
            }

            void unregister_processor(event_t event)
            {
                struct sigaction action;
                action.sa_handler = SIG_DFL;
                sigemptyset(&action.sa_mask);

                if (sigaction(event, &action, NULL) == -1)
                    throw std::runtime_error("Failed to unregister system signal event, sig no: " + event);
            }
            
            void unregister_all()
            {
                for (auto& [event, _ ] : processors)
                    unregister_processor(event);
            }

            template <typename ProcessorType>
            bool register_processor(event_t event, ProcessorType processor)
            {
                struct sigaction action;
                action.sa_flags = SA_SIGINFO;
                action.sa_sigaction = system_signal_processor;

                sigemptyset(&action.sa_mask);

                if (sigaction(event, &action, NULL) == -1)
                    return false;

                processor_vec_t process_vec = processors[event];

                process_vec.push_back(std::make_unique< processor < ProcessorType > >
                    (processor, 
                    event, 
                    [this](const std::string& msg)
                    { system_signal_error_cb(msg); } ));

                return true;
            }

        public:
            virtual ~system_event_listener()
            {
                std::atomic_exchange(&atomic_this, nullptr);
                unregister_all();
            }
    };
}

#endif