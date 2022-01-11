#include "wixel.h"

#ifndef __COMMANDS_H__
#define __COMMANDS_H__

// Commands
#define COMMAND_MOVE_LEGS 0x81 // 0x81 NUM_LEGS LEG_NUM POS_DEG ...
#define COMMAND_GET_LEGS_POSITION 0x82
#define COMMAND_STOP 0xAA

#define RESPONSE_COMMAND(cmd) cmd + 0x40

/** Process commands received by the radio. */
void radioCommandService();

#endif
