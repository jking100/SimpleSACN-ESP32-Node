#include <cassert>
#include <iostream>

#include "../sacnData.hpp"

// g++ -std=c++17 -Wall -Wextra tests/test_swaps.cpp -o test_swaps && ./test_swaps && rm test_swaps.exe

using namespace util;

static void test_swap16()
{
    assert(swap16(0x0000) == 0x0000);
    assert(swap16(0xABCD) == 0xCDAB);
    assert(swap16(0xAFFA) == 0xFAAF);
    assert(swap16(0xAFAF) == 0xAFAF);
    std::cout << "test_swap16 good\n";
}

static void test_swap32()
{
    // std::cout << std::hex << swap32(0xABCDEF00) << std::endl;
    assert(swap32(0x00000000) == 0x00000000);
    assert(swap32(0xABCDEF00) == 0x00EFCDAB);
    assert(swap32(0x01234567) == 0x67452301);
    std::cout << "test_swap32 good\n";
}

static void test_read16()
{

    uint8_t buf[5]{0x0, 0x0, 0xAB, 0xCD, 0x0};
    // std::cout << std::hex << read16ntoh(buf, 2) << std::endl;
    assert(read16ntoh(buf, 0) == 0x0000);
    assert(read16ntoh(buf, 1) == 0x00AB);
    assert(read16ntoh(buf, 2) == 0xABCD);
    assert(read16ntoh(buf, 3) == 0xCD00);

    std::cout << "test_read16 good\n";
}
static void test_read32()
{

    uint8_t buf[10]{0x0, 0x0, 0x12, 0x34, 0x56, 0x78, 0x9A, 0x0, 0x0, 0x0};
    // std::cout << std::hex << read32ntoh(buf, 0) << std::endl;
    assert(read32ntoh(buf, 0) == 0x00001234);
    assert(read32ntoh(buf, 1) == 0x00123456);
    assert(read32ntoh(buf, 2) == 0x12345678);
    assert(read32ntoh(buf, 3) == 0x3456789a);
    assert(read32ntoh(buf, 4) == 0x56789a00);
    assert(read32ntoh(buf, 5) == 0x789a0000);
    assert(read32ntoh(buf, 6) == 0x9a000000);
    std::cout << "test_read32 good\n";
}

int main()
{
    test_swap16();
    test_swap32();
    test_read16();
    test_read32();
    return 0;
}