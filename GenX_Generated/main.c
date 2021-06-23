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


/// GENX_BEGIN ID:FanOn MD5:28b16ab201bc15881b8c1768daf0e756
/// <summary>
/// What is the purpose of this direct method handler function?
/// </summary>
// Direct method JSON payload example {"Duration":2}:
static DX_DIRECT_METHOD_RESPONSE_CODE FanOn_gx_handler(JSON_Value *json, DX_DIRECT_METHOD_BINDING *directMethodBinding, char **responseMsg) {
    char duration_str[] = "Duration";
    int requested_duration_seconds;

    // Allocate and initialize a response message buffer. 
    // The calling function is responsible for freeing the memory
    const size_t responseLen = 100; 
    *responseMsg = (char*)malloc(responseLen);
    memset(*responseMsg, 0, responseLen);

    JSON_Object *jsonObject = json_value_get_object(json);
    if (jsonObject == NULL) {
        snprintf(*responseMsg, responseLen, "%s call failed. Invalid JSON received type.", directMethodBinding->methodName);
        return DX_METHOD_FAILED;
    }

    // check JSON properties sent through are the correct type
    if (!json_object_has_value_of_type(jsonObject, duration_str, JSONNumber)) {
        snprintf(*responseMsg, responseLen, "%s call failed. Incorrect JSON type.", directMethodBinding->methodName);
        return DX_METHOD_FAILED;
    }

    requested_duration_seconds = (int)json_object_get_number(jsonObject, duration_str);
    Log_Debug("Duration %d \n", requested_duration_seconds);

    if (requested_duration_seconds < 0 || requested_duration_seconds > 120 ) {
        snprintf(*responseMsg, responseLen, "%s call failed. Duration seconds (%d) out of range.", directMethodBinding->methodName, requested_duration_seconds);
        return DX_METHOD_FAILED;
    }

    // IMPLEMENT YOUR ACTION HERE

    snprintf(*responseMsg, responseLen, "%s called successfully", directMethodBinding->methodName);

    return DX_METHOD_SUCCEEDED;
}
/// GENX_END ID:FanOn

