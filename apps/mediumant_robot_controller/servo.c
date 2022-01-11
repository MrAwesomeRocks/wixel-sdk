#include "servo.h"

#include <uart1.h>

/* DEFINES ********************************************************************/

// Servo motor angle read config
#define SERVO_ADC_MIN 100
#define SERVO_ADC_MAX 3000

// Servo motor speeds
#define SERVO_SPEED_FORWARDS 1600
#define SERVO_SPEED_STOP 1500
#define SERVO_SPEED_BACKWARDS 1400

// Resolution of the servo angle control in degrees
#define SERVO_ANGLE_RESOLUTION 5

// Set the speed of one servo
#define SET_SERVO(num, speed)         \
    {                                 \
        putCommand(0x84);                \
        putCommand(num);                 \
        putCommand(speed & 0x7f);        \
        putCommand((speed >> 7) & 0x7f); \
    }

/* GLOBAL VARIABLES ***********************************************************/

/** A big buffer of commands to send to the servo controller. */
uint8 XDATA servoCommandBuffer[128];

/** The length of the servo command buffer. */
uint8 DATA servoCommandLength = 0;

/** The number of command bytes sent to the servo controller already. */
uint8 DATA servoCommandBytesSent = 0;

/** The target positions of the legs. */
uint16 legTargets[6];

/* FUNCTIONS ******************************************************************/

static int putCommand(int c)
{
    servoCommandBuffer[servoCommandLength] = c;
    servoCommandLength++;
    return (uint8)c;
}

uint16 getLegPosition(enum robotLeg leg)
{
    uint16 millivolts = adcConvertToMillivolts(adcRead(leg) * 2);
    return (100 * millivolts - SERVO_ADC_MIN) * 360 /
           (SERVO_ADC_MAX - SERVO_ADC_MIN + 1);
}

void setLegPosition(enum robotLeg leg, uint16 degrees)
{
    // Make sure degrees is in a valid range
    while (degrees > 360)
    {
        degrees -= 360;
    }

    // Set the leg target
    legTargets[leg] = degrees;
}

void clearServoCommandBuffer()
{
    servoCommandLength = 0;
}

void servoService()
{
    static uint32 lastServoUpdate;

    if (getMs() - lastServoUpdate >= param_servo_update_ms && servoCommandLength == 0)
    {
        lastServoUpdate = getMs();
        servoCommandBytesSent = 0;

        // Read the leg positions
        adcSetMillivoltCalibration(adcReadVddMillivolts());

        // Set the servos
        for (uint8 i = 0; i < 6; i++)
        {
            uint16 legPosition = getLegPosition(i);

            if (legTargets[i] - legPosition > SERVO_ANGLE_RESOLUTION)
            {
                // Move forward
                SET_SERVO(i, SERVO_SPEED_FORWARDS);
            }
            else if (legTargets[i] - legPosition < SERVO_ANGLE_RESOLUTION)
            {
                // Move backwards
                SET_SERVO(i, SERVO_SPEED_BACKWARDS);
            }
            else
            {
                SET_SERVO(i, SERVO_SPEED_STOP);
            }
        }
    }

    // Send the commands to the servo controller in chunks
    if (servoCommandLength > 0)
    {
        uint8 bytesToSend = uart1TxAvailable();
        if (bytesToSend > servoCommandLength - servoCommandBytesSent)
        {
            // Send the last part of the report
            uart1TxSend(servoCommandBuffer + servoCommandBytesSent,
                        servoCommandLength - servoCommandBytesSent);
            servoCommandLength = 0;
        }
        else
        {
            // Send a chunk of the report
            uart1TxSend(servoCommandBuffer + servoCommandBytesSent, bytesToSend);
            servoCommandBytesSent += bytesToSend;
        }
    }
}
