/*
 * golf.h
 *
 *  Created on: Feb 26, 2019
 *      Author: burns
 *
 *      This file includes header files and contains overall definitions that are used throughout the rt-ihu code.
 *      Note that configuration information is in config.h, not here.
 *
 */

#ifndef PACSAT_H_
#define PACSAT_H_
#include <stdarg.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "printf.h" //Get the tiny printf stuff
#define TRUE true
#define FALSE false
#include "config.h"
#include "watchdogSupport.h"
#include "gpioDriver.h"

// Let's skip stdio which has a lot of stuff and just define printf

extern int printf(const char *fmt, ...);
extern int sprintf(char* str, const char *fmt, ...);

#define VOID ((void *)0)
#endif /* PACSAT_H_ */
