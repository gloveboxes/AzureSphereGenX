#pragma once

#include "dx_azure_iot.h"
#include "dx_config.h"
#include "dx_exit_codes.h"
#include "dx_gpio.h"
#include "dx_json_serializer.h"
#include "dx_terminate.h"
#include "dx_timer.h"
#include "dx_version.h"
#include "dx_deferred_update.h"


/****************************************************************************************
 * Forward declarations
 ****************************************************************************************/
static void PressureAlertLevel_gx_handler(DX_DEVICE_TWIN_BINDING* deviceTwinBinding);
static DX_DECLARE_DIRECTMETHOD_HANDLER(FanOn_gx_handler);
static DX_DECLARE_DIRECTMETHOD_HANDLER(OfficeLightOn_gx_handler);
static DX_DECLARE_TIMER_HANDLER(MeasureTemperature_gx_handler);


/****************************************************************************************
* Binding declarations
****************************************************************************************/
static DX_DEVICE_TWIN_BINDING dt_PressureAlertLevel = { .twinProperty = "PressureAlertLevel", .twinType = DX_TYPE_FLOAT, .handler = PressureAlertLevel_gx_handler };
static DX_GPIO_BINDING gpio_Light = { .pin = NETWORK_CONNECTED_LED, .name = "Light", .direction = DX_OUTPUT, .initialState = GPIO_Value_Low, .invertPin = false };
static DX_DIRECT_METHOD_BINDING dm_OfficeLightOn = { .methodName = "OfficeLightOn", .handler = OfficeLightOn_gx_handler, .context=&gpio_Light };
static DX_DIRECT_METHOD_BINDING dm_FanOn = { .methodName = "FanOn", .handler = FanOn_gx_handler, .context=&gpio_Fan };
static DX_TIMER_BINDING tmr_MeasureTemperature = { .period = { 5, 0 }, .name = "MeasureTemperature", .handler = MeasureTemperature_gx_handler };


// All direct methods referenced in direct_method_bindings will be subscribed to in the gx_initPeripheralAndHandlers function
static DX_DEVICE_TWIN_BINDING* device_twin_binding_set[] = { &dt_PressureAlertLevel };

// All direct methods referenced in direct_method_bindings will be subscribed to in the gx_initPeripheralAndHandlers function
static DX_DIRECT_METHOD_BINDING *direct_method_binding_set[] = { &dm_OfficeLightOn, &dm_FanOn };

// All GPIOs referenced in gpio_bindings with be opened in the gx_initPeripheralAndHandlers function
static DX_GPIO_BINDING *gpio_binding_set[] = { &gpio_Light };

// All timers referenced in timer_bindings will be opened in the gx_initPeripheralAndHandlers function
#define DECLARE_DX_TIMER_BINDINGS
static DX_TIMER_BINDING *timer_binding_set[] = { &tmr_MeasureTemperature };

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