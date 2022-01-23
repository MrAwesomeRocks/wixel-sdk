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

/** Tell the robot controller about the moving state of the robot. */
void setIsMoving(uint8 isMoving);

/** Controlls the robot based on speed and direction. Should be called often. */
void robotControlService();

/** How often to update the robot (in milleseconds). */
extern int32 CODE param_robot_update_ms;

#endif // __ROBOT_CONTROL_H__
