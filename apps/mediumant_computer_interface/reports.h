#include "errors.h"

#include <wixel.h>

#ifndef __REPORTS_H__
#define __REPORTS_H__

/** Report an error to the user. */
void reportError(enum errorCode code);

/** Send any outstanding reports. Should be called often. */
void reportsService();

/** Whether to send ANSI color codes with the reports. */
extern int32 CODE param_terminal_colors;

#endif // __REPORTS_H__
