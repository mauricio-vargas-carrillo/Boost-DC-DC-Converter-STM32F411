#include <main.h>
#include <Low_Pass.h>

UART_HandleTypeDef huart1 ={0};
TIM_HandleTypeDef htim1 = {0};
ADC_HandleTypeDef hadc1 = {0};
SemaphoreHandle_t semParse;

TimerHandle_t xTimer;
QueueHandle_t xqueue;

//Control variables
float error = 0;
uint16_t uOUT; // Variable for PWM output
float Pruebas = 8;//Value for testing PWM without load

// Variable for sending and receiving data
uint8_t byteRec, bufIn[64], idx;

// Variable for voltage detection in the ADC
float vADC, vreal, vref = 24.0, vfil;

int main(void)
{
	System_Init();
	xqueue = xQueueCreate(64, sizeof(char));
	semParse = xSemaphoreCreateBinary();
	xTimer = xTimerCreate("Blink", pdMS_TO_TICKS(250), pdTRUE, NULL, blinkFunction);
	xTaskCreate(control_task, "control", configMINIMAL_STACK_SIZE, NULL, 4, NULL);
	xTaskCreate(parse_task, "Parse", configMINIMAL_STACK_SIZE * 4, NULL, 1, NULL);
	xTaskCreate(debug_task, "Debug", configMINIMAL_STACK_SIZE * 4, NULL, 0, NULL);
	xTimerStart(xTimer, 10);
	vTaskStartScheduler();
	return 0;
}

void control_task(void *pvParameters)
{
	UNUSED(pvParameters);
	TickType_t xLastWakeTime;
	const TickType_t xFrecuency = pdMS_TO_TICKS(1);
	xLastWakeTime = xTaskGetTickCount();


	uOUT = 8;
	uint16_t Valor_ideal = uOUT;
	uint16_t Incremento_PWM;

	const float Margen_error = 0.05f;
	const float Margen_Setpoint = 0.10f;

	uint8_t Controlador = 0;

	const uint16_t PWM_MAX = 550;
	const uint16_t PWM_MIN = 6;
	const float Error_GD = 1.0f;	// If error > 1V -> high speed.
	const float Error_MD   = 0.2f;	// If error < 0.2V -> medium speed.
	const uint16_t Incremento_GD = 6;	// Big step.
	const uint16_t Incremento_MD = 2;	// Medium step.
	const uint16_t Incremento_CH   = 1;	// Fine step.


	while(1)
	{
		vTaskDelayUntil(&xLastWakeTime, xFrecuency);


		// Reading the ADC.
		uint16_t adc_value = HAL_ADC_GetValue(&hadc1);
		vADC = adc_value * (3.3/4095.0);  //Conversion factor.

		// Conversion to actual voltage
		vreal = vADC * ((47.0 + 7.0) / 7.0);  // (R1=47k, R2=6.8k)

		// Digital Filtering
		vfil = filter_compute(vreal);

		// Error calculation
		error = vref - vfil;

		if(fabs(error) <= Margen_Setpoint)
		{
			if (Controlador == 0)
			{
				// The voltage is within range.
				Valor_ideal = uOUT;
				Controlador = 1;
			}
			// We update the PWM with the correct value.
			__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, Valor_ideal);
		}

		else
		{
			float abs_err = fabs(error);


			if (abs_err > Error_GD) Incremento_PWM = Incremento_GD;
			else if (abs_err > Error_MD) Incremento_PWM = Incremento_MD;
			else Incremento_PWM = Incremento_CH;


			Controlador = 0;
			if (error > Margen_error)
			{
				uOUT += Incremento_PWM;
			}
			else if (error < -Margen_error)
			{
				uOUT -= Incremento_PWM;
			}

			//Safety limits
			if (uOUT > PWM_MAX) uOUT = PWM_MAX;
			if (uOUT < PWM_MIN) uOUT = PWM_MIN;

			__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, uOUT);
		}

		//*/

		/*Tests
		__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, (uint16_t)Pruebas);
		//*/
	}
}

void debug_task(void *pvParameters)
{
	char buflen, bufout[64];
	UNUSED(pvParameters);
	while(1)
	{
		buflen = sprintf(bufout, "%i\r\t%.3f\r\t%.3f\r\t%.3f\r\n", uOUT, error, vfil, vref);
		for (int i = 0; i <buflen; i++)
			xQueueSend(xqueue, bufout + i, portMAX_DELAY);
		vTaskDelay(pdMS_TO_TICKS(50));
	}
}

void parse_task(void *pvParameters)
{
	UNUSED(pvParameters);
	HAL_UART_Receive_IT(&huart1, &byteRec, 1);
	while(1)
	{
		xSemaphoreTake(semParse, portMAX_DELAY);
		sscanf((const char *)bufIn, "%f", &vref);

		char msg[32];
		int len = sprintf(msg, "New voltage: %.2fV\r\n", vref);
		for (int i = 0; i < len; i++)
			xQueueSend(xqueue, msg + i, portMAX_DELAY);

		HAL_UART_Receive_IT(&huart1, &byteRec, 1);
	}
}

void vApplicationIdleHook(void)
{
	char byteToSend;
	if (xQueueReceive(xqueue, &byteToSend, 0) == pdPASS)
	{
		HAL_UART_Transmit(&huart1, (uint8_t *)&byteToSend, 1, HAL_MAX_DELAY);
	}
}

void blinkFunction(TimerHandle_t xTimer)
{
   UNUSED(xTimer);
   HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	if(huart->Instance == USART1)
	{
		if (byteRec == '\n' || byteRec == '\r')
		{
			if (idx > 0)
			{
				//The whole string is received
				bufIn[idx] = '\0';
				idx = 0;
				//xSemaphoreGive(semParse);
				xSemaphoreGiveFromISR(semParse, &xHigherPriorityTaskWoken);
			}
		}

		else
		{
			if (idx < sizeof(bufIn) - 1)
				bufIn[idx++] = byteRec;
		}
	}

	HAL_UART_Receive_IT(&huart1, &byteRec, 1);

	if (xHigherPriorityTaskWoken == pdTRUE)
	{
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	}
}
