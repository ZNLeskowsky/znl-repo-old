//  Actor library detail/actor_detail.hpp header file
//
//  Simple actor implementation.
//  Copyright (c) 2013-? Zoltan Leskowsky
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_ACTOR_DETAIL_HPP
#define BOOST_ACTOR_DETAIL_HPP

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
#include <boost/actor/detail/csp_task_detail.hpp>
#include <boost/thread/thread.hpp>

#if defined(_MSC_VER)
#pragma warning(push)
//#pragma warning(disable: 4324) // structure was padded due to __declspec(align())
#endif

namespace boost {
namespace actor {

namespace detail {

#if 0 // moved to csp_task_detail.cpp
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
#endif // moved to csp_task_detail.cpp

class base_actor
{
public:
    void queue_message(detail::queueable_task* message) { message_queue_->push(message); }
protected:
    base_actor() : message_queue_(0) {}
    base_actor(task_runner& message_runner) : message_queue_(&(message_runner.task_queue_ref())) {}
    void associate_runner(task_runner& message_runner) { message_queue_ = &(message_runner.task_queue_ref()); }
    task_queue* message_queue() { return message_queue_; }
private:
    task_queue* message_queue_;
};

#if 0 //TODO: variadic template version
template <class DerivedActor, typename Act, typename... Ts>
class message
{
public:
    message(DerivedActor& actor, Act act, Ts... params)
      : actor_(actor), act_(act), ... {}
protected:
    virtual void run()
    {
        (actor_.*act_)(...);
        delete this;
    }
private:
    DerivedActor& actor_;
    Act act_;
    ...
};
#endif //TODO: variadic template version

template <class DerivedActor, typename Act,
    typename T1=void, typename T2=void, typename T3=void, typename T4=void,
    typename T5=void, typename T6=void, typename T7=void, typename T8=void>
class message;

template <class DerivedActor, typename Act>
class message<DerivedActor, Act> : public queueable_task
{
public:
    message(DerivedActor& actor, Act act)
      : actor_(actor), act_(act) {}
protected:
    virtual void run()
    {
        (actor_.*act_)();
        delete this;
    }
private:
    DerivedActor& actor_;
    Act act_;
};

template <class DerivedActor, typename Act, typename T>
class message<DerivedActor, Act, T> : public queueable_task
{
public:
    message(DerivedActor& actor, Act act, T param)
      : actor_(actor), act_(act), param_(param) {}
protected:
    virtual void run()
    {
        (actor_.*act_)(param_);
        delete this;
    }
private:
    DerivedActor& actor_;
    Act act_;
    typename ::boost::call_traits<T>::value_type param_;
};

template <class DerivedActor, typename Act, typename T1, typename T2>
class message<DerivedActor, Act, T1, T2> : public queueable_task
{
public:
    message(DerivedActor& actor, Act act, T1 param1, T2 param2)
      : actor_(actor), act_(act), param1_(param1), param2_(param2) {}
protected:
    virtual void run()
    {
        (actor_.*act_)(param1_, param2_);
        delete this;
    }
private:
    DerivedActor& actor_;
    Act act_;
    typename ::boost::call_traits<T1>::value_type param1_;
    typename ::boost::call_traits<T2>::value_type param2_;
};

template <class DerivedActor, typename Act, typename T1, typename T2, typename T3>
class message<DerivedActor, Act, T1, T2, T3> : public queueable_task
{
public:
    message(DerivedActor& actor, Act act, T1 param1, T2 param2, T3 param3)
      : actor_(actor), act_(act), param1_(param1), param2_(param2), param3_(param3) {}
protected:
    virtual void run()
    {
        (actor_.*act_)(param1_, param2_, param3_);
        delete this;
    }
private:
    DerivedActor& actor_;
    Act act_;
    typename ::boost::call_traits<T1>::value_type param1_;
    typename ::boost::call_traits<T2>::value_type param2_;
    typename ::boost::call_traits<T3>::value_type param3_;
};

#if 0 //TEMPORARY
template <class DerivedActor, typename Act, typename T1, typename T2, typename T3, typename T4>
class message<DerivedActor, Act, T1, T2, T3, T4> : public queueable_task
{
public:
    message(DerivedActor& actor, Act act, T1 param1, T2 param2, T3 param3, T4 param4)
      : actor_(actor), act_(act),
        param1_(param1), param2_(param2), param3_(param3), param4_(param4) {}
protected:
    virtual void run()
    {
        (actor_.*act_)(param1_, param2_, param3_, param4_);
        delete this;
    }
private:
    DerivedActor& actor_;
    Act act_;
    typename ::boost::call_traits<T1>::value_type param1_;
    typename ::boost::call_traits<T2>::value_type param2_;
    typename ::boost::call_traits<T3>::value_type param3_;
    typename ::boost::call_traits<T4>::value_type param4_;
};

template <class DerivedActor, typename Act, typename T1, typename T2, typename T3, typename T4, typename T5>
class message<DerivedActor, Act, T1, T2, T3, T4, T5> : public queueable_task
{
public:
    message(DerivedActor& actor, Act act, T1 param1, T2 param2, T3 param3, T4 param4, T5 param5)
      : actor_(actor), act_(act),
        param1_(param1), param2_(param2), param3_(param3), param4_(param4), param5_(param5) {}
protected:
    virtual void run()
    {
        (actor_.*act_)(param1_, param2_, param3_, param4_, param5_);
        delete this;
    }
private:
    DerivedActor& actor_;
    Act act_;
    typename ::boost::call_traits<T1>::value_type param1_;
    typename ::boost::call_traits<T2>::value_type param2_;
    typename ::boost::call_traits<T3>::value_type param3_;
    typename ::boost::call_traits<T4>::value_type param4_;
    typename ::boost::call_traits<T5>::value_type param5_;
};

template <class DerivedActor, typename Act, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
class message<DerivedActor, Act, T1, T2, T3, T4, T5, T6> : public queueable_task
{
public:
    message(DerivedActor& actor, Act act, T1 param1, T2 param2, T3 param3, T4 param4, T5 param5, T6 param6)
      : actor_(actor), act_(act),
        param1_(param1), param2_(param2), param3_(param3), param4_(param4), param5_(param5), param6_(param6) {}
protected:
    virtual void run()
    {
        (actor_.*act_)(param1_, param2_, param3_, param4_, param5_, param6_);
        delete this;
    }
private:
    DerivedActor& actor_;
    Act act_;
    typename ::boost::call_traits<T1>::value_type param1_;
    typename ::boost::call_traits<T2>::value_type param2_;
    typename ::boost::call_traits<T3>::value_type param3_;
    typename ::boost::call_traits<T4>::value_type param4_;
    typename ::boost::call_traits<T5>::value_type param5_;
    typename ::boost::call_traits<T6>::value_type param6_;
};
#endif //TEMPORARY

} // namespace detail

} // namespace actor
} // namespace boost

#if defined(_MSC_VER)
#pragma warning(pop)
#endif

#endif // BOOST_ACTOR_DETAIL_HPP
