#include "nrf.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "app_util_platform.h"
#include "app_pwm.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"

#define RED_LED          					20
#define GREEN_LED       					21
#define BLUE_LED         					19
#define STEPPER_LED								13

APP_PWM_INSTANCE(PWM1,1);                   // Create the instance "PWM1" using TIMER1.
APP_PWM_INSTANCE(PWM2,2);                   // Create the instance "PWM2" using TIMER2.

static volatile bool ready_flag;            // A flag indicating PWM status.

void pwm_ready_callback(uint32_t pwm_id)    // PWM callback function
{
    ready_flag = true;
}

void init_pwm(void)
{
		ret_code_t err_code;
	
		app_pwm_config_t pwm1_cfg = APP_PWM_DEFAULT_CONFIG_2CH(5000L, RED_LED, GREEN_LED);
		app_pwm_config_t pwm2_cfg = APP_PWM_DEFAULT_CONFIG_2CH(5000L, BLUE_LED, STEPPER_LED);
	
		pwm2_cfg.pin_polarity[1] = APP_PWM_POLARITY_ACTIVE_HIGH;
		
		err_code = app_pwm_init(&PWM1,&pwm1_cfg,pwm_ready_callback);
    APP_ERROR_CHECK(err_code);
	
		err_code = app_pwm_init(&PWM2,&pwm2_cfg,pwm_ready_callback);
    APP_ERROR_CHECK(err_code);
	
		app_pwm_enable(&PWM1);
		app_pwm_enable(&PWM2);
}

void deinit_pwm(void)
{
		ret_code_t err_code;
	
		err_code = app_pwm_uninit(&PWM1);
		APP_ERROR_CHECK(err_code);
	
		err_code = app_pwm_uninit(&PWM2);
		APP_ERROR_CHECK(err_code);
	
		nrf_gpio_cfg_output(RED_LED);
		nrf_gpio_cfg_output(GREEN_LED);
		nrf_gpio_cfg_output(BLUE_LED);
		nrf_gpio_cfg_output(STEPPER_LED);
}

void setRGBled(uint8_t RedDuty, uint8_t GreenDuty, uint8_t BlueDuty)
{
		while (app_pwm_channel_duty_set(&PWM1, 0, RedDuty) == NRF_ERROR_BUSY);
		while (app_pwm_channel_duty_set(&PWM1, 1, GreenDuty) == NRF_ERROR_BUSY);
		while (app_pwm_channel_duty_set(&PWM2, 0, BlueDuty) == NRF_ERROR_BUSY);
}

void setMled(uint8_t MledDuty)
{
		while (app_pwm_channel_duty_set(&PWM2, 1, MledDuty) == NRF_ERROR_BUSY);
}
