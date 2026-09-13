#ifndef __WATCHDOG_H
#define __WATCHDOG_H

#include "config.h"

/* WDT_CONTR fields. */
#define WDT_RESET_FLAG_MASK        0x80
#define WDT_ENABLE_MASK            0x20
#define WDT_FEED_MASK              0x10
#define WDT_RUN_IN_IDLE            0x08
#define WDT_STOP_IN_IDLE           0x00
#define WDT_PRESCALER_MASK         0x07

/* Timeout = 12 * prescaler * 32768 / MAIN_Fosc seconds. */
#define WDT_PRESCALER_2            0
#define WDT_PRESCALER_4            1
#define WDT_PRESCALER_8            2
#define WDT_PRESCALER_16           3
#define WDT_PRESCALER_32           4
#define WDT_PRESCALER_64           5
#define WDT_PRESCALER_128          6
#define WDT_PRESCALER_256          7

/*
 * Start the watchdog and reset its counter.
 * IDLE_MODE: WDT_RUN_IN_IDLE or WDT_STOP_IN_IDLE
 * PRESCALER: WDT_PRESCALER_2 through WDT_PRESCALER_256
 */
#define WDT_INIT(IDLE_MODE, PRESCALER) do { \
	WDT_CONTR = WDT_ENABLE_MASK | WDT_FEED_MASK | (IDLE_MODE) | \
		((PRESCALER) & WDT_PRESCALER_MASK); \
} while (0)

/* Restart the watchdog counter without changing its configuration. */
#define WDT_FEED() do { \
	WDT_CONTR |= WDT_FEED_MASK; \
} while (0)

/*
 * Attempt to stop the watchdog without changing its configuration.
 * On STC15 variants where EN_WDT cannot be cleared after being set, this has
 * no effect; feed the watchdog during long operations instead.
 */
#define WDT_STOP() do { \
	WDT_CONTR &= ~WDT_ENABLE_MASK; \
} while (0)

/* WDT_RESET_OCCURRED() remains true until the flag is cleared explicitly. */
#define WDT_RESET_OCCURRED() \
	((WDT_CONTR & WDT_RESET_FLAG_MASK) != 0)
#define WDT_CLEAR_RESET_FLAG() do { \
	WDT_CONTR &= ~WDT_RESET_FLAG_MASK; \
} while (0)

#endif
