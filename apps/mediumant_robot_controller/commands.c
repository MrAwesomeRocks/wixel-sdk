#include "commands.h"
#include "errors.h"

/* COMMANDS *******************************************************************/
#define COMMAND_MOVE_LEG 0x81
#define COMMAND_GET_LEG_POSITION 0x82
#define COMMAND_STOP 0xAA

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
 * If dataBytesLeft==0, this is undefined. */
uint8 dataBytesReceived;

/* FUNCTIONS ******************************************************************/

/** Execute a command received from the radio. */
void executeCommand()
{
    switch (commandByte)
    {
    case COMMAND_MOVE_LEG:
        /* code */
        break;

    case COMMAND_GET_LEG_POSITION:
        break;

    case COMMAND_STOP:
        break;
    }
}

void processByte(uint8 byteReceived)
{
    if (byteReceived & 0x80)
    {
        // Command byte

        if (dataBytesLeft > 0)
        {
            errorOccurred();
        }

        commandByte = byteReceived;
        dataBytesReceived = 0;
        dataBytesLeft = 0;

        // Look at the command byte to see if it is valid and how many bytes to expect
        switch (commandByte)
        {
        case COMMAND_MOVE_LEG:
            dataBytesLeft = 2;
            break;

        case COMMAND_GET_LEG_POSITION:
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

        if (dataBytesLeft == 0)
        {
            // Single byte command
            executeCommand();
        }
    }
    else if (dataBytesLeft > 0)
    {
        // Received a parameter byte
        dataBytes[dataBytesReceived] = byteReceived;
        dataBytesLeft--;
        dataBytesReceived++;

        if (dataBytesLeft == 0)
        {
            // Received last byte
            executeCommand();
        }
    }
    else
    {
        // Byte that's not part of a command or a data byte, ERROR!
        errorOccurred();
    }
}
