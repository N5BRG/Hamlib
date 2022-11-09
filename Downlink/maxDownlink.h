#ifndef MAXDOWNLINK_H_RTIHU
#define MAXDOWNLINK_H_RTIHU
//Created by GenFlightStruct.py from file DownlinkSpecGolf-T.xlsx
// Do not edit this file.
#ifndef RTIHU
#error Wrong Archtecture
#endif
typedef struct  __attribute__((__packed__)) _maxSpecific_t {
    uint32_t maxTimestampUptime;       //Offset=512
    uint16_t maxTimestampEpoch;       //Offset=544
    uint8_t maxCRCerror;       //Offset=560
    uint8_t pad0;       //Offset=568
} maxSpecific_t; // Total Size=576 bits or 72 bytes with 0 left over
#endif
