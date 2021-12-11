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
static void InitPeripheralAndHandlers(void) {{
	dx_Log_Debug_Init(Log_Debug_buffer, sizeof(Log_Debug_buffer));
	gx_initPeripheralAndHandlers();

	// Uncomment the StartWatchdog when ready for production
	// StartWatchdog();
}}

/// <summary>
///     Close Timers, GPIOs, Device Twins, Direct Methods
/// </summary>
static void ClosePeripheralAndHandlers(void) {{
	dx_azureToDeviceStop();
	gx_closePeripheralAndHandlers();
	dx_timerEventLoopStop();
}}

/// <summary>
///  Main event loop for the app
/// </summary>
int main(int argc, char* argv[]) {{
	dx_registerTerminationHandler();
	if (!dx_configParseCmdLineArguments(argc, argv, &dx_config)) {{
		return dx_getTerminationExitCode();
	}}

	InitPeripheralAndHandlers();

	// Main loop
	while (!dx_isTerminationRequired()) {{
		int result = EventLoop_Run(dx_timerGetEventLoop(), -1, true);
		// Continue if interrupted by signal, e.g. due to breakpoint being set.
		if (result == -1 && errno != EINTR) {{
			dx_terminate(DX_ExitCode_Main_EventLoopFail);
		}}
	}}

	ClosePeripheralAndHandlers();
	return dx_getTerminationExitCode();
}}

// Main code blocks


/// GENX_BEGIN ID:SetDesiredPressure MD5:0f8a00a047f42fe4d548236695a75157
/// <summary>
/// What is the purpose of this device twin handler function?
/// </summary>
/// <param name="deviceTwinBinding"></param>
static void SetDesiredPressure_gx_handler(DX_DEVICE_TWIN_BINDING* deviceTwinBinding) 
{
	bool result = false;

    Log_Debug("Device Twin Property Name: %s\n", deviceTwinBinding->twinProperty);

    // Checking the twinStateUpdated here will always be true.
    // But it's useful property for other areas of your code.
    Log_Debug("Device Twin state updated %s\n", deviceTwinBinding->twinStateUpdated ? "true" : "false");

    void *value = deviceTwinBinding->twinState;

	// The following is to cover all types from the generator

	switch (deviceTwinBinding->twinType) {
	case DX_TYPE_BOOL:
		result = *(bool*)value;
		Log_Debug("Device twin value: %d\n", *(bool*)value ? "True" : "False");
		break;
	case DX_TYPE_INT:
		result = *(int*)value > 0 && *(int*)value < 100;
		Log_Debug("Device twin value: %d\n", *(int*)value);
		break;
	case DX_TYPE_FLOAT:
		result = *(float*)value > 0.0f && *(float*)value < 100.0f;
		Log_Debug("Device twin value: %f\n", *(float*)value);
		break;
	case DX_TYPE_DOUBLE:
		result = *(double*)value > 0.0 && *(double*)value < 100.0;
		Log_Debug("Device twin value: %f\n", *(double*)value);
		break;
	case DX_TYPE_STRING:
		result = !dx_isStringNullOrEmpty((char*)value);
		Log_Debug("Device twin value: %s\n", (char*)value);
		break;
	default:
		break;
	}

    if (result) {

        // IMPLEMENT YOUR CODE HERE

        dx_deviceTwinAckDesiredState(deviceTwinBinding, deviceTwinBinding->twinState, DX_DEVICE_TWIN_COMPLETED);
    } else {
        dx_deviceTwinAckDesiredState(deviceTwinBinding, deviceTwinBinding->twinState, DX_DEVICE_TWIN_ERROR);
    }
}
/// GENX_END ID:SetDesiredPressure


/// GENX_BEGIN ID:ButtonA MD5:f4ad977d757748a39b8bc73a4aec9001
/// <summary>
/// Implement your GPIO input timer function
/// </summary>
static void ButtonA_gx_handler(EventLoopTimer *eventLoopTimer) {
    static GPIO_Value_Type gpio_ButtonANewState;

    if (ConsumeEventLoopTimerEvent(eventLoopTimer) != 0) {
        dx_terminate(DX_ExitCode_ConsumeEventLoopTimeEvent);
        return;
    }

    if (dx_gpioStateGet(&gpio_ButtonA, &gpio_ButtonANewState)) {
        Log_Debug("gpio_ButtonA: %d\n", gpio_ButtonANewState);
    }
}
/// GENX_END ID:ButtonA


/// GENX_BEGIN ID:MeasureTemperature MD5:a9d9eda494110a8cc6eb9ab4004226df
/// <summary>
/// Implement your timer function
/// </summary>
static DX_DEFINE_TIMER_HANDLER(MeasureTemperature_gx_handler)
{
    Log_Debug("Periodic timer MeasureTemperature_handler called\n");

    // Implement your timer function
    
}
DX_END_TIMER_HANDLER
/// GENX_END ID:MeasureTemperature

