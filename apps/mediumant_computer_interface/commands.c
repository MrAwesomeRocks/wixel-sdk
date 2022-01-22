#include <radio_com.h>
#include <stdio.h>
#include <usb_com.h>

#include "commands.h"
#include "errors.h"
#include "robot_control.h"

/* GLOBAL VARIABLES ***********************************************************/

/** The binary command byte received from the computer. */
uint8 commandByte;

/** The binary data bytes received from the computer. */
uint8 dataBytes[32];

/** The number of data bytes we are waiting for to complete the current command.
 * If this is zero, it means we are not currently in the middle of processing
 * a binary command. */
uint8 dataBytesLeft = 0;

/** The number of data bytes received so far for the current binary command.
 * If dataBytesLeft == 0, this is undefined. */
uint8 dataBytesReceived;

/* FUNCTIONS ******************************************************************/

/** Execute a command received from the radio. */
void executeCommand()
{
    switch (commandByte) {

        case COMMAND_STOP:
            clearRobotCommandBuffer();
            stopRobot();
            break;

        case COMMAND_FORWARD:
        case COMMAND_FORWARD_RIGHT:
        case COMMAND_RIGHT:
        case COMMAND_BACKWARDS_RIGHT:
        case COMMAND_BACKWARDS:
        case COMMAND_BACKWARDS_LEFT:
        case COMMAND_LEFT:
        case COMMAND_FORWARDS_LEFT:
            setDirection(commandByte - 0x80);
            break;

        case COMMAND_SPEED:
            setSpeed(dataBytes[0]);

        case COMMAND_ECHO:
            printf("Echo: 0x%X", dataBytes[0]);
            break;

        case COMMAND_MOVE_LEGS:
            clearRobotCommandBuffer(); // Probably a good idea to get instant
            stopRobot();               // feedback

            putRobotCommand(0x81);
            putRobotCommand(dataBytes[0]);
            for (uint8 i = 0; i < dataBytes[0]; i++) {
                // Reconstruct the position
                uint8 pos = (i * 3) + 1;

                putRobotCommand(dataBytes[pos]);     // Leg number
                putRobotCommand(dataBytes[pos + 1]); // High bits
                putRobotCommand(dataBytes[pos + 2]); // Low bits
            }
            break;

        case COMMAND_GET_LEGS_POSITION:
        case COMMAND_IS_MOVING:
            // Not urgent, just dump the command in
            putRobotCommand(commandByte - 0x20);
            break;
    }
}

void processByte(uint8 byteReceived)
{
    if (byteReceived & 0x80) {
        // Command byte

        if (dataBytesLeft > 0) errorOccurred(0x10);

        commandByte = byteReceived;
        dataBytesReceived = 0;
        dataBytesLeft = 0;

        // Look at the command byte to see if it is valid and how many bytes to
        // expect
        switch (commandByte) {
            case COMMAND_SPEED:
            case COMMAND_MOVE_LEGS:
            case COMMAND_GET_LEGS_POSITION:
            case COMMAND_ECHO:
                dataBytesLeft = 1;
                break;

            case COMMAND_STOP:
            case COMMAND_FORWARD:
            case COMMAND_FORWARD_RIGHT:
            case COMMAND_RIGHT:
            case COMMAND_BACKWARDS_RIGHT:
            case COMMAND_BACKWARDS:
            case COMMAND_BACKWARDS_LEFT:
            case COMMAND_LEFT:
            case COMMAND_FORWARDS_LEFT:
            case COMMAND_IS_MOVING:
                dataBytesLeft = 0;
                break;

            default:
                // Invalid command byte
                errorOccurred(0x11);
                break;
        }

        if (dataBytesLeft == 0) {
            // Single byte command
            executeCommand();
        }
    } else if (dataBytesLeft > 0) {
        // Received a parameter byte
        dataBytes[dataBytesReceived] = byteReceived;
        dataBytesReceived++;

        if (commandByte == COMMAND_MOVE_LEGS && dataBytesReceived == 1) {
            // Second parameter is the number of bytes left
            dataBytesLeft = byteReceived * 3;
        } else {
            dataBytesLeft--;
        }

        if (dataBytesLeft == 0) {
            // Received last byte
            executeCommand();
        }
    } else {
        // Byte that's not part of a command or a data byte, ERROR!
        errorOccurred(0x12);
    }
}

void usbCommandService()
{
    while (usbComRxAvailable()) {
        processByte(usbComRxReceiveByte());
    }
}
