#include "servo.h"

#include <uart1.h>

/* DEFINES
 * ********************************************************************/

// Servo motor angle read config
#define SERVO_ADC_MIN 100
#define SERVO_ADC_MAX 3000

// Servo motor speeds in us
#define SERVO_SPEED_FORWARDS 6200
#define SERVO_SPEED_STOP 6000
#define SERVO_SPEED_BACKWARDS 5800

// Resolution of the servo angle control in degrees
#define SERVO_ANGLE_RESOLUTION 10

// Set the speed of one servo
#define SET_SERVO(num, speed)                                                  \
    {                                                                          \
        putCommand(0x84);                                                      \
        putCommand(num);                                                       \
        putCommand(speed & 0x7f);                                              \
        putCommand((speed >> 7) & 0x7f);                                       \
    }

/* GLOBAL VARIABLES
 * ***********************************************************/

/** A big buffer of commands to send to the servo controller. */
uint8 XDATA servoCommandBuffer[128];

/** The length of the servo command buffer. */
uint8 DATA servoCommandLength = 0;

/** The number of command bytes sent to the servo controller already. */
uint8 DATA servoCommandBytesSent = 0;

/** The target positions of the legs. */
int16 legTargets[6] = {0};

/* FUNCTIONS
 * ******************************************************************/

static int putCommand(int c)
{
    servoCommandBuffer[servoCommandLength] = c;
    servoCommandLength++;
    return (uint8)c;
}

uint16 getLegPosition(enum robotLeg leg)
{
    uint32 millivolts = adcConvertToMillivolts(adcRead(leg) * 2);
    return (millivolts - SERVO_ADC_MIN) * 360 /
           (SERVO_ADC_MAX - SERVO_ADC_MIN + 1);
}

void setLegPosition(enum robotLeg leg, uint16 degrees)
{
    // Make sure degrees is in a valid range
    while (degrees > 360) {
        degrees -= 360;
    }

    // Set the leg target
    legTargets[leg] = degrees;
}

inline void clearServoCommandBuffer()
{
    servoCommandLength = 0;
}

void servoService()
{
    static uint32 lastServoUpdate;
    static int8 lastServoDirections[6];

    if (getMs() - lastServoUpdate >= param_servo_update_ms &&
        servoCommandLength == 0) {
        lastServoUpdate = getMs();
        servoCommandBytesSent = 0;

        // Read the leg positions
        adcSetMillivoltCalibration(adcReadVddMillivolts());

        // Set the servos
        for (uint8 i = 0; i < 6; i++) {
            int16 legPosition = getLegPosition(i);

            if (legPosition - legTargets[i] < SERVO_ANGLE_RESOLUTION) {
                // Not far enough, Move forward
                if (lastServoDirections[i] != 1) {
                    // Need to change direction
                    lastServoDirections[i] = 1;
                    SET_SERVO(i, SERVO_SPEED_FORWARDS);
                }
            } else if (legPosition - legTargets[i] > SERVO_ANGLE_RESOLUTION) {
                // Too far, Move backwards
                if (lastServoDirections[i] != -1) {
                    // Need to change direction
                    lastServoDirections[i] = -1;
                    SET_SERVO(i, SERVO_SPEED_BACKWARDS);
                }
            } else {
                // Within range, stop
                if (lastServoDirections[i] != 0) {
                    // Need to change direction
                    lastServoDirections[i] = 0;
                    SET_SERVO(i, SERVO_SPEED_STOP);
                }
            }
        }
    }

    // Send the commands to the servo controller in chunks
    if (servoCommandLength > 0) {
        uint8 bytesToSend = uart1TxAvailable();
        if (bytesToSend > servoCommandLength - servoCommandBytesSent) {
            // Send the last part of the report
            uart1TxSend(servoCommandBuffer + servoCommandBytesSent,
                        servoCommandLength - servoCommandBytesSent);
            servoCommandLength = 0;
        } else {
            // Send a chunk of the report
            uart1TxSend(servoCommandBuffer + servoCommandBytesSent,
                        bytesToSend);
            servoCommandBytesSent += bytesToSend;
        }
    }
}
