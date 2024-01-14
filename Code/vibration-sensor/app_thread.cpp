#include "app_thread.h"
#include "app_main.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include <openthread/cli.h>
#include <openthread/dataset_ftd.h>
#include <openthread/instance.h>
#include <openthread/message.h>
#include <openthread/thread.h>
#include <openthread/udp.h>
#include <openthread/platform/logging.h>
#include <common/code_utils.hpp>
#include <common/logging.hpp>



#include "sl_component_catalog.h"
#ifdef SL_CATALOG_POWER_MANAGER_PRESENT
#include "sl_power_manager.h"
#endif



otInstance *otGetInstance(void);

constexpr static bool LOW_POWER_MODE = true;

void sleepyInit(uint32_t poll_period)
{
    otError error;

    otLinkModeConfig config;
    SuccessOrExit(error = otLinkSetPollPeriod(otGetInstance(), poll_period));

    config.mRxOnWhenIdle = false;
    config.mDeviceType   = 0;
    config.mNetworkData  = 0;
    SuccessOrExit(error = otThreadSetLinkMode(otGetInstance(), config));

exit:
    return;
}

/*
 * Callback from sl_ot_is_ok_to_sleep to check if it is ok to go to sleep.
 */
inline bool efr32AllowSleepCallback(void)
{
    return LOW_POWER_MODE;
}

/*
 * Override default network settings, such as panid, so the devices can join a network
 */
void setNetworkConfiguration(void)
{

}


