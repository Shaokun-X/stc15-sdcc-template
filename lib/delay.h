
#ifndef	__DELAY_H
#define	__DELAY_H

/* Busy-wait delays; interrupt execution can extend the requested time. */
void  delay_us(unsigned char us);
void  delay_ms(unsigned int ms);

#endif
