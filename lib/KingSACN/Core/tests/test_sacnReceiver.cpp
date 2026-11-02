#include <cassert>
#include <cstring>
#include <iostream>

#include "../sacnReceiver.hpp"

using Core::sacnReceiver;
using state = sacnReceiver::rxState;

static const sacnReceiver::rxConfig u1_h{1, sacnPolicy::timeoutPolicy::HOLD};
static const sacnReceiver::rxConfig u1_s{1, sacnPolicy::timeoutPolicy::STOP};

// uni 1 packet; cid byte 0, sequence number and channel 1 are the parts the tests vary
static sacnData pkt(uint8_t cid, uint8_t seq, uint8_t ch1 = 255, uint8_t options = 0)
{
    sacnData p{};
    p.universe = 1;
    p.CID[0] = cid;
    p.sequenceNumber = seq;
    p.options = options;
    p.propertyValues[1] = ch1;
    return p;
}

static void test_no_frame_before_packet()
{
    sacnReceiver rx(u1_h);
    DMXFrame out;
    assert(rx.getCurrentState() == state::WAITING);
    assert(!rx.getCurrentFrame(&out, 0));
    std::cout << "test_no_frame_before_packet good\n";
}

// frame holds slots 1-512, start code skipped
static void test_frame_contents()
{
    sacnReceiver rx(u1_h);
    sacnData p = pkt(1, 0);
    p.propertyValues[0] = 9; // start code, must not land in the frame
    p.propertyValues[512] = 42;
    assert(rx.onSacnPacket(p, 0));

    DMXFrame out;
    assert(rx.getCurrentFrame(&out, 0));
    assert(out.data[0] == 255);
    assert(out.data[511] == 42);
    for (int i = 1; i < 511; i++)
        assert(out.data[i] == 0);
    std::cout << "test_frame_contents good\n";
}

static void test_state_transitions()
{
    sacnReceiver rx(u1_h);
    rx.onSacnPacket(pkt(1, 0), 0);
    assert(rx.getCurrentState() == state::STREAMING);

    DMXFrame out;
    rx.getCurrentFrame(&out, 2499);
    assert(rx.getCurrentState() == state::STREAMING);
    rx.getCurrentFrame(&out, 2500);
    assert(rx.getCurrentState() == state::LOST);
    std::cout << "test_state_transitions good\n";
}

static void test_dataloss_stop()
{
    sacnReceiver rx(u1_s);
    rx.onSacnPacket(pkt(1, 0), 0);

    DMXFrame out;
    assert(rx.getCurrentFrame(&out, 2499));
    assert(!rx.getCurrentFrame(&out, 2500));
    std::cout << "test_dataloss_stop good\n";
}

// while streaming, other sources are ignored and the frame is kept
static void test_cid_lock()
{
    sacnReceiver rx(u1_h);
    rx.onSacnPacket(pkt(1, 0), 0);
    assert(!rx.onSacnPacket(pkt(2, 1, 7), 100));

    DMXFrame out;
    assert(rx.getCurrentFrame(&out, 100));
    assert(out.data[0] == 255);
    std::cout << "test_cid_lock good\n";
}

// after loss, a new source takes over from any sequence number
static void test_cid_takeover()
{
    sacnReceiver rx(u1_h);
    rx.onSacnPacket(pkt(1, 0), 0);

    // loss is detected on input too, without a getCurrentFrame call
    assert(!rx.onSacnPacket(pkt(2, 0, 7), 2499));

    // same sequence number as the old source, would be out of order if not reset
    assert(rx.onSacnPacket(pkt(2, 0, 7), 3000));
    assert(rx.getCurrentState() == state::STREAMING);
    DMXFrame out;
    assert(rx.getCurrentFrame(&out, 3000));
    assert(out.data[0] == 7);
    std::cout << "test_cid_takeover good\n";
}

// terminate drops the stream back to waiting, then any source can start a new one
static void test_terminate()
{
    for (auto cfg : {u1_h, u1_s})
    {
        sacnReceiver rx(cfg);
        rx.onSacnPacket(pkt(1, 0), 0);

        assert(!rx.onSacnPacket(pkt(1, 1, 255, constants::OPTIONS_STREAM_TERMINATED), 100));
        assert(rx.getCurrentState() == state::WAITING);

        DMXFrame out;
        assert(!rx.getCurrentFrame(&out, 100));

        // new source, old sequence number
        assert(rx.onSacnPacket(pkt(2, 1), 200));
        assert(rx.getCurrentState() == state::STREAMING);
    }
    std::cout << "test_terminate good\n";
}

// terminate from another source while streaming is ignored
static void test_terminate_other_cid()
{
    sacnReceiver rx(u1_h);
    rx.onSacnPacket(pkt(1, 0), 0);
    assert(!rx.onSacnPacket(pkt(2, 1, 255, constants::OPTIONS_STREAM_TERMINATED), 100));
    assert(rx.getCurrentState() == state::STREAMING);
    std::cout << "test_terminate_other_cid good\n";
}

// preview data is not displayed
static void test_preview()
{
    sacnReceiver rx(u1_h);
    assert(!rx.onSacnPacket(pkt(1, 0, 255, constants::OPTIONS_PREVIEW_DATA), 0));
    assert(rx.getCurrentState() == state::WAITING);
    std::cout << "test_preview good\n";
}

int main()
{
    test_no_frame_before_packet();
    test_frame_contents();
    test_state_transitions();
    test_dataloss_stop();
    test_cid_lock();
    test_cid_takeover();
    test_terminate();
    test_terminate_other_cid();
    test_preview();

    std::cout << "All tests passed" << std::endl;
    return 0;
}
