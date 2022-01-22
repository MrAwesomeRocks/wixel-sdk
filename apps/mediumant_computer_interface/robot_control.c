#include "robot_control.h"

/* GLOBAL VARIABLES **********************************************************/

/** A big buffer of commands to send to the robot controller. */
uint8 XDATA robotCommandBuffer[256];

/** The length of the servo command buffer. */
uint8 DATA robotCommandLength = 0;

/** The number of command bytes sent to the servo controller already. */
uint8 DATA robotCommandBytesSent = 0;

/** The current speed of the robot. */
uint8 robotSpeed = 10;

/** The direction the robot is going.
 *
 * 0 = stop
 * 1 = forward
 * 2 = forward-right
 * 3 = right
 * 4 = backwards-right
 * 5 = backwards
 * 6 = backwards-left
 * 7 = left
 * 8 = forwards-left
 */
uint8 robotDirection = 0;

/* FUNCTIONS *****************************************************************/

int putRobotCommand(int c)
{
    robotCommandBuffer[robotCommandLength] = c;
    robotCommandLength++;
    return (uint8)c;
}

inline void clearRobotCommandBuffer()
{
    robotCommandLength = 0;
}

void setSpeed(uint8 speed)
{
    robotSpeed = speed > 10 ? 10 : speed;
}

void setDirection(uint8 direction)
{
    robotDirection = direction > 8 ? 0 : direction;
}

inline void stopRobot()
{
    setDirection(0);
    putRobotCommand(0xAA);
}
