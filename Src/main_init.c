#include <main.h>

extern UART_HandleTypeDef huart1;
extern TIM_HandleTypeDef htim1;
extern ADC_HandleTypeDef hadc1;

void System_Init()
{
	HAL_Init();
	Clock_Init();
	GPIO_Init();
	UART1_Init();
	TIM1_Init();
	ADC1_Init();
}

void Clock_Init(void)
{
	 RCC_OscInitTypeDef osc_config = {0};
	 osc_config.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	 osc_config.HSEState = RCC_HSE_ON;
	 osc_config.HSIState = RCC_HSI_OFF;
	 osc_config.LSEState = RCC_LSE_OFF;
	 osc_config.LSIState = RCC_LSI_OFF;
	 osc_config.PLL.PLLState = RCC_PLL_ON;
	 osc_config.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	 osc_config.PLL.PLLM = 25;
	 osc_config.PLL.PLLN = 200;
	 osc_config.PLL.PLLP = RCC_PLLP_DIV2;

	 if(HAL_RCC_OscConfig(&osc_config) != HAL_OK)
	      Error_Handler();

	 RCC_ClkInitTypeDef clk_config = {0};
	 clk_config.ClockType = 0x0F;
	 clk_config.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	 clk_config.AHBCLKDivider = RCC_SYSCLK_DIV1;
	 clk_config.APB1CLKDivider = RCC_HCLK_DIV2;
	 clk_config.APB2CLKDivider = RCC_HCLK_DIV1;
	 if(HAL_RCC_ClockConfig(&clk_config, FLASH_LATENCY_4) != HAL_OK)
	    Error_Handler();
}

void ADC1_Init(void)
{
    __HAL_RCC_ADC1_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();

    GPIO_InitTypeDef adc_pin = {0};
    adc_pin.Pin = GPIO_PIN_0;
    adc_pin.Mode = GPIO_MODE_ANALOG;
    HAL_GPIO_Init(GPIOA, &adc_pin);

    hadc1.Instance = ADC1;
    hadc1.Init.ContinuousConvMode = ENABLE;
    hadc1.Init.NbrOfConversion = 1;
    hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV8;
    hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
    if (HAL_ADC_Init(&hadc1) != HAL_OK)
        Error_Handler();

    ADC_ChannelConfTypeDef adc_channel = {0};
    adc_channel.Channel = ADC_CHANNEL_0;
    adc_channel.Rank = 1;
    adc_channel.SamplingTime = ADC_SAMPLETIME_112CYCLES;
    if (HAL_ADC_ConfigChannel(&hadc1, &adc_channel) != HAL_OK)
        Error_Handler();

    HAL_ADC_Start(&hadc1);
}

void TIM1_Init(void)
{
  /* 1. Habilitar el reloj para el periférico */
  __HAL_RCC_TIM1_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /* 2. Configuración de bajo nivel (pines e interrupciones) */
  GPIO_InitTypeDef oc_pin = {0};
  oc_pin.Pin = GPIO_PIN_8 | GPIO_PIN_7;
  oc_pin.Mode = GPIO_MODE_AF_PP;
  oc_pin.Alternate = GPIO_AF1_TIM1;
  HAL_GPIO_Init(GPIOA, &oc_pin);

  /* 3. Configuración de alto nivel */
  htim1.Instance = TIM1;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Prescaler = 0; /* CK_CNT = Periph_clock / (1 + Prescaler)*/
  htim1.Init.Period = 999;  /* Timer_Freq = CK_CNT / (1 + Period) */
  HAL_TIM_Base_Init(&htim1);

  TIM_OC_InitTypeDef output_channel = {0};
  output_channel.OCMode = TIM_OCMODE_PWM1;
  output_channel.OCPolarity = TIM_OCPOLARITY_HIGH;
  output_channel.Pulse = 0;
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &output_channel, TIM_CHANNEL_1) != HAL_OK)
    Error_Handler();

  TIM_BreakDeadTimeConfigTypeDef deadTimeConfig = {0};
  deadTimeConfig.DeadTime = computeDeadTime(0);
  if (HAL_TIMEx_ConfigBreakDeadTime(&htim1, &deadTimeConfig) != HAL_OK)
    Error_Handler();

  /* Arrancar el periférico */
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
  HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_1);
}


void UART1_Init(void)
{
	/* 1. CONFIGURACION DEL RELOJ */
	__HAL_RCC_USART1_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();

	/* 2. CONFIGURACION DE LOS PINES DEL PERIFERICO*/
	GPIO_InitTypeDef tx_pin = {0};
	tx_pin.Pin = GPIO_PIN_9 | GPIO_PIN_10;  // TX y RX
	tx_pin.Mode = GPIO_MODE_AF_PP;  // FUNCION ALTERNATIVA Push-Pull
	tx_pin.Alternate = GPIO_AF7_USART1;  // AF7 PARA USART1 EN PA9 y PA10
	HAL_GPIO_Init(GPIOA, &tx_pin);

	/* 3. DAR DE ALTA LA INTERRUPCION */
	HAL_NVIC_SetPriority(USART1_IRQn, 15, 0);
	HAL_NVIC_EnableIRQ(USART1_IRQn);

	/* 4. CONFIGURACION DE ALTO NIVEL */
	huart1.Instance = USART1;
	huart1.Init.BaudRate = 115200;  // VELOCIDAD DE TRANSMISION
	huart1.Init.Mode = UART_MODE_TX_RX;  // TRANSMISIÃ“N y RECEPCIÃ“N
	if (HAL_UART_Init(&huart1) != HAL_OK)
	{
		Error_Handler();
	}
}

void GPIO_Init(void)
{
	/* 1. CONFIGURACION DEL RELOJ */
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();

	/* 2. CONFIGURACION DE LOS PINES DEL PERIFERICO*/
	// Configuración del Led (Pin 13)
	GPIO_InitTypeDef Led_pin ={0};
	Led_pin.Pin = GPIO_PIN_13;
	Led_pin.Mode = GPIO_MODE_OUTPUT_PP;
	Led_pin.Pull = GPIO_NOPULL;
	Led_pin.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOC, &Led_pin);

	// Configuración de los pines de dirección,  (PB7)
	GPIO_InitTypeDef Dir_pines = {0};
	Dir_pines.Pin = GPIO_PIN_2 | GPIO_PIN_3;
	//Dir_pines.Pin = GPIO_PIN_7;
	Dir_pines.Mode = GPIO_MODE_OUTPUT_PP;   // Push-pull
	Dir_pines.Pull = GPIO_PULLUP;           // Activar resistencia pull-up
	Dir_pines.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOA, &Dir_pines);
}

uint16_t computeDeadTime(uint16_t dead_time)
{
    float clockFreq;
    clockFreq = 100000000.0 / HAL_RCC_GetSysClockFreq();

    if(dead_time == 0 && dead_time <= 1764)
        dead_time = dead_time / clockFreq;
    else if(dead_time >= 1778 && dead_time <= 3529)
        dead_time = dead_time / (clockFreq * 2) + 64;
    else if(dead_time >= 3556 && dead_time <= 7001)
        dead_time = dead_time / (clockFreq * 8) + 128;
    else if(dead_time >= 7112 && dead_time <= 14001)
        dead_time = dead_time / (clockFreq * 16) + 192;
    else
        dead_time = dead_time;

    return dead_time;
}

void Error_Handler(void)
{
	while(1)
	{
		HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
		HAL_Delay(100);
	}
}
