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

#include "API_debounce.h"
#include "API_delay.h"
#include "API_led.h"
#include <string.h>
#include <stdbool.h>

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
UART_HandleTypeDef huart3;

/* USER CODE BEGIN PV */

// Definimos los objetos LED (Abstracción Genérica)

// Definimos el arreglo de leds de la secuencia
led_t Leds[] = {
		{LED_1_GPIO_Port, LED_1_Pin, false}, 	//LED 1
		{LED_2_GPIO_Port, LED_2_Pin, false},	//LED 2
		{LED_3_GPIO_Port, LED_3_Pin, false},	//LED 3
		{LED_4_GPIO_Port, LED_4_Pin, false}		//LED 4
};


// Guardamos el tamaño del grupo para que sea dinámico
const uint8_t CANT_LEDS = sizeof(Leds) / sizeof(led_t);

led_t Leds_board[] = {
		{LD1_GPIO_Port,	LD1_Pin, false},
		{LD2_GPIO_Port, LD2_Pin, false},
		{LD3_GPIO_Port, LD3_Pin, false},
};

const uint8_t CANT_LEDS_BOARD = sizeof(Leds_board) / sizeof(led_t);

// Definimos el botón en PB11 (Active Low / Inverted = true)
button_t botones[] = {
		{BTN_1_GPIO_Port, BTN_1_Pin, true},     // Botón de Modo (PB11 - Active Low)
		{BTN_2_GPIO_Port, BTN_2_Pin, true}
};

// Cálculo dinámico de la cantidad de botones
const uint8_t CANT_BOTONES = sizeof(botones) / sizeof(button_t);

/* --- Definición de Escenas --- */
typedef enum {
	MODO_SYNC, MODO_CARRERA, MODO_REBOTE,
	MODO_STREAK, MODO_ALARMA, MODO_RANDOM,
	CANT_MODOS // Truco para resetear el contador automáticamente
} escena_t;

escena_t escenaActual = MODO_SYNC;
delay_t timerSecuencia;
uint8_t paso = 0;
bool direccion = true; // Para el efecto rebote

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART3_UART_Init(void);
/* USER CODE BEGIN PFP */

void Debug_Log(const char *msg);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/**
 * @brief Envía un mensaje por UART3 de forma simplificada.
 * @param msg Cadena de texto a transmitir.
 */
