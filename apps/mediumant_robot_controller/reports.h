#include "servo.h"
#include "wixel.h"

#ifndef __REPORTS_H__
#define __REPORTS_H__

/** Send a radio report of each of the servo positions of each bit enabled
 * in bits 0:5 of legs. */
void reportServoPositions(uint8 legs);

/** Send one byte to the radio.
 *
 * Can be prefixed by specifying the `prefix` variable.
 * If prefix is less than 0x7f, no prefix will be set.
 */
void reportByte(uint8 prefix, uint8 byte);

/** Send any outstanding reports. Should be called often. */
void reportsService();

#endif
