//  Actor library mpsc_queue_detail.hpp detail header file
//
//  Intrusive lock-free multi-producer single-consumer queue.
//  Copyright (c) 2013-4 Zoltan Leskowsky
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_ACTOR_DETAIL_MPSC_QUEUE_HPP
#define BOOST_ACTOR_DETAIL_MPSC_QUEUE_HPP

#include <boost/assert.hpp>
#ifdef BOOST_NO_CXX11_DELETED_FUNCTIONS
#include <boost/noncopyable.hpp>
#endif
#include <boost/static_assert.hpp>
//#include <boost/type_traits/has_trivial_assign.hpp>
//#include <boost/type_traits/has_trivial_destructor.hpp>

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

namespace detail {

class queueable
{
public:
    queueable() : next_(0) {}
    virtual ~queueable();
private:
    friend class mpsc_queue;
private:
    queueable* volatile next_;
};

class mpsc_queue
{
public:
    mpsc_queue() : first_(&end_), last_(&end_) {}

    bool push(queueable* item) {
        queueable* prev = last_->exchange(item); // may block
        prev->next_ = item;
        return (prev != &end_);
    }

    queueable* try_pop();

    queueable* wait_pop() {
        queueable* item;
        while (0 == (item = try_pop()))
            ; // busy wait which will only exit if there's a corresponding push
        return item;
    }

private:
    queueable* first_;
    atomic<queueable*> last_;
    queueable end_;
};

} // namespace detail

} // namespace actor
} // namespace boost

#if defined(_MSC_VER)
#pragma warning(pop)
#endif

#endif // BOOST_ACTOR_DETAIL_MPSC_QUEUE_HPP
