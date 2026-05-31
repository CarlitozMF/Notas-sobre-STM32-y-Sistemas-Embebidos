/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2026 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "tone_generator.h"
#include "siren_service.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef htim4;
TIM_HandleTypeDef htim5;

UART_HandleTypeDef huart3;

/* USER CODE BEGIN PV */

tone_gen_t      my_buzzer;
siren_service_t vehicular_siren;

SirenMode_t actual_mode = MODE_OFF;
volatile uint8_t button_event = 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART3_UART_Init(void);
static void MX_TIM5_Init(void);
static void MX_TIM4_Init(void);
/* USER CODE BEGIN PFP */

void     PAL_STM32_GPIO_Write(generic_gpio_t gpio, bool state);
bool     PAL_STM32_GPIO_Read(generic_gpio_t gpio);
void     PAL_STM32_PWM_Write(generic_pwm_t ch, uint16_t value);
uint32_t PAL_STM32_GetTick(void);
uint32_t PAL_STM32_OC_Read(generic_pwm_t ch);
void     PAL_STM32_OC_Write(generic_pwm_t ch, uint32_t value);
uint32_t PAL_STM32_GetTimerCnt(generic_pwm_t ch);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

void PAL_STM32_GPIO_Write(generic_gpio_t gpio, bool state) {
	HAL_GPIO_WritePin((GPIO_TypeDef*)gpio.port, gpio.pin, state ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

bool PAL_STM32_GPIO_Read(generic_gpio_t gpio) {
	return (HAL_GPIO_ReadPin((GPIO_TypeDef*)gpio.port, gpio.pin) == GPIO_PIN_SET);
}

void PAL_STM32_PWM_Write(generic_pwm_t ch, uint16_t value) {
	__HAL_TIM_SET_COMPARE((TIM_HandleTypeDef*)ch.timer_handle, ch.channel, value);
}

uint32_t PAL_STM32_GetTick(void) {
	return HAL_GetTick();
}

uint32_t PAL_STM32_OC_Read(generic_pwm_t ch) {
	// Lectura directa y limpia del registro CCRx asignado
	return HAL_TIM_ReadCapturedValue((TIM_HandleTypeDef*)ch.timer_handle, ch.channel);
}

void PAL_STM32_OC_Write(generic_pwm_t ch, uint32_t value) {
	// __HAL_TIM_SET_COMPARE escribe el nuevo valor elástico de disparo en el CCRx
	__HAL_TIM_SET_COMPARE((TIM_HandleTypeDef*)ch.timer_handle, ch.channel, value);
}

uint32_t PAL_STM32_GetTimerCnt(generic_pwm_t ch) {
	return __HAL_TIM_GET_COUNTER((TIM_HandleTypeDef*)ch.timer_handle);
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART3_UART_Init();
  MX_TIM5_Init();
  MX_TIM4_Init();
  /* USER CODE BEGIN 2 */

	// 1. Habilitar Interrupciones en el NVIC (Imprescindible para el Output Compare)
	HAL_NVIC_SetPriority(TIM5_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(TIM5_IRQn);

	// 2. Configurar Tabla de Despacho PAL Universal
	hal_interface_t sys_pal = {
			.gpio_write    = PAL_STM32_GPIO_Write,
			.gpio_read     = PAL_STM32_GPIO_Read,
			.pwm_write     = PAL_STM32_PWM_Write,
			.get_tick      = PAL_STM32_GetTick,
			.oc_read       = PAL_STM32_OC_Read,
			.oc_write      = PAL_STM32_OC_Write,
			.get_timer_cnt = PAL_STM32_GetTimerCnt // <-- INYECTAMOS EL NUEVO MAPEO
	};

	// 3. Mapeo de Descriptores de Hardware Físicos
	generic_pwm_t  oc_channel = { .timer_handle = &htim5, .channel = TIM_CHANNEL_1 };
	generic_gpio_t buzzer_pin = { .port = buzzer_pin_GPIO_Port, .pin = buzzer_pin_Pin };
	generic_pwm_t  led_izq    = { .timer_handle = &htim4, .channel = TIM_CHANNEL_1 };
	generic_pwm_t  led_der    = { .timer_handle = &htim4, .channel = TIM_CHANNEL_2 };

	// 4. Inicialización en Cadena Inyectando la PAL
	TONE_GENERATOR_Init(&my_buzzer, oc_channel, buzzer_pin, sys_pal, 1000000);
	SIREN_SERVICE_Init(&vehicular_siren, &my_buzzer, led_izq, led_der, sys_pal);

	// 5. CORRECCIÓN CRÍTICA: Establecer primero el modo operativo lógicamente
	// Esto limpia los flags de estado del driver genérico antes del latido del hardware
	SIREN_SERVICE_SetMode(&vehicular_siren, actual_mode);
	__HAL_TIM_CLEAR_IT(&htim5, TIM_IT_CC1);
	// 6. Encendido físico del periférico del silicio (Siempre al final)
	HAL_TIM_OC_Start_IT(&htim5, TIM_CHANNEL_1);
	HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_1);
	HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_2);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	while (1)
	{
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

		static uint32_t button_press_tick = 0;
		static uint8_t button_state = 0;             // 0: Reposo, 1: Validando tiempo, 2: Ráfaga HORN activa
		static SirenMode_t modo_guardado = MODE_OFF; // Almacén dinámico para el retorno atómico

		// 1. Captura instantánea de la Capa 1 (Lectura directa del pin PB11)
		uint8_t pin_ahora = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_11);

		// 2. Máquina de estados para discriminar Click de Hold (Pulsación sostenida)
		switch (button_state)
		{
		case 0: // [ESTADO: REPOSO] Esperando que la ISR avise el flanco de bajada (Falling)
			if (button_event)
			{
				button_event = 0;              // Consumimos el evento asíncrono
				button_press_tick = HAL_GetTick(); // Capturamos la estampa de tiempo inicial
				button_state = 1;              // Transicionamos a evaluar la duración del pulso
			}
			break;

		case 1: // [ESTADO: EVALUANDO DURACIÓN] El usuario tiene el dedo sobre el pulsador
			if (pin_ahora == GPIO_PIN_RESET)
			{
				// CONDICIÓN HOLD: Si pasa más de 400ms retenido, se dispara la bocina de aire manual
				if (HAL_GetTick() - button_press_tick > 400)
				{
					modo_guardado = actual_mode; // Guardamos el modo actual (ej: MODE_YELP) para no perder el contexto
					actual_mode = MODE_HORN;
					SIREN_SERVICE_SetMode(&vehicular_siren, actual_mode); // Clava los 420 Hz físicos
					button_state = 2; // Bloqueamos la máquina en estado de retención
				}
			}
			else // El usuario soltó el botón ANTES de llegar a los 400ms -> Es un click corto
			{
				// Antirebote mínimo de seguridad: Confirmamos si el click duró al menos 50ms
				if (HAL_GetTick() - button_press_tick > 50)
				{
					actual_mode++;
					// Excluimos el MODE_HORN del ciclo secuencial automático
					if (actual_mode >= MODE_HORN) {
						actual_mode = MODE_OFF;
					}
					SIREN_SERVICE_SetMode(&vehicular_siren, actual_mode);
				}
				button_state = 0; // Reseteamos la máquina al reposo
			}
			break;

		case 2: // [ESTADO: RETENCIÓN HORN] La bocina ruge en background mientras el pin siga en LOW
			if (pin_ahora == GPIO_PIN_SET) // El usuario quitó el dedo del pulsador (Flanco de subida)
			{
				// RESTAURACIÓN ATÓMICA: Volvemos de inmediato a la sirena que estaba corriendo antes
				actual_mode = modo_guardado;
				SIREN_SERVICE_SetMode(&vehicular_siren, actual_mode);

				// Limpieza preventiva de flags por ruidos mecánicos al soltar el resorte del botón
				button_event = 0;
				button_state = 0;
			}
			break;

		default:
			button_state = 0;
			break;
		}

		// 3. Despacho asíncrono cooperativo: ejecuta las rampas de audio y multiplexado de balizas
		SIREN_SERVICE_Update(&vehicular_siren);

	}
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 180;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Activate the Over-Drive mode
  */
  if (HAL_PWREx_EnableOverDrive() != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief TIM4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM4_Init(void)
{

  /* USER CODE BEGIN TIM4_Init 0 */

  /* USER CODE END TIM4_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM4_Init 1 */

  /* USER CODE END TIM4_Init 1 */
  htim4.Instance = TIM4;
  htim4.Init.Prescaler = 179;
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.Period = 999;
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim4, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM4_Init 2 */

  /* USER CODE END TIM4_Init 2 */
  HAL_TIM_MspPostInit(&htim4);

}

/**
  * @brief TIM5 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM5_Init(void)
{

  /* USER CODE BEGIN TIM5_Init 0 */

  /* USER CODE END TIM5_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM5_Init 1 */

  /* USER CODE END TIM5_Init 1 */
  htim5.Instance = TIM5;
  htim5.Init.Prescaler = 89;
  htim5.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim5.Init.Period = 4294967295;
  htim5.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim5.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim5) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim5, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_OC_Init(&htim5) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim5, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_TOGGLE;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_OC_ConfigChannel(&htim5, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM5_Init 2 */

  /* USER CODE END TIM5_Init 2 */
  HAL_TIM_MspPostInit(&htim5);

}

/**
  * @brief USART3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART3_UART_Init(void)
{

  /* USER CODE BEGIN USART3_Init 0 */

  /* USER CODE END USART3_Init 0 */

  /* USER CODE BEGIN USART3_Init 1 */

  /* USER CODE END USART3_Init 1 */
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 115200;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART3_Init 2 */

  /* USER CODE END USART3_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, LD1_Pin|LD3_Pin|LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(USB_PowerSwitchOn_GPIO_Port, USB_PowerSwitchOn_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : USER_Btn_Pin */
  GPIO_InitStruct.Pin = USER_Btn_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(USER_Btn_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : LD1_Pin LD3_Pin LD2_Pin */
  GPIO_InitStruct.Pin = LD1_Pin|LD3_Pin|LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : usr_btn_ext_Pin */
  GPIO_InitStruct.Pin = usr_btn_ext_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(usr_btn_ext_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : USB_PowerSwitchOn_Pin */
  GPIO_InitStruct.Pin = USB_PowerSwitchOn_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(USB_PowerSwitchOn_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : USB_OverCurrent_Pin */
  GPIO_InitStruct.Pin = USB_OverCurrent_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(USB_OverCurrent_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 2, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if (GPIO_Pin == GPIO_PIN_11)
	{
		button_event = 1;
	}
}

/**
 * @brief Callback universal de comparación de canales de la HAL de ST.
 * @details Se ejecuta de manera directa al levantar el flag CC1, saltándose
 * cualquier filtro intermedio de la capa Base u OC.
 */
void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef *htim)
{
	if (htim->Instance == TIM5)
	{
		HAL_GPIO_TogglePin(GPIOB, LD1_Pin); // Testigo visual
		TONE_GENERATOR_IRQ_Handler(&my_buzzer);
	}
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1)
	{
	}
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
	/* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
