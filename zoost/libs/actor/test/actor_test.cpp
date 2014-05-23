/////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Zoltan Leskowsky 2014.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/actor for documentation.
//
/////////////////////////////////////////////////////////////////////////////

#include <boost/ref.hpp>
#include <boost/actor/actor.hpp>
#include <boost/thread/thread.hpp>
#include <boost/detail/lightweight_test.hpp>

using namespace boost;
//using namespace boost::actor;

class TestActor : public actor::actor<TestActor>
{
public:
    TestActor(boost::actor::task_runner& actor_runner) : actor::actor<TestActor>(actor_runner) {}
    void test0() { std::cerr << "nil" << std::endl << std::flush; }
    void test1(int i) { std::cerr << "i=" << i << std::endl << std::flush; }
    void test2(int i, double f) { std::cerr << "i=" << i << ", f=" << f << std::endl << std::flush; }
    void test3(int i, double f, boost::reference_wrapper<const int> cref_j) {
        std::cerr << "i=" << i << ", f=" << f << std::endl << ", cref_j[&=" << get_pointer(cref_j) << "]=" << cref_j << std::flush; }
};

int main( int, char* [] )
{
    actor::task_runner actor_runner;
    thread::thread actor_thread(boost::ref(actor_runner));
    std::cerr << "started thread with actor runner " << &actor_runner << std::endl << std::flush;

    TestActor actorA(actor_runner);
    TestActor actorB(actor_runner);
    TestActor actorC(actor_runner);

    actor::send(actorA, &TestActor::test0);
    actor::send(actorB, &TestActor::test1, 1);

    int g3 = 3;
    std::cerr << "&g3 is " << &g3 << std::endl << std::flush;
    actor::send(actorC, &TestActor::test3, 3, 3.5, boost::cref(g3));

    std::cerr << "stopping actor runner " << &actor_runner << std::endl << std::flush;
    actor_runner.queue_stop();
    std::cerr << "joining actor thread" << std::endl << std::flush;
    actor_thread.join();
    std::cerr << "joined actor thread" << std::endl << std::flush;

    BOOST_TEST(1 == 1);
    //BOOST_TEST(1 != 1);

   return boost::report_errors();
}
