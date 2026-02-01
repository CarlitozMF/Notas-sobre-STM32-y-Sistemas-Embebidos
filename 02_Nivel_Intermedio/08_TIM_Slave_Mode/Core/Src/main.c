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
#include "tcs3200_stm32.h"
#include "servo_sg90.h"
#include <stdio.h>
#include <string.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
typedef enum {
	ESTADO_NEGRO = 0,
	ESTADO_BLANCO,
	ESTADO_ROJO,
	ESTADO_VERDE,
	ESTADO_AZUL,
	ESTADO_AMARILLO, // R+G
	ESTADO_CIAN,     // G+B
	ESTADO_MAGENTA   // R+B
} EstadoColor_t;

// Estructura para evitar hardcoding
typedef struct {
	uint32_t negro;
	uint32_t blanco_entrar;
	uint32_t blanco_salir;
	uint8_t  estabilidad_requerida;
} Config_Umbrales_t;
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim4;
TIM_HandleTypeDef htim5;

UART_HandleTypeDef huart3;

/* USER CODE BEGIN PV */

// Valores calibrados para tu entorno
Config_Umbrales_t umbrales = {
		.negro = 5000,
		.blanco_entrar = 30000,
		.blanco_salir = 12000,
		.estabilidad_requerida = 2
};

// Variables de estado (deben ser globales o estáticas)
static EstadoColor_t estado_actual = ESTADO_NEGRO;
static uint8_t contador_estabilidad = 0;

Servo_t servoBrazo;

/* --- Objeto para el Led RGB --- */
RGB_LED_t ledRGB;

// Instancia del Sensor
TCS3200_t colorSensor;

// Buffer para mensajes UART
char msg_buffer[64];

// Variables para lógica de cambio de filtro (Máquina de estados simple)
uint8_t filtro_actual = 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART3_UART_Init(void);
static void MX_TIM3_Init(void);
static void MX_TIM4_Init(void);
static void MX_TIM1_Init(void);
static void MX_TIM5_Init(void);
/* USER CODE BEGIN PFP */
void Debug_Log(const char *msg);
void Procesar_Logica_Color(void);
void Heartbeat_Handler(void);
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

