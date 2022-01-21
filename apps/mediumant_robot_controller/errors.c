#include "errors.h"
#include "commands.h"
#include "reports.h"

#include <radio_com.h>
#include <radio_link.h>
#include <uart1.h>

/** Set to 1 if a framing error has occurred to detect error states. */
BIT framingErrorActive = 0;

BIT errorOccurredRecently = 0;
uint8 lastErrorTime;
BIT uartRxDisabled = 0;

void errorOccurred(uint8 code)
{
    lastErrorTime = (uint8)getMs();
    errorOccurredRecently = 1;
    reportByte(ERROR_RESPONSE, code);
}

void errorService()
{
    static uint8 lastRxLowTime;

    if (uart1RxBufferFullOccurred) {
        uart1RxBufferFullOccurred = 0;
        errorOccurred(0x01);
    }

    if (uart1RxFramingErrorOccurred) {
        uart1RxFramingErrorOccurred = 0;

        // A framing error occurred.
        framingErrorActive = 1;
        errorOccurred(0x02);

        if (param_framing_error_ms > 0) {
            // Disable the UART's receiver.
            U1CSR &= ~0x40; // U1CSR.RE = 0.  Disables reception of bytes on the
                            // UART.
            uartRxDisabled = 1;
            lastRxLowTime = (uint8)getMs(); // Initialize lastRxLowTime even if
                                            // the line isn't low right now.
        }
    }

    if (framingErrorActive) {
        if (!isPinHigh(17)) {
            errorOccurred(0x03);
        } else {
            framingErrorActive = 0;
        }
    }

    if (uartRxDisabled) {
        if (!isPinHigh(17)) {
            // The line is low.
            lastRxLowTime = (uint8)getMs();
        } else if ((uint8)(getMs() - lastRxLowTime) > param_framing_error_ms) {
            // The line has been high for long enough, so re-enable the
            // receiver.
            U1CSR |= 0x40;
            uartRxDisabled = 0;
        }
    }
}
