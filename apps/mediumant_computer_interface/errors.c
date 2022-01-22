#include "errors.h"
#include "reports.h"

#include <radio_com.h>
#include <radio_link.h>

/** Set to 1 if a framing error has occurred to detect error states. */
BIT framingErrorActive = 0;

BIT errorOccurredRecently = 0;
uint8 lastErrorTime;
BIT uartRxDisabled = 0;

void errorOccurred(uint8 code)
{
    lastErrorTime = (uint8)getMs();
    errorOccurredRecently = 1;

    reportError(code);
}

void errorService()
{
    // TODO: move errorOccurredRecently here
}
