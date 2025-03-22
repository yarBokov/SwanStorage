#ifndef __CORE_IS_RUNNING_H
#define __CORE_IS_RUNNING_H

#include <functional>

namespace core
{
    using is_running = std::function<bool()>;
}

#endif