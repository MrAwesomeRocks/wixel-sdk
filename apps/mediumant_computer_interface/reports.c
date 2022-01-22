#include "reports.h"
#include "ansi.h"

#include <stdio.h>
#include <usb_com.h>

/* VARIABLES ******************************************************************/

// A big buffer for holding a report.  This allows us to print more than
// 128 bytes at a time to USB.
uint8 XDATA report[1024];

// The length (in bytes) of the report currently in the report buffer.
// If zero, then there is no report in the buffer.
uint16 DATA reportLength = 0;

// The number of bytes of the current report that have already been
// send to the computer over USB.
uint16 DATA reportBytesSent = 0;

/* FUNCTIONS ******************************************************************/

// This gets called by puts, printf, and printBar to populate
// the report buffer.  The result is sent to USB later.
#ifdef OLD_PUTCHAR
void putchar(char c)
{
    report[reportLength] = c;
    reportLength++;
}
#else
int putchar(int c)
{
    report[reportLength] = c;
    reportLength++;
    return (uint8)c;
}
#endif

void reportError(enum errorCode code)
{
    if (param_terminal_colors) {
        printf(B_RED WHT_B "ERROR!" RESET " Code 0x%X.\a\n\r", code);
    } else {
        printf("ERROR! Code 0x%X.\a\n\r", code);
    }

    switch (code) {
        case ERROR_UART_OVERFLOW:
            printf("UART overflow!\n");
            break;

        case ERROR_UART_FRAMING:
        case ERROR_UART_FRAMING_SEND:
            printf("UART framing error!\n");
            break;

        case ERROR_COMMAND_BYTE_IN_DATA:
            printf("Command byte in data!\n");
            break;

        case ERROR_COMMAND_BYTE_INVALID:
            printf("Invalid command byte!\n");
            break;

        case ERROR_DATA_BYTE_INVALID:
            printf("Invalid data byte!\n");
            break;

        case ERROR_LEG_NO_OUT_OF_RANGE:
            printf("Leg number out of range!\n");
            break;

        default:
            printf("Uknown error!\n");
            break;
    }
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
            reportLength = 0;
        } else {
            usbComTxSend(report + reportBytesSent, bytesToSend);
            reportBytesSent += bytesToSend;
        }
    }
}
