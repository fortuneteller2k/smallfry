#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <vector>

extern "C" {
#include "hardware/spi.h"
#include "pico/stdlib.h"
#include "pico/time.h"
}

#include "mfrc522.hh"

MFRC522::MFRC522(uint8_t sck, uint8_t mosi, uint8_t miso, uint8_t cs, uint8_t rst, spi_inst_t* spi) {
  this->sck = sck;
  this->mosi = mosi;
  this->miso = miso;
  this->cs = cs;
  this->rst = rst;
  this->spi = spi;
};

void MFRC522::init() {
  gpio_set_function(mosi, GPIO_FUNC_SPI);
  gpio_set_function(miso, GPIO_FUNC_SPI);
  gpio_set_function(sck, GPIO_FUNC_SPI);

  gpio_init(rst);
  gpio_set_dir(rst, GPIO_OUT);
  gpio_put(rst, false);

  gpio_init(cs);
  gpio_set_dir(cs, GPIO_OUT);
  chip_select(true);

  spi_init(spi, 1000000);  // this thing operates at 10 kHz apparently
  spi_set_format(spi, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);

  gpio_put(rst, true);
}

inline void MFRC522::chip_select(bool value) { gpio_put(cs, value); }

void MFRC522::write_register(Register reg, uint8_t val) {
  std::vector<uint8_t> data{{(uint8_t)(0xff & ((reg << 1)) & 0x7e), val}};

  chip_select(false);
  spi_write_blocking(spi, &data.at(0), 2);
  chip_select(true);
}

void MFRC522::write_register(Register reg, std::vector<uint8_t> val) {
  for (const uint8_t b : val) write_register(reg, b);
}

uint8_t MFRC522::read_register(Register reg) {
  std::vector<uint8_t> data(1);
  uint8_t addr = 0xff & (((reg << 1) & 0x7e) | 0x80);

  chip_select(false);
  spi_write_blocking(spi, &addr, 1);
  spi_read_blocking(spi, 0, &data.at(0), 1);
  chip_select(true);

  return data.at(0);
}

std::vector<uint8_t> MFRC522::read_register(Register reg, size_t size) {
  std::vector<uint8_t> data;
  for (size_t i = 0; i != size; i++) data.push_back(read_register(reg));
  return data;
}

void MFRC522::version() {
  uint8_t version = read_register(Version);

  switch (version) {
    case 0x92:
      printf("version: MFRC522 v2.0 (%02Xh)\n\n", version);
      break;
    default:
      printf("version: unknown (%02Xh)\n\n", version);
      break;
  }
}

bool MFRC522::self_test() {
  printf("MFRC522 self test\n");
  version();

  std::vector<uint8_t> zero_buf(25);
  std::fill(zero_buf.begin(), zero_buf.end(), 0x0u);

  // Section 16.1.1 Self test
  write_register(Command, SoftReset);  // soft reset
  write_register(FIFOLevel, 0x80);     // flush the buffer
  write_register(FIFOData, zero_buf);  // zero out the fifo buffer
  write_register(Command, Mem);        // transfer the contents of the fifo to the internal buffer
  write_register(AutoTest, 0x9);       // enable self-test
  write_register(FIFOData, 0x0u);      // write 0x0 to the fifo buffer
  write_register(Command, CalcCRC);    // initiate self-test

  size_t fifo_buf_size = read_register(FIFOLevel);

  // wait until we have 64 bytes in the fifo buffer
  while (fifo_buf_size < 64) fifo_buf_size = read_register(FIFOLevel);

  // done with the self-test
  write_register(AutoTest, 0x0u);
  write_register(Command, Idle);

  // read 64 bytes off the fifo buffer
  std::vector<uint8_t> buf = read_register(FIFOData, 64);

  auto print_contents = [i = 0](const uint8_t& b) mutable {
    printf("%02Xh ", b);
    if ((i + 1) % 8 == 0) printf("\n");
    ++i;
  };

  printf("got: \n");
  std::for_each(buf.cbegin(), buf.cend(), print_contents);
  printf("\nexpected: \n");
  std::for_each(mfrc522_v2_test_buf.cbegin(), mfrc522_v2_test_buf.cend(), print_contents);
  printf("\n");

  return std::equal(buf.cbegin(), buf.cend(), mfrc522_v2_test_buf.cbegin());
}

void MFRC522::toggle_antenna(bool value) {
  uint8_t tx_control = read_register(TxControl);

  if (value)
    if ((tx_control & 0x03) != 0x03)
      write_register(TxControl, tx_control | 0x03);
    else
      write_register(TxControl, tx_control & (~0x03));
}
