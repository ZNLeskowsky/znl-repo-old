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

#include <boost/assert.hpp>
#ifdef BOOST_NO_CXX11_DELETED_FUNCTIONS
#include <boost/noncopyable.hpp>
#endif
#include <boost/static_assert.hpp>
#include <boost/type_traits/has_trivial_assign.hpp>
#include <boost/type_traits/has_trivial_destructor.hpp>

#include <boost/atomic/atomic.hpp>
#include <boost/lockfree/queue.hpp>
#include <boost/thread/thread.hpp>

#include <boost/actor/detail/actor_detail.hpp>

#if defined(_MSC_VER)
#pragma warning(push)
//#pragma warning(disable: 4324) // structure was padded due to __declspec(align())
#endif

namespace boost {
namespace actor {

class base_task : public detail::queuable_task
{
private:
    virtual void run() = 0;
};

class task_scheduler;

#if 0 //OBSOLETE
class task_thread : private detail::queuable_task
{
public:
    task_thread() : thread_(::boost::ref(*this)), scheduler_(0) {}
    ~task_thread() { exit(); thread_.join(); }
    void queue_task(base_task& task) { task_queue_.push(&task); }
    void exit(int status = 0) { exit_.set_status(status); task_queue_.push(&exit_); }
    int join() { thread_.join(); return exit_.status(); }
    void detach() { thread_.detach(); }

protected:
    task_thread(task_scheduler& scheduler);
    detail::task_queue& task_queue() { return task_queue_; }
    bool is_exit(const detail::queuable_task* task) { return (task == &exit_); }

private:
    virtual void run();
    friend class detail::base_actor;
    void queue_detail_task(detail::queuable_task& task) { task_queue_.push(&task); }
    friend class ::boost::thread::thread;
    void operator()() { task_thread::run(); }

private:
    ::boost::thread::thread thread_;
    detail::task_queue task_queue_;
    task_scheduler* scheduler_;
    detail::queuable_task exit_;
};
#endif //OBSOLETE

class task_runner : private detail::queuable_task
{
public:
    task_runner() : scheduler_(0) {}
    task_runner(task_scheduler& scheduler) : scheduler_(&scheduler) {}
    ~task_runner() {}
    void operator()() { task_runner::run(); } // for use by class ::boost::thread::thread
    void associate_scheduler(task_scheduler& scheduler) { scheduler_ = &scheduler; }
    void queue_task(base_task& task) { task_queue_.push(&task); }
    void queue_stop(int status = 0) { stop_.set_status(status); task_queue_.push(&stop_); }
    int status() const { return stop_.status(); }

protected:
    detail::task_queue& task_queue() { return task_queue_; }
    bool is_stop(const detail::queuable_task* task) { return (task == &stop_); }

private:
    task_runner(const task_runner& runner); //disallow
    virtual void run();
    friend class detail::base_actor;
    void queue_detail_task(detail::queuable_task& task) { task_queue_.push(&task); }

private:
    detail::task_queue task_queue_;
    task_scheduler* scheduler_;
    detail::queuable_task stop_;
};

class task_scheduler : public task_runner
{
public:
    task_scheduler() {}
    //OBSOLETE: task_scheduler() : task_thread(*this) {}
    void operator()() { task_runner::run(); } // for use by class ::boost::thread::thread (hides task_runner::operator())

private:
    task_scheduler(const task_scheduler& scheduler); //disallow
    detail::task_queue& task_runner_queue() { return task_runner_queue_; }
    virtual void run();

private:
    detail::task_queue task_runner_queue_;
};

//OBSOLETE: inline task_thread::task_thread(task_scheduler& scheduler) : thread_(::boost::ref(*this)), scheduler_(&scheduler) {}

template <class DerivedActor> class actor : public detail::base_actor
{
protected:
    actor(task_runner& runner) : detail::base_actor(runner) {}

public:
    void associate_runner(task_runner& runner) { detail::base_actor::associate_runner(runner); }
    void associate_scheduler(task_scheduler& scheduler) { associate_runner(scheduler); }

    template <typename Act>
    friend void send(DerivedActor& actor, Act act)
    {
        actor.queue_message(
            new detail::message<DerivedActor, Act>(actor, act));
    }

    template <typename Act, typename T>
    friend void send(DerivedActor& actor, Act act, T param)
    {
        actor.queue_message(
            new detail::message<DerivedActor, Act, T>(actor, act, param));
    }

    template <typename Act, typename T1, typename T2>
    friend void send(DerivedActor& actor, Act act, T1 param1, T2 param2)
    {
        actor.queue_message(
            new detail::message<DerivedActor, Act, T1, T2>(actor, act, param1, param2));
    }

    template <typename Act, typename T1, typename T2, typename T3>
    friend void send(DerivedActor& actor, Act act, T1 param1, T2 param2, T3 param3)
    {
        actor.queue_message(
            new detail::message<DerivedActor, Act, T1, T2, T3>(actor, act, param1, param2, param3));
    }

    template <typename Act, typename T1, typename T2, typename T3, typename T4>
    friend void send(DerivedActor& actor, Act act, T1 param1, T2 param2, T3 param3, T4 param4)
    {
        actor.queue_message(
            new detail::message<DerivedActor, Act, T1, T2, T3, T4>(
                            actor, act, param1, param2, param3, param4));
    }

    template <typename Act, typename T1, typename T2, typename T3, typename T4, typename T5>
    friend void send(DerivedActor& actor, Act act,
                     T1 param1, T2 param2, T3 param3, T4 param4, T5 param5)
    {
        actor.queue_message(
            new detail::message<DerivedActor, Act, T1, T2, T3, T4, T5>(
                            actor, act, param1, param2, param3, param4, param5));
    }

    template <typename Act, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
    friend void send(DerivedActor& actor, Act act,
                     T1 param1, T2 param2, T3 param3, T4 param4, T5 param5, T6 param6)
    {
        actor.queue_message(
            new detail::message<DerivedActor, Act, T1, T2, T3, T4, T5, T6>(
                            actor, act, param1, param2, param3, param4, param5, param6));
    }
};

} // namespace actor
} // namespace boost

#if defined(_MSC_VER)
#pragma warning(pop)
#endif

#endif // BOOST_ACTOR_HPP
