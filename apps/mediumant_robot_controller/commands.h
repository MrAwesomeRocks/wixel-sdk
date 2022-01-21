#include "wixel.h"
#include "reports.h"

#ifndef __COMMANDS_H__
#define __COMMANDS_H__

// Commands
#define COMMAND_MOVE_LEGS 0x81 // 0x81 NUM_LEGS LEG_NUM POS_DEG_HIGH POS_DEG_LOW ...
#define COMMAND_GET_LEGS_POSITION 0x82
#define COMMAND_STOP 0xAA
#define COMMAND_ECHO 0xB0

// Responses
#define RESPONSE_COMMAND(cmd) (uint8)(cmd + 0x40)

#define PRINT_RESPONSE 0xF1
#define PRINT(byte) reportByte(PRINT_RESPONSE, byte)

#define ERROR_RESPONSE 0xFF

/** Process commands received by the radio. */
void radioCommandService();

#endif
