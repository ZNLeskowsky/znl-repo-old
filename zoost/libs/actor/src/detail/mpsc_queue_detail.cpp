//  Actor library mpsc_queue.cpp implementation file
//
//  Intrusive lock-free multi-producer single-consumer queue implementation.
//  Copyright (c) 2013-? Zoltan Leskowsky
//
//  Distributed under the Boost Software License, Version 1.0.
//  (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

//#include <cstddef>
//#include <boost/config.hpp>
#include <boost/actor/detail/mpsc_queue_detail.hpp>

namespace boost {
namespace actor {

namespace detail {

/*virtual*/ queueable::~queueable()
{
}

queueable* mpsc_queue::try_pop()
{
    queueable* item = first_;
    queueable* tail = item->next_;
    if (item == &end_) {
        if (!tail)
            return 0;
        first_ = item = tail;
        tail = item->next_;
    }
    if (tail) {
        first_ = tail;
        item->next_ = 0;
        return item;
    }
    if (item == last_.load()) {
        end_.next_ = 0;
        last_.exchange(&end_)->next_ = &end_; // may block
        tail = item->next_;
        if (tail) {
            first_ = tail;
            item->next_ = 0;
            return item;
        }
    }
    return 0;
}

} // namespace detail

} // namespace actor
} // namespace boost
