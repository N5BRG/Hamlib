#ifndef RT2ERRORSDOWNLINK_H_RTIHU
#define RT2ERRORSDOWNLINK_H_RTIHU
//Created by GenFlightStruct.py from file DownlinkSpecGolf-T.xlsx
// Do not edit this file.
#ifndef RTIHU
#error Wrong Archtecture
#endif
typedef struct  __attribute__((__packed__)) _rt2Errors_t {
    unsigned int RT2wdReports:9;       //Offset=704
    unsigned int RT2errorCode:5;       //Offset=713
    unsigned int RT2taskNumber:4;       //Offset=718
    unsigned int RT2previousTask:4;       //Offset=722
    unsigned int RT2earlyResetCount:3;       //Offset=726
    unsigned int RT2wasStillEarlyInBoot:1;       //Offset=729
    unsigned int RT2Valid:1;       //Offset=730
    unsigned int pad374:5;       //Offset=731
    uint32_t RT2errorData;       //Offset=736
    uint32_t RT2RAMCorAddr1;       //Offset=768
    uint32_t RT2RAMCorAddr2;       //Offset=800
    uint32_t RT2ROMCorAddr1;       //Offset=832
    uint32_t RT2ROMCorAddr2;       //Offset=864
    uint8_t DCT2CmdFailCRCCnt;       //Offset=896
    uint8_t DCT2CmdFailTimeCnt;       //Offset=904
    uint8_t DCT2CmdFailAuthenticateCnt;       //Offset=912
    uint8_t DCT2CmdFailCommandCnt;       //Offset=920
    uint8_t DCT2CmdFailNamespaceCnt;       //Offset=928
    uint8_t RT2I2C1ErrorCnt;       //Offset=936
    uint8_t RT2I2C2ErrorCnt;       //Offset=944
    uint8_t RT2I2C1ResetCnt;       //Offset=952
    uint8_t RT2I2C2ResetCnt;       //Offset=960
    uint8_t RT2I2C1RetryCnt;       //Offset=968
    uint8_t RT2I2C2RetryCnt;       //Offset=976
    uint8_t RT2SPIRetriesCnt;       //Offset=984
    uint8_t RT2MramCRCCnt;       //Offset=992
    uint8_t RT2MramRdErrorCnt;       //Offset=1000
    uint8_t RT2MramWtErrorCnt;       //Offset=1008
    uint8_t RT2FlashCorCnt;       //Offset=1016
    uint8_t RT2RamEvenCorCnt;       //Offset=1024
    uint8_t RT2RamOddCorCnt;       //Offset=1032
    uint8_t RT2PLLSlipCnt;       //Offset=1040
    uint8_t RT2ClockMonitorFailCnt;       //Offset=1048
    uint8_t RT2VIMRamParityCnt;       //Offset=1056
    uint8_t RT2CAN1RamParityCnt;       //Offset=1064
    uint8_t RT2CAN2RamParityCnt;       //Offset=1072
    uint8_t RT2DCC1ErrorCnt;       //Offset=1080
    uint8_t RT2DCC2ErrorCnt;       //Offset=1088
    uint8_t RT2N2HET2RamParityCnt;       //Offset=1096
    uint8_t RT2IOMMAccvioCnt;       //Offset=1104
    uint8_t RT2NonFatalCnt;       //Offset=1112
    uint8_t RT2SWVersion[2];       //Offset=1120
    uint16_t pad420;       //Offset=1136
} rt2Errors_t; // Total Size=1152 bits or 144 bytes with 0 left over
#endif
