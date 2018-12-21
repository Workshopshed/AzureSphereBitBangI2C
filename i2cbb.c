#include <applibs/gpio.h>
#include <applibs/log.h>
#include <stdbool.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include "i2cbb.h"

#define BIT0 1

//Port from https://github.com/AlaskaResearchCubeSat/bit-bang-I2C
//Ref http://i2c.info/i2c-bus-specification


/// <summary>
///     setup bit bang I2C pins
/// </summary>
/// <returns>0 on success, or -1 on failure</returns>
int i2c_bb_setup(i2cbus_t* bus) {
    //set pins as outputs opendrain
    bus->sclFd = GPIO_OpenAsOutput(bus->scl, GPIO_OutputMode_OpenDrain, GPIO_Value_High);
    if (bus->sclFd < 0) {
        Log_Debug("ERROR: Could not open SCL GPIO: %s (%d).\n", strerror(errno), errno);
        return -1;
    }
    bus->sdaFd = GPIO_OpenAsOutput(bus->sda, GPIO_OutputMode_OpenDrain, GPIO_Value_High);
    if (bus->sdaFd < 0) {
        Log_Debug("ERROR: Could not open SDA GPIO: %s (%d).\n", strerror(errno), errno);
        return -1;
    }
    return 0;
}

//wait for 1/2 of I2C clock period (5000 = 100Khz clock)
void i2c_bb_hc(void) {
    const struct timespec clockhalftime = { 0, 5000 };
    //wait for 0.05ms
    nanosleep(&clockhalftime, NULL);
}

int setSDA(i2cbus_t* bus, GPIO_Value_Type val) {
    int result = GPIO_SetValue(bus->sdaFd, val);
    if (result != 0) {
        Log_Debug("ERROR: Could not set SDA output value: %s (%d).\n", strerror(errno), errno);
        return -1;
    }
    return 0;
}

int setSCL(i2cbus_t* bus, GPIO_Value_Type val) {
    int result = GPIO_SetValue(bus->sclFd, val);
    if (result != 0) {
        Log_Debug("ERROR: Could not set SCL output value: %s (%d).\n", strerror(errno), errno);
        return -1;
    }
    return 0;
}

unsigned char getSDA(i2cbus_t* bus) {
    GPIO_Value_Type val;
    int result = GPIO_GetValue(bus->sdaFd, &val);
    if (result != 0) {
        Log_Debug("ERROR: Could not read SDA value: %s (%d).\n", strerror(errno), errno);
    }
    return val;
}

void i2c_bb_start(i2cbus_t* bus) {
    i2c_bb_hc();
    //float SCL
    setSCL(bus, GPIO_Value_High);
    //wait for 1/2 clock first
    i2c_bb_hc();
    //pull SDA low
    setSDA(bus, GPIO_Value_Low);
    //wait for 1/2 clock for end of start
    i2c_bb_hc();
}

void i2c_bb_stop(i2cbus_t* bus) {
    i2c_bb_hc();
    //pull SDA low
    setSDA(bus, GPIO_Value_Low);
    //wait for 1/2 clock for end of start
    i2c_bb_hc();
    //float SCL
    setSCL(bus, GPIO_Value_High);
    //wait for 1/2 clock
    i2c_bb_hc();
    //float SDA
    setSDA(bus, GPIO_Value_High);
    //wait for 1/2 clock
    i2c_bb_hc();
}

//send value over I2C return 1 if slave ACKed
short i2c_bb_tx_byte(i2cbus_t* bus, unsigned char val) {
    int i;
    //shift out bits
    for (i = 0;i < 8;i++) {
        //pull SCL low
        setSCL(bus, GPIO_Value_Low);
        //check bit
        if (val & 0x80) {
            //float SDA
            setSDA(bus, GPIO_Value_High);
        }
        else {
            //pull SDA low
            setSDA(bus, GPIO_Value_Low);
        }
        //shift
        val <<= 1;
        //wait for 1/2 clock
        i2c_bb_hc();
        //float SCL
        setSCL(bus, GPIO_Value_High);
        //wait for 1/2 clock
        i2c_bb_hc();
    }
    //check ack bit
    //pull SCL low
    setSCL(bus, GPIO_Value_Low);
    //float SDA
    setSDA(bus, GPIO_Value_High);
    //wait for 1/2 clock
    i2c_bb_hc();
    //float SCL
    setSCL(bus, GPIO_Value_High);
    //wait for 1/2 clock
    i2c_bb_hc();
    //sample SDA
    val = getSDA(bus);
    //pull SCL low
    setSCL(bus, GPIO_Value_Low);
    //return sampled value
    return !val;
}
//send value over I2C return 1 if slave ACKed
unsigned char i2c_bb_rx_byte(i2cbus_t* bus, unsigned short ack) {
    unsigned char val;
    int i;
    val = 0;

    //shift out bits
    for (i = 0;i < 8;i++) {
        //pull SCL low
        setSCL(bus, GPIO_Value_Low);
        //wait for 1/2 clock
        i2c_bb_hc();
        //float SCL
        setSCL(bus, GPIO_Value_High);
        //wait for 1/2 clock
        i2c_bb_hc();

        //shift value to make room
        val <<= 1;

        //sample data
        unsigned char readbit;
        readbit = getSDA(bus);

        if (readbit == GPIO_Value_High) {
            val |= 1;
        }
    }

    //check ack bit
    //pull SCL low
    setSCL(bus, GPIO_Value_Low);
    //check if we are ACKing this byte
    if (ack) {
        //pull SDA low for ACK
        setSDA(bus, GPIO_Value_Low);
    }
    else {
        //float SDA for NACK
        setSDA(bus, GPIO_Value_High);
    }
    //wait for 1/2 clock
    i2c_bb_hc();
    //float SCL
    setSCL(bus, GPIO_Value_High);
    //wait for 1/2 clock
    i2c_bb_hc();
    //pull SCL low
    setSCL(bus, GPIO_Value_Low);
    //float SDA
    setSDA(bus, GPIO_Value_High);
    //return value
    return val;
}

short i2c_bb_tx(i2cbus_t* bus, unsigned char addr, const unsigned char *dat, unsigned short len) {
    short ack;
    int i;
    //send start
    i2c_bb_start(bus);
    //send address with W bit
    ack = i2c_bb_tx_byte(bus, (addr << 1));
    //send data bytes
    for (i = 0;i < len && ack;i++) {
        //transmit next byte
        ack = i2c_bb_tx_byte(bus, dat[i]);
    }
    //transmit stop
    i2c_bb_stop(bus);
    //return if slave NACKed
    return ack;
}

short i2c_bb_rx(i2cbus_t* bus, unsigned char addr, unsigned char *dest, unsigned short len) {
    int i;
    //send start
    i2c_bb_start(bus);
    //send address with R bit
    if (!i2c_bb_tx_byte(bus, (addr << 1) | BIT0)) {
        //got NACK return error
        return 0;
    }
    //receive data bytes
    for (i = 0;i < len;i++) {
        //receive next byte
        dest[i] = i2c_bb_rx_byte(bus, i < (len - 1));
    }
    //transmit stop
    i2c_bb_stop(bus);
    //return if slave NACKed
    return 1;
}