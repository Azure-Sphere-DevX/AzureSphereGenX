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


/// GENX_BEGIN ID:ReportStartState MD5:9aa8972320737bc700454c2cfa2a766d
/// <summary>
/// Implement your timer function
/// </summary>
static void ReportStartState_gx_handler(EventLoopTimer *eventLoopTimer) {
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
		dx_timerOneShotSet(&tmr_ReportStartState, &(struct timespec) { 5, 0 });
	}
}
/// GENX_END ID:ReportStartState


/// GENX_BEGIN ID:DeferredUpdateCalculate MD5:1228ab408e101ae072880291ce561421
/// <summary>
/// Determine whether or not to defer an update
/// </summary>
/// <param name="max_deferral_time_in_minutes">The maximum number of minutes you can defer</param>
/// <returns>Return 0 to start update, return greater than zero to defer</returns>
static uint32_t DeferredUpdateCalculate_gx_handler(uint32_t max_deferral_time_in_minutes, SysEvent_UpdateType type,
	SysEvent_Status status, const char* typeDescription,
	const char* statusDescription) {

	uint32_t requested_minutes = 0;
	time_t now = time(NULL);
	struct tm* t = gmtime(&now);
	char msgBuffer[128];
	char utc[40];

	// UTC +10 would work well for devices in Australia
	t->tm_hour += 10;
	t->tm_hour = t->tm_hour % 24;

	// Set requested_minutes to zero to allow update, greater than zero to defer the update
	requested_minutes = t->tm_hour >= 1 && t->tm_hour <= 5 ? 0 : 10;

	snprintf(msgBuffer, sizeof(msgBuffer), "Utc: %s, Type: %s, Status: %s, Max defer minutes: %i, Requested minutes: %i", 
		dx_getCurrentUtc(utc, sizeof(utc)), typeDescription, statusDescription, max_deferral_time_in_minutes, requested_minutes);

	dx_deviceTwinReportState(&dt_DeferredUpdateRequest, msgBuffer);

	return requested_minutes;
}
/// GENX_END ID:DeferredUpdateCalculate


/// GENX_BEGIN ID:DeferredUpdateNotification MD5:548e7399dbcdc65dd44c4c79def80d3f
static void DeferredUpdateNotification_gx_handler(uint32_t max_deferral_time_in_minutes, SysEvent_UpdateType type,
	SysEvent_Status status, const char* typeDescription, const char* statusDescription) {

	// do something here like update a device twin before updating
	char msgBuffer[128];
	char utc[40];

	snprintf(msgBuffer, sizeof(msgBuffer), "Utc: %s, Type: %s, Status: %s, Max defer minutes: %i", dx_getCurrentUtc(utc, sizeof(utc)), typeDescription, statusDescription, max_deferral_time_in_minutes);

	dx_deviceTwinReportState(&dt_DeferredUpdateNotification, msgBuffer);
}
/// GENX_END ID:DeferredUpdateNotification


/// GENX_BEGIN ID:LightOff MD5:c071f9c3862c47f9bac4372ca00706f7
/// <summary>
/// Direct method to turn off light/relay
/// </summary>
static DX_DIRECT_METHOD_RESPONSE_CODE LightOff_gx_handler(JSON_Value *json, DX_DIRECT_METHOD_BINDING *directMethodBinding, char **responseMsg) {
   
    dx_gpioOff(&gpio_Light1);

    return DX_METHOD_SUCCEEDED;
}
/// GENX_END ID:LightOff


/// GENX_BEGIN ID:LightOn MD5:908beb2beb759eb1a91570f6e0866333
/// <summary>
/// Direct method to turn on light/relay
/// </summary>
static DX_DIRECT_METHOD_RESPONSE_CODE LightOn_gx_handler(JSON_Value *json, DX_DIRECT_METHOD_BINDING *directMethodBinding, char **responseMsg) {
   
    dx_gpioOn(&gpio_Light1);

    return DX_METHOD_SUCCEEDED;
}
/// GENX_END ID:LightOn


/// GENX_BEGIN ID:RestartDevice MD5:ec6a23190fc9be14a395546953112b46
/// <summary>
/// Start Device Power Restart Direct Method 'ResetMethod' integer seconds eg 5
/// </summary>
static DX_DIRECT_METHOD_RESPONSE_CODE RestartDevice_gx_handler(JSON_Value* json, DX_DIRECT_METHOD_BINDING* directMethodBinding, char** responseMsg)
{
    if (json_value_get_type(json) != JSONNumber) { return DX_METHOD_FAILED; }

    int seconds = (int)json_value_get_number(json);

    if (seconds > 2 && seconds < 10)
    {
        PowerManagement_ForceSystemReboot();        
        return DX_METHOD_SUCCEEDED;
    }
    else
    {
        return DX_METHOD_FAILED;
    }
}
/// GENX_END ID:RestartDevice


/// GENX_BEGIN ID:Watchdog MD5:4f6bc7d013dfa4e93826c0b945f7a4a1
/// <summary>
/// This timer extends the app level lease watchdog timer
/// </summary>
/// <param name="eventLoopTimer"></param>
static void Watchdog_gx_handler(EventLoopTimer *eventLoopTimer) {
    if (ConsumeEventLoopTimerEvent(eventLoopTimer) != 0) {
        dx_terminate(DX_ExitCode_ConsumeEventLoopTimeEvent);
        return;
    }
    timer_settime(watchdogTimer, 0, &watchdogInterval, NULL);
}

/// <summary>
/// Set up watchdog timer - the lease is extended via the Watchdog_handler function
/// </summary>
/// <param name=""></param>
void StartWatchdog(void) {
	struct sigevent alarmEvent;
	alarmEvent.sigev_notify = SIGEV_SIGNAL;
	alarmEvent.sigev_signo = SIGALRM;
	alarmEvent.sigev_value.sival_ptr = &watchdogTimer;

	if (timer_create(CLOCK_MONOTONIC, &alarmEvent, &watchdogTimer) == 0) {
		if (timer_settime(watchdogTimer, 0, &watchdogInterval, NULL) == -1) {
			Log_Debug("Issue setting watchdog timer. %s %d\n", strerror(errno), errno);
		}
	}
}
/// GENX_END ID:Watchdog

