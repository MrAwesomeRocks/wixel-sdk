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
 *       but has not reached the USB Configured State (this avoids the problem of
 *       having 0 LEDs on when the Wixel is in USB-UART mode and self powered)
 */

/* DEPENDENCIES ***************************************************************/
#include <wixel.h>

#include <usb.h>
#include <usb_com.h>

#include <radio_com.h>
#include <radio_link.h>

#include <uart1.h>

#include "errors.h"
#include "commands.h"

/* PARAMETERS *****************************************************************/

int32 CODE param_framing_error_ms = 0;

/** The baud rate to run UART 1 at. */
int32 CODE param_baud_rate = 9600;


/* GLOBAL VARIABLES ***********************************************************/

/* FUNCTIONS ******************************************************************/

void updateLeds()
{
    static BIT dimYellowLed = 0;
    static uint16 lastRadioActivityTime;
    uint16 now;

    usbShowStatusWithGreenLed();

    now = (uint16)getMs();

    if (!radioLinkConnected())
    {
        // We have not connected to another device wirelessly yet, so do a
        // 50% blink with a period of 1024 ms.
        LED_YELLOW(now & 0x200 ? 1 : 0);
    }
    else
    {
        // We have connected.

        if ((now & 0x3FF) <= 20)
        {
            // Do a heartbeat every 1024ms for 21ms.
            LED_YELLOW(1);
        }
        else if (dimYellowLed)
        {
            static uint8 DATA count;
            count++;
            LED_YELLOW((count & 0x7) == 0);
        }
        else
        {
            LED_YELLOW(0);
        }
    }

    if (radioLinkActivityOccurred)
    {
        radioLinkActivityOccurred = 0;
        dimYellowLed ^= 1;
        lastRadioActivityTime = now;
    }

    if ((uint16)(now - lastRadioActivityTime) > 32)
    {
        dimYellowLed = 0;
    }

    if ((uint8)(now - lastErrorTime) > 100)
    {
        errorOccurredRecently = 0;
    }

    LED_RED(errorOccurredRecently || uartRxDisabled);
}

/* Returns the logical values of the input control signal pins.
   Bit 0 is DSR.
   Bit 1 is CD. */
uint8 ioRxSignals()
{
    uint8 signals = 0;

    return signals;
}

/* Sets the logical values of the output control signal pins.
   This should be called frequently (not just when the values change).
   Bit 0 is DTR.
   Bit 1 is RTS. */
void ioTxSignals(uint8 signals)
{
}

/** Checks for new bytes from the radio and process them */
void processBytesFromRadio()
{
    while (radioComRxAvailable() && uart1TxAvailable())
    {
        processByte(radioComRxReceiveByte());
    }
}

void uartToRadioService()
{
    // Data
    while (uart1RxAvailable() && radioComTxAvailable())
    {
        radioComTxSendByte(uart1RxReceiveByte());
    }

    while (radioComRxAvailable() && uart1TxAvailable())
    {
        uart1TxSendByte(radioComRxReceiveByte());
    }

    // Control Signals.
    ioTxSignals(radioComRxControlSignals());
    radioComTxControlSignals(ioRxSignals());
}

void main()
{
    systemInit();

    ioTxSignals(0);

    usbInit();

    uart1Init();
    uart1SetBaudRate(param_baud_rate);

    radioComRxEnforceOrdering = 1;
    radioComInit();

    while (1)
    {
        boardService();
        updateLeds();
        errorService();

        radioComTxService();

        usbComService();
        uartToRadioService();
    }
}
