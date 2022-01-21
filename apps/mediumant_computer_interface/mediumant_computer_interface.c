/* wireless_serial app:
 * This app allows you to connect two Wixels together to make a wireless,
 * bidirectional, lossless serial link.
 * See description.txt or the Wixel User's Guide for more information.
 */

/*
 * TODO: To avoid damage, don't enable nDTR and nRTS outputs by default.
 * TODO: use LEDs to give feedback about sending/receiving bytes.
 * TODO: UART flow control.
 * TODO: Obey CDC-ACM Set Line Coding commands:
 *       In USB-RADIO mode, bauds 0-255 would correspond to radio channels.
 * TODO: shut down radio when we are in a different serial mode
 * TODO: make the heartbeat blinks on the Wixels be synchronized (will require
 *       major changes to the radio_link library)
 * TODO: turn on red LED or flash it if the Wixel is in a mode that requires USB
 *       but has not reached the USB Configured State (this avoids the problem
 * of having 0 LEDs on when the Wixel is in USB-UART mode and self powered)
 */

/** Dependencies **************************************************************/
#include <wixel.h>

#include <usb.h>
#include <usb_com.h>

#include <radio_com.h>
#include <radio_link.h>

#include <uart1.h>

#include "errors.h"

/** Parameters ****************************************************************/

int32 CODE param_framing_error_ms = 0;

/** Global Variables **********************************************************/

/** Functions *****************************************************************/

void updateLeds()
{
    static BIT dimYellowLed = 0;
    static uint16 lastRadioActivityTime;
    uint16 now;

    usbShowStatusWithGreenLed();

    now = (uint16)getMs();

    if (!radioLinkConnected()) {
        // We have not connected to another device wirelessly yet, so do a
        // 50% blink with a period of 1024 ms.
        LED_YELLOW(now & 0x200 ? 1 : 0);
    } else {
        // We have connected.

        if ((now & 0x3FF) <= 20) {
            // Do a heartbeat every 1024ms for 21ms.
            LED_YELLOW(1);
        } else if (dimYellowLed) {
            static uint8 DATA count;
            count++;
            LED_YELLOW((count & 0x7) == 0);
        } else {
            LED_YELLOW(0);
        }
    }

    if (radioLinkActivityOccurred) {
        radioLinkActivityOccurred = 0;
        dimYellowLed ^= 1;
        // dimYellowLed = 1;
        lastRadioActivityTime = now;
    }

    if ((uint16)(now - lastRadioActivityTime) > 32) { dimYellowLed = 0; }

    if ((uint8)(now - lastErrorTime) > 100) { errorOccurredRecently = 0; }

    LED_RED(errorOccurredRecently || uartRxDisabled);
}


void usbToRadioService()
{
    // Data
    while (usbComRxAvailable() && radioComTxAvailable()) {
        radioComTxSendByte(usbComRxReceiveByte());
    }

    while (radioComRxAvailable() && usbComTxAvailable()) {
        usbComTxSendByte(radioComRxReceiveByte());
    }
}

void main()
{
    systemInit();

    // ioTxSignals(0);

    usbInit();

    // radioComRxEnforceOrdering = 1;
    radioComInit();

    while (1) {
        boardService();
        updateLeds();
        errorService();

        radioComTxService();
        usbComService();

        usbToRadioService();
    }
}
