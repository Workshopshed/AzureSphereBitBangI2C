#pragma once

//Some details thanks to AdaFruit library for the HMC5883 magnentometer/compass written by Kevin Townsend for Adafruit Industries.  

//I2C ADDRESS / BITS
//I2C Address 8 - bit read address  0x3D
//            8 - bit write address 0x3C

#define HMC5883_ADDRESS_MAG            (0x3C >> 1)         // 0011110x

//REGISTERS
typedef enum
{
	HMC5883_REGISTER_MAG_CRA_REG_M = 0x00,
	HMC5883_REGISTER_MAG_CRB_REG_M = 0x01,
	HMC5883_REGISTER_MAG_MR_REG_M =  0x02,
	HMC5883_REGISTER_MAG_OUT_X_H_M = 0x03,
	HMC5883_REGISTER_MAG_OUT_X_L_M = 0x04,
	HMC5883_REGISTER_MAG_OUT_Z_H_M = 0x05,
	HMC5883_REGISTER_MAG_OUT_Z_L_M = 0x06,
	HMC5883_REGISTER_MAG_OUT_Y_H_M = 0x07,
	HMC5883_REGISTER_MAG_OUT_Y_L_M = 0x08,
	HMC5883_REGISTER_MAG_SR_REG_Mg = 0x09,
	HMC5883_REGISTER_MAG_IRA_REG_M = 0x0A,  
	HMC5883_REGISTER_MAG_IRB_REG_M = 0x0B,
	HMC5883_REGISTER_MAG_IRC_REG_M = 0x0C,
	HMC5883_REGISTER_MAG_TEMP_OUT_H_M = 0x31,
	HMC5883_REGISTER_MAG_TEMP_OUT_L_M = 0x32
} hmc5883MagRegisters_t;

/* from the data sheet
00 Configuration Register A   Read/Write
01 Configuration Register B   Read/Write
02 Mode Register              Read/Write
03 Data Output X MSB Register Read
04 Data Output X LSB Register Read
05 Data Output Z MSB Register Read
06 Data Output Z LSB Register Read
07 Data Output Y MSB Register Read
08 Data Output Y LSB Register Read
09 Status Register            Read
10 Identification Register A  Read
11 Identification Register B  Read
12 Identification Register C  Read

Note if you read from register 3 and then read again, it will read from 4,5-8 before returning to 3 again.
*/

/*=========================================================================
    MAGNETOMETER GAIN SETTINGS
    -----------------------------------------------------------------------*/
typedef enum
{
    HMC5883_MAGGAIN_1_3 = 0x20,  // +/- 1.3
    HMC5883_MAGGAIN_1_9 = 0x40,  // +/- 1.9
    HMC5883_MAGGAIN_2_5 = 0x60,  // +/- 2.5
    HMC5883_MAGGAIN_4_0 = 0x80,  // +/- 4.0
    HMC5883_MAGGAIN_4_7 = 0xA0,  // +/- 4.7
    HMC5883_MAGGAIN_5_6 = 0xC0,  // +/- 5.6
    HMC5883_MAGGAIN_8_1 = 0xE0   // +/- 8.1
} hmc5883MagGain;
/*=========================================================================*/