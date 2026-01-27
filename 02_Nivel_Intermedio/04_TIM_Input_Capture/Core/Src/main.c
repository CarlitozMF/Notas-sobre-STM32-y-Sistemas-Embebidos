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
#include "utils_delay.h"
#include "hcsr04_driver.h"
#include "rgb_driver.h"
#include "Display_7Seg_stm32.h"
#include <string.h>  // Necesaria para strlen()
#include <stdio.h>   // Útil si después quieres usar sprintf()
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
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim4;

UART_HandleTypeDef huart3;

/* USER CODE BEGIN PV */

/* --- Objeto para el Sensor Ultrasonico --- */
HCSR04_Handle ultrasonic;

/* --- Objeto para el Led RGB --- */
RGB_LED_t ledRGB;

/* --- Objeto para los display de 7 segmentos --- */
display_7seg_t Display;
uint8_t bufferDisplay[3]; //buffer para la cantidad de displays

// Variables de control
uint32_t last_ultrasonic_tick = 0;
uint8_t fallos_consecutivos = 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART3_UART_Init(void);
static void MX_TIM4_Init(void);
static void MX_TIM3_Init(void);
static void MX_TIM2_Init(void);
/* USER CODE BEGIN PFP */

/* --- Prototipos de Funciones Auxiliares --- */
void Actualizar_Alerta_Visual(float d);
void Debug_Log(const char *msg);
float Aplicar_Filtro_Mediana(float nueva_dist);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

void Actualizar_Alerta_Visual(float d) {
	if (d < 10.0f) {
		RGB_Set_Color_Direct(&ledRGB, 255, 0, 0);   // ROJO: ¡Peligro!
		Display7Seg_SetFlash(&Display, 200); // Hacemos que el display parpadee
	} else if (d < 30.0f) {
		RGB_Set_Color_Direct(&ledRGB, 255, 120, 0); // NARANJA: Cuidado
		Display7Seg_SetFlash(&Display, 0);   // Parpadeo OFF
	} else {
		RGB_Set_Color_Direct(&ledRGB, 0, 255, 0);   // VERDE: Seguro
		Display7Seg_SetFlash(&Display, 0);
	}
}

void Debug_Log(const char *msg) {
	HAL_UART_Transmit(&huart3, (uint8_t*)msg, strlen(msg), 100);
}