void Procesar_Logica_Color(void) {
	colorSensor.measurement_ready = 0; // Ack de la medición

	// Paso 1: Solo procesamos cuando el ciclo RGB está completo (filtro azul terminado)
	if (filtro_actual == 2) {
		uint32_t r = colorSensor.frequency_red;
		uint32_t g = colorSensor.frequency_green;
		uint32_t b = colorSensor.frequency_blue;
		uint8_t nuevo_estado_raw;

		// --- Lógica de Decisión con Histéresis ---
		uint32_t limite_blanco = (estado_actual == ESTADO_BLANCO) ?
				umbrales.blanco_salir : umbrales.blanco_entrar;

		if (r < umbrales.negro && g < umbrales.negro && b < umbrales.negro) {
			nuevo_estado_raw = ESTADO_NEGRO;
		}
		else if (r > limite_blanco && g > limite_blanco && b > limite_blanco) {
			nuevo_estado_raw = ESTADO_BLANCO;
		}
		else {

			// 1. Creamos un buffer temporal local
			char msg_calibracion[64];

			// 2. Formateamos el texto con los números
			// Usamos %lu porque las frecuencias son uint32_t (long unsigned)
			sprintf(msg_calibracion, "R:%lu G:%lu B:%lu\r\n", r, g, b);

			// 3. Se lo pasamos a TU función de siempre
			Debug_Log(msg_calibracion);

			// AMARILLO (Rojo y Verde altos, Azul bajo)
			if (r > b*1.5f && g > b*1.5f) {
				nuevo_estado_raw = ESTADO_AMARILLO;
			}
			// CIAN (Verde y Azul altos, Rojo bajo)
			else if (b > r && g > r) {
				// Si el Verde es mayor a la MITAD del Azul, es CIAN
				// (44400 / 2 = 22200). Como tu G es 27400, entra perfecto.
				if (g > (b >> 1)) {
					nuevo_estado_raw = ESTADO_CIAN;
				} else {
					nuevo_estado_raw = ESTADO_AZUL;
				}
			}
			// MAGENTA (Rojo y Azul altos, Verde bajo)
			else if (r > g*1.5f && b > g*1.5f) {
				nuevo_estado_raw = ESTADO_MAGENTA;
			}
			// --- COLORES PRIMARIOS (Si no hubo combinación clara) ---
			else if (r > g && r > b)      nuevo_estado_raw = ESTADO_ROJO;
			else nuevo_estado_raw = ESTADO_VERDE;
		}

		// --- Filtro de Estabilidad (Anti-rebote) ---
		if (nuevo_estado_raw != estado_actual) {
			contador_estabilidad++;
			if (contador_estabilidad >= umbrales.estabilidad_requerida) {
				estado_actual = nuevo_estado_raw;
				contador_estabilidad = 0;

				// --- Ejecución de Acciones (Actuadores) ---
				switch (estado_actual) {
				case ESTADO_NEGRO:
					RGB_Set_Preset(&ledRGB, COLOR_OFF);
					SERVO_SG90_SetSpeedAngle(&servoBrazo, 0, 180.0f); // Regresa suave a 0°
					Debug_Log(">> ESTABLE: NEGRO\r\n");
					break;
				case ESTADO_ROJO:
					RGB_Set_Preset(&ledRGB, COLOR_RED);
					SERVO_SG90_SetSpeedAngle(&servoBrazo, 25, 180.0f); // Regresa suave a 25°
					Debug_Log(">> ESTABLE: ROJO\r\n");
					break;
				case ESTADO_AMARILLO:
					RGB_Set_Preset(&ledRGB, COLOR_YELLOW);
					SERVO_SG90_SetSpeedAngle(&servoBrazo, 50, 180.0f); // Regresa suave a 50°
					Debug_Log(">> ESTABLE: AMARILLO\r\n");
					break;
				case ESTADO_VERDE:
					RGB_Set_Preset(&ledRGB, COLOR_GREEN);
					SERVO_SG90_SetSpeedAngle(&servoBrazo, 75, 180.0f); // Regresa suave a 75°
					Debug_Log(">> ESTABLE: VERDE\r\n");
					break;
				case ESTADO_CIAN:
					RGB_Set_Preset(&ledRGB, COLOR_CYAN);
					SERVO_SG90_SetSpeedAngle(&servoBrazo, 100, 180.0f); // Regresa suave a 100°
					Debug_Log(">> ESTABLE: CIAN\r\n");
					break;
				case ESTADO_AZUL:
					RGB_Set_Preset(&ledRGB, COLOR_BLUE);
					SERVO_SG90_SetSpeedAngle(&servoBrazo, 125, 180.0f); // Regresa suave a 125°
					Debug_Log(">> ESTABLE: AZUL\r\n");
					break;
				case ESTADO_MAGENTA:
					RGB_Set_Preset(&ledRGB, COLOR_MAGENTA);
					SERVO_SG90_SetSpeedAngle(&servoBrazo, 150, 180.0f); // Regresa suave a 150°
					Debug_Log(">> ESTABLE: MAGENTA\r\n");
					break;
				case ESTADO_BLANCO:
					RGB_Set_Preset(&ledRGB, COLOR_WHITE);
					SERVO_SG90_SetSpeedAngle(&servoBrazo, 180, 180.0f); // Regresa suave a 0°
					Debug_Log(">> ESTABLE: BLANCO\r\n");
					break;
				}
			}
		} else {
			contador_estabilidad = 0; // Reset si el color se mantiene igual
		}
	}

	// Paso 2: Secuenciador de filtros (Independiente del procesamiento)
	// Esto garantiza que el sensor siempre esté rotando los filtros
	filtro_actual++;
	if(filtro_actual > 2) filtro_actual = 0;

	switch(filtro_actual) {
	case 0: TCS3200_SetFilter(&colorSensor, TCS_FILTER_RED);   break;
	case 1: TCS3200_SetFilter(&colorSensor, TCS_FILTER_GREEN); break;
	case 2: TCS3200_SetFilter(&colorSensor, TCS_FILTER_BLUE);  break;
	}
}


