#ifndef RAGWODSPECIFICDOWNLINK_H_RTIHU
#define RAGWODSPECIFICDOWNLINK_H_RTIHU
//Created by GenFlightStruct.py from file DownlinkSpecGolf-T.xlsx
// Do not edit this file.
#ifndef RTIHU
#error Wrong Archtecture
#endif
typedef struct  __attribute__((__packed__)) _ragWodSpecific_t {
    uint32_t WODTimestampUptime;       //Offset=672
    uint16_t WODTimestampReset;       //Offset=704
    uint8_t WodCRCError;       //Offset=720
    uint8_t pad241;       //Offset=728
} ragWodSpecific_t; // Total Size=736 bits or 92 bytes with 0 left over
#endif
