#ifndef RTERRORSDOWNLINK_H_RTIHU
#define RTERRORSDOWNLINK_H_RTIHU
//Created by GenFlightStruct.py from file DownlinkSpecGolf-T.xlsx
// Do not edit this file.
#ifndef RTIHU
#error Wrong Archtecture
#endif
typedef struct  __attribute__((__packed__)) _rtErrors_t {
    uint32_t RT1HardError;       //Offset=0
    uint32_t RT1RAMCorAddr1;       //Offset=32
    uint32_t RT1RAMCorAddr2;       //Offset=64
    uint32_t RT1ROMCorAddr1;       //Offset=96
    uint32_t RT1ROMCorAddr2;       //Offset=128
    uint8_t DCT1CmdFailCRC;       //Offset=160
    uint8_t DCT1CmdFailTime;       //Offset=168
    uint8_t DCT1CmdFailAuthenticate;       //Offset=176
    unsigned int DCT1CmdFailCommand:4;       //Offset=184
    unsigned int DCT1CmdFailNamespace:4;       //Offset=188
    unsigned int RT1I2C1Error:4;       //Offset=192
    unsigned int RT1I2C2Error:4;       //Offset=196
    unsigned int RT1I2C1Reset:4;       //Offset=200
    unsigned int RT1I2C2Reset:4;       //Offset=204
    uint8_t RT1SPIRetries;       //Offset=208
    uint8_t RT1MramCRC;       //Offset=216
    uint8_t RT1FlashCorCnt;       //Offset=224
    uint8_t RT1RamEvenCorCnt;       //Offset=232
    uint8_t RT1RamOddCorCnt;       //Offset=240
    unsigned int RT1PLLSlip:4;       //Offset=248
    unsigned int RT1ClockMonitorFail:4;       //Offset=252
    unsigned int RT1VIMRamParity:4;       //Offset=256
    unsigned int RT1CAN1RamParity:4;       //Offset=260
    unsigned int RT1CAN2RamParity:4;       //Offset=264
    unsigned int RT1DCC1Error:4;       //Offset=268
    unsigned int RT1DCC2Error:4;       //Offset=272
    unsigned int RT1N2HET2RamParity:4;       //Offset=276
    unsigned int RT1IOMMAccvio:4;       //Offset=280
    unsigned int fill3:4;       //Offset=284
} rtErrors_t; // Total Size=288 bits or 36 bytes with 0 left over
#endif