void Heartbeat_Handler(void) {
	static uint32_t last_tick = 0;
	// Parpadea cada 500ms sin importar qué pase con el resto del código
	if (HAL_GetTick() - last_tick >= 500) {
		HAL_GPIO_TogglePin(usr_ledRojo_GPIO_Port, usr_ledRojo_Pin);
		last_tick = HAL_GetTick();
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
	MX_TIM3_Init();
	MX_TIM4_Init();
	MX_TIM1_Init();
	MX_TIM5_Init();
	/* USER CODE BEGIN 2 */

	// --- 1. Inicialización de Actuadores (Servo y RGB) ---
	// 1. Empaquetamos la configuración de hardware
	RGB_Config_t configRGB = {
			.htim = &htim1,						//Timer utilizado para el led RGB
			.R_channel = TIM_CHANNEL_1,			//RGB_R pin
			.G_channel = TIM_CHANNEL_2,			//RGB_G pin
			.B_channel = TIM_CHANNEL_3,			//RGB_B pin
			.led_type = LED_TYPE_ANODE_COMMON,	//Tipo de RGB -Cambiar si es catodo comun-
			.max_brightness = 1000				//Brillo 0-1000
	};
	// 2. Inicializamos el driver con el paquete
	RGB_Init_Single(&ledRGB, &configRGB);

	// Usamos TIM5, Canal 1 (habitualmente pin PA0 en la Nucleo)
	// Calibración estándar: 500us (0°) a 2500us (180°)
	SERVO_SG90_Init(&servoBrazo, &htim5, TIM_CHANNEL_1, 500, 2500);

	// Posición de inicio
	SERVO_SG90_SetAngle(&servoBrazo, 0);

	// --- 2. Inicialización del Sensor TCS3200 ---
	TCS3200_Init(&colorSensor, &htim3, &htim4);

	// Configuración de Pines usando las MACROS de main.h (Más seguro)
	// Asegúrate de que estos nombres coincidan con los que pusiste en CubeMX.
	// Si CubeMX generó otros nombres, cámbialos aquí.
	TCS3200_ConfigGPIO(&colorSensor,
			TCS_S0_GPIO_Port, TCS_S0_Pin,
			TCS_S1_GPIO_Port, TCS_S1_Pin,
			TCS_S2_GPIO_Port, TCS_S2_Pin,
			TCS_S3_GPIO_Port, TCS_S3_Pin,
			TCS_LED_GPIO_Port, TCS_LED_Pin);

	// --- 3. Configuración Inicial ---
	TCS3200_ControlLED(&colorSensor, TCS_LED_ON); // Prender luz frontal
	TCS3200_SetFilter(&colorSensor, TCS_FILTER_RED); // Empezar leyendo ROJO

	Debug_Log("--- Sistema Iniciado: Sensor TCS3200 ---\r\n");

	// --- 4. Arrancar Medición Continua ---
	TCS3200_StartMeasurement(&colorSensor);

	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1)
	{
		/* USER CODE END WHILE */

		/* USER CODE BEGIN 3 */
		// 1. Tarea del Sensor (Cada 100ms procesa color)
		if (colorSensor.measurement_ready) {
			Procesar_Logica_Color();
		}

		// 2. Tarea del Servo (Actualiza la posición suave en cada vuelta)
		SERVO_SG90_Update(&servoBrazo);

		// 3. Heartbeat (LED de estado)
		Heartbeat_Handler();
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
 * @brief TIM1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_TIM1_Init(void)
{

	/* USER CODE BEGIN TIM1_Init 0 */

	/* USER CODE END TIM1_Init 0 */

	TIM_ClockConfigTypeDef sClockSourceConfig = {0};
	TIM_MasterConfigTypeDef sMasterConfig = {0};
	TIM_OC_InitTypeDef sConfigOC = {0};
	TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

	/* USER CODE BEGIN TIM1_Init 1 */

	/* USER CODE END TIM1_Init 1 */
	htim1.Instance = TIM1;
	htim1.Init.Prescaler = 179;
	htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim1.Init.Period = 999;
	htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim1.Init.RepetitionCounter = 0;
	htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
	{
		Error_Handler();
	}
	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
	if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
	{
		Error_Handler();
	}
	if (HAL_TIM_PWM_Init(&htim1) != HAL_OK)
	{
		Error_Handler();
	}
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
	{
		Error_Handler();
	}
	sConfigOC.OCMode = TIM_OCMODE_PWM1;
	sConfigOC.Pulse = 0;
	sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
	sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
	sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
	sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
	sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
	if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
	{
		Error_Handler();
	}
	if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
	{
		Error_Handler();
	}
	if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_3) != HAL_OK)
	{
		Error_Handler();
	}
	sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
	sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
	sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
	sBreakDeadTimeConfig.DeadTime = 0;
	sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
	sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
	sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
	if (HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig) != HAL_OK)
	{
		Error_Handler();
	}
	/* USER CODE BEGIN TIM1_Init 2 */

	/* USER CODE END TIM1_Init 2 */
	HAL_TIM_MspPostInit(&htim1);

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
	htim3.Init.Prescaler = 9000-1;
	htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim3.Init.Period = 1000-1;
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
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_OC1REF;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_ENABLE;
	if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
	{
		Error_Handler();
	}
	sConfigOC.OCMode = TIM_OCMODE_PWM1;
	sConfigOC.Pulse = 0;
	sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
	sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
	if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
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

	TIM_SlaveConfigTypeDef sSlaveConfig = {0};
	TIM_MasterConfigTypeDef sMasterConfig = {0};

	/* USER CODE BEGIN TIM4_Init 1 */

	/* USER CODE END TIM4_Init 1 */
	htim4.Instance = TIM4;
	htim4.Init.Prescaler = 0;
	htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim4.Init.Period = 65535;
	htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	if (HAL_TIM_Base_Init(&htim4) != HAL_OK)
	{
		Error_Handler();
	}
	sSlaveConfig.SlaveMode = TIM_SLAVEMODE_EXTERNAL1;
	sSlaveConfig.InputTrigger = TIM_TS_TI1FP1;
	sSlaveConfig.TriggerPolarity = TIM_TRIGGERPOLARITY_RISING;
	sSlaveConfig.TriggerFilter = 0;
	if (HAL_TIM_SlaveConfigSynchro(&htim4, &sSlaveConfig) != HAL_OK)
	{
		Error_Handler();
	}
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
	{
		Error_Handler();
	}
	/* USER CODE BEGIN TIM4_Init 2 */

	/* USER CODE END TIM4_Init 2 */

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
	htim5.Init.Period = 19999;
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
	if (HAL_TIM_PWM_Init(&htim5) != HAL_OK)
	{
		Error_Handler();
	}
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	if (HAL_TIMEx_MasterConfigSynchronization(&htim5, &sMasterConfig) != HAL_OK)
	{
		Error_Handler();
	}
	sConfigOC.OCMode = TIM_OCMODE_PWM1;
	sConfigOC.Pulse = 0;
	sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
	sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
	if (HAL_TIM_PWM_ConfigChannel(&htim5, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
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
	__HAL_RCC_GPIOF_CLK_ENABLE();
	__HAL_RCC_GPIOE_CLK_ENABLE();
	__HAL_RCC_GPIOD_CLK_ENABLE();
	__HAL_RCC_GPIOG_CLK_ENABLE();

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOB, LD1_Pin|LD3_Pin|LD2_Pin|usr_ledRojo_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(TCS_LED_GPIO_Port, TCS_LED_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(TCS_S0_GPIO_Port, TCS_S0_Pin, GPIO_PIN_SET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOG, TCS_S1_Pin|USB_PowerSwitchOn_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOC, TCS_S2_Pin|TCS_S3_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin : USER_Btn_Pin */
	GPIO_InitStruct.Pin = USER_Btn_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(USER_Btn_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pins : LD1_Pin LD3_Pin LD2_Pin usr_ledRojo_Pin */
	GPIO_InitStruct.Pin = LD1_Pin|LD3_Pin|LD2_Pin|usr_ledRojo_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	/*Configure GPIO pin : TCS_LED_Pin */
	GPIO_InitStruct.Pin = TCS_LED_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(TCS_LED_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pins : TCS_S0_Pin TCS_S1_Pin USB_PowerSwitchOn_Pin */
	GPIO_InitStruct.Pin = TCS_S0_Pin|TCS_S1_Pin|USB_PowerSwitchOn_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

	/*Configure GPIO pin : USB_OverCurrent_Pin */
	GPIO_InitStruct.Pin = USB_OverCurrent_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(USB_OverCurrent_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pins : TCS_S2_Pin TCS_S3_Pin */
	GPIO_InitStruct.Pin = TCS_S2_Pin|TCS_S3_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

	/* USER CODE BEGIN MX_GPIO_Init_2 */

	/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/**
 * @brief Callback que se ejecuta cuando el TIM3 cumple su periodo (500ms).
 * Es el "metrónomo" del sistema.
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	// Verificamos que la interrupción venga del TIM3
	if (htim->Instance == TIM3) {
		// Ejecutamos la lógica de cálculo del driver
		TCS3200_ProcessCallback(&colorSensor);
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