// Función para ordenar y sacar la mediana
float Aplicar_Filtro_Mediana(float nueva_dist) {
	static float muestras[5] = {0};
	static uint8_t indice = 0;
	float ordenada[5];

	// 1. Insertar nueva muestra
	muestras[indice] = nueva_dist;
	indice = (indice + 1) % 5;

	// 2. Copiar para no arruinar el buffer original
	for(int i=0; i<5; i++) ordenada[i] = muestras[i];

	// 3. Ordenar (Bubble sort simple)
	for(int i=0; i<4; i++) {
		for(int j=i+1; j<5; j++) {
			if(ordenada[i] > ordenada[j]) {
				float temp = ordenada[i];
				ordenada[i] = ordenada[j];
				ordenada[j] = temp;
			}
		}
	}
	// 4. Devolver el valor central (el más estable)
	return ordenada[2];
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
	MX_TIM4_Init();
	MX_TIM3_Init();
	MX_TIM2_Init();
	/* USER CODE BEGIN 2 */
	Debug_Log("\r\n==========================================\r\n");
	Debug_Log("   LABORATORIO 04: INPUT CAPTURE (TIM3)   \r\n");
	Debug_Log("   SISTEMA DE TELEMETRIA ULTRASONICA      \r\n");
	Debug_Log("   Plataforma: STM32 Nucleo-F439ZI        \r\n");
	Debug_Log("==========================================\r\n");
	Debug_Log("[OK] Nucleo Clock: 180 MHz\r\n");
	Debug_Log(">>> Iniciando Perifericos...\r\n");

	Utils_Delay_Init();
	Debug_Log("[OK] Periferico DWT: Base de tiempo Activa\r\n");

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

	//***********************Configuracion del Sensor Ultrasonico************************
	// 1. Inicializar el sensor (TIM3 y sus respectivos canales/pines)
	HCSR04_Init(&ultrasonic, &htim3, TIM_CHANNEL_1, HCSR04_TRIG_GPIO_Port, HCSR04_TRIG_Pin);
	Debug_Log("[OK] Periferico TIM3: Captura de Eco (PA6)\r\n");

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

	//Debug_Log(">>> Sistema listo para medicion...\r\n");
	Debug_Log(">>> Sistema listo para medicion...\r\n\r\n");
	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1)
	{
		/* USER CODE END WHILE */

		/* USER CODE BEGIN 3 */

		uint32_t current_tick = HAL_GetTick();

		if (current_tick - last_ultrasonic_tick >= 100) {
			last_ultrasonic_tick = current_tick;

			float distancia = HCSR04_ReadDistance_cm(&ultrasonic);

			if (distancia > 0.0f && distancia < 400.0f) {
				// 1. Opcional: Aplicar filtro de mediana aquí si ves mucho rebote

				// 2. Mostrar en display (esto actualiza el buffer que lee la ISR del TIM2)
				Display7Seg_WriteNumber(&Display, (uint32_t)distancia);

				// 3. Log para control
				char msg[40];
				sprintf(msg, "Display: %d cm\r\n", (int)distancia);
				Debug_Log(msg);

				Actualizar_Alerta_Visual(distancia);
			}

			HCSR04_Trigger(&ultrasonic);
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
	htim2.Init.Prescaler = 8999;
	htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim2.Init.Period = 54;
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
 * @brief TIM3 Initialization Function
 * @param None
 * @retval None
 */
static void MX_TIM3_Init(void)
{

	/* USER CODE BEGIN TIM3_Init 0 */

	/* USER CODE END TIM3_Init 0 */

	TIM_ClockConfigTypeDef sClockSourceConfig = {0};
	TIM_MasterConfigTypeDef sMasterConfig = {0};
	TIM_IC_InitTypeDef sConfigIC = {0};

	/* USER CODE BEGIN TIM3_Init 1 */

	/* USER CODE END TIM3_Init 1 */
	htim3.Instance = TIM3;
	htim3.Init.Prescaler = 89;
	htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim3.Init.Period = 65535;
	htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
	{
		Error_Handler();
	}
	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
	if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
	{
		Error_Handler();
	}
	if (HAL_TIM_IC_Init(&htim3) != HAL_OK)
	{
		Error_Handler();
	}
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
	{
		Error_Handler();
	}
	sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_RISING;
	sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
	sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
	sConfigIC.ICFilter = 0;
	if (HAL_TIM_IC_ConfigChannel(&htim3, &sConfigIC, TIM_CHANNEL_1) != HAL_OK)
	{
		Error_Handler();
	}
	/* USER CODE BEGIN TIM3_Init 2 */

	/* USER CODE END TIM3_Init 2 */

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
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOG_CLK_ENABLE();
	__HAL_RCC_GPIOD_CLK_ENABLE();

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOE, SEG_C_Pin|SEG_A_Pin|SEG_B_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOF, SEG_E_Pin|SEG_D_Pin|SEG_F_Pin|usr_ledAzul_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOB, LD1_Pin|HCSR04_TRIG_Pin|LD3_Pin|LD2_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOG, SEG_G_Pin|USB_PowerSwitchOn_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOC, EN1_Pin|EN2_Pin|EN3_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pins : SEG_C_Pin SEG_A_Pin SEG_B_Pin */
	GPIO_InitStruct.Pin = SEG_C_Pin|SEG_A_Pin|SEG_B_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

	/*Configure GPIO pins : SEG_E_Pin SEG_D_Pin SEG_F_Pin usr_ledAzul_Pin */
	GPIO_InitStruct.Pin = SEG_E_Pin|SEG_D_Pin|SEG_F_Pin|usr_ledAzul_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

	/*Configure GPIO pins : LD1_Pin HCSR04_TRIG_Pin LD3_Pin LD2_Pin */
	GPIO_InitStruct.Pin = LD1_Pin|HCSR04_TRIG_Pin|LD3_Pin|LD2_Pin;
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

	/* USER CODE BEGIN MX_GPIO_Init_2 */

	/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/**
 * @brief Callback del Timer para la multiplexación del Display.
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
	if (htim->Instance == TIM2) {
		Display7Seg_Refresh_ISR(&Display);
	}
}

/**
 * @brief Callback de captura de flanco para el sensor ultrasónico.
 * @details Se ejecuta en contexto de interrupción (ISR).
 * Maneja la máquina de estados del driver HC-SR04.
 */
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim) {
	// 1. Verificar que la interrupción provenga del TIM3
	if (htim->Instance == TIM3) {

		// 2. Verificar que sea el canal configurado para el Echo (CH1)
		if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1) {

			// FEEDBACK VISUAL: Toggle del LED azul para confirmar captura de flanco
			// Útil para debugging; se puede eliminar en la versión final de producción.
			HAL_GPIO_TogglePin(usr_ledAzul_GPIO_Port, usr_ledAzul_Pin);

			// 3. Delegar la lógica de estados al driver
			// Aquí se registra el tiempo (CCR) y se cambia la polaridad del flanco.
			HCSR04_InputCapture_Callback(&ultrasonic);
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
