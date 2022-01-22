#include <wixel.h>

#ifndef __ROBOT_CONTROL_H__
#define __ROBOT_CONTROL_H__

/** Send a command to the robot.
 *
 * DANGER: This lets you send bytes directly to the robot. BE CAREFUL!
 */
int putRobotCommand(int c);

/** Clear the robot's command buffer. */
void clearRobotCommandBuffer();

/** Stop the robot.
 *
 * May be useful to call clearRobotCommandBuffer() before this.
 */
void stopRobot();

/** Set the robot's speed. */
void setSpeed(uint8 speed);

/** Set the robot's direction. */
void setDirection(uint8 direction);

/** Controlls the robot based on speed. Should be called often. */
void robotControlService();

#endif // __ROBOT_CONTROL_H__
