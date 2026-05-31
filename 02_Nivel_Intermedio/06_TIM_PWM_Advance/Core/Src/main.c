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
#include "rgb_led.h"
#include "Display_7Seg.h"
#include "encoder_ky040.h"
#include "servo_sg90.h"
#include "stdio.h"
#include "string.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* Estados de colores */
typedef enum {
	ST_RED,
	ST_YELLOW,
	ST_MAGENTA,
	ST_GREEN,
	ST_BLUE,
	ST_NONE
} LED_State_t;
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
LED_State_t last_led_state = ST_NONE;
LED_State_t current_led_state = ST_NONE;
/* --- Objeto para el Led RGB --- */
rgb_led_t ledRGB;

/* --- Objeto para los display de 7 segmentos --- */
display_7seg_t Display;
uint8_t bufferDisplay[4]; //buffer para la cantidad de displays

/* --- Objeto para el Servo SG90 --- */
Servo_t servo_main;

/* --- Objeto para el Encoder KY-040 --- */
KY040_t encoder_ctrl;

/* Variables de control de la aplicación */
int32_t last_angle = -1; // Para refrescar display solo si cambia

char uart_buf[100];
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART3_UART_Init(void);
static void MX_TIM2_Init(void);
static void MX_TIM3_Init(void);
static void MX_TIM4_Init(void);
/* USER CODE BEGIN PFP */

/* --- Prototipos de Funciones Auxiliares --- */
void Debug_Log(const char *msg);
void UI_Update_Feedback(int32_t angle);
/* --- Prototipos de Funciones Adaptadoras (Capa de Acoplamiento / PAL) --- */
void	 PAL_STM32_GPIO_Write(generic_gpio_t gpio, bool state);
bool     PAL_STM32_GPIO_Read(generic_gpio_t gpio);
void     PAL_STM32_PWM_Write(generic_pwm_t ch, uint16_t value);
uint32_t PAL_STM32_GetTick(void);

