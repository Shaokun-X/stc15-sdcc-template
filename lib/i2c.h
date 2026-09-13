#ifndef __I2C_H
#define __I2C_H

#include <stdbool.h>

/* Configure each line with its GPIO port number and pin number. */
#ifndef I2C_SDA_PORT
#define I2C_SDA_PORT 1
#endif

#ifndef I2C_SDA_PIN
#define I2C_SDA_PIN 0
#endif

#ifndef I2C_SCL_PORT
#define I2C_SCL_PORT 1
#endif

#ifndef I2C_SCL_PIN
#define I2C_SCL_PIN 1
#endif

/*
 * Configure the selected pins as open-drain and release the bus. External
 * pull-up resistors are required.
 */
void i2c_init(void);

/* i2c_start() also generates a repeated START when called during a transfer. */
bool i2c_start(void);
void i2c_stop(void);

/* Returns true when the receiver acknowledges the byte. */
bool i2c_write_byte(unsigned char value);

/* Pass true to acknowledge the byte, or false to finish with a NACK. */
unsigned char i2c_read_byte(bool acknowledge);

#endif
