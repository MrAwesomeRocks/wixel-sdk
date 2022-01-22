#include "wixel.h"

#ifndef __ERRORS_H__
#define __ERRORS_H__

/** Call to indicate that an error has occurred.
 *
 * Pass in a code to log to the radio.
 */
void errorOccurred(uint8 code);

/** Call in the main loop to process errors. */
void errorService();

/** Set to one if an error has occurred within the last 100 ms. */
extern BIT errorOccurredRecently;

/** The last time the Wixel encountered a serial communication error. */
extern uint8 lastErrorTime;

/** Approximate number of milliseconds to disable UART's receiver for after a
 * framing error is encountered.
 * Valid values are 0-250.
 * A value of 0 disables the feature (the UART's receiver will not be disabled).
 * The actual number of milliseconds that the receiver is disabled for will be
 * between param_framing_error_ms and param_framing_error_ms + 1.
 */
extern int32 CODE param_framing_error_ms;

/** This bit is 1 if the UART's receiver has been disabled due to a framing
 * error. This bit should be equal to !U1CSR.RE, but we need this variable
 * because we don't want to be reading U1CSR in the main loop, because reading
 * it might cause the FE or ERR bits to be cleared and then the ISR would not
 * receive notice of those errors.
 */
extern BIT uartRxDisabled;

// Error codes
enum errorCode {
    // UART
    ERROR_UART_OVERFLOW = 0x01,
    ERROR_UART_FRAMING = 0x02,
    ERROR_UART_FRAMING_SEND = 0x03,

    // Command
    ERROR_COMMAND_BYTE_IN_DATA = 0x10,
    ERROR_COMMAND_BYTE_INVALID = 0x11,
    ERROR_COMMAND_DATA_BYTE_INVALID = 0x12,
    ERROR_LEG_NO_OUT_OF_RANGE = 0x13,

    // Status
    ERROR_STATUS_BYTE_IN_DATA = 0x20,
    ERROR_INVALID_STATUS_BYTE = 0x21,
    ERROR_STATUS_DATA_BYTE_INVALID = 0x22,
};

#endif //__ERRORS_H__
