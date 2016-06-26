#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

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
#define CENTER_STEP					79
#define RAMP_UP_STEP				50				
#define DIAGNOSTIC_SPEED 		5000
#define SPEED_LIMIT					1300

#define USB_CONNECTED										0x80
#define USB_DISCONNECTED								0x00
#define NOT_CHARGING										0x00
#define CHARGING												0x40

#define STEPPER_STEP_INTERVAL   				5				//min timer tick is 5 which with a 32768hz crystal and no prescaler is 150uS

extern uint8_t adc_result;
extern uint8_t position;
extern uint16_t speed;

void init_pwm(void);
void deinit_pwm(void);
void setRGBled(uint8_t RedDuty, uint8_t GreenDuty, uint8_t BlueDuty);
//uint32_t getRGBled();
void setMled(uint8_t MledDuty);
//uint8_t getMled();
void adc_init(void);
void init_gpio(void);
uint32_t is_charging(void);

void stepper_begin(void);
void stepTo(uint8_t i);
void stepper_end(void);
