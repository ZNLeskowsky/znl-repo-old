//  Actor library csp_task.cpp implementation file
//
//  Simple CSP task implementation.
//  Copyright (c) 2013-? Zoltan Leskowsky
//
//  Distributed under the Boost Software License, Version 1.0.
//  (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

//#include <cstddef>
//#include <boost/config.hpp>
#include <boost/actor/csp_task.hpp>

namespace boost {
namespace actor {

/*virtual*/ void base_task::run()
{
}

/*virtual*/ void task_runner::run()
{
    std::cerr << "runner " << this << ": started" << std::endl << std::flush;
    for (;;) {
        std::cerr << "runner " << this << ": awaiting task ..." << std::endl << std::flush;
        base_task* task = wait_get_task();
        if (!task)
            break;
        std::cerr << "runner: got task " << task << std::endl << std::flush;
        task->run();
    }
    std::cerr << "runner: stopped" << std::endl << std::flush;
}

} // namespace actor
} // namespace boost