void Debug_Log(const char *msg) {
	HAL_UART_Transmit(&huart3, (uint8_t*)msg, strlen(msg), 100);
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
	/* USER CODE BEGIN 2 */

	Debug_Log("\r\n==================================\r\n");
	Debug_Log("PROYECTO INTEGRADOR: SECUENCIADOR 6\r\n");
	Debug_Log("UTN FRT - Nivel Basico Finalizado\r\n");
	Debug_Log("==================================\r\n");
	/* Inicialización de Drivers API */
	for (uint8_t i = 0; i < CANT_BOTONES; i++) {
		debounceFSM_Init(&botones[i]);
	}
	delayInit(&timerSecuencia, 500);
	Debug_Log("Sistema Inicializado: Modo Reentrante Activo\r\n");


	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1)
	{
		/* USER CODE END WHILE */

		/* USER CODE BEGIN 3 */

		/* 1. Actualizar TODAS las MEF de debounce */
		for (uint8_t i = 0; i < CANT_BOTONES; i++) {
			debounceFSM_Update(&botones[i]);
		}
		/* 2A. Procesar lógica del Botón de Modo (Índice 0) */
		if (readKey(&botones[0])) {
			escenaActual = (escenaActual + 1) % CANT_MODOS;
			paso = 0;
			direccion = true;
			LED_All_Off(Leds, CANT_LEDS);
			Debug_Log("\r\n--- CAMBIO DE MODO (Multinstancia) ---\r\n");
		}
		/* 2B. Procesar lógica del Botón de Modo (Índice 1) */
		if (readKey(&botones[1])) {
			LED_ToggleAll(Leds_board, CANT_LEDS_BOARD);
			Debug_Log("\r\n--- Toggle LEDS Placa ---\r\n");
		}

		/* 3. Lógica de las escenas (Timer no bloqueante) */
		if (delayRead(&timerSecuencia)) {
			switch (escenaActual) {

			case MODO_SYNC:
				delayWrite(&timerSecuencia, 500);
				LED_ToggleAll(Leds, CANT_LEDS);
				Debug_Log("MODO: Sincronizado - Toggle All\r\n");
				break;

			case MODO_CARRERA:
				delayWrite(&timerSecuencia, 150);
				LED_All_Off(Leds, CANT_LEDS);
				LED_On(&Leds[paso]);
				// Log que indica qué LED se enciende
				if(paso == 0) Debug_Log("CARRERA: LED Rojo\r\n");
				if(paso == 1) Debug_Log("CARRERA: LED Amarillo\r\n");
				if(paso == 2) Debug_Log("CARRERA: LED Verde\r\n");
				if(paso == 3) Debug_Log("CARRERA: LED Azul\r\n");

				paso = (paso + 1) % CANT_LEDS;
				break;

			case MODO_REBOTE:
				delayWrite(&timerSecuencia, 100);
				LED_All_Off(Leds, CANT_LEDS);
				LED_On(&Leds[paso]);
				Debug_Log("REBOTE: Posicion actual\r\n");
				if (direccion) paso++; else paso--;
				if (paso == (CANT_LEDS - 1) || paso == 0){
					direccion = !direccion;
					Debug_Log("REBOTE: Cambio de sentido\r\n");
				}
				break;

			case MODO_STREAK:
				delayWrite(&timerSecuencia, 200);
				if (paso < CANT_LEDS) {
					LED_On(&Leds[paso]);
					Debug_Log("STREAK: Agregando LED\r\n");
				} else {
					LED_All_Off(Leds, CANT_LEDS);
					Debug_Log("STREAK: Reset\r\n");
				}
				paso = (paso + 1) % (CANT_LEDS + 1);
				break;

			case MODO_ALARMA:
				delayWrite(&timerSecuencia, 80);
				LED_Toggle(&Leds[0]); // LED 1
				LED_Toggle(&Leds[3]); // LED 4
				LED_Off(&Leds[1]);    // LED 2
				LED_Off(&Leds[2]);    // LED 3
				Debug_Log("ALARMA: Strobe activo\r\n");
				break;

			case MODO_RANDOM:
				delayWrite(&timerSecuencia, 120);
				LED_All_Off(Leds, CANT_LEDS);
				uint8_t r = HAL_GetTick() % CANT_LEDS;
				LED_On(&Leds[r]);
				Debug_Log("RANDOM: LED Aleatorio\r\n");
				break;

			default:
				escenaActual = MODO_SYNC;
				break;
			}
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
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOF_CLK_ENABLE();
	__HAL_RCC_GPIOE_CLK_ENABLE();
	__HAL_RCC_GPIOD_CLK_ENABLE();
	__HAL_RCC_GPIOG_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOB, LD1_Pin|LED_2_Pin|LD3_Pin|LD2_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(LED_4_GPIO_Port, LED_4_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(LED_3_GPIO_Port, LED_3_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOG, USB_PowerSwitchOn_Pin|LED_1_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin : USER_Btn_Pin */
	GPIO_InitStruct.Pin = USER_Btn_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(USER_Btn_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pins : LD1_Pin LED_2_Pin LD3_Pin LD2_Pin */
	GPIO_InitStruct.Pin = LD1_Pin|LED_2_Pin|LD3_Pin|LD2_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	/*Configure GPIO pin : LED_4_Pin */
	GPIO_InitStruct.Pin = LED_4_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(LED_4_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pin : LED_3_Pin */
	GPIO_InitStruct.Pin = LED_3_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(LED_3_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pins : BTN_2_Pin BTN_1_Pin */
	GPIO_InitStruct.Pin = BTN_2_Pin|BTN_1_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	/*Configure GPIO pins : USB_PowerSwitchOn_Pin LED_1_Pin */
	GPIO_InitStruct.Pin = USB_PowerSwitchOn_Pin|LED_1_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

	/*Configure GPIO pin : USB_OverCurrent_Pin */
	GPIO_InitStruct.Pin = USB_OverCurrent_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(USB_OverCurrent_GPIO_Port, &GPIO_InitStruct);

	/* USER CODE BEGIN MX_GPIO_Init_2 */

	/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

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
