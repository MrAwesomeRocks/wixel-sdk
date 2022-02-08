#include <radio_com.h>
#include <stdio.h>
#include <wixel.h>

#include "ansi.h"
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

/** Whether the robot says it's currently moving or not. */
BIT robotIsMoving = 0;

/** The sequence of movements for the robot. */
#define SEQUENCE_LENGTH 6
uint16 CODE walkSequence[6][SEQUENCE_LENGTH] = {
    {10, 160, 10, 160, 10, 160},
    {160, 160, 160, 160, 160, 160},
    {160, 280, 160, 280, 160, 280}, {160, 10, 160, 10, 160, 10},
    {160, 160, 160, 160, 160, 160}, {280, 160, 280, 160, 280, 160},
};

/** Where we are in our command sequence. */
uint8 DATA commandSequenceStep = 0;

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
    robotCommandBytesSent = 0;
}

void setSpeed(uint8 speed)
{
    robotSpeed = speed > 10 ? 10 : speed;
}

void setDirection(uint8 direction)
{
    robotDirection = direction > 8 ? 0 : direction;
}

void setIsMoving(uint8 isMoving)
{
    robotIsMoving = isMoving != 0;
}

inline void stopRobot()
{
    setDirection(0);
    putRobotCommand(0xAA);
}

void robotControlService()
{
    static uint32 lastRobotUpdate;
    // static uint8 prevLegPositions[6];

    if (getMs() - lastRobotUpdate >= param_robot_update_ms &&
        robotCommandLength == 0) {
        lastRobotUpdate = getMs();

        if (robotIsMoving) {
            // Poll to see if the robot is still moving
            printf("P" LF);
            putRobotCommand(0x83);
        } else {
            printf("C" LF);
            // Not moving and command buffer empty, send command!
            putRobotCommand(0x81); // Set leg positions
            putRobotCommand(6);    // All 6 legs - TODO OPTIMIZE

            // Set this byte later
            // uint8 numLegsBytePos = robotCommandLength;
            // robotCommandLength++;

            // uint8 numLegsChanged = 0;
            for (uint8 i = 0; i < 6; i++) {
                uint16 legPos = walkSequence[commandSequenceStep][i];
                // if (prevLegPositions[i] != legPos) {
                // Update
                // prevLegPositions[i] = legPos;
                // numLegsChanged++;

                putRobotCommand(i);                    // Leg number
                putRobotCommand((legPos >> 7) & 0x7F); // First 7 bits
                putRobotCommand(legPos & 0x7F);        // Last 7 bits
                // }
            }

            // robotCommandBuffer[numLegsBytePos] = numLegsChanged;
            robotIsMoving = 1;

            commandSequenceStep++;
            if (commandSequenceStep >= SEQUENCE_LENGTH) commandSequenceStep = 0;
            printf("Seq: %d" LF, commandSequenceStep);
        }
    }

    // Send the commands to radio in chunks.
    if (robotCommandLength > 0) {
        while (radioComTxAvailable() &&
               robotCommandBytesSent < robotCommandLength) {
            radioComTxSendByte(robotCommandBuffer[robotCommandBytesSent]);
            robotCommandBytesSent++;
        }

        if (robotCommandBytesSent == robotCommandLength) {
            // We've sent the whole report
            clearRobotCommandBuffer();
        }
    }
}
