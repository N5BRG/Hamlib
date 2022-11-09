/*
 * TMS570Hardware.h
 *
 *  Created on: Jun 1, 2019
 *      Author: bfisher
 */

#ifndef DRIVERS_INC_TMS570HARDWARE_H_
#define DRIVERS_INC_TMS570HARDWARE_H_
#include <pacsat.h>

/*
 * This module contains routines that talk directly to the TMS570 but which are not
 * generated by or related to HalCoGen.
 */

/*
 * The code that raises privilege is located in FreeRTOS' port asm module.  FreeRTOS does not use it
 * but we will occasionally do that here.
 */

void ProcessorReset(void);
#define RESET_POWER_ON 0x8000
#define RESET_OSCRST 0x4000
#define RESET_WATCHDOG 0x2000
#define RESET_CPURESET 0x20
#define RESET_SOFTWARE 0x10
#define RESET_EXTERNAL 0x8

void ScrubECCMemory(uint64_t *start,uint32_t length);


/*
 * ESM Error Channels
 */

typedef enum _ESM1Chans_ {
     Reserved0 = 0
    ,MibADC2RamParity
    ,DMaMPuConfigViolation
    ,DMaControlPacketParity
    ,Reserved4 = 4
    ,DMAReadErr
    ,FlashEccCorrectable
    ,N2HET1RamParity
    ,HETTU12ControlPacketParity
    ,HETTU12MPUConfigViolation
    ,PLL1Slip
    ,ClockMonitorOscFail
    ,Reserved12 = 12
    ,DMAWriteError
    ,Reserved14 = 14
    ,VIMRamParity
    ,Reseved16=16
    ,MibSPI1RamParity
    ,MibSPI3RamParity
    ,MibADC1RamParity
    ,Reserved20=20
    ,CAN1RamParity
    ,CAN3RamParity
    ,CAN2RamParity
    ,MibSPI5RamParity
    ,Reserved25 = 25
    ,RAMEvenBankCorrectableECC
    ,CPUSelfTestFail
    ,RAMOddBankCorrectableECC
    ,Reserved29 = 29
    ,DCC1Error
    ,CCMR4SelfTestFail
    ,Reserved32 = 32
    ,Reserved33 = 33
    ,N2HET2RamParity
    ,FlashCorrectableEccBank7
    ,FlashUnorrectableEccBank7
    ,IOMMAccvio
    ,PowerCompareError
    ,PowerSelftestError
    ,eFuseControlError
    ,eFuseSelfTestError
    ,DCC2Error = 62
    ,EndOfGroup1Errors
}ESMGroup1Channel;
typedef enum _ESM2Chans_ {
     DualCPULockStepError=2
    ,FlashUncorrAddrError=4
    ,RAMEvenBankUncorrAddressDecode=6
    ,RAMOddBankUncorrAddressDecode=8
    ,RAMEvenBankAddressParity=10
    ,RAMOddBankAddressParity=12
    ,ECCLiveLockDetect = 16
    ,WWDViolation = 24
} ESMGroup2Channel;
typedef enum _ESM3Chans_ {
     eFuseAutoloadError=1
    ,RAMEvenBankECCUncorrError=3
    ,RAMOddBankECCUncorrError=5
    ,FlashECCUncorrError=7
} ESMGroup3Channel;

#endif /* DRIVERS_INC_TMS570HARDWARE_H_ */
