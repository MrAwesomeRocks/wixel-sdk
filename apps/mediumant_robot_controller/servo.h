#include <wixel.h>

#ifndef __SERVO_H__
#define __SERVO_H__

// Legs of the robot
enum robotLeg {
    LEG_RIGHT_FRONT,
    LEG_RIGHT_MIDDLE,
    LEG_RIGHT_BACK,
    LEG_LEFT_FRONT,
    LEG_LEFT_MIDDLE,
    LEG_LEFT_BACK
};

/** Set the leg position of one of the robot's legs */
void setLegPosition(enum robotLeg leg, uint16 degrees);

/** Get the leg position of one of the robot's legs */
uint16 getLegPosition(enum robotLeg leg);

/** Clear the servo command buffer. */
inline void clearServoCommandBuffer();

/** Stop all the servos.
 *
 * Should probably also call clearServoCommandBuffer() before this.
 */
inline void stopAllServos();

/** Renable the servos. */
inline void enableServos();

/** Update the servos. Should be called often. */
void servoService();

/** How often to update the servos (in milleseconds). */
extern int32 CODE param_servo_update_ms;
#endif
