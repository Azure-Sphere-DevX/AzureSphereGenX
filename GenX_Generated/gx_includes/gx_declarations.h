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
static uint32_t DeferredUpdateCalculate_gx_handler(uint32_t max_deferral_time_in_minutes, SysEvent_UpdateType type, SysEvent_Status status, const char* typeDescription, const char* statusDescription);
static void DeferredUpdateNotification_gx_handler(uint32_t max_deferral_time_in_minutes, SysEvent_UpdateType type, SysEvent_Status status, const char* typeDescription, const char* statusDescription);
static void SetPressure_gx_handler(DX_DEVICE_TWIN_BINDING* deviceTwinBinding);
static void SetDesiredPressure_gx_handler(DX_DEVICE_TWIN_BINDING* deviceTwinBinding);
static DX_DIRECT_METHOD_RESPONSE_CODE FanOff_gx_handler(JSON_Value *json, DX_DIRECT_METHOD_BINDING *directMethodBinding, char **responseMsg);
static DX_DIRECT_METHOD_RESPONSE_CODE FanOn_gx_handler(JSON_Value *json, DX_DIRECT_METHOD_BINDING *directMethodBinding, char **responseMsg);
static DX_DIRECT_METHOD_RESPONSE_CODE LightControl_gx_handler(JSON_Value *json, DX_DIRECT_METHOD_BINDING *directMethodBinding, char **responseMsg);
static void ButtonA_gx_handler(EventLoopTimer *eventLoopTimer);
static void MeasureTemperature_gx_handler(EventLoopTimer *eventLoopTimer);
static void Watchdog_gx_handler(EventLoopTimer *eventLoopTimer);


/****************************************************************************************
* Binding declarations
****************************************************************************************/
static DX_DEVICE_TWIN_BINDING dt_SetDesiredPressure = { .twinProperty = "SetDesiredPressure", .twinType = DX_TYPE_FLOAT, .handler = SetDesiredPressure_gx_handler };
static DX_DEVICE_TWIN_BINDING dt_SetPressure = { .twinProperty = "SetPressure", .twinType = DX_TYPE_FLOAT, .handler = SetPressure_gx_handler };
static DX_DIRECT_METHOD_BINDING dm_LightControl = { .methodName = "LightControl", .handler = LightControl_gx_handler };
static DX_GPIO_BINDING gpio_Light = { .pin = NETWORK_CONNECTED_LED, .name = "Light", .direction = DX_OUTPUT, .initialState = GPIO_Value_Low, .invertPin = false };
static DX_GPIO_BINDING gpio_ButtonA = { .pin = BUTTON_A, .name = "ButtonA", .direction = DX_INPUT, .detect = DX_GPIO_DETECT_LOW };
static DX_TIMER_BINDING tmr_ButtonA = { .period = { 0, 200000000 }, .name = "ButtonA", .handler = ButtonA_gx_handler };
static DX_DIRECT_METHOD_BINDING dm_FanOn = { .methodName = "FanOn", .handler = FanOn_gx_handler };
static DX_DIRECT_METHOD_BINDING dm_FanOff = { .methodName = "FanOff", .handler = FanOff_gx_handler };
static DX_TIMER_BINDING tmr_MeasureTemperature = { .name = "MeasureTemperature", .handler = MeasureTemperature_gx_handler };
static DX_TIMER_BINDING tmr_Watchdog = { .period = { 30, 0 }, .name = "Watchdog", .handler = Watchdog_gx_handler };
static DX_DEVICE_TWIN_BINDING dt_DeferredUpdateNotification = { .twinProperty = "DeferredUpdateNotification", .twinType = DX_TYPE_STRING };
static DX_DEVICE_TWIN_BINDING dt_DeferredUpdateRequest = { .twinProperty = "DeferredUpdateRequest", .twinType = DX_TYPE_STRING };


// All direct methods referenced in direct_method_bindings will be subscribed to in the gx_initPeripheralAndHandlers function
static DX_DEVICE_TWIN_BINDING* device_twin_binding_set[] = { &dt_SetDesiredPressure, &dt_SetPressure, &dt_DeferredUpdateNotification, &dt_DeferredUpdateRequest };

// All direct methods referenced in direct_method_bindings will be subscribed to in the gx_initPeripheralAndHandlers function
static DX_DIRECT_METHOD_BINDING *direct_method_binding_set[] = { &dm_LightControl, &dm_FanOn, &dm_FanOff };

// All GPIOs referenced in gpio_bindings with be opened in the gx_initPeripheralAndHandlers function
static DX_GPIO_BINDING *gpio_binding_set[] = { &gpio_Light, &gpio_ButtonA };

// All timers referenced in timer_bindings will be opened in the gx_initPeripheralAndHandlers function
#define DECLARE_DX_TIMER_BINDINGS
static DX_TIMER_BINDING *timer_binding_set[] = { &tmr_ButtonA, &tmr_MeasureTemperature, &tmr_Watchdog };

// All timers referenced in timer_bindings_oneshot will be started in the gx_initPeripheralAndHandlers function
static DX_TIMER_BINDING *timer_bindings_oneshot[] = {  };


/****************************************************************************************
* Initialise bindings
****************************************************************************************/

static void gx_initPeripheralAndHandlers(void)
{

#ifdef GX_AZURE_IOT
    dx_azureConnect(&dx_config, NETWORK_INTERFACE, PNP_MODEL_ID);
#else
    if (NELEMS(device_twin_binding_set) > 0 || NELEMS(direct_method_binding_set) > 0) {
        dx_azureConnect(&dx_config, NETWORK_INTERFACE, PNP_MODEL_ID);
    }
#endif // GX_AZURE_IOT

    if (NELEMS(gpio_binding_set) > 0) {
        dx_gpioSetOpen(gpio_binding_set, NELEMS(gpio_binding_set));
    }

    if (NELEMS(device_twin_binding_set) > 0) {
        dx_deviceTwinSubscribe(device_twin_binding_set, NELEMS(device_twin_binding_set));
    }

    if (NELEMS(direct_method_binding_set) > 0 ) {
        dx_directMethodSubscribe(direct_method_binding_set, NELEMS(direct_method_binding_set));
    }

    if (NELEMS(timer_binding_set) > 0) {
        dx_timerSetStart(timer_binding_set, NELEMS(timer_binding_set));
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
    if (NELEMS(timer_binding_set) > 0) {
	    dx_timerSetStop(timer_binding_set, NELEMS(timer_binding_set));
    }

    if (NELEMS(gpio_binding_set) > 0) {
        dx_gpioSetClose(gpio_binding_set, NELEMS(gpio_binding_set));
    }

	dx_deviceTwinUnsubscribe();
	dx_directMethodUnsubscribe();
    dx_azureToDeviceStop();
}
