#pragma once

#include "hw/azure_sphere_learning_path.h" // Hardware definition

#include "dx_azure_iot.h"
#include "dx_config.h"
#include "dx_exit_codes.h"
#include "dx_gpio.h"
#include "dx_json_serializer.h"
#include "dx_terminate.h"
#include "dx_timer.h"
#include "dx_version.h"
#include "dx_deferred_update.h"

#include "gx_includes.h"

#include <time.h>

DX_USER_CONFIG dx_config;   // cmd args from app_manifest.json

extern const char PNP_MODEL_ID[];
extern const char NETWORK_INTERFACE[];

#define ONE_MS 1000000		// used to simplify timer defn.
#define Log_Debug(f_, ...) dx_Log_Debug((f_), ##__VA_ARGS__)
static char Log_Debug_buffer[128];


/****************************************************************************************
 * Forward declarations
 ****************************************************************************************/
static void ButtonA_gx_handler(EventLoopTimer *eventLoopTimer);
static void CloudStatusLed_gx_handler(EventLoopTimer *eventLoopTimer);
static uint32_t DeferredUpdateCalculate_gx_handler(uint32_t max_deferral_time_in_minutes, SysEvent_UpdateType type, SysEvent_Status status, const char* typeDescription, const char* statusDescription);
static void DeferredUpdateNotification_gx_handler(uint32_t max_deferral_time_in_minutes, SysEvent_UpdateType type, SysEvent_Status status, const char* typeDescription, const char* statusDescription);
static DX_DIRECT_METHOD_RESPONSE_CODE LightOff_gx_handler(JSON_Value *json, DX_DIRECT_METHOD_BINDING *directMethodBinding, char **responseMsg);
static DX_DIRECT_METHOD_RESPONSE_CODE LightOn_gx_handler(JSON_Value *json, DX_DIRECT_METHOD_BINDING *directMethodBinding, char **responseMsg);
static void MeasureCarbonMonoxide_gx_handler(EventLoopTimer *eventLoopTimer);
static void MeasureTemperature_gx_handler(EventLoopTimer *eventLoopTimer);
static void PublishTelemetry_gx_handler(EventLoopTimer *eventLoopTimer);
static void ReportStartTime_gx_handler(EventLoopTimer *eventLoopTimer);
static void ReportStartup_gx_handler(EventLoopTimer *eventLoopTimer);
static DX_DIRECT_METHOD_RESPONSE_CODE RestartDevice_gx_handler(JSON_Value *json, DX_DIRECT_METHOD_BINDING *directMethodBinding, char **responseMsg);
static void RestartDeviceDelay_gx_handler(EventLoopTimer *eventLoopTimer);
static void SetDesiredCO2AlertLevel_gx_handler(DX_DEVICE_TWIN_BINDING* deviceTwinBinding);
static void SetDesiredTemperature_gx_handler(DX_DEVICE_TWIN_BINDING* deviceTwinBinding);
static void Watchdog_gx_handler(EventLoopTimer *eventLoopTimer);


/****************************************************************************************
* Binding declarations
****************************************************************************************/
static DX_DEVICE_TWIN_BINDING dt_SetDesiredTemperature = { .twinProperty = "SetDesiredTemperature", .twinType = DX_TYPE_FLOAT, .handler = SetDesiredTemperature_gx_handler };
static DX_DEVICE_TWIN_BINDING dt_SetDesiredCO2AlertLevel = { .twinProperty = "SetDesiredCO2AlertLevel", .twinType = DX_TYPE_DOUBLE, .handler = SetDesiredCO2AlertLevel_gx_handler };
static DX_DEVICE_TWIN_BINDING dt_ReportTemperature = { .twinProperty = "ReportTemperature", .twinType = DX_TYPE_FLOAT };
static DX_DIRECT_METHOD_BINDING dm_LightOn = { .methodName = "LightOn", .handler = LightOn_gx_handler };
static DX_DIRECT_METHOD_BINDING dm_LightOff = { .methodName = "LightOff", .handler = LightOff_gx_handler };
static DX_TIMER_BINDING tmr_MeasureTemperature = { .period = { 5, 0 }, .name = "MeasureTemperature", .handler = MeasureTemperature_gx_handler };
static DX_TIMER_BINDING tmr_MeasureCarbonMonoxide = { .name = "MeasureCarbonMonoxide", .handler = MeasureCarbonMonoxide_gx_handler };
static DX_TIMER_BINDING tmr_ReportStartTime = { .name = "ReportStartTime", .handler = ReportStartTime_gx_handler };
static DX_TIMER_BINDING tmr_ButtonA = { .period = { 0, 200000000 }, .name = "ButtonA", .handler = ButtonA_gx_handler };
static DX_GPIO_BINDING gpio_ButtonA = { .pin = BUTTON_A, .name = "ButtonA", .direction = DX_INPUT, .detect = DX_GPIO_DETECT_LOW };
static DX_TIMER_BINDING tmr_CloudStatusLed = { .period = { 5, 0 }, .name = "CloudStatusLed", .handler = CloudStatusLed_gx_handler };
static DX_GPIO_BINDING gpio_CloudStatusLed = { .pin = NETWORK_CONNECTED_LED, .name = "CloudStatusLed", .direction = DX_OUTPUT, .initialState = GPIO_Value_Low, .invertPin = true };
static DX_GPIO_BINDING gpio_Relay1 = { .pin = RELAY, .name = "Relay1", .direction = DX_OUTPUT, .initialState = GPIO_Value_Low, .invertPin = true };
static DX_DEVICE_TWIN_BINDING dt_DeferredUpdateNotification = { .twinProperty = "DeferredUpdateNotification", .twinType = DX_TYPE_STRING };
static DX_DEVICE_TWIN_BINDING dt_DeferredUpdateRequest = { .twinProperty = "DeferredUpdateRequest", .twinType = DX_TYPE_STRING };
static DX_TIMER_BINDING tmr_PublishTelemetry = { .period = { 5, 0 }, .name = "PublishTelemetry", .handler = PublishTelemetry_gx_handler };
static DX_DIRECT_METHOD_BINDING dm_RestartDevice = { .methodName = "RestartDevice", .handler = RestartDevice_gx_handler };
static DX_TIMER_BINDING tmr_RestartDeviceDelay = { .name = "RestartDeviceDelay", .handler = RestartDeviceDelay_gx_handler };
static DX_TIMER_BINDING tmr_Watchdog = { .period = { 30, 0 }, .name = "Watchdog", .handler = Watchdog_gx_handler };
static DX_DEVICE_TWIN_BINDING dt_DeviceStartUtc = { .twinProperty = "DeviceStartUtc", .twinType = DX_TYPE_STRING };
static DX_TIMER_BINDING tmr_ReportStartup = { .name = "ReportStartup", .handler = ReportStartup_gx_handler };
static DX_DEVICE_TWIN_BINDING dt_SoftwareVersion = { .twinProperty = "SoftwareVersion", .twinType = DX_TYPE_STRING };