void     PAL_Display_WritePin(display_gpio_t pin, bool state);
bool     PAL_Display_ReadPin(display_gpio_t pin);
uint32_t PAL_Display_GetTick(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

void Debug_Log(const char *msg) {
	HAL_UART_Transmit(&huart3, (uint8_t*)msg, strlen(msg), 100);
}


/**
 * @brief Gestiona el feedback visual (LED RGB) según la posición del servo.
 * @param angle Ángulo entero actual del sistema.
 */
void UI_Update_Feedback(int32_t angle) {
	// Determinación del estado basada en la posición solicitada
	if (angle == 0)                          current_led_state = ST_RED;
	else if (angle > 0 && angle < 90)        current_led_state = ST_YELLOW;
	else if (angle == 90)                   current_led_state = ST_MAGENTA;
	else if (angle > 90 && angle < 180)      current_led_state = ST_GREEN;
	else if (angle == 180)                  current_led_state = ST_BLUE;
	else                                    current_led_state = ST_NONE;

	// Actualización estática del hardware
	if (current_led_state != last_led_state) {
		last_led_state = current_led_state;

		switch (current_led_state) {
		case ST_RED:     RGB_LED_SetColor(&ledRGB, 1000, 0, 0);     break;
		case ST_YELLOW:  RGB_LED_SetColor(&ledRGB, 1000, 1000, 0);  break;
		case ST_MAGENTA: RGB_LED_SetColor(&ledRGB, 1000, 0, 1000); break;
		case ST_GREEN:   RGB_LED_SetColor(&ledRGB, 0, 1000, 0);   break;
		case ST_BLUE:    RGB_LED_SetColor(&ledRGB, 0, 0, 1000);    break;
		default:         RGB_LED_SetColor(&ledRGB, 0, 0, 0);     break;
		}
	}
}

/* ========================================================================== */
/* ---- IMPLEMENTACIÓN DE ADAPTADORES DE HARDWARE (CONTRATOS PAL) ----------- */
/* ========================================================================== */

/* --- Adaptadores para PAL Universal --- */

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


/* --- Adaptadores para PAL del Display de 7 Segmentos --- */
void PAL_Display_WritePin(display_gpio_t pin, bool state) {
	HAL_GPIO_WritePin((GPIO_TypeDef*)pin.port, pin.pin, state ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

bool PAL_Display_ReadPin(display_gpio_t pin) {
	return (HAL_GPIO_ReadPin((GPIO_TypeDef*)pin.port, pin.pin) == GPIO_PIN_SET);
}

uint32_t PAL_Display_GetTick(void) {
	return HAL_GetTick();
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
	MX_TIM2_Init();
	MX_TIM3_Init();
	MX_TIM4_Init();
	/* USER CODE BEGIN 2 */

	/* --- Log de Bienvenida: Laboratorio 06 --- */
	Debug_Log("\r\n==========================================\r\n");
	Debug_Log("   LABORATORIO 06: PWM AVANZADO  \r\n");
	Debug_Log("   CONTROL DE SERVOMOTOR SG90   \r\n");
	Debug_Log("   Plataforma: STM32 Nucleo-F439ZI        \r\n");
	Debug_Log("==========================================\r\n");
	Debug_Log("[OK] Nucleo Clock: 180 MHz\r\n");
	Debug_Log(">>> Iniciando Perifericos...\r\n");

	// ==========================================================================
	// 0. CONFIGURACIÓN DE LA PAL UNIVERSAL COMPARTIDA
	// ==========================================================================
	hal_interface_t sys_pal = {
			.pwm_write = PAL_STM32_PWM_Write,
			.gpio_read = PAL_STM32_GPIO_Read,
			.get_tick  = PAL_STM32_GetTick
	};

	// ==========================================================================
	// 1. CONFIGURACIÓN DEL SERVO SG90
	// ==========================================================================
	// Descriptor del canal PWM para el Servo (TIM3 Canal 2)
	generic_pwm_t servo_pwm = { .timer_handle = &htim3, .channel = TIM_CHANNEL_2 };

	// Inicialización con inyección de la PAL Universal
	SERVO_SG90_Init(&servo_main, servo_pwm, sys_pal, 520, 2540);

	/* Iniciar hardware del Timer (Específico de la capa de acoplamiento ST) */
	HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
	Debug_Log("[OK] Periferico TIM3: PWM Servo Listo (Canal 2)\r\n");


	// ==========================================================================
	// 2. CONFIGURACION DEL DISPLAY DE 7 SEGMENTOS
	// ==========================================================================
	static display_gpio_t segmentos[] = {
			{SEG_A_GPIO_Port, SEG_A_Pin}, {SEG_B_GPIO_Port, SEG_B_Pin},
			{SEG_C_GPIO_Port, SEG_C_Pin}, {SEG_D_GPIO_Port, SEG_D_Pin},
			{SEG_E_GPIO_Port, SEG_E_Pin}, {SEG_F_GPIO_Port, SEG_F_Pin},
			{SEG_G_GPIO_Port, SEG_G_Pin}
	};
	static display_gpio_t comunes[] = {
			{EN4_GPIO_Port, EN4_Pin}, {EN3_GPIO_Port, EN3_Pin},
			{EN2_GPIO_Port, EN2_Pin}, {EN1_GPIO_Port, EN1_Pin}
	};

	display_7seg_pal_t display_pal = {
			.write_pin = PAL_Display_WritePin,
			.read_pin  = PAL_Display_ReadPin,
			.get_tick  = PAL_Display_GetTick
	};

	Display7Seg_Init(&Display, display_pal, segmentos, comunes, 4, bufferDisplay, DISPLAY_CATHODE);
	HAL_TIM_Base_Start_IT(&htim2);

	Display7Seg_SetBrightness(&Display, 80);
	Display7Seg_WriteString(&Display, "HOLA");
	HAL_Delay(1000);
	Debug_Log("[OK] Periferico TIM2: Multiplexado Display\r\n");

	// ==========================================================================
	// 3. CONFIGURACION DEL LED RGB
	// ==========================================================================
	generic_pwm_t ch_r = { .timer_handle = &htim4, .channel = TIM_CHANNEL_2 };
	generic_pwm_t ch_g = { .timer_handle = &htim4, .channel = TIM_CHANNEL_3 };
	generic_pwm_t ch_b = { .timer_handle = &htim4, .channel = TIM_CHANNEL_4 };

	// Inyectamos la misma PAL Universal compartida
	RGB_LED_Init(&ledRGB, ch_r, ch_g, ch_b, RGB_ANODE_COMMON, 500, sys_pal);
	RGB_LED_SetColor(&ledRGB, 1000, 0, 0);

	HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_2);
	HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_3);
	HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_4);
	Debug_Log("[OK] Periferico TIM4: PWM RGB Activo\r\n");

	// ==========================================================================
	// 4. CONFIGURACIÓN DEL ENCODER KY-040
	// ==========================================================================
	// Armamos los descriptores de los pines físicos
	generic_gpio_t enc_clk = { .port = ENC_CLK_GPIO_Port, .pin = ENC_CLK_Pin };
	generic_gpio_t enc_dt  = { .port = ENC_DT_GPIO_Port,  .pin = ENC_DT_Pin  };
	generic_gpio_t enc_sw  = { .port = ENC_SW_GPIO_Port,  .pin = ENC_SW_Pin  };

	// Inicialización agnóstica inyectando la PAL Universal
	KY040_Init(&encoder_ctrl, sys_pal, enc_clk, enc_dt, enc_sw, 0, 180);

	// Sincronizamos la posición inicial del servo con el encoder (90 grados)
	SERVO_SG90_SetAngle(&servo_main, encoder_ctrl.position);
	Debug_Log("[OK] Encoder KY-040: Rango 0-180 Configurado\r\n");

	Display7Seg_SetBrightness(&Display, 80);
	Display7Seg_WriteString(&Display, "HI");
	Debug_Log(">>> Sistema listo...\r\n\r\n");


	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1)
	{
		/* USER CODE END WHILE */

		/* USER CODE BEGIN 3 */
		// 1. MOTOR: Actualizar posición intermedia del servo (interpolación suave)
		SERVO_SG90_Update(&servo_main);

		// 2. CONTROL: Sincronizar el objetivo del servo con la perilla del encoder
		int32_t target_from_encoder = encoder_ctrl.position;
		SERVO_SG90_SetSpeedAngle(&servo_main, (int)target_from_encoder, 150.0f);

		/* --- 3. FEEDBACK VISUAL 1: LED RGB (Lógica por Segmentos Fijos) --- */

		UI_Update_Feedback((int)servo_main.current_angle);

		// 4. FEEDBACK VISUAL 2: Display de 7 Segmentos
		// Solo actualizamos el display cuando el ángulo entero cambia
		if ((int)servo_main.current_angle != last_angle) {
			last_angle = (int)servo_main.current_angle;
			Display7Seg_WriteNumber(&Display, last_angle);

			// Log opcional para debug en PC
			sprintf(uart_buf, "Angulo: %d | Target: %d\r\n", (int)last_angle, (int)target_from_encoder);
			Debug_Log(uart_buf);
		}

		// 5. ACCIÓN: Botón Reset del Encoder
		if (encoder_ctrl.sw_pressed) {
			encoder_ctrl.position = 90;
			encoder_ctrl.sw_pressed = 0;
			HAL_GPIO_TogglePin(usr_ledAzul_GPIO_Port, usr_ledAzul_Pin);
			Debug_Log(">>> Encoder Reset: 90 Grad\r\n");
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
	TIM_OC_InitTypeDef sConfigOC = {0};

	/* USER CODE BEGIN TIM3_Init 1 */

	/* USER CODE END TIM3_Init 1 */
	htim3.Instance = TIM3;
	htim3.Init.Prescaler = 89;
	htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim3.Init.Period = 19999;
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
	if (HAL_TIM_PWM_Init(&htim3) != HAL_OK)
	{
		Error_Handler();
	}
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
	{
		Error_Handler();
	}
	sConfigOC.OCMode = TIM_OCMODE_PWM1;
	sConfigOC.Pulse = 0;
	sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
	sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
	if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
	{
		Error_Handler();
	}
	/* USER CODE BEGIN TIM3_Init 2 */

	/* USER CODE END TIM3_Init 2 */
	HAL_TIM_MspPostInit(&htim3);

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
	HAL_GPIO_WritePin(GPIOB, LD1_Pin|LD3_Pin|LD2_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOG, SEG_G_Pin|USB_PowerSwitchOn_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOC, EN1_Pin|EN2_Pin|EN3_Pin|EN4_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pins : SEG_C_Pin SEG_A_Pin SEG_B_Pin */
	GPIO_InitStruct.Pin = SEG_C_Pin|SEG_A_Pin|SEG_B_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

	/*Configure GPIO pin : USER_Btn_Pin */
	GPIO_InitStruct.Pin = USER_Btn_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(USER_Btn_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pins : SEG_E_Pin SEG_D_Pin SEG_F_Pin usr_ledAzul_Pin */
	GPIO_InitStruct.Pin = SEG_E_Pin|SEG_D_Pin|SEG_F_Pin|usr_ledAzul_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

	/*Configure GPIO pin : ENC_DT_Pin */
	GPIO_InitStruct.Pin = ENC_DT_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(ENC_DT_GPIO_Port, &GPIO_InitStruct);

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

	/*Configure GPIO pin : USB_OverCurrent_Pin */
	GPIO_InitStruct.Pin = USB_OverCurrent_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(USB_OverCurrent_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pins : EN1_Pin EN2_Pin EN3_Pin EN4_Pin */
	GPIO_InitStruct.Pin = EN1_Pin|EN2_Pin|EN3_Pin|EN4_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

	/*Configure GPIO pin : ENC_CLK_Pin */
	GPIO_InitStruct.Pin = ENC_CLK_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(ENC_CLK_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pin : ENC_SW_Pin */
	GPIO_InitStruct.Pin = ENC_SW_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(ENC_SW_GPIO_Port, &GPIO_InitStruct);

	/* EXTI interrupt init*/
	HAL_NVIC_SetPriority(EXTI9_5_IRQn, 5, 0);
	HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

	HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

	/* USER CODE BEGIN MX_GPIO_Init_2 */

	/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/**
 * @brief Callback para interrupciones de Timer (Multiplexado del Display)
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	// Verificamos que la interrupción provenga del TIM2 (el del display)
	if (htim->Instance == TIM2)
	{
		// Llamamos a la función de refresco del driver del display
		// Esta función apaga el dígito anterior y enciende el siguiente
		Display7Seg_Refresh_ISR(&Display);
	}
}

/**
 * @brief Callback para interrupciones externas (Giro y Botón del Encoder)
 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	/* Instanciamos el descriptor genérico para identificar el pin disparado */
	generic_gpio_t triggered_gpio;
	triggered_gpio.pin = GPIO_Pin;

	/* Mapeamos el puerto correcto según el pin físico de la STM32 */
	if (GPIO_Pin == ENC_CLK_Pin)
	{
		triggered_gpio.port = ENC_CLK_GPIO_Port;
		KY040_IRQ_Handler(&encoder_ctrl, triggered_gpio);
	}
	else if (GPIO_Pin == ENC_SW_Pin)
	{
		triggered_gpio.port = ENC_SW_GPIO_Port;
		KY040_IRQ_Handler(&encoder_ctrl, triggered_gpio);
	}

	// Opcional: Si tienes el botón de la placa (USER_Btn_Pin), puedes manejarlo aquí
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
