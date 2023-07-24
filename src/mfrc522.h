#pragma once
#include "hardware/spi.h"
#include <stdint.h>

typedef struct mfrc522 {
  int32_t sck;
  int32_t mosi;
  int32_t miso;
  int32_t cs;
  spi_inst_t *spi;
} mfrc522;

typedef enum mfrc522_register {
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
} mfrc522_register;

typedef enum mfrc522_command {
  Idle = 0x0,
  Mem = 0x1,
  GenerateRandomID = 0x2,
  CalcCRC = 0x3,
  Transmit = 0x4,
  NoCmdChange = 0x7,
  Receive = 0x8,
  Transceive = 0xc,
  MFAuthent = 0xe,
  SoftReset = 0xf
} mfrc522_command;
