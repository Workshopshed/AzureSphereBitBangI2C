#pragma once

//I2CBus
typedef struct {
	GPIO_Id scl;
	int sclFd;
	GPIO_Id sda;
	int sdaFd;
} i2cbus_t;

//Function prototypes
int i2c_bb_setup(i2cbus_t* bus);
short i2c_bb_tx(i2cbus_t* bus, unsigned char addr, const unsigned char *dat, unsigned short len);
short i2c_bb_rx_reg(i2cbus_t* bus, unsigned char addr, unsigned char *dest, unsigned short len);
short i2c_bb_rx(i2cbus_t* bus, unsigned char addr, unsigned char *dest, unsigned short len);