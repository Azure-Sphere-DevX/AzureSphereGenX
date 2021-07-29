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


/// GENX_BEGIN ID:SetPressure MD5:6d7e617fec44d5a2980c61129b6bf7fb
/// <summary>
/// What is the purpose of this device twin handler function?
/// </summary>
/// <param name="deviceTwinBinding"></param>
static void SetPressure_gx_handler(DX_DEVICE_TWIN_BINDING* deviceTwinBinding) 
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

    }
}
/// GENX_END ID:SetPressure


/// GENX_BEGIN ID:LightControl MD5:2c85a1e25cbb1dea11acfad8035dd0a8
/// <summary>
/// What is the purpose of this direct method
/// </summary>
static DX_DIRECT_METHOD_RESPONSE_CODE LightControl_gx_handler(JSON_Value *json, DX_DIRECT_METHOD_BINDING *directMethodBinding, char **responseMsg) {
   
    // Example direct method with no payload
    // Implement your logic here

    return DX_METHOD_SUCCEEDED;
}
/// GENX_END ID:LightControl


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


/// GENX_BEGIN ID:FanOn MD5:dd557f8700192da87bcb5304d21770eb
/// <summary>
/// What is the purpose of this direct method
/// </summary>
static DX_DIRECT_METHOD_RESPONSE_CODE FanOn_gx_handler(JSON_Value *json, DX_DIRECT_METHOD_BINDING *directMethodBinding, char **responseMsg) {
   
    // Example direct method with no payload
    // Implement your logic here

    return DX_METHOD_SUCCEEDED;
}
/// GENX_END ID:FanOn


/// GENX_BEGIN ID:FanOff MD5:c84830bce3283bb329ae4665fa0f45d9
/// <summary>
/// What is the purpose of this direct method
/// </summary>
static DX_DIRECT_METHOD_RESPONSE_CODE FanOff_gx_handler(JSON_Value *json, DX_DIRECT_METHOD_BINDING *directMethodBinding, char **responseMsg) {
   
    // Example direct method with no payload
    // Implement your logic here

    return DX_METHOD_SUCCEEDED;
}
/// GENX_END ID:FanOff


/// GENX_BEGIN ID:MeasureTemperature MD5:862427771aefa4848defddee55ea5b35
/// <summary>
/// Implement your oneshot timer function
/// </summary>
static void MeasureTemperature_gx_handler(EventLoopTimer *eventLoopTimer) {
    if (ConsumeEventLoopTimerEvent(eventLoopTimer) != 0) {
        dx_terminate(DX_ExitCode_ConsumeEventLoopTimeEvent);
        return;
    }

    Log_Debug("Oneshot timer MeasureTemperature_handler called\n");
    // Implement your timer function


    // reload the oneshot timer
    dx_timerOneShotSet(&tmr_MeasureTemperature, &(struct timespec){5, 0});
}
/// GENX_END ID:MeasureTemperature


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


/// GENX_BEGIN ID:DeferredUpdate MD5:1228ab408e101ae072880291ce561421
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
/// GENX_END ID:DeferredUpdate


/// GENX_BEGIN ID:DeferredUpdate MD5:548e7399dbcdc65dd44c4c79def80d3f
static void DeferredUpdateNotification_gx_handler(uint32_t max_deferral_time_in_minutes, SysEvent_UpdateType type,
	SysEvent_Status status, const char* typeDescription, const char* statusDescription) {

	// do something here like update a device twin before updating
	char msgBuffer[128];
	char utc[40];

	snprintf(msgBuffer, sizeof(msgBuffer), "Utc: %s, Type: %s, Status: %s, Max defer minutes: %i", dx_getCurrentUtc(utc, sizeof(utc)), typeDescription, statusDescription, max_deferral_time_in_minutes);

	dx_deviceTwinReportState(&dt_DeferredUpdateNotification, msgBuffer);
}
/// GENX_END ID:DeferredUpdate

