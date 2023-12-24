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
#include "IIS3DWB/iis3dwb_reg.h"
#include "arm_math.h"
static otInstance *sInstance = NULL;

constexpr static uint32_t VSENSE_DIV_MULTIPLIER = 2;
extern "C" void otAppCliInit(otInstance *aInstance);
static void app_srp_init(void);





/* accelerometer info */
#define FIFO_WATERMARK    512 // max
static iis3dwb_fifo_out_raw_t fifo_data[FIFO_WATERMARK];
iis3dwb_fifo_status_t fifo_status;
stmdev_ctx_t dev_ctx;

/* FFT */
static arm_rfft_fast_instance_f32 *S;
const static uint16_t N = 4096U;
static float32_t fftInBuf[N];
static uint16_t fftInIdx = 0;
static float32_t fftOutBuf[N];
static float32_t fftMagBuf[N/2];
static float32_t fftPhaseBuf[N/2];
#define FFT_RMS



eui_t eui; // device EUI64
app_data_t app_data = { }; // application public variables

constexpr static uint32_t SLEEPY_POLL_PERIOD_MS = 5 * 1000;
constexpr static uint32_t ALIVE_SLEEPTIMER_INTERVAL_MS = 60 * 1000;
constexpr static uint32_t MEASUREMENT_INTERVAL_MS = 1800 * 1000;

static bool appSrpDone = false;
static bool appCoapSendAlive = false;
sl_sleeptimer_timer_handle_t alive_timer;

dns d(otGetInstance, 4, 4);



/** HANDLERS/ISR **/
void alive_cb(sl_sleeptimer_timer_handle_t *handle, void *data) {
	appCoapSendAlive = true;
}

void IADC_IRQHandler(void) {
	IADC_Result_t sample;
	sample = IADC_pullSingleFifoResult(IADC0);
	//app_data.vdd = ((sample.data * 1200) / 1000) * VSENSE_DIV_MULTIPLIER;
	IADC_clearInt(IADC0, IADC_IF_SINGLEDONE);
}

void BURTC_IRQHandler(void) {
	BURTC_IntClear(BURTC_IF_COMP); // compare match
	BURTC_CounterReset();
	BURTC_Stop();

	BURTC_IntDisable(BURTC_IEN_COMP);
}

/** Application STARTUP **/

void app_init(void) {
	sleepyInit(SLEEPY_POLL_PERIOD_MS);
	setNetworkConfiguration();
	assert(otIp6SetEnabled(sInstance, true) == OT_ERROR_NONE);
	assert(otThreadSetEnabled(sInstance, true) == OT_ERROR_NONE);
	appCoapInit();
	eui._64b = SYSTEM_GetUnique();
	app_srp_init();
	GPIO_PinOutClear(ACT_LED_PORT, ACT_LED_PIN);
	sl_sleeptimer_start_periodic_timer_ms(&alive_timer,
			ALIVE_SLEEPTIMER_INTERVAL_MS, alive_cb, NULL, 0, 0);
}

/** Application LOOP **/

void app_process_action(void) {
	otTaskletsProcess(sInstance);
	otSysProcessDrivers(sInstance);
	// algo here

}

static void app_dft_init() {
	arm_rfft_fast_init_f32(S, N);

}

static void app_dft_proc() {
	arm_rfft_fast_f32(S, fftInBuf, fftOutBuf, 0);

	// find mag
	arm_cmplx_mag_f32(fftOutBuf, fftOutBuf, N/2);
	// find std
	// find mean
	float32_t mean;
	arm_mean_f32(fftMagBuf, N/2, &mean);


	// find MAD
	float32_t mad = 0;
	for(int16_t i=0; i<(N/2); i++)
	{
		mad += fabsf(fftMagBuf[i] - mean);
	}
	mad /= (N/2);

	// find NF

	// O(n) iterate over X(k)

	// call parser return
}

