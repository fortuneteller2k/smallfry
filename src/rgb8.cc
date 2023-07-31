#include "rgb8.hh"

RGB8::RGB8() {
  this->r = 0;
  this->g = 0;
  this->b = 0;
}

RGB8::RGB8(uint8_t r, uint8_t g, uint8_t b) {
  this->r = r;
  this->g = g;
  this->b = b;
}

RGB8 RGB8::brightness(uint8_t brightness) {
  r = (uint8_t)((uint16_t)r * ((uint16_t)brightness + 1) / (UINT8_MAX + 1));
  g = (uint8_t)((uint16_t)g * ((uint16_t)brightness + 1) / (UINT8_MAX + 1));
  b = (uint8_t)((uint16_t)b * ((uint16_t)brightness + 1) / (UINT8_MAX + 1));

  return *this;
}

RGB8 RGB8::wheel(uint8_t pos) {
  uint8_t r, g, b;
  pos = UINT8_MAX - pos;

  if (pos < 85) {
    r = UINT8_MAX - pos * 3;
    g = 0;
    b = pos * 3;
  } else if (pos < 170) {
    pos -= 85;
    r = 0;
    g = pos * 3;
    b = UINT8_MAX - pos * 3;
  } else {
    pos -= 170;
    r = pos * 3;
    g = UINT8_MAX - pos * 3;
    b = 0;
  }

  return RGB8(r, g, b);
}

uint32_t RGB8::rgb8_as_u32(RGB8 color) {
  return ((uint32_t)(color.r) << 8) | ((uint32_t)(color.g) << 16) | (uint32_t)(color.b);
}
