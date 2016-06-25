#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#define USB_CONNECTED										0x80
#define USB_DISCONNECTED								0x00
#define NOT_CHARGING										0x00
#define CHARGING												0x40

extern uint8_t adc_result;
extern uint8_t position;

void init_pwm(void);
void deinit_pwm(void);
void setRGBled(uint8_t RedDuty, uint8_t GreenDuty, uint8_t BlueDuty);
void setMled(uint8_t MledDuty);
void adc_init(void);
void init_gpio(void);
uint32_t is_charging(void);

void stepper_begin(void);
void step1(void);
void step2(void);
void step3(void);
void step4(void);
void step5(void);
void step6(void);
void step7(void);
void step8(void);
void rotateC(uint8_t i, uint16_t speed);
void rotateCC(uint8_t i, uint16_t speed);
void stepper_end(void);
