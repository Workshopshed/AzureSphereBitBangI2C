#include <signal.h>
#include <stdbool.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <time.h>

// applibs_versions.h defines the API struct versions to use for applibs APIs.
#include "applibs_versions.h"
#include <applibs/gpio.h>
#include <applibs/log.h>

#include "mt3620_rdb.h"

#include "i2cbb.h"
#include "hmc5883.h"

// This C application for the MT3620 Reference Development Board (Azure Sphere)
// connects to an hmc5883 I2C compass and outputs the X,Y,Z readings from the device
// Uses a "bitbang" approach to I2C communication as the hardware version is not yet released
//
// It uses the API for the following Azure Sphere application libraries:
// - log (messages shown in Visual Studio's Device Output window during debugging)

static volatile sig_atomic_t terminationRequired = false;

static i2cbus_t i2cbus1 = { .sclFd = -1,
							.scl = MT3620_RDB_HEADER2_PIN13_GPIO,
							.sdaFd = -1,
							.sda = MT3620_RDB_HEADER2_PIN11_GPIO };

/// <summary>
///     Signal handler for termination requests. This handler must be async-signal-safe.
/// </summary>
static void TerminationHandler(int signalNumber)
{
    // Don't use Log_Debug here, as it is not guaranteed to be async-signal-safe.
    terminationRequired = true;
}

/// <summary>
///     Set up SIGTERM termination handler, initialize peripherals, and set up event handlers.
/// </summary>
/// <returns>0 on success, or -1 on failure</returns>
static int InitPeripheralsAndHandlers(void) {
	// Register a SIGTERM handler for termination requests
	struct sigaction action;
	memset(&action, 0, sizeof(struct sigaction));
	action.sa_handler = TerminationHandler;
	sigaction(SIGTERM, &action, NULL);

    //Configure the pins for the bus
    if (i2c_bb_setup(&i2cbus1) < 0) { return -1; };

    return 0;
}

/// <summary>
///     Main entry point for this sample.
/// </summary>
int main(int argc, char *argv[])
{
    Log_Debug("Application starting.\n");

    if (InitPeripheralsAndHandlers() < 0) {
        terminationRequired = 1;
    };

    // Main loop
    const struct timespec sleepTime = {1, 0 };
    const struct timespec waitTime = { 0, 6000000 };


    const unsigned char msgEnable[2] = { HMC5883_REGISTER_MAG_MR_REG_M, 0x00 }; // Enable Magnetometer, Continuous mode, Low Speed I2C 
    const unsigned char msgSetGain[2] = { HMC5883_REGISTER_MAG_CRB_REG_M, HMC5883_MAGGAIN_1_3 }; // Gain
    const unsigned char msgSelectMagRegister[1] = { HMC5883_REGISTER_MAG_OUT_X_H_M }; // Get mag reading

    while (!terminationRequired) {

        unsigned char rec[6];
        short ret;

        i2c_bb_tx(&i2cbus1, HMC5883_ADDRESS_MAG, &msgEnable[0], 2);
        i2c_bb_tx(&i2cbus1, HMC5883_ADDRESS_MAG, &msgSetGain[0], 2);

        nanosleep(&waitTime, NULL);

        i2c_bb_tx(&i2cbus1, HMC5883_ADDRESS_MAG, &msgSelectMagRegister[0], 1);
        ret = i2c_bb_rx(&i2cbus1, HMC5883_ADDRESS_MAG, &rec[0], 6);
        Log_Debug("Sucesss: %d\n", ret);
        Log_Debug("Reading: %d %d, %d %d, %d %d \n", rec[0], rec[1], rec[2], rec[3], rec[4], rec[5]);

        nanosleep(&sleepTime, NULL);

    }

    Log_Debug("Application exiting.\n");
    return 0;
}
