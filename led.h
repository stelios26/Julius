#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

void init_pwm(void);
void deinit_pwm(void);
void setRGBled(uint8_t RedDuty, uint8_t GreenDuty, uint8_t BlueDuty);
void setMled(uint8_t MledDuty);
