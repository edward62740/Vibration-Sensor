#include <app_main.h>
#include <app_thread.h>
#include <app_coap.h>
#include "app_dns.h"
#include <assert.h>
#include <openthread-core-config.h>
#include <openthread/config.h>
#include <openthread/cli.h>
#include <openthread/diag.h>
#include <openthread/tasklet.h>
#include <openthread/thread.h>
#include <openthread/srp_client.h>
#include <openthread/srp_client_buffers.h>
#include "openthread-system.h"
#include "sl_ot_init.h"
#include "sl_component_catalog.h"
#include "em_burtc.h"
#include "em_iadc.h"
#include "em_gpio.h"
#include "sl_sleeptimer.h"
#include "sl_system_process_action.h"
#include <stdio.h>
#include <string.h>

static otInstance* sInstance = NULL;

constexpr static uint32_t VSENSE_DIV_MULTIPLIER = 2;
extern "C" void otAppCliInit(otInstance *aInstance);
static void appSrpInit(void);

static void app_power_status(void);

eui_t eui; // device EUI64
app_data_t app_data = {}; // application public variables


constexpr static uint32_t SLEEPY_POLL_PERIOD_MS = 5 * 1000;
constexpr static uint32_t ALIVE_SLEEPTIMER_INTERVAL_MS = 60 * 1000;
constexpr static uint32_t MEASUREMENT_INTERVAL_MS = 1800 * 1000;

static bool appSrpDone = false;
static bool appCoapSendAlive = false;
sl_sleeptimer_timer_handle_t alive_timer;


dns d(otGetInstance, 4, 4);

/** HANDLERS/ISR **/
void alive_cb(sl_sleeptimer_timer_handle_t *handle, void *data)
{
	appCoapSendAlive = true;
}

void IADC_IRQHandler(void) {
	IADC_Result_t sample;
	sample = IADC_pullSingleFifoResult(IADC0);
	app_data.vdd = ((sample.data * 1200) / 1000) * VSENSE_DIV_MULTIPLIER;
	IADC_clearInt(IADC0, IADC_IF_SINGLEDONE);
}


void BURTC_IRQHandler(void)
{
    BURTC_IntClear(BURTC_IF_COMP); // compare match
	BURTC_CounterReset();
	BURTC_Stop();




	BURTC_IntDisable(BURTC_IEN_COMP);
}


/** Application STARTUP **/

void app_init(void)
{
    sleepyInit(SLEEPY_POLL_PERIOD_MS);
    setNetworkConfiguration();
    assert(otIp6SetEnabled(sInstance, true) == OT_ERROR_NONE);
    assert(otThreadSetEnabled(sInstance, true) == OT_ERROR_NONE);
    appCoapInit();
	eui._64b = SYSTEM_GetUnique();
    appSrpInit();
    GPIO_PinOutClear(ACT_LED_PORT, ACT_LED_PIN);
	sl_sleeptimer_start_periodic_timer_ms(&alive_timer,
			ALIVE_SLEEPTIMER_INTERVAL_MS, alive_cb, NULL, 0, 0);
}

/** Application LOOP **/

void app_process_action(void)
{
    otTaskletsProcess(sInstance);
    otSysProcessDrivers(sInstance);
    // algo here
    if(app_data.isPend) app_data.isPend = !appCoapCts(&app_data, MSG_DATA);
    else if(appCoapSendAlive) { appCoapCts(&app_data, MSG_ALIVE); appCoapSendAlive = false; }
}


static void appSrpInit(void)
{
    if(appSrpDone) return;
    appSrpDone = true;

    otError error = OT_ERROR_NONE;

    char *hostName;
    const char *HOST_NAME = "OT-CO2SN-1";
    uint16_t size;
    hostName = otSrpClientBuffersGetHostNameString(sInstance, &size);
    error = otSrpClientSetHostName(sInstance, HOST_NAME);
    memcpy(hostName, HOST_NAME, sizeof(HOST_NAME) + 1);

    otSrpClientEnableAutoHostAddress(sInstance);


    otSrpClientBuffersServiceEntry *entry = NULL;
    char *string;

    entry = otSrpClientBuffersAllocateService(sInstance);

    entry->mService.mPort = 33434;
    char INST_NAME[32];
    snprintf(INST_NAME, 32, "ipv6bc%d", (uint8_t)(eui._64b & 0xFF));
    const char *SERV_NAME = "_ot._udp";
    string = otSrpClientBuffersGetServiceEntryInstanceNameString(entry, &size);
    memcpy(string, INST_NAME, size);


    string = otSrpClientBuffersGetServiceEntryServiceNameString(entry, &size);
    memcpy(string, SERV_NAME, size);

    error = otSrpClientAddService(sInstance, &entry->mService);

    assert(OT_ERROR_NONE == error);

    entry = NULL;

    otSrpClientEnableAutoStartMode(sInstance, /* aCallback */ NULL, /* aContext */ NULL);
}




static void app_power_status(void)
{
	app_data.pwr_state = (GPIO_PinInGet(PWR_ST_0_PORT, PWR_ST_0_PIN) & 0x1) |
			(GPIO_PinInGet(PWR_ST_1_PORT, PWR_ST_1_PIN) & 0x1) << 1;
}


/*
 * Provide, if required an "otPlatLog()" function
 */
#if OPENTHREAD_CONFIG_LOG_OUTPUT == OPENTHREAD_CONFIG_LOG_OUTPUT_APP
void otPlatLog(otLogLevel aLogLevel, otLogRegion aLogRegion, const char *aFormat, ...)
{
    OT_UNUSED_VARIABLE(aLogLevel);
    OT_UNUSED_VARIABLE(aLogRegion);
    OT_UNUSED_VARIABLE(aFormat);

    va_list ap;
    va_start(ap, aFormat);
    otCliPlatLogv(aLogLevel, aLogRegion, aFormat, ap);
    va_end(ap);
}
#endif

void sl_ot_create_instance(void)
{
#if OPENTHREAD_CONFIG_MULTIPLE_INSTANCE_ENABLE
    size_t   otInstanceBufferLength = 0;
    uint8_t *otInstanceBuffer       = NULL;

    // Call to query the buffer size
    (void)otInstanceInit(NULL, &otInstanceBufferLength);

    // Call to allocate the buffer
    otInstanceBuffer = (uint8_t *)malloc(otInstanceBufferLength);
    assert(otInstanceBuffer);

    // Initialize OpenThread with the buffer
    sInstance = otInstanceInit(otInstanceBuffer, &otInstanceBufferLength);
#else
    sInstance = otInstanceInitSingle();
#endif
    assert(sInstance);
}

void sl_ot_cli_init(void)
{
    //otAppCliInit(sInstance);
}


otInstance *otGetInstance(void)
{
    return sInstance;
}


/**************************************************************************//**
 * Application Exit.`
 *****************************************************************************/
void app_exit(void)
{
    otInstanceFinalize(sInstance);
}