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

static void test_displayable_packet()
{
    sacnData p{};
    p.universe = 1;
    p.sequenceNumber = 10;

    assert(displayablePacket(p, 1, 9, true));
    assert(!displayablePacket(p, 2, 9, true));

    // out of order only matters once started
    assert(!displayablePacket(p, 1, 10, true));
    assert(displayablePacket(p, 1, 10, false));

    p.options = constants::OPTIONS_STREAM_TERMINATED;
    assert(!displayablePacket(p, 1, 9, true));
    p.options = constants::OPTIONS_PREVIEW_DATA;
    assert(!displayablePacket(p, 1, 9, true));
    std::cout << "test_displayable_packet good\n";
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
    test_displayable_packet();
    test_should_serve_frames();
    std::cout << "all passed\n";
    return 0;
}
