//  Actor library actor.hpp header file
//
//  Simple actor implementation.
//  Copyright (c) 2013-? Zoltan Leskowsky
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_ACTOR_HPP
#define BOOST_ACTOR_HPP

#include <memory> // for auto_ptr

#include <boost/assert.hpp>
#ifdef BOOST_NO_CXX11_DELETED_FUNCTIONS
#include <boost/noncopyable.hpp>
#endif
#include <boost/static_assert.hpp>
#include <boost/type_traits/has_trivial_assign.hpp>
#include <boost/type_traits/has_trivial_destructor.hpp>

//#include <boost/atomic/atomic.hpp>
//#include <boost/lockfree/queue.hpp>
//#include <boost/thread/thread.hpp>

#include <boost/actor/detail/csp_task_detail.hpp>

#if defined(_MSC_VER)
#pragma warning(push)
//#pragma warning(disable: 4324) // structure was padded due to __declspec(align())
#endif

namespace boost {
namespace actor {

template <class Queueable> class queueable : private detail::queueable
{
protected:
    queueable() {}
};

template <class Queueable> class csp_channel : private detail::mpsc_queue
{
public:
    void push(std::auto_ptr<queueable<Queueable> > pobj) {
        detail::mpsc_queue::push(pobj.release()); // may block
    }

    std::auto_ptr<Queueable> try_pop() { // may block
        return static_cast<Queueable*>(detail::mpsc_queue::try_pop());
    }

    std::auto_ptr<Queueable> wait_pop(volatile int* status = 0) {
        detail::queueable* pobj;
        while ((0 == (pobj = detail::mpsc_queue::try_pop()))
               && !(status && *status)); // busy wait
        return static_cast<Queueable*>(pobj);
    }
};

class base_task : public detail::queueable_task
{
private:
    virtual void run() = 0;
};

class task_runner : public detail::task_runner
{
public:
    task_runner() {}

    // The () operator is for use with class ::boost::thread::thread,
    // but can be called from any thread (including main):
    // it runs a loop which does not exit until queue_stop() is called.
    void operator()() { detail::task_runner::run(); }

    void queue_task(base_task& task) { detail::task_runner::queue_task(task); }
    void queue_stop() { detail::task_runner::queue_stop(); }
    void queue_stop(int status) { detail::task_runner::queue_stop(status); }
    int status() const { return detail::task_runner::status(); }

private:
    task_runner(const task_runner& runner); //disallow
};

} // namespace actor
} // namespace boost

#if defined(_MSC_VER)
#pragma warning(pop)
#endif

#endif // BOOST_ACTOR_HPP
