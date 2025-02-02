#include "reports.h"
#include "commands.h"
#include "errors.h"

#include <radio_com.h>
#include <stdarg.h>

/* GLOBAL VARIABLES ***********************************************************/
#define REPORT_MAX_SIZE 512

// A big buffer for holding a report.  This allows us to print more than
// 128 bytes at a time to radio.
uint8 XDATA report[REPORT_MAX_SIZE];

// The length (in bytes) of the report currently in the report buffer.
// If zero, then there is no report in the buffer.
uint16 DATA reportLength = 0;

// The number of bytes of the current report that have already been
// send to the computer over USB.
uint16 DATA reportBytesSent = 0;

/* FUNCTIONS ******************************************************************/

inline void clearReportBuffer()
{
    reportBytesSent = 0;
    reportLength = 0;
}

static int putResponse(int c)
{
    if (reportLength < REPORT_MAX_SIZE - 1) {
        report[reportLength] = c;
        reportLength++;
    } else {
        clearReportBuffer();
        errorOccurred(0x31);
    }
    return (uint8)c;
}

void reportServoPositions(uint8 legs)
{
    putResponse(RESPONSE_COMMAND(COMMAND_GET_LEGS_POSITION));

    // Set this byte later
    uint16 numLegsBytePos = reportLength;
    reportLength++;

    uint8 numLegs = 0;
    for (uint8 i = 0; i < 6; i++) {
        if (legs & (1 << i)) {
            numLegs++;
            uint16 legPosition = getLegPosition(i);

            putResponse(i);                         // Leg number
            putResponse((legPosition >> 7) & 0x7F); // First 7 bits
            putResponse(legPosition & 0x7F);        // Last 7 bits
        }
    }

    report[numLegsBytePos] = numLegs;
}

void reportByte(uint8 prefix, uint8 byte)
{
    if (prefix > 0x7F) putResponse(prefix);
    putResponse(byte);
}

void reportsService()
{
    // Send the report to radio in chunks.
    if (reportLength > 0) {
        while (radioComTxAvailable() && reportBytesSent < reportLength) {
            radioComTxSendByte(report[reportBytesSent]);
            reportBytesSent++;
        }

        if (reportBytesSent == reportLength) {
            // We've sent the whole report
            reportLength = 0;
            reportBytesSent = 0;
        }
    }
}
