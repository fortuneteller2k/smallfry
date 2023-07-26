#include <cstdio>
#include <cstdlib>
#include <algorithm>

extern "C" {
#include "hardware/spi.h"
#include "pico/stdlib.h"
#include "pico/time.h"
}

#include "mfrc522.hh"

MFRC522::MFRC522(uint8_t sck, uint8_t mosi, uint8_t miso, uint8_t cs, uint8_t rst, spi_inst_t *spi) {
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

  spi_init(spi, 1000000); // this thing operates at 10 kHz apparently
  spi_set_format(spi, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
  
  gpio_put(rst, true);
}

void MFRC522::chip_select(bool value) {
  asm volatile("nop \n nop \n nop");
  gpio_put(cs, value);
  asm volatile("nop \n nop \n nop");
}

void MFRC522::write_register(Register reg, uint8_t val) {
  uint8_t data[2] = {0xff & ((reg << 1) & 0x7e), val};

  chip_select(false);
  spi_write_blocking(spi, data, 2);
  chip_select(true);
}

void MFRC522::write_register(Register reg, uint8_t *val) {
  for (uint8_t i = 0; i < sizeof(val) / sizeof(val[0]); i++)
    write_register(reg, val[i]);
}

uint8_t MFRC522::read_register(Register reg) {
  uint8_t data[1];
  uint8_t addr = 0xff & (((reg << 1) & 0x7e) | 0x80);

  chip_select(false);
  spi_write_blocking(spi, &addr, 1);
  spi_read_blocking(spi, 0, data, 1);
  chip_select(true);

  return data[0];
}

void MFRC522::print_version() {
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
  uint8_t zero_buf[25] = {0x0};

  // Section 16.1.1 Self test
  write_register(Command, SoftReset); // soft reset
  write_register(FIFOLevel, 0x80); // flush the buffer
  write_register(FIFOData, zero_buf); // zero out the fifo buffer
  write_register(Command, Mem); // transfer the contents of the fifo to the internal buffer
  write_register(AutoTest, 0x9);  // enable self-test
  write_register(FIFOData, (uint8_t) 0x0); // write 0x0 to the fifo buffer
  write_register(Command, CalcCRC); // initiate self-test

  uint8_t j;

  // repeatedly check if we have 64 bytes in the fifo buffer
  for (uint8_t i = 0; i < UINT8_MAX; i++) {
    j = read_register(FIFOLevel);
    if (j >= 64) break;
  }

  // done with the self-test
  write_register(AutoTest, (uint8_t)0x0);
  write_register(Command, Idle);

  print_version();

  uint8_t buf[64];

  printf("got: \n");

  // read 64 bytes off the fifo buffer
  for (uint8_t i = 0; i < 64; i++) {
    buf[i] = read_register(FIFOData);

    printf("%02Xh ", buf[i]);

    if ((i + 1) % 8 == 0)
      printf("\n");
  }

  printf("\nexpected: \n");

  for (uint8_t i = 0; i < 64; i++) {
    printf("%02Xh ", mfrc522_v2_self_test_expected_buffer[i]);

    if ((i + 1) % 8 == 0)
      printf("\n");
  }

  printf("\n");

  return std::equal(buf, buf + sizeof(buf), mfrc522_v2_self_test_expected_buffer);
}