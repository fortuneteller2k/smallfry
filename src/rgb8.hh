#pragma once
#include <cstdint>

class RGB8 {
  public:
    uint8_t r, g, b;

    RGB8();
    RGB8(uint8_t r, uint8_t g, uint8_t b);
    RGB8 wheel(uint8_t pos);
    RGB8 brightness(uint8_t brightness);
    static uint32_t rgb8_as_u32(RGB8 color);
};