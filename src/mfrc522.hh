#pragma once

#include <array>
#include <cstdint>
#include <span>
#include <vector>

extern "C" {
#include "hardware/spi.h"
}

class MFRC522 {
 private:
  uint8_t sck;
  uint8_t mosi;
  uint8_t miso;
  uint8_t cs;
  uint8_t rst;
  spi_inst_t* spi;

 public:
  enum Register {
    Command = 0x01,
    ComIEn = 0x02,
    DivIEn = 0x03,
    ComIrq = 0x04,
    DivIrq = 0x05,
    Error = 0x06,
    Status1 = 0x07,
    Status2 = 0x08,
    FIFOData = 0x09,
    FIFOLevel = 0x0a,
    WaterLevel = 0x0b,
    Control = 0x0c,
    BitFraming = 0x0d,
    Coll = 0x0e,
    Mode = 0x11,
    TxMode = 0x12,
    RxMode = 0x13,
    TxControl = 0x14,
    TxASK = 0x15,
    TxSel = 0x16,
    RxSel = 0x17,
    RxThreshold = 0x18,
    Demod = 0x19,
    MfTx = 0x1c,
    MfRx = 0x1d,
    SerialSpeed = 0x1f,
    CRCResult_1 = 0x21,
    CRCResult_2 = 0x22,
    ModWidth = 0x24,
    RFCfg = 0x26,
    GsN = 0x27,
    CWGsP = 0x28,
    ModGsP = 0x29,
    TMode = 0x2a,
    TPrescaler = 0x2b,
    TReload1 = 0x2c,
    TReload2 = 0x2d,
    TCounterVal_1 = 0x2e,
    TCounterVal_2 = 0x2f,
    TestSel1 = 0x31,
    TestSel2 = 0x32,
    TestPinEn = 0x33,
    TestPinValue = 0x34,
    TestBus = 0x35,
    AutoTest = 0x36,
    Version = 0x37,
    AnalogTest = 0x38,
    TestDAC1 = 0x39,
    TestDAC2 = 0x3a,
    TestADC = 0x3b
  };

  enum Command {
    Idle = 0x00,
    Mem = 0x01,
    GenerateRandomID = 0x02,
    CalcCRC = 0x03,
    Transmit = 0x04,
    NoCmdChange = 0x07,
    Receive = 0x08,
    Transceive = 0x0c,
    MFAuthent = 0x0e,
    SoftReset = 0x0f
  };

  MFRC522(uint8_t sck, uint8_t mosi, uint8_t miso, uint8_t cs, uint8_t rst, spi_inst_t* spi);
  void init();
  void write_register(Register reg, uint8_t byte);
  void write_register(Register reg, std::span<const uint8_t> bytes);
  uint8_t read_register(Register reg);
  std::vector<uint8_t> read_register(Register reg, size_t size);
  void chip_select(bool value);
  void version();
  bool self_test();
  void toggle_antenna(bool value);
  uint8_t antenna_gain();
  uint8_t antenna_gain(uint8_t value);
};

const std::array<uint8_t, 64> mfrc522_v2_test_buf{
    {0x00, 0xEB, 0x66, 0xBA, 0x57, 0xBF, 0x23, 0x95, 0xD0, 0xE3, 0x0D, 0x3D, 0x27, 0x89, 0x5C, 0xDE,
     0x9D, 0x3B, 0xA7, 0x00, 0x21, 0x5B, 0x89, 0x82, 0x51, 0x3A, 0xEB, 0x02, 0x0C, 0xA5, 0x00, 0x49,
     0x7C, 0x84, 0x4D, 0xB3, 0xCC, 0xD2, 0x1B, 0x81, 0x5D, 0x48, 0x76, 0xD5, 0x71, 0x61, 0x21, 0xA9,
     0x86, 0x96, 0x83, 0x38, 0xCF, 0x9D, 0x5B, 0x6D, 0xDC, 0x15, 0xBA, 0x3E, 0x7D, 0x95, 0x3B, 0x2F}};