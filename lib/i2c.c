#include "i2c.h"

#include "delay.h"
#include "stc15.h"

#define I2C_HALF_PERIOD_US 2
#define I2C_PIN_(port, pin) P##port##pin
#define I2C_PIN(port, pin) I2C_PIN_(port, pin)
#define I2C_MODE0_(port) P##port##M0
#define I2C_MODE0(port) I2C_MODE0_(port)
#define I2C_MODE1_(port) P##port##M1
#define I2C_MODE1(port) I2C_MODE1_(port)
#define I2C_PIN_MASK(pin) ((unsigned char)(1U << (pin)))

#define I2C_SDA I2C_PIN(I2C_SDA_PORT, I2C_SDA_PIN)
#define I2C_SCL I2C_PIN(I2C_SCL_PORT, I2C_SCL_PIN)
#define I2C_SDA_MODE0 I2C_MODE0(I2C_SDA_PORT)
#define I2C_SDA_MODE1 I2C_MODE1(I2C_SDA_PORT)
#define I2C_SCL_MODE0 I2C_MODE0(I2C_SCL_PORT)
#define I2C_SCL_MODE1 I2C_MODE1(I2C_SCL_PORT)

static void i2c_delay(void)
{
    delay_us(I2C_HALF_PERIOD_US);
}

static void i2c_scl_release_and_wait(void)
{
    I2C_SCL = 1;
    while (!I2C_SCL)
        ;
}

void i2c_init(void)
{
    I2C_SDA = 1;
    I2C_SCL = 1;
    I2C_SDA_MODE1 |= I2C_PIN_MASK(I2C_SDA_PIN);
    I2C_SDA_MODE0 |= I2C_PIN_MASK(I2C_SDA_PIN);
    I2C_SCL_MODE1 |= I2C_PIN_MASK(I2C_SCL_PIN);
    I2C_SCL_MODE0 |= I2C_PIN_MASK(I2C_SCL_PIN);
}

bool i2c_start(void)
{
    I2C_SDA = 1;
    i2c_delay();
    i2c_scl_release_and_wait();
    i2c_delay();

    if (!I2C_SDA)
    {
        return false;
    }

    I2C_SDA = 0;
    i2c_delay();
    I2C_SCL = 0;

    return true;
}

void i2c_stop(void)
{
    I2C_SDA = 0;
    i2c_delay();
    i2c_scl_release_and_wait();
    i2c_delay();
    I2C_SDA = 1;
    i2c_delay();
}

bool i2c_write_byte(unsigned char value)
{
    unsigned char mask;
    bool acknowledged;

    for (mask = 0x80; mask != 0; mask >>= 1)
    {
        if (value & mask)
        {
            I2C_SDA = 1;
        }
        else
        {
            I2C_SDA = 0;
        }

        i2c_delay();
        i2c_scl_release_and_wait();
        i2c_delay();
        I2C_SCL = 0;
    }

    I2C_SDA = 1;
    i2c_delay();
    i2c_scl_release_and_wait();
    i2c_delay();
    acknowledged = !I2C_SDA;
    I2C_SCL = 0;

    return acknowledged;
}

unsigned char i2c_read_byte(bool acknowledge)
{
    unsigned char bit_count;
    unsigned char value = 0;

    I2C_SDA = 1;

    for (bit_count = 0; bit_count < 8; ++bit_count)
    {
        value <<= 1;
        i2c_delay();
        i2c_scl_release_and_wait();
        i2c_delay();

        if (I2C_SDA)
        {
            value |= 1;
        }

        I2C_SCL = 0;
    }

    if (acknowledge)
    {
        I2C_SDA = 0;
    }
    else
    {
        I2C_SDA = 1;
    }

    i2c_delay();
    i2c_scl_release_and_wait();
    i2c_delay();
    I2C_SCL = 0;
    I2C_SDA = 1;

    return value;
}
