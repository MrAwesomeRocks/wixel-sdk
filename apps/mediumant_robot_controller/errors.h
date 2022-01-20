#include "wixel.h"

#ifndef __ERRORS_H__
#define __ERRORS_H__

/** Call to indicate that an error has occurred */
void errorOccurred();

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

#endif
