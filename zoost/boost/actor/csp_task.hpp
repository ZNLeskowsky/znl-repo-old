//  Actor library csp_task.hpp header file
//
//  Simple CSP task implementation.
//  Copyright (c) 2013-? Zoltan Leskowsky
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_CSP_TASK_HPP
#define BOOST_CSP_TASK_HPP

#include <boost/assert.hpp>
#ifdef BOOST_NO_CXX11_DELETED_FUNCTIONS
#include <boost/noncopyable.hpp>
#endif
#include <boost/static_assert.hpp>
#include <boost/type_traits/has_trivial_assign.hpp>
#include <boost/type_traits/has_trivial_destructor.hpp>

#include <boost/actor/detail/mpsc_queue_detail.hpp>

#include <boost/atomic/atomic.hpp>
//#include <boost/call_traits.hpp>
//#include <boost/lockfree/queue.hpp>
//#include <boost/thread/thread.hpp>

#if defined(_MSC_VER)
#pragma warning(push)
//#pragma warning(disable: 4324) // structure was padded due to __declspec(align())
#endif

namespace boost {
namespace actor {

class task_runner; // friend of class base_task

class base_task : public detail::queueable
{
public:
    int status() const { return status_; }
    void operator()() { run(); }
protected:
    base_task() : status_(0) {}
    //TODO: use compare-and-swap so that first error is kept?
    void set_status(int status) { if (status) status_ = status; }
private:
    virtual void run();
    friend class task_runner;
private:
    atomic<int> status_;
};

namespace detail {
class base_actor; // friend of task_runner class
} // namespace detail

class task_runner
{
public:
    task_runner() {}

    // The () operator is for use with class ::boost::thread::thread,
    // but can be called from any thread (including main): it runs a loop
    // which does not exit unless queue_stop() is called, after which
    // it runs all previously queued tasks before exiting
    void operator()() { run(); }

    void queue_task(base_task& task) { task_queue_.push(&task); }
    void queue_stop() { task_queue_.push(&end_); }
    void queue_stop(int status) {
        end_.set_status(status);
        task_queue_.push(&end_);
    }

    int status() const { return end_.status(); }

protected:
    friend class detail::base_actor;
    detail::mpsc_queue& task_queue_ref() { return task_queue_; }

    virtual void run(); // could be made non-virtual

    base_task* try_get_task() {
        base_task* task = static_cast<base_task*>(task_queue_.try_pop());
        return (task == &end_) ? (base_task*)0 : task;
    }

    base_task* wait_get_task() {
        base_task* task = static_cast<base_task*>(task_queue_.wait_pop());
        return (task == &end_) ? (base_task*)0 : task;
    }

private:
    task_runner(const task_runner& runner); //disallow

private:
    detail::mpsc_queue task_queue_;
    base_task end_;
};

} // namespace actor
} // namespace boost

#if defined(_MSC_VER)
#pragma warning(pop)
#endif

#endif // BOOST_CSP_TASK_HPP
