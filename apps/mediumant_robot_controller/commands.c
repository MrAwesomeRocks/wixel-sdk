#include <radio_com.h>
#include <uart1.h>

#include "commands.h"
#include "errors.h"
#include "reports.h"
#include "servo.h"

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
        case COMMAND_MOVE_LEGS:
            for (uint8 i = 1; i < dataBytes[0]; i += 2) {
                // Reconstruct the position
                uint16 legPos = (dataBytes[i + 1] << 7) | dataBytes[i + 2];

                setLegPosition(dataBytes[i], legPos);
            }
            break;

        case COMMAND_GET_LEGS_POSITION:
            reportServoPositions(dataBytes[0]);
            break;

        case COMMAND_STOP:
            for (uint8 i = 0; i < 6; ++i) {
                setLegPosition(i, getLegPosition(i));
            }
            clearServoCommandBuffer();
            break;
    }
}

void processByte(uint8 byteReceived)
{
    if (byteReceived & 0x80) {
        // Command byte

        if (dataBytesLeft > 0) errorOccurred();

        commandByte = byteReceived;
        dataBytesReceived = 0;
        dataBytesLeft = 0;

        // Look at the command byte to see if it is valid and how many bytes to
        // expect
        switch (commandByte) {
            case COMMAND_MOVE_LEGS:
            case COMMAND_GET_LEGS_POSITION:
                dataBytesLeft = 1;
                break;

            case COMMAND_STOP:
                dataBytesLeft = 0;
                break;

            default:
                // Invalid command byte
                errorOccurred();
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

        if (commandByte == COMMAND_MOVE_LEGS) {
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
        errorOccurred();
    }
}

void radioCommandService()
{
    while (radioComRxAvailable() && uart1TxAvailable()) {
        processByte(radioComRxReceiveByte());
    }
}
