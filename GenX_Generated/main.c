/* Copyright (c) Microsoft Corporation. All rights reserved.
 * Licensed under the MIT License.
 *
 *   DISCLAIMER
 *
 *   The DevX library supports the Azure Sphere Developer Learning Path:
 *
 *	   1. are built from the Azure Sphere SDK Samples at https://github.com/Azure/azure-sphere-samples
 *	   2. are not intended as a substitute for understanding the Azure Sphere SDK Samples.
 *	   3. aim to follow best practices as demonstrated by the Azure Sphere SDK Samples.
 *	   4. are provided as is and as a convenience to aid the Azure Sphere Developer Learning experience.
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

#include "gx_declarations.h"

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

	//dx_timerOneShotSet(&tmr_ReportStartTime, &(struct timespec) { 5, 0 });

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


/// GENX_BEGIN ID:DeferredUpdateCalculate MD5:3dc655c0544d93b553f256e4c73f3859
/// <summary>
/// Algorithm to determine if a deferred update can proceed
/// </summary>
/// <param name="max_deferral_time_in_minutes">The maximum number of minutes you can defer the update</param>
/// <returns>true to start update, false to defer update</returns>
static bool DeferredUpdateCalculate_gx_handler(unsigned int max_deferral_time_in_minutes) {
	// make deferral update decision
	// Examples 
	// - what is local time - could be as simple as GMT + 8hrs is midnight
	// - Is it dark
	// - What is the device doing
	// - recent activities
	// - Orientation of the device

	time_t now = time(NULL);
	struct tm* t = gmtime(&now);

	// this would work well enough if all devices AU
	// based on perth time zone GMT + 8 AWST
	// otherwise use geotime library https://github.com/Azure/azure-sphere-gallery/tree/main/SetTimeFromLocation

	t->tm_hour += 10;
	t->tm_hour = t->tm_hour % 24;

	// do update between 1am and 5am perth time = 12am and 3am sydney time
	// this gives 3 hour window for update

	return (t->tm_hour >= 1 && t->tm_hour <= 5);
}
/// GENX_END ID:DeferredUpdateCalculate


/// GENX_BEGIN ID:DeferredUpdateNotification MD5:957c99508d5fa2d5521482ea5f0ced22
static void DeferredUpdateNotification_gx_handler(SysEvent_UpdateType type, const char* typeDescription, SysEvent_Status status, const char* statusDescription) {
	// do something here like update a device twin before updating
	char msgBuffer[128];
	char utc[40];

	snprintf(msgBuffer, sizeof(msgBuffer), "%s, Type: %s, Status %s", dx_getCurrentUtc(utc, sizeof(utc)), typeDescription, statusDescription);

	dx_deviceTwinReportState(&dt_DeferredUpdateStatus, msgBuffer);
}
/// GENX_END ID:DeferredUpdateNotification


/// GENX_BEGIN ID:DesiredTemperature MD5:e37563137d58088626040950f1b177e2
/// <summary>
/// What is the purpose of this device twin handler function?
/// </summary>
/// <param name="deviceTwinBinding"></param>
static void DesiredTemperature_gx_handler(DX_DEVICE_TWIN_BINDING* deviceTwinBinding) {
    Log_Debug("Device Twin Property Name: %s\n", deviceTwinBinding->twinProperty);

    // Checking the twinStateUpdated here will always be true.
    // But it's useful property for other areas of your code.
    Log_Debug("Device Twin state updated %s\n", deviceTwinBinding->twinStateUpdated ? "true" : "false");

    float device_twin_value = *(float*)deviceTwinBinding->twinState;

    if (device_twin_value > 0.0f && device_twin_value < 100.0f){
        Log_Debug("Device twin value: %f\n", device_twin_value);

        // IMPLEMENT YOUR CODE HERE

        dx_deviceTwinAckDesiredState(deviceTwinBinding, deviceTwinBinding->twinState, DX_DEVICE_TWIN_COMPLETED);
    } else {
        dx_deviceTwinAckDesiredState(deviceTwinBinding, deviceTwinBinding->twinState, DX_DEVICE_TWIN_ERROR);
    }
}
/// GENX_END ID:DesiredTemperature


/// GENX_BEGIN ID:DeviceStartup MD5:bfc6e9de5655e78aae364f9d6b0be6fa
/// <summary>
/// Implement your timer function
/// </summary>
static void DeviceStartup_gx_handler(EventLoopTimer *eventLoopTimer) {
   if (ConsumeEventLoopTimerEvent(eventLoopTimer) != 0) {
       dx_terminate(DX_ExitCode_ConsumeEventLoopTimeEvent);
       return;
   }

	char version[60];
	char utc[30];

	if (dx_isAzureConnected()) {

		dx_deviceTwinReportState(&dt_DeviceStartUtc, dx_getCurrentUtc(utc, sizeof(utc)));

		snprintf(version, sizeof(version), "Application version: %s, DevX version: %s", APPLICATION_VERSION, AZURE_SPHERE_DEVX_VERSION);
		dx_deviceTwinReportState(&dt_SoftwareVersion, version);

	} else {
		dx_timerOneShotSet(&tmr_DeviceStartup, &(struct timespec) { 5, 0 });
	}
}
/// GENX_END ID:DeviceStartup

