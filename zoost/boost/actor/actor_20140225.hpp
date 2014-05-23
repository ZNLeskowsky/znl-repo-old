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
//#include <boost/lockfree/queue.hpp>
//#include <boost/thread/thread.hpp>

#include <boost/actor/detail/actor_detail.hpp>

#if defined(_MSC_VER)
#pragma warning(push)
//#pragma warning(disable: 4324) // structure was padded due to __declspec(align())
#endif

namespace boost {
namespace actor {

class base_task : public detail::queueable_task
{
private:
    virtual void run() = 0;
};

class task_runner : public detail::task_runner
{
public:
    task_runner() {}

    // The () operator is for use by class ::boost::thread::thread,
    // but it can also be called from any thread (including main).
    // It runs a loop which does not exit until queue_stop() is called.
    void operator()() { detail::task_runner::run(); }

    void queue_task(base_task& task) { detail::task_runner::queue_task(task); }
    void queue_stop(int status = 0) { detail::task_runner::queue_stop(status); }
    int status() const { return detail::task_runner::status(); }

protected:
    //bool is_stop(const base_task* task) const { return detail::is_stop(task); }

private:
    task_runner(const task_runner& runner); //disallow
    //virtual void run();
};

template <class DerivedActor> class actor : public detail::base_actor
{
public:
    void associate_runner(task_runner& message_runner) { detail::base_actor::associate_runner(message_runner); }

protected:
    actor() {}
    actor(task_runner& message_runner) : detail::base_actor(message_runner) {}
};

template <class DerivedActor, typename Act>
void send(DerivedActor& actor, Act act)
{
    std::cerr << "sending with no args" << std::endl << std::flush;
    actor.queue_message(
        new detail::message<DerivedActor, Act>(actor, act));
    std::cerr << "sent with no args" << std::endl << std::flush;
}

template <class DerivedActor, typename Act, typename T>
void send(DerivedActor& actor, Act act, T param)
{
    std::cerr << "sending with 1 arg: " << param << std::endl << std::flush;
    actor.queue_message(
        new detail::message<DerivedActor, Act, T>(actor, act, param));
    std::cerr << "sent with 1 arg" << std::endl << std::flush;
}

template <class DerivedActor, typename Act, typename T1, typename T2>
void send(DerivedActor& actor, Act act, T1 param1, T2 param2)
{
    std::cerr << "sending with 2 args: " << param1 << ", " << param2 << std::endl << std::flush;
    actor.queue_message(
        new detail::message<DerivedActor, Act, T1, T2>(actor, act, param1, param2));
    std::cerr << "sent with 2 args" << std::endl << std::flush;
}

template <class DerivedActor, typename Act, typename T1, typename T2, typename T3>
void send(DerivedActor& actor, Act act, T1 param1, T2 param2, T3 param3)
{
    std::cerr << "sending with 3 args: " << param1 << ", " << param2 << ", " << param3 << std::endl << std::flush;
    actor.queue_message(
        new detail::message<DerivedActor, Act, T1, T2, T3>(actor, act, param1, param2, param3));
    std::cerr << "sent with 3 args" << std::endl << std::flush;
}

#if 0 //TEMPORARY
template <class DerivedActor, typename Act, typename T1, typename T2, typename T3, typename T4>
void send(DerivedActor& actor, Act act, T1 param1, T2 param2, T3 param3, T4 param4)
{
    actor.queue_message(
        new detail::message<DerivedActor, Act, T1, T2, T3, T4>(
                        actor, act, param1, param2, param3, param4));
}

template <class DerivedActor, typename Act, typename T1, typename T2, typename T3, typename T4, typename T5>
void send(DerivedActor& actor, Act act,
                 T1 param1, T2 param2, T3 param3, T4 param4, T5 param5)
{
    actor.queue_message(
        new detail::message<DerivedActor, Act, T1, T2, T3, T4, T5>(
                        actor, act, param1, param2, param3, param4, param5));
}

template <class DerivedActor, typename Act, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
void send(DerivedActor& actor, Act act,
                 T1 param1, T2 param2, T3 param3, T4 param4, T5 param5, T6 param6)
{
    actor.queue_message(
        new detail::message<DerivedActor, Act, T1, T2, T3, T4, T5, T6>(
                        actor, act, param1, param2, param3, param4, param5, param6));
}
#endif //TEMPORARY

} // namespace actor
} // namespace boost

#if defined(_MSC_VER)
#pragma warning(pop)
#endif

#endif // BOOST_ACTOR_HPP
