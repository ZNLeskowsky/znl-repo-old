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

namespace boost {
namespace actor {

namespace detail {

/*virtual*/ void queueable_task::run()
{
}

queueable_task* task_queue::try_pop()
{
    queueable_task* task = head_;
    queueable_task* next = task->next();
    if (task == &dummy_) {
        if (!next)
            return 0;
        head_ = task = next;
        next = task->next();
    }
    if (next) {
        head_ = next;
        return task;
    }
    if (task == tail_.load()) {
        dummy_.set_next(0);
        tail_.exchange(&dummy_)->set_next(&dummy_); // atomic exchange may block, in which case push() may block too
        //assert(next == task->next()); //next = task->next();
        assert(!next); //if (next) { head_ = next; return task; }
        next = task->next();
        if (next) {
            head_ = next;
            return task;
        }
    }
    return 0;
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

#ifdef BOOST_ACTOR_HAS_SCHEDULER
void scheduled_task_runner::run()
{
    std::cerr << "scheduled-runner " << this << ": started" << std::endl << std::flush;
    for (;;) {
        queueable_task* task;
        if (0 == (task = task_queue_ref().try_pop())) {
            std::cerr << "scheduled-runner: have scheduler " << scheduler_ << std::endl << std::flush;
            assert(scheduler_);
            scheduler_->task_runner_queue().push(this);
            std::cerr << "scheduled-runner " << this << ": awaiting task ..." << std::endl << std::flush;
            while (0 == (task = task_queue_ref().try_pop())); // busy wait
            std::cerr << "scheduled-runner: got task " << task << std::endl << std::flush;
        }
        if (is_stop(task))
            break;
        task->run();
    }
    std::cerr << "scheduled-runner: stopped" << std::endl << std::flush;
}

void task_scheduler::run()
{
    std::cerr << "scheduler " << this << ": started" << std::endl << std::flush;
    for (;;) {
        queueable_task* task;
        std::cerr << "scheduler " << this << ": awaiting task ..." << std::endl << std::flush;
        while (0 == (task = task_queue_ref().try_pop())); // busy wait
        std::cerr << "scheduler: got task " << task << std::endl << std::flush;
        queueable_task* runner;
        if (is_stop(task)) {
            std::cerr << "scheduler: stop received" << std::endl << std::flush;
            while ((runner = task_runner_queue_.try_pop())) //TODO: ?
                static_cast<task_runner*>(runner)->queue_stop(task->status());
            break;
        }
        std::cerr << "scheduler " << this << ": awaiting runners ..." << std::endl << std::flush;
        while (0 == (runner = task_runner_queue_.try_pop())); // busy wait
        std::cerr << "scheduler: got runner " << runner << std::endl << std::flush;
        static_cast<task_runner*>(runner)->task_queue_ref().push(task);
    }
    std::cerr << "scheduler: stopped" << std::endl << std::flush;
}
#endif //BOOST_ACTOR_HAS_SCHEDULER

} // namespace detail

} // namespace actor
} // namespace boost
