#pragma once

#include <cstdint>

extern "C" {
  #include "hardware/spi.h"
}

class MFRC522 {
  public:
    uint8_t sck;
    uint8_t mosi;
    uint8_t miso;
    uint8_t cs;
    uint8_t rst;
    spi_inst_t *spi;

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

    MFRC522(uint8_t sck, uint8_t mosi, uint8_t miso, uint8_t cs, uint8_t rst, spi_inst_t *spi);
    void init();
    void write_register(Register reg, uint8_t val);
    void write_register(Register reg, uint8_t *val);
    uint8_t read_register(Register reg);
    void chip_select(bool value);
    void self_test();
};