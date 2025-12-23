#ifndef __MAIN_H
#define __MAIN_H

#include <stm32f4xx_hal.h>
#include <stm32f4xx_hal_tim.h>
#include <FreeRTOS.h>
#include <task.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <timers.h>
#include <queue.h>
#include <semphr.h>
#include <stdbool.h>

void System_Init(void);
void Clock_Init(void);
void UART1_Init(void);
void GPIO_Init(void);
void Error_Handler(void);

void TIM1_Init(void);
void ADC1_Init(void);
void blinkFunction(TimerHandle_t);

//FreeRTOS Tasks
void control_task(void *);
void parse_task(void *);
void debug_task (void *);

void vApplicationIdleHook(void);
uint16_t computeDeadTime(uint16_t dead_time);

#endif /* __MAIN_H */
