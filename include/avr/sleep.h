/* SPDX-License-Identifier: Apache-2.0 */

#ifndef MESHBUS_ARDUBOY_AVR_SLEEP_H_
#define MESHBUS_ARDUBOY_AVR_SLEEP_H_

#define SLEEP_MODE_IDLE 0

static inline void set_sleep_mode(int mode)
{
	(void)mode;
}

static inline void sleep_mode() {}

#endif /* MESHBUS_ARDUBOY_AVR_SLEEP_H_ */
