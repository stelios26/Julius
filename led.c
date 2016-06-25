#include "nrf.h"
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "app_util_platform.h"
#include "app_pwm.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "nrf_adc.h"
#include "nrf_delay.h"

#define RED_LED          								20
#define GREEN_LED       								21
#define BLUE_LED         								19
#define STEPPER_LED											13
#define CHRG_STAT_PIN	 									0
#define EN_5V_PIN												4												
#define	STEPPER_SLEEP_PIN								18
#define M_IN1														17
#define M_IN2														16
#define M_IN3														15
#define M_IN4														14

#define MAX_STEPS						158
#define RAMP_UP_STEP				500
#define DIAGNOSTIC_SPEED 		9000
#define SPEED_LIMIT					2500

uint8_t adc_result;
uint8_t position;

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

void step1(void)
{
		nrf_gpio_pin_set(M_IN1);
		nrf_gpio_pin_clear(M_IN2);
		nrf_gpio_pin_set(M_IN3);
		nrf_gpio_pin_clear(M_IN4);
}

void step2(void)
{
		nrf_gpio_pin_clear(M_IN1);
		nrf_gpio_pin_clear(M_IN2);
		nrf_gpio_pin_set(M_IN3);
		nrf_gpio_pin_clear(M_IN4);
}

void step3(void)
{
		nrf_gpio_pin_clear(M_IN1);
		nrf_gpio_pin_set(M_IN2);
		nrf_gpio_pin_set(M_IN3);
		nrf_gpio_pin_clear(M_IN4);
}

void step4(void)
{
		nrf_gpio_pin_clear(M_IN1);
		nrf_gpio_pin_set(M_IN2);
		nrf_gpio_pin_clear(M_IN3);
		nrf_gpio_pin_clear(M_IN4);
}

void step5(void)
{
		nrf_gpio_pin_clear(M_IN1);
		nrf_gpio_pin_set(M_IN2);
		nrf_gpio_pin_clear(M_IN3);
		nrf_gpio_pin_set(M_IN4);
}

void step6(void)
{
		nrf_gpio_pin_clear(M_IN1);
		nrf_gpio_pin_clear(M_IN2);
		nrf_gpio_pin_clear(M_IN3);
		nrf_gpio_pin_set(M_IN4);
}

void step7(void)
{
		nrf_gpio_pin_set(M_IN1);
		nrf_gpio_pin_clear(M_IN2);
		nrf_gpio_pin_clear(M_IN3);
		nrf_gpio_pin_set(M_IN4);
}

void step8(void)
{
		nrf_gpio_pin_set(M_IN1);
		nrf_gpio_pin_clear(M_IN2);
		nrf_gpio_pin_clear(M_IN3);
		nrf_gpio_pin_clear(M_IN4);
}

void rotateC(uint8_t i, uint16_t speed)
{
	while(position < i)
	{
		/*step7();
		nrf_delay_us(speed);
		step8();
		nrf_delay_us(speed);
		step2();
		nrf_delay_us(speed);
		step3();
		nrf_delay_us(speed);
		step4();
		nrf_delay_us(speed);
		step6();
		nrf_delay_us(speed);*/
		step3();
		nrf_delay_us(speed);
		step5();
		nrf_delay_us(speed);
		step7();
		nrf_delay_us(speed);
		step1();
		nrf_delay_us(speed);
		position++;
	}
}

void rotateCC(uint8_t i, uint16_t speed)
{
	while(position > i)
	{
		/*step6();
		nrf_delay_us(speed);
		step4();
		nrf_delay_us(speed);
		step3();
		nrf_delay_us(speed);
		step2();
		nrf_delay_us(speed);
		step8();
		nrf_delay_us(speed);
		step7();
		nrf_delay_us(speed);*/
		
		step1();
		nrf_delay_us(speed);
		step7();
		nrf_delay_us(speed);
		step5();
		nrf_delay_us(speed);
		step3();
		nrf_delay_us(speed);
		position--;
	}
}

void stepper_begin(void)
{
		uint32_t speed = DIAGNOSTIC_SPEED;
	
		nrf_gpio_pin_set(EN_5V_PIN);
		nrf_delay_ms(1); //let 5V boost stabilize
		nrf_gpio_pin_set(STEPPER_SLEEP_PIN);
		nrf_delay_ms(2);	//let stepper charge pump stabilize
		
		//for (position=0; position<MAX_STEPS; position++)
		//{
			/*step6();
			nrf_delay_ms(speed/1000);
			step4();
			nrf_delay_ms(speed/1000);
			step3();
			nrf_delay_ms(speed/1000);
			step2();
			nrf_delay_ms(speed/1000);
			step8();
			nrf_delay_ms(speed/1000);
			step7();
			nrf_delay_ms(speed/1000);*/
		//	step7();
			//nrf_delay_us(speed/1000);
			//step5();
		//	nrf_delay_us(speed/1000);
		//	step3();
		///	nrf_delay_us(speed/1000);
		//	step1();
		//	nrf_delay_us(speed/1000);
		//}
		
		position = 0;
		speed = 1500;
		rotateC(MAX_STEPS,speed);
		rotateCC(0,speed);
		rotateC(MAX_STEPS,speed);
		rotateCC(0,speed);
		rotateC(MAX_STEPS,speed);
		rotateCC(0,speed);
		rotateC(MAX_STEPS,speed);
		rotateCC(0,speed);
		rotateC(MAX_STEPS,speed);
		rotateCC(0,speed);
				rotateC(MAX_STEPS,speed);
		rotateCC(0,speed);
				rotateC(MAX_STEPS,speed);
		rotateCC(0,speed);
				rotateC(MAX_STEPS,speed);
		rotateCC(0,speed);
				rotateC(MAX_STEPS,speed);
		rotateCC(0,speed);
				rotateC(MAX_STEPS,speed);
		rotateCC(0,speed);
				rotateC(MAX_STEPS,speed);
		rotateCC(0,speed);
		
		
}

void stepper_end(void)
{
		while(position < MAX_STEPS)
		{
				step7();
				nrf_delay_ms(10);
				step8();
				nrf_delay_ms(10);
				step2();
				nrf_delay_ms(10);
				step3();
				nrf_delay_ms(10);
				step4();
				nrf_delay_ms(10);
				step6();
				nrf_delay_ms(10);
				position++;
		}
		
		nrf_gpio_pin_clear(EN_5V_PIN);
		nrf_gpio_pin_clear(STEPPER_SLEEP_PIN);
}

