#include "ansi.h"
#include "reports.h"

#include <stdio.h>
#include <usb_com.h>

/* VARIABLES ******************************************************************/
#define REPORT_MAX_LEN 1024

// A big buffer for holding a report.  This allows us to print more than
// 128 bytes at a time to USB.
uint8 XDATA report[REPORT_MAX_LEN];

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

// This gets called by puts, printf, and printBar to populate
// the report buffer.  The result is sent to USB later.
#ifdef OLD_PUTCHAR
void putchar(char c)
{
    if (reportLength < REPORT_MAX_LEN - 1) {
        report[reportLength] = c;
        reportLength++;
    } else {
        clearReportBuffer();
        errorOccurred(ERROR_REPORT_BUFFER_OVERFLOW);
    }
}
#else
int putchar(int c)
{
    if (reportLength < REPORT_MAX_LEN - 1) {
        report[reportLength] = c;
        reportLength++;
    } else {
        clearReportBuffer();
        errorOccurred(ERROR_REPORT_BUFFER_OVERFLOW);
    }
    return (uint8)c;
}
#endif

void reportError(enum errorCode code)
{
    if (param_terminal_colors) {
        printf(B_RED WHT_B "ERROR!" RESET " Code 0x%X.\a" LF, code);
    } else {
        printf("ERROR! Code 0x%X.\a" LF, code);
    }

    switch (code) {
        case ERROR_UART_OVERFLOW:
            printf("UART overflow!");
            break;

        case ERROR_UART_FRAMING:
        case ERROR_UART_FRAMING_SEND:
            printf("UART framing error!");
            break;

        case ERROR_COMMAND_BYTE_IN_DATA:
            printf("Command byte in data!");
            break;

        case ERROR_COMMAND_BYTE_INVALID:
            printf("Invalid command byte!");
            break;

        case ERROR_COMMAND_DATA_BYTE_INVALID:
            printf("Invalid command data byte!");
            break;

        case ERROR_LEG_NO_OUT_OF_RANGE:
            printf("Leg number out of range!");
            break;

        case ERROR_STATUS_BYTE_IN_DATA:
            printf("Status byte in data!");
            break;

        case ERROR_INVALID_STATUS_BYTE:
            printf("Invalid status byte!");
            break;

        case ERROR_STATUS_DATA_BYTE_INVALID:
            printf("Invalid status data byte!");
            break;

        case ERROR_REPORT_BUFFER_OVERFLOW:
            printf("Report buffer overflow!");
            break;

        case ERROR_ROBOT_REPORT_BUFFER_OVERFLOW:
            printf("Robot report buffer overflow!");
            break;

        default:
            printf("Unknown error!");
            break;
    }

    printf(LF);
}

void reportsService()
{
    // Send the report to USB in chunks.
    if (reportLength > 0) {
        uint8 bytesToSend = usbComTxAvailable();
        if (bytesToSend > reportLength - reportBytesSent) {
            // Send the last part of the report.
            usbComTxSend(report + reportBytesSent,
                         reportLength - reportBytesSent);
            clearReportBuffer();
        } else {
            usbComTxSend(report + reportBytesSent, bytesToSend);
            reportBytesSent += bytesToSend;
        }
    }
}
