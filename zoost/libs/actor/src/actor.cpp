//  Actor library actor.cpp implementation file
//
//  Simple actor implementation.
//  Copyright (c) 2013-? Zoltan Leskowsky
//
//  Distributed under the Boost Software License, Version 1.0.
//  (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

//#include <cstddef>
//#include <boost/config.hpp>
#include <boost/actor/actor.hpp>
#include <boost/actor/csp_task.hpp>

namespace boost {
namespace actor {

namespace detail {

#if 0 // moved to csp_task.cpp
/*virtual*/ void queueable_task::run()
{
}

void task_runner::run()
{
    std::cerr << "runner " << this << ": started" << std::endl << std::flush;
    for (;;) {
        detail::queueable_task* task;
        std::cerr << "runner " << this << ": awaiting task ..." << std::endl << std::flush;
        while (0 == (task = task_queue_.try_pop())); // busy wait
        std::cerr << "runner: got task " << task << std::endl << std::flush;
        if (is_stop(task))
            break;
        task->run();
    }
    std::cerr << "runner: stopped" << std::endl << std::flush;
}
#endif // moved to csp_task.cpp

} // namespace detail

} // namespace actor
} // namespace boost
