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
#include "rgb_driver.h"
#include "Display_7Seg_stm32.h"
#include "step_motor_28BYJ48.h"
#include "stdio.h"
#include "string.h"
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
TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim4;
TIM_HandleTypeDef htim5;

UART_HandleTypeDef huart3;

/* USER CODE BEGIN PV */

/* --- Objeto para el Led RGB --- */
RGB_LED_t ledRGB;

/* --- Objeto para los display de 7 segmentos --- */
display_7seg_t Display;
uint8_t bufferDisplay[3]; //buffer para la cantidad de displays

/* --- Objeto para el motor --- */
Stepper_t motor1;

/* Banderas y Variables de Estado */
typedef enum {
	MOTOR_STOPPED = 0,
	MOTOR_RUNNING
} MotorState_t;

volatile MotorState_t current_state = MOTOR_STOPPED;
volatile uint8_t flag_update_display = 0;
uint32_t last_telemetry_tick = 0;
uint32_t last_display_tick = 0;

char uart_buf[100];
uint32_t step_delay = 850; // 850 us entre medios pasos = velocidad inicial

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART3_UART_Init(void);
static void MX_TIM5_Init(void);
static void MX_TIM2_Init(void);
static void MX_TIM4_Init(void);
/* USER CODE BEGIN PFP */

/* --- Prototipos de Funciones Auxiliares --- */
void Debug_Log(const char *msg);
void Update_Display_String(void);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

void Debug_Log(const char *msg) {
	HAL_UART_Transmit(&huart3, (uint8_t*)msg, strlen(msg), 100);
}

