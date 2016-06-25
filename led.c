#include "nrf.h"
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "app_util_platform.h"
#include "app_pwm.h"
#include "app_timer.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "nrf_adc.h"
#include "nrf_delay.h"
#include "led.h"
#include "stepper.h"

uint8_t adc_result;
uint8_t position = 0;
uint16_t speed = DIAGNOSTIC_SPEED;

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
/*
uint32_t getRGBled()
{
	uint8_t red, green, blue;
	
	red 	= app_pwm_channel_duty_get(&PWM1, 0);
	green = app_pwm_channel_duty_get(&PWM1, 1);
	blue	= app_pwm_channel_duty_get(&PWM2, 0);
	
	return ( (blue << 16) | (green << 8) | red );
}

uint8_t getMled()
{
		return app_pwm_channel_duty_get(&PWM2, 1);
}
*/
void setMled(uint8_t MledDuty)
{
		while (app_pwm_channel_duty_set(&PWM2, 1, MledDuty) == NRF_ERROR_BUSY);
}

//ADC initialization
void adc_init(void)
{	
	  nrf_adc_config_t nrf_adc_config;

		memset(&nrf_adc_config, 0, sizeof(nrf_adc_config));
		nrf_adc_config.reference 	= NRF_ADC_CONFIG_REF_SUPPLY_ONE_THIRD;
		nrf_adc_config.resolution = NRF_ADC_CONFIG_RES_8BIT;
		nrf_adc_config.scaling 		= NRF_ADC_CONFIG_SCALING_INPUT_ONE_THIRD;
	
    // Initialize and configure ADC
    nrf_adc_configure( (nrf_adc_config_t *)&nrf_adc_config);
    nrf_adc_input_select(NRF_ADC_CONFIG_INPUT_2);
    nrf_adc_int_enable(ADC_INTENSET_END_Enabled << ADC_INTENSET_END_Pos);
    NVIC_SetPriority(ADC_IRQn, NRF_APP_PRIORITY_HIGH);
    NVIC_EnableIRQ(ADC_IRQn);
}

/* Interrupt handler for ADC data ready event */
void ADC_IRQHandler(void)
{
    nrf_adc_conversion_event_clean();

    adc_result = nrf_adc_result_get();
}

void init_gpio(void)
{
		nrf_gpio_cfg_input(CHRG_STAT_PIN, NRF_GPIO_PIN_PULLUP);
	
		nrf_gpio_cfg_output(EN_5V_PIN);
		nrf_gpio_cfg_output(STEPPER_SLEEP_PIN);
		nrf_gpio_cfg_output(M_IN1);
		nrf_gpio_cfg_output(M_IN2);
		nrf_gpio_cfg_output(M_IN3);
		nrf_gpio_cfg_output(M_IN4);
}

uint32_t is_charging(void)
{
	return !(nrf_gpio_pin_read(CHRG_STAT_PIN));
}

void step(uint8_t step, uint16_t speed)
{
	switch(step)
	{
		case 1:
				nrf_gpio_pin_clear(M_IN1);
				nrf_gpio_pin_set(M_IN2);
				nrf_gpio_pin_set(M_IN3);
				nrf_gpio_pin_clear(M_IN4);
				break;
		
		case 2:
				nrf_gpio_pin_clear(M_IN1);
				nrf_gpio_pin_set(M_IN2);
				nrf_gpio_pin_clear(M_IN3);
				nrf_gpio_pin_set(M_IN4);
				break;
		
		case 3:
				nrf_gpio_pin_set(M_IN1);
				nrf_gpio_pin_clear(M_IN2);
				nrf_gpio_pin_clear(M_IN3);
				nrf_gpio_pin_set(M_IN4);
				break;
		
		case 4:
				nrf_gpio_pin_set(M_IN1);
				nrf_gpio_pin_clear(M_IN2);
				nrf_gpio_pin_set(M_IN3);
				nrf_gpio_pin_clear(M_IN4);
				break;
	}
	nrf_delay_us(speed);
}

void stepTo(uint8_t i)//, uint16_t speed)
{
	if (position < i)
	{
		step(1,speed);
		step(2,speed);
		step(3,speed);
		step(4,speed-150);
		position++;
	}
	else if (position > i)
	{
		step(4,speed);
		step(3,speed);
		step(2,speed);
		step(1,speed-150);
		position--;
	}
}

void stepper_begin(void)
{
		uint8_t i;
	
		nrf_gpio_pin_set(EN_5V_PIN);
		nrf_delay_ms(1); //let 5V boost stabilize
		nrf_gpio_pin_set(STEPPER_SLEEP_PIN);
		nrf_delay_ms(2);	//let stepper charge pump stabilize
		
		for (i=1; i<MAX_STEPS+1; i++)
		{
			speed -= RAMP_UP_STEP;
			stepTo(i);
		}
		
		//speed = 1366 at the end of the for loop, just a bit of margin
		//speed is actually period between successive steps so - means faster
		//to actually force a step, need to be one below/above the position watch variable
}
/*

void stepper_end(void)
{
		stepTo(0,SPEED_LIMIT);
	
		nrf_gpio_pin_clear(EN_5V_PIN);
		nrf_gpio_pin_clear(STEPPER_SLEEP_PIN);
}*/

