//  Actor library mpsc_channel.hpp header file
//
//  Intrusive lock-free multi-producer single-consumer channel.
//  Copyright (c) 2013-4 Zoltan Leskowsky
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_ACTOR_MPSC_CHANNEL_HPP
#define BOOST_ACTOR_MPSC_CHANNEL_HPP

#include <memory> // for std::auto_ptr

#include <boost/assert.hpp>
#ifdef BOOST_NO_CXX11_DELETED_FUNCTIONS
#include <boost/noncopyable.hpp>
#endif
//#include <boost/ref.h>
#include <boost/static_assert.hpp>
//#include <boost/type_traits/add_const.hpp>
//#include <boost/type_traits/has_trivial_assign.hpp>
//#include <boost/type_traits/has_trivial_destructor.hpp>
#include <boost/type_traits/is_base_and_derived.hpp>

#include <boost/actor/detail/mpsc_queue_detail.hpp>

#if defined(_MSC_VER)
#pragma warning(push)
//#pragma warning(disable: 4324) // structure was padded due to __declspec(align())
#endif

namespace boost {
namespace actor {

// Queueable class template is based on the Curiously Recurrent Template
// Pattern (CRTP), used in this way:
// class MyItem : public ::boost::actor::queueable<MyItem>
// {
//     ...
// };
template <typename Item> class queueable : public detail::queueable
{
protected:
    queueable() {}
};

template <class Item> class mpsc_channel;

template <Item>
class mpsc_channel<Item*>
{
public:
    const int OPEN = 0;
    const int CLOSED = 1;

    mpsc_channel() : status_(OPEN) {
        BOOST_STATIC_ASSERT((::boost::is_base_and_derived<queueable<Item>, Item>::value));
    }

    int status() const { return status_; }
    bool is_open() const { return (status_ == OPEN); }
    operator bool() const { return (status_ == OPEN); }
    bool operator!() const { return (status_!= OPEN); }

    void open() { status_ = OPEN; }
    void close() { queue_.push(&end_); }

    void put(const Item* pitem) {
        queue_.push(const_cast<detail::queueable*>(
                        static_cast<const detail::queueable*>(pitem)));
    }

    const Item* try_get() {
        return item_unless_end(queue_.try_pop());
    }

    const Item* wait_get() {
        return item_unless_end(queue_.wait_pop());
    }

protected:
    Item* item_unless_end(detail::queueable* pitem) {
        if (pitem) {
            if (pitem != &end_)
                return static_cast<Item*>(pitem);
            status_ = CLOSED;
        }
        return (Item*)0;
    }

    detail::mpsc_queue& queue() { return queue_; }

private:
    detail::mpsc_queue queue_;
    detail::queueable end_;
    int status_;
};

template <Item>
class mpsc_channel<const Item*> : public mpsc_channel<Item*>
{ 
};

template <Item>
class mpsc_channel<std::auto_ptr<Item> >
  : public mpsc_channel<Item*>
{
public:
    void put(std::auto_ptr<Item> pitem) {
        queue().push(pitem.release());
    }

    std::auto_ptr<Item> try_get() {
        return item_unless_end(queue().try_pop());
    }

    std::auto_ptr<Item> wait_get() {
        return item_unless_end(queue().wait_pop());
    }
};

} // namespace actor
} // namespace boost

#if defined(_MSC_VER)
#pragma warning(pop)
#endif

#endif // BOOST_ACTOR_MPSC_CHANNEL_HPP
