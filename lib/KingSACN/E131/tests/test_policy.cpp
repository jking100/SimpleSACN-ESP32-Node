#include <cassert>
#include <iostream>

#include "../sacnPolicy.hpp"

using namespace sacnPolicy;

static void test_dataloss()
{
    assert(!dataLoss(2499, 0));
    assert(dataLoss(2500, 0));
    assert(!dataLoss(1000, 1000));
    // millis wrap
    assert(!dataLoss(100, 0xFFFFFFFF - 100));
    assert(dataLoss(2500, 0xFFFFFFFF));
    // now read just before rx was stamped
    assert(!dataLoss(999, 1000));
    std::cout << "test_dataloss good\n";
}

static void test_out_of_order()
{
    assert(outOfOrder(10, 10));
    assert(outOfOrder(9, 10));
    assert(outOfOrder(10 - 19, 10));
    assert(!outOfOrder(10 - 20, 10));
    assert(!outOfOrder(11, 10));
    // wrap
    assert(!outOfOrder(0, 255));
    assert(outOfOrder(250, 5));
    std::cout << "test_out_of_order good\n";
}

static sacnData pk(uint8_t cid, uint8_t seq, uint8_t prio = 100)
{
    sacnData p{};
    p.universe = 1;
    p.CID[0] = cid;
    p.sequenceNumber = seq;
    p.priority = prio;
    return p;
}

static void test_relevant_packet()
{
    sacnData last = pk(1, 9);

    assert(relevantPacket(pk(1, 10), 1, true, last));
    assert(!relevantPacket(pk(1, 10), 2, true, last));

    // out of order only matters once started
    assert(!relevantPacket(pk(1, 9), 1, true, last));
    assert(relevantPacket(pk(1, 9), 1, false, last));

    // sequence is per source: another source's sequence is not compared
    assert(relevantPacket(pk(2, 9, 101), 1, true, last));

    // priority: higher wins, lower ignored
    assert(relevantPacket(pk(0, 50, 101), 1, true, last));
    assert(!relevantPacket(pk(2, 50, 99), 1, true, last));

    // same priority: greater cid wins
    assert(relevantPacket(pk(2, 50), 1, true, last));
    assert(!relevantPacket(pk(0, 50), 1, true, last));
    std::cout << "test_relevant_packet good\n";
}

static void test_flags()
{
    sacnData p{};
    assert(!shouldTerminate(p) && !shouldNotDisplay(p));
    p.options = constants::OPTIONS_STREAM_TERMINATED;
    assert(shouldTerminate(p) && !shouldNotDisplay(p));
    p.options = constants::OPTIONS_PREVIEW_DATA;
    assert(!shouldTerminate(p) && shouldNotDisplay(p));
    std::cout << "test_flags good\n";
}

static void test_should_serve_frames()
{
    assert(shouldServeFrames(timeoutPolicy::HOLD, 2499, 0));
    assert(shouldServeFrames(timeoutPolicy::HOLD, 100000, 0));
    assert(shouldServeFrames(timeoutPolicy::STOP, 2499, 0));
    assert(!shouldServeFrames(timeoutPolicy::STOP, 2500, 0));
    std::cout << "test_should_serve_frames good\n";
}

int main()
{
    test_dataloss();
    test_out_of_order();
    test_relevant_packet();
    test_flags();
    test_should_serve_frames();
    std::cout << "all passed\n";
    return 0;
}
