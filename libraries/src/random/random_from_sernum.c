/*! \file random_from_sernum.c
 * See random.h for more information about this library.
 */

// TODO: make sure random number generator doesn't get in to a state with all 1s!

#include <cc2511_map.h>
#include <random.h>
#include <board.h>

void randomSeedFromSerialNumber(void)
{
	// The random number generator only has 16 bits of state, while the
	// serial number is 32 bits.  No matter what we do here, there will be
	// a 1-in-2^16 chance that two Wixels with different serial numbers
	// start up with their random number generators in the same state.
	// So there is no point in reading all 4 bytes of the serial number.

	// See "A note about RNDL" in random_from_adc.c for an explanation of this:
	RNDL = serialNumber[0];
	RNDL = serialNumber[1];

    randomNumber();
    randomNumber();
    randomNumber();
}
