#include <wixel.h>

#ifndef __ROBOT_STATUS_H__
#define __ROBOT_STATUS_H__

// Status prefixes
#define STATUS_LEG_POSITIONS 0xC2
#define STATUS_IS_MOVING 0xC3

#define STATUS_ECHO 0xF0
#define STATUS_PRINT 0xF1
#define STATUS_ERROR 0xFF

/** Receives information from the robot. Should be called often. */
void robotStatusService();

#endif // __ROBOT_STATUS_H__
