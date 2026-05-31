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
#include "servo_sg90.h"
#include "utils.h"
#include "tcs3200.h"
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
		.blanco_entrar = 14000,
		.blanco_salir = 11000,
		.estabilidad_requerida = 2
};

// Variables de estado (deben ser globales o estáticas)
static EstadoColor_t estado_actual = ESTADO_NEGRO;
static uint8_t contador_estabilidad = 0;

Servo_t servoBrazo;

/* --- Objeto para el Led RGB --- */
rgb_led_t ledRGB;

// Instancia del Sensor
TCS3200_t       colorSensor;      // Instancia del driver de Capa 2

/* ========================================================================== */
/* --- DECLARACIÓN DE DESCRIPTORES AGAL PARA TU NUEVO DRIVER MULTIPLATAFORMA - */
/* ========================================================================== */
generic_pwm_t   tcs_counter_ch;
generic_gpio_t  tcs_s0;
generic_gpio_t  tcs_s1;
generic_gpio_t  tcs_s2;
generic_gpio_t  tcs_s3;
generic_gpio_t  tcs_led;

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

/* --- Prototipos de Funciones Adaptadoras (Capa de Acoplamiento / PAL) --- */
void	 PAL_STM32_GPIO_Write(generic_gpio_t gpio, bool state);
bool     PAL_STM32_GPIO_Read(generic_gpio_t gpio);
void     PAL_STM32_PWM_Write(generic_pwm_t ch, uint16_t value);
uint32_t PAL_STM32_GetTick(void);
uint32_t PAL_STM32_GetTimerCounter(generic_pwm_t ch);
void 	 PAL_STM32_SetTimerCounter(generic_pwm_t ch, uint32_t value);
uint32_t PAL_STM32_GetUs(void);

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

		// --- Lógica de Decisión con Histéresis Optimizada (UTN FRT) ---
		uint32_t limite_blanco = (estado_actual == ESTADO_BLANCO) ?
				umbrales.blanco_salir : umbrales.blanco_entrar;

		// 1. Filtro de Negro / Fondo (Blindado contra ruido ambiente a < 5500)
		if (r < 5500 && g < 5500 && b < 5500) {
			nuevo_estado_raw = ESTADO_NEGRO;
		}
		// 2. Filtro de Blanco (Todos firmes arriba del límite de saturación)
		else if (r > limite_blanco && g > limite_blanco && b > limite_blanco) {
			nuevo_estado_raw = ESTADO_BLANCO;
		}
		// 3. Paleta de Colores por Relaciones de Aspecto Reales (Evidencia empírica)
		else {
			// Imprimimos la telemetría cruda en la UART para auditoría visual
			char msg_calibracion[64];
			sprintf(msg_calibracion, "RAW-> R:%lu G:%lu B:%lu\r\n", r, g, b);
			Debug_Log(msg_calibracion);

			// AMARILLO (R y G explotan hacia arriba, superan holgadamente al Azul)
			if (r > (b * 1.3f) && g > (b * 1.1f) && r > 12000) {
				nuevo_estado_raw = ESTADO_AMARILLO;
			}
			// MAGENTA (Exigimos que B supere los 7500 para dejar afuera al Rojo Puro de 6420)
			else if (r > (g * 1.5f) && b > (g * 1.5f) && b > 7500) {
				nuevo_estado_raw = ESTADO_MAGENTA;
			}
			// CIAN o AZUL (B y G dominan por completo al Rojo)
			else if (b > r && g > r && b > 10000) {
				// Si G aporta significativamente respecto a B es CIAN, sino es Azul Puro
				if (g > (b * 0.6f)) {
					nuevo_estado_raw = ESTADO_CIAN;
				} else {
					nuevo_estado_raw = ESTADO_AZUL;
				}
			}
			// --- COLORES PRIMARIOS PUROS (Por descarte directo si fallan las mezclas) ---
			else {
				if (r > g && r > b) {
					nuevo_estado_raw = ESTADO_ROJO;
				}
				else if (b > r && b > g) {
					nuevo_estado_raw = ESTADO_AZUL;
				}
				else {
					nuevo_estado_raw = ESTADO_VERDE;
				}
			}
		}

		// --- Filtro de Estabilidad (Anti-rebote de Capa 3) ---
		if (nuevo_estado_raw != estado_actual) {
			contador_estabilidad++;
			if (contador_estabilidad >= umbrales.estabilidad_requerida) {
				estado_actual = nuevo_estado_raw;
				contador_estabilidad = 0;

				// --- Ejecución de Acciones (Actuadores mediante abstracción PAL) ---
				switch (estado_actual) {
				case ESTADO_NEGRO:
					RGB_LED_SetColor(&ledRGB, 0, 0, 0);                 // LED Apagado
					SERVO_SG90_SetSpeedAngle(&servoBrazo, 0, 180.0f);   // Regresa suave a 0°
					Debug_Log(">> ESTABLE: NEGRO / VACÍO\r\n");
					break;
				case ESTADO_ROJO:
					RGB_LED_SetColor(&ledRGB, 1000, 0, 0);              // Rojo Puro
					SERVO_SG90_SetSpeedAngle(&servoBrazo, 25, 180.0f);  // Mover a 25°
					Debug_Log(">> ESTABLE: ROJO\r\n");
					break;
				case ESTADO_AMARILLO:
					RGB_LED_SetColor(&ledRGB, 1000, 1000, 0);           // Mezcla Amarillo
					SERVO_SG90_SetSpeedAngle(&servoBrazo, 50, 180.0f);  // Mover a 50°
					Debug_Log(">> ESTABLE: AMARILLO\r\n");
					break;
				case ESTADO_VERDE:
					RGB_LED_SetColor(&ledRGB, 0, 1000, 0);              // Verde Puro
					SERVO_SG90_SetSpeedAngle(&servoBrazo, 75, 180.0f);  // Mover a 75°
					Debug_Log(">> ESTABLE: VERDE\r\n");
					break;
				case ESTADO_CIAN:
					RGB_LED_SetColor(&ledRGB, 0, 1000, 1000);           // Mezcla Cian
					SERVO_SG90_SetSpeedAngle(&servoBrazo, 100, 180.0f); // Mover a 100°
					Debug_Log(">> ESTABLE: CIAN\r\n");
					break;
				case ESTADO_AZUL:
					RGB_LED_SetColor(&ledRGB, 0, 0, 1000);              // Azul Puro
					SERVO_SG90_SetSpeedAngle(&servoBrazo, 125, 180.0f); // Mover a 125°
					Debug_Log(">> ESTABLE: AZUL\r\n");
					break;
				case ESTADO_MAGENTA:
					RGB_LED_SetColor(&ledRGB, 1000, 0, 1000);           // Mezcla Magenta
					SERVO_SG90_SetSpeedAngle(&servoBrazo, 150, 180.0f); // Mover a 150°
					Debug_Log(">> ESTABLE: MAGENTA\r\n");
					break;
				case ESTADO_BLANCO:
					RGB_LED_SetColor(&ledRGB, 1000, 1000, 1000);        // Blanco Brillante
					SERVO_SG90_SetSpeedAngle(&servoBrazo, 180, 180.0f); // Mover a 180°
					Debug_Log(">> ESTABLE: BLANCO\r\n");
					break;
				}
			}
		} else {
			contador_estabilidad = 0; // Reset si el color se consolidó
		}
	}

	// Paso 2: Secuenciador de filtros (Independiente del procesamiento)
	// Mantiene al hardware rotando de manera constante
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


