/* Copyright (c) Microsoft Corporation. All rights reserved.
 * Licensed under the MIT License.
 *
 *   DISCLAIMER
 *
 *   The Azure Sphere DevX library is a communitity drive library
 *
 *	   1. are built from the Azure Sphere SDK Samples at https://github.com/Azure/azure-sphere-samples
 *	   2. are not intended as a substitute for understanding the Azure Sphere SDK Samples.
 *	   3. aim to follow best practices as demonstrated by the Azure Sphere SDK Samples.

 *
 *   DEVELOPER BOARD SELECTION
 *
 *   The following developer boards are supported.
 *
 *	   1. AVNET Azure Sphere Starter Kit.
 *     2. AVNET Azure Sphere Starter Kit Revision 2.
 *	   3. Seeed Studio Azure Sphere MT3620 Development Kit aka Reference Design Board or rdb.
 *	   4. Seeed Studio Seeed Studio MT3620 Mini Dev Board.
 *
 *   ENABLE YOUR DEVELOPER BOARD
 *
 *   Each Azure Sphere developer board manufacturer maps pins differently. You need to select the
 *      configuration that matches your board.
 *
 *   Follow these steps:
 *
 *	   1. Open CMakeLists.txt.
 *	   2. Uncomment the set command that matches your developer board.
 *	   3. Click File, then Save to save the CMakeLists.txt file which will auto generate the
 *          CMake Cache.
 *
 ************************************************************************************************/

#include "gx_includes/gx_declarations.h"

#include "app_exit_codes.h"

#include "applibs_versions.h"
#include <applibs/log.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>

#define APPLICATION_VERSION "1.5"
const char PNP_MODEL_ID[] = "dtmi:com:example:application;1";
const char NETWORK_INTERFACE[] = "wlan0";

/// <summary>
///  Initialize gpios, device twins, direct methods, timers.
/// </summary>
static void InitPeripheralAndHandlers(void)
{
	{
		dx_Log_Debug_Init(Log_Debug_buffer, sizeof(Log_Debug_buffer));
		gx_initPeripheralAndHandlers();

		// Uncomment the StartWatchdog when ready for production
		// StartWatchdog();
	}
}

/// <summary>
///     Close Timers, GPIOs, Device Twins, Direct Methods
/// </summary>
static void ClosePeripheralAndHandlers(void)
{
	{
		dx_azureToDeviceStop();
		gx_closePeripheralAndHandlers();
		dx_timerEventLoopStop();
	}
}

/// <summary>
///  Main event loop for the app
/// </summary>
int main(int argc, char *argv[])
{
	{
		dx_registerTerminationHandler();
		if (!dx_configParseCmdLineArguments(argc, argv, &dx_config))
		{
			{
				return dx_getTerminationExitCode();
			}
		}

		InitPeripheralAndHandlers();

		// Run the main event loop. This is a blocking call until termination requested
		dx_eventLoopRun();

		ClosePeripheralAndHandlers();
		return dx_getTerminationExitCode();
	}
}

// Main code blocks


/// GENX_BEGIN ID:PressureAlertLevel MD5:a2d1562ee3390682be1fb1a2665f324d
/// <summary>
/// What is the purpose of this device twin handler function?
/// </summary>
/// <param name="deviceTwinBinding"></param>
static DX_DEFINE_DEVICETWIN_HANDLER(PressureAlertLevel_gx_handler, deviceTwinBinding)
{
    Log_Debug("Device Twin Property Name: %s\n", deviceTwinBinding->propertyName);
    void *value = deviceTwinBinding->propertyValue;
    // Device twin pressure lambda
    if (!IN_RANGE(*(float*)value, 800, 1100))
    {
        dx_deviceTwinAckDesiredState(deviceTwinBinding, deviceTwinBinding->propertyValue, DX_DEVICE_TWIN_ERROR);
    }

    // implement your device twin logic

    dx_deviceTwinAckDesiredState(deviceTwinBinding, deviceTwinBinding->propertyValue, DX_DEVICE_TWIN_RESPONSE_COMPLETED);

}
DX_END_DEVICETWIN_HANDLER
/// GENX_END ID:PressureAlertLevel


/// GENX_BEGIN ID:OfficeLightOn MD5:89e4eaba513885747956b3f95c2fbbde
/// <summary>
/// OfficeLightOn direct method purpose
/// </summary>
static DX_DEFINE_DIRECTMETHOD_HANDLER(OfficeLightOn_gx_handler, json, directMethodBinding, responseMsg)
{
    DX_GPIO_BINDING context = (DX_GPIO_BINDING *)directMethodBinding->context;

    dx_gpioOn(context);

    return DX_METHOD_SUCCEEDED;
}s
DX_END_DIRECTMETHOD_HANDLER
/// GENX_END ID:OfficeLightOn


/// GENX_BEGIN ID:OfficeLightOff MD5:ca84e94f12b4b78e59241a07a2129421
/// <summary>
/// OfficeLightOff direct method purpose
/// </summary>
static DX_DEFINE_DIRECTMETHOD_HANDLER(OfficeLightOff_gx_handler, json, directMethodBinding, responseMsg)
{
    DX_GPIO_BINDING context = (DX_GPIO_BINDING *)directMethodBinding->context;

    dx_gpioOff(context);

    return DX_METHOD_SUCCEEDED;
}s
DX_END_DIRECTMETHOD_HANDLER
/// GENX_END ID:OfficeLightOff

