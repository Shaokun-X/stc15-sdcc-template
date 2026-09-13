
#ifndef	__DELAY_H
#define	__DELAY_H

/* Busy-wait delays calibrated for SDCC, MAIN_Fosc, and the STC15 1T core.
 * They do not disable interrupts, so interrupt execution extends the delay.
 */

/* Wait approximately 0 to 255 microseconds; zero returns immediately. */
void  delay_us(unsigned char us);

/* Wait approximately 0 to 65535 milliseconds; zero returns immediately. */
void  delay_ms(unsigned int ms);

#endif
