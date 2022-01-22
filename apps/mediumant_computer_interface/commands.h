#include <wixel.h>

#ifndef __COMMANDS_H__
#define __COMMANDS_H__

// Commands
#define COMMAND_STOP 0x80
#define COMMAND_FORWARD 0x81
#define COMMAND_FORWARD_RIGHT 0x82
#define COMMAND_RIGHT 0x83
#define COMMAND_BACKWARDS_RIGHT 0x84
#define COMMAND_BACKWARDS 0x85
#define COMMAND_BACKWARDS_LEFT 0x86
#define COMMAND_LEFT 0x87
#define COMMAND_FORWARDS_LEFT 0x88

#define COMMAND_SPEED 0x90

#define COMMAND_ECHO 0xA0
#define COMMAND_MOVE_LEGS 0xA1
#define COMMAND_GET_LEGS_POSITION 0xA2
#define COMMAND_IS_MOVING 0xA3

/** Process commands received through USB. */
void usbCommandService();

#endif // __COMMANDS_H__
