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

/* DEPENDENCIES ***************************************************************/
#include <wixel.h>

#include <usb.h>
#include <usb_com.h>

#include <radio_com.h>
#include <radio_link.h>

#include <uart1.h>

#include "commands.h"
#include "errors.h"
#include "reports.h"
#include "servo.h"

/* PARAMETERS *****************************************************************/

int32 CODE param_framing_error_ms = 0;
int32 CODE param_servo_update_ms = 50;

/** The baud rate to run UART 1 at. */
int32 CODE param_baud_rate = 9600;

/* GLOBAL VARIABLES ***********************************************************/

/* FUNCTIONS ******************************************************************/

/** Update the Wixel's LEDs to show program status. */
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
        lastRadioActivityTime = now;
    }

    if ((uint16)(now - lastRadioActivityTime) > 32) {
        dimYellowLed = 0;
    }

    if ((uint8)(now - lastErrorTime) > 100) {
        errorOccurredRecently = 0;
    }

    LED_RED(errorOccurredRecently || uartRxDisabled);
}

// void uartToRadioService()
// {
// Info from the servo controller
// while (uart1RxAvailable() && radioComTxAvailable())
// {
//     radioComTxSendByte(uart1RxReceiveByte());
// }

// Control Signals.
// ioTxSignals(radioComRxControlSignals());
// radioComTxControlSignals(ioRxSignals());
// }

void main()
{
    systemInit();

    // ioTxSignals(0);

    usbInit();

    uart1Init();
    uart1SetBaudRate(param_baud_rate);
    uart1TxSendByte(0xAA); // Start serial connection with Micro Maestro
    uart1TxSendByte(0xA1); // Clear errors on the Maestro

    // radioComRxEnforceOrdering = 1;
    radioComInit();

    // Enable pull-down resistors for all pins on Port 0
    P2INP |= (1 << 5); // PDUP0 = 1: Pull-downs on Port 0.
    P0INP = 0; // This line should not be necessary because P0SEL is 0 on reset.

    while (1) {
        boardService();
        updateLeds();
        errorService();

        radioComTxService();
        usbComService();

        radioCommandService();
        reportsService();
        servoService();
    }
}
