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
static uint32_t DeferredUpdateCalculate_gx_handler(uint32_t max_deferral_time_in_minutes, SysEvent_UpdateType type,
	SysEvent_Status status, const char* typeDescription, const char* statusDescription);

static void DeferredUpdateNotification_gx_handler(uint32_t max_deferral_time_in_minutes, SysEvent_UpdateType type,
	SysEvent_Status status, const char* typeDescription, const char* statusDescription);

static DX_DIRECT_METHOD_RESPONSE_CODE LightOff_gx_handler(JSON_Value *json, DX_DIRECT_METHOD_BINDING *directMethodBinding, char **responseMsg);
static DX_DIRECT_METHOD_RESPONSE_CODE LightOn_gx_handler(JSON_Value *json, DX_DIRECT_METHOD_BINDING *directMethodBinding, char **responseMsg);
static void ReportStartState_gx_handler(EventLoopTimer *eventLoopTimer);
static DX_DIRECT_METHOD_RESPONSE_CODE RestartDevice_gx_handler(JSON_Value *json, DX_DIRECT_METHOD_BINDING *directMethodBinding, char **responseMsg);
static void Watchdog_gx_handler(EventLoopTimer *eventLoopTimer);


static DX_DEVICE_TWIN_BINDING dt_DeferredUpdateNotification = { .twinProperty = "DeferredUpdateNotification", .twinType = DX_TYPE_STRING };
static DX_DEVICE_TWIN_BINDING dt_DeferredUpdateRequest = { .twinProperty = "DeferredUpdateRequest", .twinType = DX_TYPE_STRING };
static DX_DIRECT_METHOD_BINDING dm_RestartDevice = { .methodName = "RestartDevice", .handler = RestartDevice_gx_handler };
static DX_GPIO_BINDING gpio_Light1 = { .pin = RELAY, .name = "Light1", .direction = DX_OUTPUT, .initialState = GPIO_Value_Low, .invertPin = true };
static DX_DIRECT_METHOD_BINDING dm_LightOff = { .methodName = "LightOff", .handler = LightOff_gx_handler };
static DX_DIRECT_METHOD_BINDING dm_LightOn = { .methodName = "LightOn", .handler = LightOn_gx_handler };
static DX_TIMER_BINDING tmr_Watchdog = { .period = { 30, 0 }, .name = "Watchdog", .handler = Watchdog_gx_handler };
static DX_DEVICE_TWIN_BINDING dt_DeviceStartUtc = { .twinProperty = "DeviceStartUtc", .twinType = DX_TYPE_STRING };
static DX_TIMER_BINDING tmr_ReportStartState = { .period = { 0, 0 }, .name = "ReportStartState", .handler = ReportStartState_gx_handler };
static DX_DEVICE_TWIN_BINDING dt_SoftwareVersion = { .twinProperty = "SoftwareVersion", .twinType = DX_TYPE_STRING };



// All direct methods referenced in direct_method_bindings will be subscribed to in the InitPeripheralsAndHandlers function
static DX_DEVICE_TWIN_BINDING* device_twin_bindings[] = { &dt_DeferredUpdateNotification, &dt_DeferredUpdateRequest, &dt_DeviceStartUtc, &dt_SoftwareVersion };

// All direct methods referenced in direct_method_bindings will be subscribed to in the InitPeripheralsAndHandlers function
static DX_DIRECT_METHOD_BINDING *direct_method_bindings[] = { &dm_RestartDevice, &dm_LightOff, &dm_LightOn };

// All GPIOs referenced in gpio_bindings with be opened in the InitPeripheralsAndHandlers function
static DX_GPIO_BINDING *gpio_bindings[] = { &gpio_Light1 };

// All timers referenced in timer_bindings will be opened in the InitPeripheralsAndHandlers function
#define DECLARE_DX_TIMER_BINDINGS
static DX_TIMER_BINDING *timer_bindings[] = { &tmr_Watchdog, &tmr_ReportStartState };


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
