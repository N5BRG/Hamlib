#ifndef RT1ERRORSDOWNLINK_H_RTIHU
#define RT1ERRORSDOWNLINK_H_RTIHU
//Created by GenFlightStruct.py from file DownlinkSpecGolf-T.xlsx
// Do not edit this file.
#ifndef RTIHU
#error Wrong Archtecture
#endif
typedef struct  __attribute__((__packed__)) _rt1Errors_t {
    unsigned int wdReports:9;       //Offset=256
    unsigned int errorCode:5;       //Offset=265
    unsigned int taskNumber:4;       //Offset=270
    unsigned int previousTask:4;       //Offset=274
    unsigned int earlyResetCount:3;       //Offset=278
    unsigned int wasStillEarlyInBoot:1;       //Offset=281
    unsigned int valid:1;       //Offset=282
    unsigned int pad316:5;       //Offset=283
    uint32_t errorData;       //Offset=288
    uint32_t RAMCorAddr1;       //Offset=320
    uint32_t RAMCorAddr2;       //Offset=352
    uint32_t ROMCorAddr1;       //Offset=384
    uint32_t ROMCorAddr2;       //Offset=416
    uint8_t DCTCmdFailCRCCnt;       //Offset=448
    uint8_t DCTCmdFailTimeCnt;       //Offset=456
    uint8_t DCTCmdFailAuthenticateCnt;       //Offset=464
    uint8_t DCTCmdFailCommandCnt;       //Offset=472
    uint8_t DCTCmdFailNamespaceCnt;       //Offset=480
    uint8_t I2C1ErrorCnt;       //Offset=488
    uint8_t I2C2ErrorCnt;       //Offset=496
    uint8_t I2C1ResetCnt;       //Offset=504
    uint8_t I2C2ResetCnt;       //Offset=512
    uint8_t I2C1RetryCnt;       //Offset=520
    uint8_t I2C2RetryCnt;       //Offset=528
    uint8_t SPIRetryCnt;       //Offset=536
    uint8_t MramCRCCnt;       //Offset=544
    uint8_t MramRdErrorCnt;       //Offset=552
    uint8_t MramWtErrorCnt;       //Offset=560
    uint8_t FlashCorCnt;       //Offset=568
    uint8_t RamEvenCorCnt;       //Offset=576
    uint8_t RamOddCorCnt;       //Offset=584
    uint8_t PLLSlipCnt;       //Offset=592
    uint8_t ClockMonitorFailCnt;       //Offset=600
    uint8_t VIMRamParityCnt;       //Offset=608
    uint8_t CAN1RamParityCnt;       //Offset=616
    uint8_t CAN2RamParityCnt;       //Offset=624
    uint8_t DCC1ErrorCnt;       //Offset=632
    uint8_t DCC2ErrorCnt;       //Offset=640
    uint8_t N2HET2RamParityCnt;       //Offset=648
    uint8_t IOMMAccvioCnt;       //Offset=656
    uint8_t nonFatalCnt;       //Offset=664
    uint8_t SWVersion[2];       //Offset=672
    uint16_t pad362;       //Offset=688
} rt1Errors_t; // Total Size=704 bits or 88 bytes with 0 left over
#endif