// All direct methods referenced in direct_method_bindings will be subscribed to in the gx_initPeripheralAndHandlers function
static DX_DEVICE_TWIN_BINDING* device_twin_bindings[] = { &dt_SetDesiredTemperature, &dt_SetDesiredCO2AlertLevel, &dt_ReportTemperature, &dt_DeferredUpdateNotification, &dt_DeferredUpdateRequest, &dt_DeviceStartUtc, &dt_SoftwareVersion };

// All direct methods referenced in direct_method_bindings will be subscribed to in the gx_initPeripheralAndHandlers function
static DX_DIRECT_METHOD_BINDING *direct_method_bindings[] = { &dm_LightOn, &dm_LightOff, &dm_RestartDevice };

// All GPIOs referenced in gpio_bindings with be opened in the gx_initPeripheralAndHandlers function
static DX_GPIO_BINDING *gpio_bindings[] = { &gpio_ButtonA, &gpio_CloudStatusLed, &gpio_Relay1 };

// All timers referenced in timer_bindings will be opened in the gx_initPeripheralAndHandlers function
#define DECLARE_DX_TIMER_BINDINGS
static DX_TIMER_BINDING *timer_bindings[] = { &tmr_MeasureTemperature, &tmr_MeasureCarbonMonoxide, &tmr_ReportStartTime, &tmr_ButtonA, &tmr_CloudStatusLed, &tmr_PublishTelemetry, &tmr_RestartDeviceDelay, &tmr_Watchdog, &tmr_ReportStartup };

// All timers referenced in timer_bindings_oneshot will be started in the gx_initPeripheralAndHandlers function
static DX_TIMER_BINDING *timer_bindings_oneshot[] = { &tmr_MeasureCarbonMonoxide, &tmr_ReportStartTime, &tmr_ReportStartup };


/****************************************************************************************
* Initialise bindings
****************************************************************************************/

static void gx_initPeripheralAndHandlers(void)
{

#ifdef GX_AZURE_IOT
    dx_azureConnect(&dx_config, NETWORK_INTERFACE, PNP_MODEL_ID);
#else
    if (NELEMS(device_twin_bindings) > 0 || NELEMS(direct_method_bindings) > 0) {
        dx_azureConnect(&dx_config, NETWORK_INTERFACE, PNP_MODEL_ID);
    }
#endif // GX_AZURE_IOT

    if (NELEMS(gpio_bindings) > 0) {
        dx_gpioSetOpen(gpio_bindings, NELEMS(gpio_bindings));
    }

    if (NELEMS(device_twin_bindings) > 0) {
        dx_deviceTwinSubscribe(device_twin_bindings, NELEMS(device_twin_bindings));
    }

    if (NELEMS(direct_method_bindings) > 0 ) {
        dx_directMethodSubscribe(direct_method_bindings, NELEMS(direct_method_bindings));
    }

    if (NELEMS(timer_bindings) > 0) {
        dx_timerSetStart(timer_bindings, NELEMS(timer_bindings));
    }
    
    if (NELEMS(timer_bindings_oneshot) > 0) {
        for (int i = 0; i < NELEMS(timer_bindings_oneshot); i++) {
            // defaults to starting all oneshot timers after 1 second
            dx_timerOneShotSet(timer_bindings_oneshot[i], &(struct timespec){ 1, 0 });
        }
    }

#ifdef GX_DEFERRED_UPDATE
   	dx_deferredUpdateRegistration(DeferredUpdateCalculate_gx_handler, DeferredUpdateNotification_gx_handler);
#endif

}

static void gx_closePeripheralAndHandlers(void){
    if (NELEMS(timer_bindings) > 0) {
	    dx_timerSetStop(timer_bindings, NELEMS(timer_bindings));
    }

    if (NELEMS(gpio_bindings) > 0) {
        dx_gpioSetClose(gpio_bindings, NELEMS(gpio_bindings));
    }

	dx_deviceTwinUnsubscribe();
	dx_directMethodUnsubscribe();
    dx_azureToDeviceStop();
}
