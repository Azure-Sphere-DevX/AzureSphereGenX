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
#include "gx_includes.h"

#include "applibs_versions.h"
#include <applibs/log.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>

#define APPLICATION_VERSION "1.0"
const char PNP_MODEL_ID[] = "dtmi:com:example:application;1";
const char NETWORK_INTERFACE[] = "wlan0";

/// <summary>
///  Initialize gpios, device twins, direct methods, timers.
/// </summary>
static void InitPeripheralAndHandlers(void) {{
	dx_Log_Debug_Init(Log_Debug_buffer, sizeof(Log_Debug_buffer));

    #ifdef GX_AZURE_IOT
        dx_azureConnect(&dx_config, NETWORK_INTERFACE, PNP_MODEL_ID);
    #else
        gx_azureConnect(&dx_config, NETWORK_INTERFACE, PNP_MODEL_ID);
    #endif // GX_AZURE_IOT

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


/// GENX_BEGIN ID:CloudStatusLed MD5:a29ad2e6641e7f3caf2bac107c752e47
/// <summary>
/// Implement your timer function
/// </summary>
static void CloudStatusLed_gx_handler(EventLoopTimer *eventLoopTimer) {
    static bool gpio_state = true;

    if (ConsumeEventLoopTimerEvent(eventLoopTimer) != 0) {
        dx_terminate(DX_ExitCode_ConsumeEventLoopTimeEvent);
        return;
    }

    dx_gpioStateSet(&gpio_CloudStatusLed, gpio_state = !gpio_state);
}
/// GENX_END ID:CloudStatusLed


/// GENX_BEGIN ID:DesiredCO2AlertLevel MD5:03f333afbc45929ef727eaa6cc3f6af9
/// <summary>
/// What is the purpose of this device twin handler function?
/// </summary>
/// <param name="deviceTwinBinding"></param>
static void DesiredCO2AlertLevel_gx_handler(DX_DEVICE_TWIN_BINDING* deviceTwinBinding) {
    Log_Debug("Device Twin Property Name: %s\n", deviceTwinBinding->twinProperty);

    // Checking the twinStateUpdated here will always be true.
    // But it's useful property for other areas of your code.
    Log_Debug("Device Twin state updated %s\n", deviceTwinBinding->twinStateUpdated ? "true" : "false");

    double device_twin_value = *(double*)deviceTwinBinding->twinState;

    if (device_twin_value > 0.0 && device_twin_value < 100.0){
        Log_Debug("Device twin value: %f\n", device_twin_value);

        // IMPLEMENT YOUR CODE HERE

        dx_deviceTwinAckDesiredState(deviceTwinBinding, deviceTwinBinding->twinState, DX_DEVICE_TWIN_COMPLETED);
    } else {
        dx_deviceTwinAckDesiredState(deviceTwinBinding, deviceTwinBinding->twinState, DX_DEVICE_TWIN_ERROR);
    }
}
/// GENX_END ID:DesiredCO2AlertLevel


/// GENX_BEGIN ID:DesiredTemperature MD5:2baa5a70cc22058c360dfb287b8cbae6
/// <summary>
/// What is the purpose of this device twin handler function?
/// </summary>
/// <param name="deviceTwinBinding"></param>
static void DesiredTemperature_gx_handler(DX_DEVICE_TWIN_BINDING* deviceTwinBinding) {
    Log_Debug("Device Twin Property Name: %s\n", deviceTwinBinding->twinProperty);

    // Checking the twinStateUpdated here will always be true.
    // But it's useful property for other areas of your code.
    Log_Debug("Device Twin state updated %s\n", deviceTwinBinding->twinStateUpdated ? "true" : "false");

    double device_twin_value = *(double*)deviceTwinBinding->twinState;

    if (device_twin_value > 0.0 && device_twin_value < 100.0){
        Log_Debug("Device twin value: %f\n", device_twin_value);

        // IMPLEMENT YOUR CODE HERE

        dx_deviceTwinAckDesiredState(deviceTwinBinding, deviceTwinBinding->twinState, DX_DEVICE_TWIN_COMPLETED);
    } else {
        dx_deviceTwinAckDesiredState(deviceTwinBinding, deviceTwinBinding->twinState, DX_DEVICE_TWIN_ERROR);
    }
}
/// GENX_END ID:DesiredTemperature


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


/// GENX_BEGIN ID:MeasureCarbonMonoxide MD5:b12fba6da837e517dfe480298fb1082d
/// <summary>
/// Implement your timer function
/// </summary>
static void MeasureCarbonMonoxide_gx_handler(EventLoopTimer *eventLoopTimer) {
    if (ConsumeEventLoopTimerEvent(eventLoopTimer) != 0) {
        dx_terminate(DX_ExitCode_ConsumeEventLoopTimeEvent);
        return;
    }

    Log_Debug("Periodic timer MeasureCarbonMonoxide_handler called\n");

    // Implement your timer function
}
/// GENX_END ID:MeasureCarbonMonoxide


/// GENX_BEGIN ID:MeasureTemperature MD5:3033c155707e37d17fa606a4083a4cf3
/// <summary>
/// Implement your timer function
/// </summary>
static void MeasureTemperature_gx_handler(EventLoopTimer *eventLoopTimer) {
    if (ConsumeEventLoopTimerEvent(eventLoopTimer) != 0) {
        dx_terminate(DX_ExitCode_ConsumeEventLoopTimeEvent);
        return;
    }

    Log_Debug("Periodic timer MeasureTemperature_handler called\n");

    // Implement your timer function
}
/// GENX_END ID:MeasureTemperature


/// GENX_BEGIN ID:PublishTelemetry MD5:ef93ec45107e9e1a3b4281209c84d6c3
/// <summary>
/// Publish environment sensor telemetry to IoT Hub/Central
/// </summary>
/// <param name="eventLoopTimer"></param>
static void PublishTelemetry_gx_handler(EventLoopTimer *eventLoopTimer) {

    if (ConsumeEventLoopTimerEvent(eventLoopTimer) != 0) {
        dx_terminate(DX_ExitCode_ConsumeEventLoopTimeEvent);
        return;
    }

    float temperature = 30.0f;
    float humidity = 60.0f;
    float pressure = 1010.0f;

    double DesiredTemperature_value = 10.0;
    double DesiredCO2AlertLevel_value = 10.0;
    float ReportedTemperature_value = 30.0f;

    if (dx_isAzureConnected()) {

        // Serialize telemetry as JSON
        bool serialization_result = dx_jsonSerialize(gx_PublishTelemetryBuffer, sizeof(gx_PublishTelemetryBuffer), 3, 
            DX_JSON_DOUBLE, "Temperature", temperature, 
            DX_JSON_DOUBLE, "Humidity", humidity,
            DX_JSON_DOUBLE, "Pressure", pressure);

        if (serialization_result) {
            Log_Debug("%s\n", gx_PublishTelemetryBuffer);
            
            dx_azurePublish(gx_PublishTelemetryBuffer, strlen(gx_PublishTelemetryBuffer), gx_PublishTelemetryMessageProperties, 
                            NELEMS(gx_PublishTelemetryMessageProperties), &gx_PublishTelemetryContentProperties);
        
        } else {
            Log_Debug("gx_PublishTelemetryBuffer to small to serialize JSON\n");
        }

        dx_deviceTwinReportState(&dt_DesiredTemperature, &DesiredTemperature_value);     // DX_TYPE_DOUBLE
        dx_deviceTwinReportState(&dt_DesiredCO2AlertLevel, &DesiredCO2AlertLevel_value);     // DX_TYPE_DOUBLE
        dx_deviceTwinReportState(&dt_ReportedTemperature, &ReportedTemperature_value);     // DX_TYPE_FLOAT
    }
}
/// GENX_END ID:PublishTelemetry


/// GENX_BEGIN ID:RestartDevice MD5:8488b41f876ef95df8a4b8512d74e8b7
/// <summary>
/// Restart the Device
/// </summary>
static void DelayRestart_gx_handler(EventLoopTimer* eventLoopTimer)
{
    if (ConsumeEventLoopTimerEvent(eventLoopTimer) != 0)
    {
        dx_terminate(DX_ExitCode_ConsumeEventLoopTimeEvent);
        return;
    }
    PowerManagement_ForceSystemReboot();
}

/// <summary>
/// Start Device Power Restart Direct Method 'ResetMethod' integer seconds eg 5
/// </summary>
static DX_DIRECT_METHOD_RESPONSE_CODE RestartDevice_gx_handler(JSON_Value* json, DX_DIRECT_METHOD_BINDING* directMethodBinding, char** responseMsg)
{
    static struct timespec period;

    if (json_value_get_type(json) != JSONNumber) { return DX_METHOD_FAILED; }

    int seconds = (int)json_value_get_number(json);

    if (seconds > 2 && seconds < 10)
    {
        // Set One Shot DX_TIMER
        period = (struct timespec){ .tv_sec = seconds, .tv_nsec = 0 };
        dx_timerOneShotSet(&tmr_DelayRestart, &period);

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