static void app_sensor_proc() {

	static int16_t *datax;
	static int16_t *datay;
	static int16_t *dataz;
	static int32_t *ts;

	uint16_t num = 0, k;
	/* Read watermark flag */
	iis3dwb_fifo_status_get(&dev_ctx, &fifo_status);

	if (fifo_status.fifo_th == 1) {
		num = fifo_status.fifo_level;

		/* read out all FIFO entries in a single read */
		iis3dwb_fifo_out_multi_raw_get(&dev_ctx, fifo_data, num);

		for (k = 0; k < num; k++) {
			iis3dwb_fifo_out_raw_t *f_data;

			/* print out first two and last two FIFO entries only */


			f_data = &fifo_data[k];

			/* Read FIFO sensor value */
			datax = (int16_t*) &f_data->data[0];
			datay = (int16_t*) &f_data->data[2];
			dataz = (int16_t*) &f_data->data[4];
			ts = (int32_t*) &f_data->data[0];

			if(fftInIdx >= N)
			{
				app_dft_proc();
				fftInIdx = 0;
			}
#ifdef FFT_RMS
			else {
				float32_t rms = (float32_t) ((*datax) * (*datax)
						+ (*datay) * (*datay) + (*dataz) * (*dataz));
				fftInBuf[fftInIdx++] = sqrtf(rms);
			}
#else
			else fftInBuf[fftInIdx++] = (float32_t)datax;
#endif
		}
	}
}

static void app_sensor_init() {
	dev_ctx.write_reg = NULL;
	dev_ctx.read_reg = NULL;
//dev_ctx.handle = NULL;

	/* Check device ID */
	uint8_t whoamI = 0;
	iis3dwb_device_id_get(&dev_ctx, &whoamI);

	if (whoamI != IIS3DWB_ID)
		while (1)
			;

	/* Restore default configuration */
	iis3dwb_reset_set(&dev_ctx, PROPERTY_ENABLE);

	uint8_t rst;
	do {
		iis3dwb_reset_get(&dev_ctx, &rst);
	} while (rst);

	/* Enable Block Data Update */
	iis3dwb_block_data_update_set(&dev_ctx, PROPERTY_ENABLE);
	/* Set full scale */
	iis3dwb_xl_full_scale_set(&dev_ctx, IIS3DWB_8g);

	/*
	 * Set FIFO watermark (number of unread sensor data TAG + 6 bytes
	 * stored in FIFO) to FIFO_WATERMARK samples
	 */
	iis3dwb_fifo_watermark_set(&dev_ctx, FIFO_WATERMARK);
	/* Set FIFO batch XL ODR to 12.5Hz */
	iis3dwb_fifo_xl_batch_set(&dev_ctx, IIS3DWB_XL_BATCHED_AT_26k7Hz);

	/* Set FIFO mode to Stream mode (aka Continuous Mode) */
	iis3dwb_fifo_mode_set(&dev_ctx, IIS3DWB_STREAM_MODE);

	/* Set Output Data Rate */
	iis3dwb_xl_data_rate_set(&dev_ctx, IIS3DWB_XL_ODR_26k7Hz);
	iis3dwb_fifo_timestamp_batch_set(&dev_ctx, IIS3DWB_DEC_8);
	iis3dwb_timestamp_set(&dev_ctx, PROPERTY_ENABLE);

}

static void app_srp_init(void) {
	if (appSrpDone)
		return;
	appSrpDone = true;

	otError error = OT_ERROR_NONE;

	char *hostName;
	const char *HOST_NAME = "VIBRATION-SENSOR";
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
	snprintf(INST_NAME, 32, "ipv6bc%d", (uint8_t) (eui._64b & 0xFF));
	const char *SERV_NAME = "_ot._udp";
	string = otSrpClientBuffersGetServiceEntryInstanceNameString(entry, &size);
	memcpy(string, INST_NAME, size);

	string = otSrpClientBuffersGetServiceEntryServiceNameString(entry, &size);
	memcpy(string, SERV_NAME, size);

	error = otSrpClientAddService(sInstance, &entry->mService);

	assert(OT_ERROR_NONE == error);

	entry = NULL;

	otSrpClientEnableAutoStartMode(sInstance, /* aCallback */NULL, /* aContext */
	NULL);
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

void sl_ot_create_instance(void) {
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

void sl_ot_cli_init(void) {
//otAppCliInit(sInstance);
}

otInstance* otGetInstance(void) {
	return sInstance;
}

/**************************************************************************//**
 * Application Exit.`
 *****************************************************************************/
void app_exit(void) {
	otInstanceFinalize(sInstance);
}