void Update_Display_String(void) {
	char status_str[5];
	if (current_state == MOTOR_RUNNING) {
		float rpm = (60.0f * 1000000.0f) / (step_delay * 4096.0f);
		char dir_char = (motor1.direction == STEP_CW) ? 'C' : 'r';
		sprintf(status_str, "%c%02d", dir_char, (int)rpm);
		Display7Seg_WriteString(&Display, status_str);
	} else {
		Display7Seg_WriteString(&Display, "StP");
	}
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
	MX_TIM2_Init();
	MX_TIM4_Init();
	/* USER CODE BEGIN 2 */

	/* --- Log de Bienvenida: Laboratorio 05 --- */
	Debug_Log("\r\n==========================================\r\n");
	Debug_Log("   LABORATORIO 05: OUTPUT COMPARE (TIM5)  \r\n");
	Debug_Log("   CONTROL DE MOTOR PASO A PASO (28BYJ)   \r\n");
	Debug_Log("   Plataforma: STM32 Nucleo-F439ZI        \r\n");
	Debug_Log("==========================================\r\n");
	Debug_Log("[OK] Nucleo Clock: 180 MHz\r\n");
	Debug_Log(">>> Iniciando Perifericos...\r\n");

	// 1. Definición de pines para el driver genérico
	GPIO_TypeDef* ports_A[] = {IN1_GPIO_Port, IN2_GPIO_Port, IN3_GPIO_Port, IN4_GPIO_Port};
	uint16_t pins_A[] = {IN1_Pin, IN2_Pin, IN3_Pin, IN4_Pin};

	// 2. Inicialización del motor
	Stepper_Init(&motor1, ports_A, pins_A, MODE_HALF_STEP);
	Stepper_Stop(&motor1); // Fuerza a que todas las bobinas inicien en LOW
	motor1.current_step = 0; // Aseguramos el punto de partida
	Debug_Log("[OK] Driver Stepper: Secuencia Half-Step Lista\r\n");

	// 3. Sincronización del Timer: Programamos el primer evento de comparación
	// Le decimos al Timer: "Interrumpí en el valor actual + el delay"
	uint32_t initial_capture = __HAL_TIM_GET_COUNTER(&htim5);
	__HAL_TIM_SET_COMPARE(&htim5, TIM_CHANNEL_1, initial_capture + step_delay);

	// 4. Iniciar el Timer en modo Output Compare con Interrupción
	HAL_TIM_OC_Start_IT(&htim5, TIM_CHANNEL_1);
	Debug_Log("[OK] Periferico TIM5: Output Compare Activo\r\n");

	// 5. Reporte de configuración inicial
	char init_msg[60];
	sprintf(init_msg, "[INFO] Delay: %lu us | RPM Estimadas: %.2f\r\n",
			step_delay, (60.0f * 1000000.0f) / (step_delay * 4096.0f));
	Debug_Log(init_msg);

	// ***********************Configuracion de los Display 7 Segmentos********************
	// Configuración de Pines del Display (Usando Labels del .ioc)
	static display_pio_t segmentos[] = {
			{SEG_A_GPIO_Port, SEG_A_Pin}, {SEG_B_GPIO_Port, SEG_B_Pin},
			{SEG_C_GPIO_Port, SEG_C_Pin}, {SEG_D_GPIO_Port, SEG_D_Pin},
			{SEG_E_GPIO_Port, SEG_E_Pin}, {SEG_F_GPIO_Port, SEG_F_Pin},
			{SEG_G_GPIO_Port, SEG_G_Pin}
	};
	static display_pio_t comunes[] = {
			{EN1_GPIO_Port, EN1_Pin},
			{EN2_GPIO_Port, EN2_Pin},
			{EN3_GPIO_Port, EN3_Pin}
	};

	//Iniciar Driver
	Display7Seg_Init(&Display, &htim2, segmentos, comunes, 3, bufferDisplay);
	HAL_TIM_Base_Start_IT(&htim2);
	Display7Seg_WriteString(&Display, "HI ");
	Debug_Log("[OK] Periferico TIM2: Multiplexado Display\r\n");

	//***********************Configuracion del LED RGB Anodo Comun***********************
	// 1. Empaquetamos la configuración de hardware
	RGB_Config_t configRGB = {
			.htim = &htim4,						//Timer utilizado para el led RGB
			.R_channel = TIM_CHANNEL_2,			//RGB_R pin
			.G_channel = TIM_CHANNEL_3,			//RGB_G pin
			.B_channel = TIM_CHANNEL_4,			//RGB_B pin
			.led_type = LED_TYPE_ANODE_COMMON,	//Tipo de RGB -Cambiar si es catodo comun-
			.max_brightness = 500				//Brillo 0-1000
	};
	// 2. Inicializamos el driver con el paquete
	RGB_Init_Single(&ledRGB, &configRGB);
	Debug_Log("[OK] Periferico TIM4: PWM RGB Activo\r\n");
	// Configuración inicial estética
	Display7Seg_SetBrightness(&Display, 80);
	Display7Seg_WriteString(&Display, "StP"); // Mensaje de "Stop" al arrancar
	Debug_Log(">>> Sistema listo. Moviendo motor...\r\n\r\n");

	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1)
	{
		/* USER CODE END WHILE */

		/* USER CODE BEGIN 3 */

		uint32_t current_tick = HAL_GetTick();

		/* --- Tarea 1: Motor de Efectos RGB --- */
		// Se encarga de las transiciones suaves si hay efectos activos
		RGB_Effects_Handler(&ledRGB, 20, 5);

		/* --- Tarea 2: Gestión de Banderas (Eventos de Botones PB10/PB11) --- */
		if (flag_update_display) {
			flag_update_display = 0;

			if (current_state == MOTOR_RUNNING) {
				RGB_Stop_Effect(&ledRGB);
				Display7Seg_SetFlash(&Display, 0);

				// Cambio de color dinámico por dirección
				if (motor1.direction == STEP_CW) {
					RGB_Set_Preset(&ledRGB, COLOR_GREEN);
					Debug_Log("[EVENT] Motor START / Direction: CW\r\n");
				} else {
					RGB_Set_Preset(&ledRGB, COLOR_BLUE);
					Debug_Log("[EVENT] Motor START / Direction: CCW\r\n");
				}
			} else {
				// Feedback visual de parada
				RGB_Set_Preset(&ledRGB, COLOR_RED);
				Display7Seg_SetFlash(&Display, 500);
				Debug_Log("[EVENT] Motor STOPPED\r\n");
			}

			// Sincronizamos el refresco visual inmediato
			last_display_tick = current_tick;
			Update_Display_String();
		}

		/* --- Tarea 3: Telemetría UART (Cada 1000ms) --- */
		if (current_tick - last_telemetry_tick >= 1000) {
			last_telemetry_tick = current_tick;

			if (current_state == MOTOR_RUNNING) {
				float rpm = (60.0f * 1000000.0f) / (step_delay * 4096.0f);
				// Agregamos el INDEX aquí para debuggear el sentido antihorario
				snprintf(uart_buf, sizeof(uart_buf),
						"[STATUS] Dir: %d | Index: %d | RPM: %.2f | Delay: %lu us\r\n",
						motor1.direction, motor1.current_step, rpm, step_delay);
				/*
		            // Reporte completo de telemetría
		            snprintf(uart_buf, sizeof(uart_buf),
		                     "[STATUS] State: RUN | Dir: %s | RPM: %.2f | Delay: %lu us\r\n",
		                     (motor1.direction == STEP_CW) ? "CW" : "CCW", rpm, step_delay);*/
			} else {
				snprintf(uart_buf, sizeof(uart_buf), "[STATUS] State: IDLE | Waiting for Start...\r\n");
			}
			Debug_Log(uart_buf);
		}

		/* --- Tarea 4: Refresco Visual del Display (Cada 200ms) --- */
		if (current_tick - last_display_tick >= 200) {
			last_display_tick = current_tick;
			Update_Display_String();
		}

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
 * @brief TIM2 Initialization Function
 * @param None
 * @retval None
 */
static void MX_TIM2_Init(void)
{

	/* USER CODE BEGIN TIM2_Init 0 */

	/* USER CODE END TIM2_Init 0 */

	TIM_ClockConfigTypeDef sClockSourceConfig = {0};
	TIM_MasterConfigTypeDef sMasterConfig = {0};

	/* USER CODE BEGIN TIM2_Init 1 */

	/* USER CODE END TIM2_Init 1 */
	htim2.Instance = TIM2;
	htim2.Init.Prescaler = 89;
	htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim2.Init.Period = 1999;
	htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
	{
		Error_Handler();
	}
	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
	if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
	{
		Error_Handler();
	}
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
	{
		Error_Handler();
	}
	/* USER CODE BEGIN TIM2_Init 2 */

	/* USER CODE END TIM2_Init 2 */

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
	if (HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
	{
		Error_Handler();
	}
	if (HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_3) != HAL_OK)
	{
		Error_Handler();
	}
	if (HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_4) != HAL_OK)
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
	__HAL_RCC_GPIOE_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOF_CLK_ENABLE();
	__HAL_RCC_GPIOH_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOG_CLK_ENABLE();
	__HAL_RCC_GPIOD_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOE, SEG_C_Pin|SEG_A_Pin|SEG_B_Pin|IN1_Pin
			|IN2_Pin|IN3_Pin|IN4_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOF, SEG_E_Pin|SEG_D_Pin|SEG_F_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOB, LD1_Pin|LD3_Pin|LD2_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOG, SEG_G_Pin|USB_PowerSwitchOn_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOC, EN1_Pin|EN2_Pin|EN3_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pins : SEG_C_Pin SEG_A_Pin SEG_B_Pin IN1_Pin
                           IN2_Pin IN3_Pin IN4_Pin */
	GPIO_InitStruct.Pin = SEG_C_Pin|SEG_A_Pin|SEG_B_Pin|IN1_Pin
			|IN2_Pin|IN3_Pin|IN4_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

	/*Configure GPIO pin : USER_Btn_Pin */
	GPIO_InitStruct.Pin = USER_Btn_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(USER_Btn_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pins : SEG_E_Pin SEG_D_Pin SEG_F_Pin */
	GPIO_InitStruct.Pin = SEG_E_Pin|SEG_D_Pin|SEG_F_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

	/*Configure GPIO pins : LD1_Pin LD3_Pin LD2_Pin */
	GPIO_InitStruct.Pin = LD1_Pin|LD3_Pin|LD2_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	/*Configure GPIO pins : SEG_G_Pin USB_PowerSwitchOn_Pin */
	GPIO_InitStruct.Pin = SEG_G_Pin|USB_PowerSwitchOn_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

	/*Configure GPIO pins : usr_btn_G_Pin usr_btn_PS_Pin */
	GPIO_InitStruct.Pin = usr_btn_G_Pin|usr_btn_PS_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	/*Configure GPIO pin : USB_OverCurrent_Pin */
	GPIO_InitStruct.Pin = USB_OverCurrent_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(USB_OverCurrent_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pins : EN1_Pin EN2_Pin EN3_Pin */
	GPIO_InitStruct.Pin = EN1_Pin|EN2_Pin|EN3_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

	/* EXTI interrupt init*/
	HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

	/* USER CODE BEGIN MX_GPIO_Init_2 */

	/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/**
 * @brief Callback del Timer para la multiplexación del Display.
 * @note Se ejecuta periódicamente (recomendado cada 2-5ms) para refrescar
 * un dígito a la vez del display de 7 segmentos.
 * @param htim Puntero a la estructura del Timer que generó la interrupción.
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
	if (htim->Instance == TIM2) {
		Display7Seg_Refresh_ISR(&Display);
	}
}


/**
 * @brief Callback de Interrupción Externa para pines GPIO.
 * @details Gestiona el pulsador en PB10 y PB11 (lógica negativa) para alternar giros y entre
 * los estados de marcha y parada del motor (Toggle). Incluye un
 * mecanismo de debouncing por software.
 * @param GPIO_Pin Pin que disparó la interrupción.
 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
	uint32_t interrupt_time = HAL_GetTick();
	static uint32_t last_btn_start_time = 0;
	static uint32_t last_btn_dir_time = 0;

	/* --- Botón START/STOP (PB11) --- */
	if (GPIO_Pin == usr_btn_PS_Pin) {
		if (interrupt_time - last_btn_start_time > 250) {
			if (current_state == MOTOR_STOPPED) {
				current_state = MOTOR_RUNNING;
			} else {
				current_state = MOTOR_STOPPED;
				Stepper_Stop(&motor1);
			}
			flag_update_display = 1;
			last_btn_start_time = interrupt_time;
		}
	}

	/* --- Botón CAMBIO DE SENTIDO (PB10) --- */
	if (GPIO_Pin == usr_btn_G_Pin) {
			if (interrupt_time - last_btn_dir_time > 250) {
				// Invertimos el sentido para que coincida con el display
				if (motor1.direction == STEP_CW) {
					Stepper_Set_Direction(&motor1, STEP_CCW); // Ahora dirá 'r' y girará CCW
				} else {
					Stepper_Set_Direction(&motor1, STEP_CW);  // Ahora dirá 'C' y girará CW
				}
				flag_update_display = 1;
				last_btn_dir_time = interrupt_time;
			}
		}
}


/**
 * @brief Callback de comparación de salida (Output Compare) del Timer.
 * @details Responsable de la generación de pasos del motor PAP. Si el sistema
 * está en RUNNING, ejecuta un paso. Independientemente del estado,
 * reprograma la próxima comparación para mantener la base de tiempo
 * constante y evitar latencias al arrancar.
 * @param htim Puntero a la estructura del Timer que generó el evento.
 */
void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef *htim) {
	if (htim->Instance == TIM5) {
		if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1) {

			/* 1. Generación del paso lógico si el motor está habilitado */
			if (current_state == MOTOR_RUNNING) {
				Stepper_Step_Sequential(&motor1);
			}

			/* 2. Reprogramación del evento de comparación (Fase de acumulador) */
			uint32_t current_capture = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
			__HAL_TIM_SET_COMPARE(htim, TIM_CHANNEL_1, current_capture + step_delay);
		}
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
