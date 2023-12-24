
#ifndef APP_H
#define APP_H

#include <openthread/instance.h>
#include "sl_sleeptimer.h"
#include "arm_math.h"

#ifdef __cplusplus
extern "C" {
#endif



typedef union {
    uint64_t _64b;
    struct {
        uint32_t l;
        uint32_t h;
    } _32b;
} eui_t;


typedef struct
{
	uint16_t k;
	uint32_t *val;
	uint16_t *idx;
} app_data_t;


extern eui_t eui;
extern app_data_t app_data;




void app_init(void);

void app_exit(void);

void appMain(void * pvParams);

otInstance *otGetInstance(void);

#define ACT_LED_PORT     gpioPortB
#define ACT_LED_PIN      2
#define ERR_LED_PORT     gpioPortB
#define ERR_LED_PIN      3


#define INT_OPT_PORT     gpioPortD
#define INT_OPT_PIN      5



#define PWR_ST_0_PORT    gpioPortC
#define PWR_ST_0_PIN     0
#define PWR_ST_1_PORT    gpioPortC
#define PWR_ST_1_PIN     1
#define PWR_ST_2_PORT    gpioPortC
#define PWR_ST_2_PIN     2
#define PWR_EN_ST_PORT   gpioPortC
#define PWR_EN_ST_PIN    3

#define PWR_PG_MCU_PORT  gpioPortC
#define PWR_PG_MCU_PIN   4
#define PWR_PG_SCD_PORT  gpioPortC
#define PWR_PG_SCD_PIN   5
#define PWR_EN_SCD_PORT  gpioPortC
#define PWR_EN_SCD_PIN   6

#define LOAD_SENSE_PORT  gpioPortC
#define LOAD_SENSE_PIN   7




void sl_ot_create_instance(void);
void sl_ot_cli_init(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif
