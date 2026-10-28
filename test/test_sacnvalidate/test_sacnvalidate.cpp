#include <unity.h>

#include "SACNParse.hpp"

/*
pio test -e native -v
*/

using namespace SACNParser;

void setUp(void) {}
void tearDown(void) {}

// any number is accepted for the first packet
void test_tracker_init(void)
{
    {
        SequenceTracker st{};
        TEST_ASSERT(st.accept(0));
    }
    {
        SequenceTracker st{};
        TEST_ASSERT(st.accept(127));
    }
    {
        SequenceTracker st{};
        TEST_ASSERT(st.accept(255));
    }
}

void test_accept_next(void)
{
    SequenceTracker st{};
    st.accept(0);

    TEST_ASSERT(st.accept(1));
    TEST_ASSERT(st.accept(2));
    TEST_ASSERT(st.accept(3));
    TEST_ASSERT(st.accept(30));
    TEST_ASSERT(st.accept(255));
    TEST_ASSERT(st.accept(0));
    TEST_ASSERT(st.accept(127));
    TEST_ASSERT(st.accept(30));
}

void test_reject_same(void)
{
    SequenceTracker st{};
    st.accept(0);
    TEST_ASSERT(!st.accept(0));

    st.accept(127);
    TEST_ASSERT(!st.accept(127));
}

// E1.31 6.7.2: out of order if (-20, 0] from the last accepted
void test_reject_1_to_19_behind(void)
{
    SequenceTracker st{};
    st.accept(50);
    TEST_ASSERT(!st.accept(49));
    TEST_ASSERT(!st.accept(40));
    TEST_ASSERT(!st.accept(31));
}

void test_accept_20_behind(void)
{
    SequenceTracker st{};
    st.accept(50);
    TEST_ASSERT(st.accept(30));
}

void test_reject_1_to_19_behind_wraparound(void)
{
    SequenceTracker st{};
    st.accept(0);
    TEST_ASSERT(!st.accept(static_cast<uint8_t>(0 - 1)));
    TEST_ASSERT(!st.accept(static_cast<uint8_t>(0 - 10)));
    TEST_ASSERT(!st.accept(static_cast<uint8_t>(0 - 19)));
}

void test_accept_20_behind_wraparound(void)
{
    SequenceTracker st{};
    st.accept(0);
    TEST_ASSERT(st.accept(static_cast<uint8_t>(0 - 20)));
}

void test_rejected_packet_does_not_move_last(void)
{
    SequenceTracker st{};
    st.accept(50);
    TEST_ASSERT(!st.accept(45));
    TEST_ASSERT(!st.accept(46));
    TEST_ASSERT(st.accept(51));
}

sacnPacket makePacket(uint16_t universe, uint8_t seq)
{
    sacnPacket p{};
    p.universe = ntoh16(universe); // stored in network order
    p.sequenceNumber = seq;
    return p;
}

const char *validate(const sacnPacket &p, const uint16_t *universe, SequenceTracker *tracker)
{
    return validateStatus_to_string(packetValidator(p, universe, tracker));
}

void test_validator_matching_universe_accepted(void)
{
    uint16_t uni{1};
    TEST_ASSERT_EQUAL_STRING("GOOD", validate(makePacket(1, 0), &uni, nullptr));
}

void test_validator_wrong_universe_rejected(void)
{
    uint16_t uni{1};
    TEST_ASSERT_EQUAL_STRING("WRONG_UNIVERSE", validate(makePacket(2, 0), &uni, nullptr));
    TEST_ASSERT_EQUAL_STRING("WRONG_UNIVERSE", validate(makePacket(256, 0), &uni, nullptr));
}

void test_validator_no_universe_accepts_any(void)
{
    TEST_ASSERT_EQUAL_STRING("GOOD", validate(makePacket(1, 0), nullptr, nullptr));
    TEST_ASSERT_EQUAL_STRING("GOOD", validate(makePacket(63999, 0), nullptr, nullptr));
}

void test_validator_out_of_order_rejected(void)
{
    uint16_t uni{1};
    SequenceTracker st{};
    TEST_ASSERT_EQUAL_STRING("GOOD", validate(makePacket(1, 10), &uni, &st));
    TEST_ASSERT_EQUAL_STRING("OUT_OF_ORDER", validate(makePacket(1, 10), &uni, &st));
    TEST_ASSERT_EQUAL_STRING("OUT_OF_ORDER", validate(makePacket(1, 5), &uni, &st));
    TEST_ASSERT_EQUAL_STRING("GOOD", validate(makePacket(1, 11), &uni, &st));
}

void test_validator_no_tracker_accepts_duplicates(void)
{
    uint16_t uni{1};
    TEST_ASSERT_EQUAL_STRING("GOOD", validate(makePacket(1, 10), &uni, nullptr));
    TEST_ASSERT_EQUAL_STRING("GOOD", validate(makePacket(1, 10), &uni, nullptr));
}

void test_validator_wrong_universe_does_not_touch_tracker(void)
{
    uint16_t uni{1};
    SequenceTracker st{};
    TEST_ASSERT_EQUAL_STRING("WRONG_UNIVERSE", validate(makePacket(2, 50), &uni, &st));
    TEST_ASSERT_EQUAL_STRING("GOOD", validate(makePacket(1, 40), &uni, &st));
}

int main(int argc, char **argv)
{
    UNITY_BEGIN();

    RUN_TEST(test_tracker_init);
    RUN_TEST(test_accept_next);
    RUN_TEST(test_reject_same);
    RUN_TEST(test_reject_1_to_19_behind);
    RUN_TEST(test_accept_20_behind);
    RUN_TEST(test_reject_1_to_19_behind_wraparound);
    RUN_TEST(test_accept_20_behind_wraparound);
    RUN_TEST(test_rejected_packet_does_not_move_last);

    RUN_TEST(test_validator_matching_universe_accepted);
    RUN_TEST(test_validator_wrong_universe_rejected);
    RUN_TEST(test_validator_no_universe_accepts_any);
    RUN_TEST(test_validator_out_of_order_rejected);
    RUN_TEST(test_validator_no_tracker_accepts_duplicates);
    RUN_TEST(test_validator_wrong_universe_does_not_touch_tracker);

    return UNITY_END();
}
