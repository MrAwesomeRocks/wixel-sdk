#include "servo.h"

#include <uart1.h>

/* DEFINES
 * ********************************************************************/

// Servo motor angle read config
#define SERVO_ADC_MIN 100
#define SERVO_ADC_MAX 3000

// Servo motor speeds in us
#define SERVO_SPEED_FORWARDS_FAST 6500
#define SERVO_SPEED_FORWARDS_SLOW 6300
#define SERVO_SPEED_STOP 6000
#define SERVO_SPEED_BACKWARDS_SLOW 5700
#define SERVO_SPEED_BACKWARDS_FAST 5500

// Servo speed getter
#define SERVO_SPEED(num, type)                                                 \
    (num < 3 ? SERVO_SPEED_BACKWARDS_##type : SERVO_SPEED_FORWARDS_##type)

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
int16 legTargets[6] = {10, 160, 10, 160, 10, 160};

/** If the servos are enabled. */
BIT servosEnabled = 1;

/* FUNCTIONS
 * ******************************************************************/

static int putCommand(int c)
{
    servoCommandBuffer[servoCommandLength] = c;
    servoCommandLength++;
    return (uint8)c;
}

inline void clearServoCommandBuffer()
{
    servoCommandLength = 0;
}

inline void stopAllServos()
{
    for (uint8 i = 0; i < 6; i++) {
        SET_SERVO(i, SERVO_SPEED_STOP);
    }
    servosEnabled = 0;
}

inline void enableServos()
{
    servosEnabled = 1;
}

uint16 getLegPosition(enum robotLeg leg)
{
    uint32 millivolts = adcConvertToMillivolts(adcRead(leg) * 2);
    uint16 degrees = (millivolts - SERVO_ADC_MIN) * 360 /
                     (SERVO_ADC_MAX - SERVO_ADC_MIN + 1);
    return leg < 3 ? degrees : 360 - degrees;
}

void setLegPosition(enum robotLeg leg, uint16 degrees)
{
    enableServos();

    // Make sure degrees is in a valid range
    while (degrees > 360) {
        degrees -= 360;
    }

    // Set the leg target
    legTargets[leg] = degrees;
}

void servoService()
{
    static uint32 lastServoUpdate;
    static uint8 previousServoSpeeds[6];

    if (servosEnabled && getMs() - lastServoUpdate >= param_servo_update_ms &&
        servoCommandLength == 0) {
        lastServoUpdate = getMs();
        servoCommandBytesSent = 0;

        // Read the leg positions
        adcSetMillivoltCalibration(adcReadVddMillivolts());

        // Set the servos
        for (uint8 i = 0; i < 6; i++) {
            int16 legPosition = getLegPosition(i);

            if (legPosition > legTargets[i]) {
                // Past the leg target
                if (legPosition > 270 && legTargets[i] < 270) {
                    // Need to wrap around
                    if (previousServoSpeeds[i] != 'f') {
                        previousServoSpeeds[i] = 'f';
                        SET_SERVO(i, SERVO_SPEED(i, FAST));
                    }
                } else {
                    // Where we need to be
                    if (previousServoSpeeds[i] != 's') {
                        previousServoSpeeds[i] = 's';
                        SET_SERVO(i, SERVO_SPEED_STOP);
                    }
                }
            } else {
                // Before the leg target, need to keep going
                if (legPosition > 270 || legPosition < 90) {
                    if (previousServoSpeeds[i] != 'f') {
                        previousServoSpeeds[i] = 'f';
                        SET_SERVO(i, SERVO_SPEED(i, FAST))
                    }
                } else {
                    if (previousServoSpeeds[i] != 'l') {
                        previousServoSpeeds[i] = 'l';
                        SET_SERVO(i, SERVO_SPEED(i, SLOW));
                    }
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