uint32_t PAL_STM32_GetTimerCounter(generic_pwm_t ch) {
	return __HAL_TIM_GET_COUNTER((TIM_HandleTypeDef*)ch.timer_handle);
}

// 2. Reutilizamos PWM_Write para resetear el contador (escribe en el registro del Timer)
void PAL_STM32_SetTimerCounter(generic_pwm_t ch, uint32_t value) {
	__HAL_TIM_SET_COUNTER((TIM_HandleTypeDef*)ch.timer_handle, value);
}

// 3. Adaptador de microsegundos mediante el registro CYCCNT del DWT interno del Cortex-M4
uint32_t PAL_STM32_GetUs(void) {
	return UTILS_GetUs();
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

	// ==========================================================================
	// INICIALIZACIÓN DEL REGISTRO DWT (Cortex-M4 Cycle Counter)
	// ==========================================================================
	UTILS_DWT_Init();

	// ==========================================================================
	// 0. CONFIGURACIÓN DE LA PAL UNIVERSAL COMPARTIDA
	// ==========================================================================
	hal_interface_t sys_pal = {
			.pwm_write     = PAL_STM32_PWM_Write,
			.gpio_read     = PAL_STM32_GPIO_Read,
			.gpio_write    = PAL_STM32_GPIO_Write,
			.get_tick      = PAL_STM32_GetTick,
			.get_timer_cnt = PAL_STM32_GetTimerCounter,
			.oc_write      = PAL_STM32_SetTimerCounter,
			.get_us        = PAL_STM32_GetUs
	};

	// ==========================================================================
	// 1. CONFIGURACIÓN DEL SERVO SG90
	// ==========================================================================
	// Descriptor del canal PWM para el Servo (TIM5 Canal 1)
	generic_pwm_t servo_pwm = { .timer_handle = &htim5, .channel = TIM_CHANNEL_1 };

	// Inicialización con inyección de la PAL Universal
	SERVO_SG90_Init(&servoBrazo, servo_pwm, sys_pal, 520, 2540);

	/* Iniciar hardware del Timer (Específico de la capa de acoplamiento ST) */
	HAL_TIM_PWM_Start(&htim5, TIM_CHANNEL_1);
	Debug_Log("[OK] Periferico TIM5: PWM Servo Listo (Canal 2)\r\n");

	// ==========================================================================
	// 2. CONFIGURACION DEL LED RGB
	// ==========================================================================
	generic_pwm_t ch_r = { .timer_handle = &htim1, .channel = TIM_CHANNEL_1 };
	generic_pwm_t ch_g = { .timer_handle = &htim1, .channel = TIM_CHANNEL_2 };
	generic_pwm_t ch_b = { .timer_handle = &htim1, .channel = TIM_CHANNEL_3 };

	// Inyectamos la misma PAL Universal compartida
	RGB_LED_Init(&ledRGB, ch_r, ch_g, ch_b, RGB_ANODE_COMMON, 500, sys_pal);
	RGB_LED_SetColor(&ledRGB, 0, 0, 0);

	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);
	Debug_Log("[OK] Periferico TIM1: PWM RGB Activo\r\n");

	// ==========================================================================
	// 3. CONFIGURACIÓN DEL SENSOR TCS3200 MULTIPLATAFORMA (Estrategia Sampling)
	// ==========================================================================
	// Vinculamos el Timer Contador (TIM4) al descriptor genérico
	tcs_counter_ch.timer_handle = &htim4;
	tcs_counter_ch.channel      = TIM_CHANNEL_1; // Cumplimos con la firma genérica

	// Mapeo de los Pines GPIO al formato de la PAL Universal
	tcs_s0.port  = TCS_S0_GPIO_Port;  tcs_s0.pin  = TCS_S0_Pin;
	tcs_s1.port  = TCS_S1_GPIO_Port;  tcs_s1.pin  = TCS_S1_Pin;
	tcs_s2.port  = TCS_S2_GPIO_Port;  tcs_s2.pin  = TCS_S2_Pin;
	tcs_s3.port  = TCS_S3_GPIO_Port;  tcs_s3.pin  = TCS_S3_Pin;
	tcs_led.port = TCS_LED_GPIO_Port; tcs_led.pin = TCS_LED_Pin;

	// Inicialización del nuevo Driver Agnóstico inyectando la PAL
	TCS3200_Init(&colorSensor, sys_pal, tcs_counter_ch);

	// Configuración de los pines a través de la API portátil
	TCS3200_ConfigGPIO(&colorSensor, tcs_s0, tcs_s1, tcs_s2, tcs_s3, tcs_led);

	// --- 3. Control Inicial Genérico ---
	TCS3200_ControlLED(&colorSensor, TCS_LED_ON);    // Prender luz frontal por PAL
	TCS3200_SetFilter(&colorSensor, TCS_FILTER_RED); // Empezar leyendo ROJO por PAL

	Debug_Log("--- Sistema Iniciado: Driver TCS3200 Multiplataforma ---\r\n");

	// --- 4. Arrancar Medición Continua (Especificidad del Silicio ST) ---
	// Reseteamos y encendemos los timers nativos para que el conteo empiece físicamente
	__HAL_TIM_SET_COUNTER(&htim4, 0);
	HAL_TIM_Base_Start(&htim4);     // TIM4 cuenta pulsos en PB6 de forma autónoma
	HAL_TIM_Base_Start_IT(&htim3);  // TIM3 abre la ventana cada 100ms con interrupción

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

// Callback del temporizador metrónomo (TIM3 - Cada 100ms)
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
	if (htim->Instance == TIM3) {
		// Llamamos al proceso del driver pasándole la instancia abstracta
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
