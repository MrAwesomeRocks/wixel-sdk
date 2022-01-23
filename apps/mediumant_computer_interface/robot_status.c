#include <radio_com.h>
#include <stdio.h>

#include "ansi.h"
#include "errors.h"
#include "robot_control.h"
#include "robot_status.h"

/* GLOBAL VARIABLES **********************************************************/

/** The binary command byte received from the Wixel. */
static uint8 statusByte;

/** The binary data bytes received from the Wixel. */
static uint8 dataBytes[32];

/** The number of data bytes we are waiting for to complete the current command.
 * If this is zero, it means we are not currently in the middle of processing
 * a binary command. */
static uint8 dataBytesLeft = 0;

/** The number of data bytes received so far for the current binary command.
 * If dataBytesLeft == 0, this is undefined. */
static uint8 dataBytesReceived;

/* FUNCTIONS *****************************************************************/

/** Process data received from the radio. */
static void processStatus()
{
    switch (statusByte) {
        case STATUS_LEG_POSITIONS:
            printf("Leg positions:" LF);
            for (uint8 i = 0; i < dataBytes[0]; i++) {
                // Reconstruct the position
                uint8 pos = (i * 3) + 1;
                uint16 legPos = (dataBytes[pos + 1] << 7) | dataBytes[pos + 2];

                printf("\tLeg %d: %d deg" LF, dataBytes[pos], legPos);
            }
            break;

        case STATUS_IS_MOVING:
            setIsMoving(dataBytes[0]);
            printf("Moving: %d" LF, dataBytes[0]);
            break;

        case STATUS_ECHO:
            printf("Robot Echo: 0x%X" LF, dataBytes[0]);
            break;

        case STATUS_ERROR:
            errorOccurred(dataBytes[0]);
            break;
    }
}

static void processByte(uint8 byteReceived)
{
    static BIT waitingForPrintData;

    if (waitingForPrintData) {
        // Just got our print data
        printf("Robot: 0x%X" LF, byteReceived);
        waitingForPrintData = 0;
    } else {
        if (byteReceived == STATUS_PRINT) {
            waitingForPrintData = 1;
        } else if (byteReceived & 0x80) {
            // Status byte

            if (dataBytesLeft > 0) errorOccurred(ERROR_STATUS_BYTE_IN_DATA);

            statusByte = byteReceived;
            dataBytesReceived = 0;
            dataBytesLeft = 0;

            // Look at the command byte to see if it is valid and how many bytes
            // to expect
            switch (statusByte) {
                case STATUS_LEG_POSITIONS:
                case STATUS_IS_MOVING:
                case STATUS_ECHO:
                case STATUS_PRINT:
                case STATUS_ERROR:
                    dataBytesLeft = 1;
                    break;

                default:
                    errorOccurred(ERROR_INVALID_STATUS_BYTE);
                    break;
            }

            if (dataBytesLeft == 0) {
                // Single byte command
                processStatus();
            }
        } else if (dataBytesLeft > 0) {
            // Received a parameter byte
            dataBytes[dataBytesReceived] = byteReceived;
            dataBytesReceived++;

            if (statusByte == STATUS_LEG_POSITIONS && dataBytesReceived == 1) {
                // Second parameter is the number of bytes left
                dataBytesLeft = byteReceived * 3;
            } else {
                dataBytesLeft--;
            }

            if (dataBytesLeft == 0) {
                // Received last byte
                processStatus();
            }
        } else {
            // Byte that's not part of a status or a data byte, ERROR!
            errorOccurred(ERROR_INVALID_STATUS_BYTE);
        }
    }
}

void robotStatusService()
{
    while (radioComRxAvailable()) {
        processByte(radioComRxReceiveByte());
    }
}
