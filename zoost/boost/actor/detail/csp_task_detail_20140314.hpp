//  Actor library detail/csp_task_detail.hpp header file
//
//  Simple CSP task implementation.
//  Copyright (c) 2013-? Zoltan Leskowsky
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_CSP_TASK_DETAIL_HPP
#define BOOST_CSP_TASK_DETAIL_HPP

#include <boost/assert.hpp>
#ifdef BOOST_NO_CXX11_DELETED_FUNCTIONS
#include <boost/noncopyable.hpp>
#endif
#include <boost/static_assert.hpp>
#include <boost/type_traits/has_trivial_assign.hpp>
#include <boost/type_traits/has_trivial_destructor.hpp>

#include <boost/atomic/atomic.hpp>
#include <boost/call_traits.hpp>
//#include <boost/lockfree/queue.hpp>
#include <boost/actor/detail/mpsc_queue_detail.hpp>
#include <boost/thread/thread.hpp>

#if defined(_MSC_VER)
#pragma warning(push)
//#pragma warning(disable: 4324) // structure was padded due to __declspec(align())
#endif

namespace boost {
namespace actor {

namespace detail {

#if 0 // moved to csp_task.hpp (non-detail)
class task_queue;
class task_runner;

class queueable_task : public queueable
{
public:
    int status() const { return status_; }
protected:
    queueable_task() : status_(0) {}
    void operator()() { run(); }
    //TODO: use compare-and-swap so that first error is kept
    void set_status(int status) { if (status) status_ = status; }
private:
    virtual void run(); // non-virtual to allow use as stop task
    friend class task_runner;
private:
    atomic<int> status_;
};

class task_queue : public mpsc_queue
{
public:
    void push(queueable_task* task) { mpsc_queue::push(task); }

    queueable_task* try_pop() { // may block
        return static_cast<queueable_task*>(mpsc_queue::try_pop());
    }

    queueable_task* wait_pop(volatile int* closed = 0) {
        return static_cast<queueable_task*>(mpsc_queue::wait_pop());
    }
};

class base_actor;

class task_runner : protected queueable_task
{
public:
    void queue_task(queueable_task& task) { task_queue_.push(&task); }
    void stop() { task_queue_.push(&stop_); }
    void stop(int status) {
        stop_.set_status(status);
        task_queue_.push(&stop_);
    }
    int status() const { return stop_.status(); }

    detail::queueable_task* get_task() {
        detail::queueable_task* task;
        while (0 == (task = task_queue_.try_pop())); // busy wait
        return is_stop(task) ? 0 : task;
    }

    void run_task(detail::queueable_task* task) const { (*task)(); }

protected:
    friend class base_actor;
    task_runner() {}
    task_queue& task_queue_ref() { return task_queue_; }
    bool is_stop(const detail::queueable_task* task) const {
        return (task == &stop_);
    }
    virtual void run();

private:
    task_runner(const task_runner& runner); //disallow

private:
    task_queue task_queue_;
    queueable_task stop_;
};
#endif // moved to csp_task.hpp (non-detail)

} // namespace detail

} // namespace actor
} // namespace boost

#if defined(_MSC_VER)
#pragma warning(pop)
#endif

#endif // BOOST_CSP_TASK_DETAIL_HPP
