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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define MAX_NIVEL 100
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart3;

/* USER CODE BEGIN PV */
#define NIVEL_VICTORIA 10  // El juego termina al ganar 10 rondas
uint8_t secuencia[MAX_NIVEL];
uint8_t nivel_actual = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART3_UART_Init(void);
/* USER CODE BEGIN PFP */

/**
 * @brief  Bloquea la ejecución hasta que se presiona uno de los 4 botones.
 * @note   Implementa una espera activa con un pequeño delay para reducir el consumo de CPU.
 * @retval uint8_t ID del botón presionado (1 a 4).
 */
uint8_t Simon_EsperarBoton(void);

/**
 * @brief  Verifica de forma no bloqueante si algún botón está siendo presionado.
 * @note   Útil para detectar el inicio del juego o flancos de bajada.
 * @retval uint8_t 1 si hay al menos un botón presionado, 0 en caso contrario.
 */
uint8_t Simon_CualquierBotonPresionado(void);

/**
 * @brief  Muestra la secuencia generada hasta el nivel actual utilizando los LEDs.
 * @details Itera sobre el arreglo 'secuencia', encendiendo el LED correspondiente
 * con tiempos de encendido (600ms) y apagado (200ms).
 * @retval None
 */
void Simon_ReproducirSecuencia(void);

/**
 * @brief  Enciende un LED específico y asegura que los demás estén apagados.
 * @param  num: ID del LED a encender (1 a 4).
 * @retval None
 */
void Simon_EncenderUno(uint8_t num);

/**
 * @brief  Apaga todos los LEDs del juego de forma simultánea.
 * @note   Utiliza operaciones de bits (OR) para manejar múltiples pines en una sola llamada HAL.
 * @retval None
 */
void LED_All_Off(void);

/**
 * @brief  Ejecuta una animación de parpadeo rápido (flash) para indicar la pérdida del juego.
 * @retval None
 */
void Game_Over_Anim(void);

/**
 * @brief  Ejecuta una coreografía de luces circular y un flash final al ganar.
 * @details Envía un mensaje de felicitaciones por UART y realiza una animación de "giro"
 * con los 4 LEDs antes de reiniciar el contador de niveles.
 * @retval None
 */
void Win_Animation(void);

/**
 * @brief  Envía una cadena de caracteres a través del periférico USART3.
 * @param  mensaje: Puntero a la cadena de texto (string) a transmitir.
 * @retval None
 */
void Debug_Log(char* mensaje);


/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* --- Lógica de Control de Hardware --- */

uint8_t Simon_EsperarBoton(void) {
    while (1) {
        // Asumiendo Pulsadores con Pull-Up interna (Active Low)
        if (HAL_GPIO_ReadPin(BTN_1_GPIO_Port, BTN_1_Pin) == GPIO_PIN_RESET) return 1;	//Boton 1
        if (HAL_GPIO_ReadPin(BTN_2_GPIO_Port, BTN_2_Pin) == GPIO_PIN_RESET) return 2;	//Boton 2
        if (HAL_GPIO_ReadPin(BTN_3_GPIO_Port, BTN_3_Pin) == GPIO_PIN_RESET) return 3;	//Boton 3
        if (HAL_GPIO_ReadPin(BTN_4_GPIO_Port, BTN_4_Pin) == GPIO_PIN_RESET) return 4;	//Boton 4
        HAL_Delay(10); // Respiro para el CPU
    }
}

uint8_t Simon_CualquierBotonPresionado(void) {
    if (HAL_GPIO_ReadPin(BTN_1_GPIO_Port, BTN_1_Pin) == GPIO_PIN_RESET ||
        HAL_GPIO_ReadPin(BTN_2_GPIO_Port, BTN_2_Pin) == GPIO_PIN_RESET ||
        HAL_GPIO_ReadPin(BTN_3_GPIO_Port, BTN_3_Pin) == GPIO_PIN_RESET ||
        HAL_GPIO_ReadPin(BTN_4_GPIO_Port, BTN_4_Pin) == GPIO_PIN_RESET) {
        return 1;
    }
    return 0;
}

void Simon_ReproducirSecuencia(void) {
    for (int i = 0; i < nivel_actual; i++) {
        LED_All_Off();
        HAL_Delay(200);
        Simon_EncenderUno(secuencia[i]);
        HAL_Delay(600);
    }
    LED_All_Off();
}

void Simon_EncenderUno(uint8_t num) {
    LED_All_Off();
    switch (num) {
        case 1: HAL_GPIO_WritePin(LED_1_GPIO_Port, LED_1_Pin, 1); break;	//Led 1
        case 2: HAL_GPIO_WritePin(LED_2_GPIO_Port, LED_2_Pin, 1); break;	//Led 2
        case 3: HAL_GPIO_WritePin(LED_3_GPIO_Port, LED_3_Pin, 1); break;	//Led 3
        case 4: HAL_GPIO_WritePin(LED_4_GPIO_Port, LED_4_Pin, 1); break;	//Led 4
    }
}

void LED_All_Off(void) {
    HAL_GPIO_WritePin(LED_1_GPIO_Port, LED_1_Pin | LED_2_Pin, 0);
    HAL_GPIO_WritePin(LED_3_GPIO_Port, LED_3_Pin | LED_4_Pin, 0);
}

void Game_Over_Anim(void) {
    for(int i=0; i<6; i++) {
        HAL_GPIO_WritePin(LED_1_GPIO_Port, LED_1_Pin | LED_2_Pin, 1);
        HAL_GPIO_WritePin(LED_3_GPIO_Port, LED_3_Pin | LED_4_Pin, 1);
        HAL_Delay(150);
        LED_All_Off();
        HAL_Delay(150);
    }
}

void Win_Animation(void) {
    Debug_Log("\r\n🏆 ¡FELICIDADES! HAS COMPLETADO EL JUEGO 🏆\r\n");
    for (int i = 0; i < 10; i++) { // Repetir el giro 10 veces
        HAL_GPIO_WritePin(LED_1_GPIO_Port, LED_1_Pin, 1); // LED 1
        HAL_Delay(50);
        HAL_GPIO_WritePin(LED_1_GPIO_Port, LED_1_Pin, 0);

        HAL_GPIO_WritePin(LED_2_GPIO_Port, LED_2_Pin, 1); // LED 2
        HAL_Delay(50);
        HAL_GPIO_WritePin(LED_2_GPIO_Port, LED_2_Pin, 0);

        HAL_GPIO_WritePin(LED_3_GPIO_Port, LED_3_Pin, 1); // LED 3
        HAL_Delay(50);
        HAL_GPIO_WritePin(LED_3_GPIO_Port, LED_3_Pin, 0);

        HAL_GPIO_WritePin(LED_4_GPIO_Port, LED_4_Pin, 1); // LED 4
        HAL_Delay(50);
        HAL_GPIO_WritePin(LED_4_GPIO_Port, LED_4_Pin, 0);
    }
    // Flash final de victoria
    for(int j=0; j<3; j++){
        HAL_GPIO_WritePin(LED_1_GPIO_Port, LED_1_Pin | LED_2_Pin, 1);
        HAL_GPIO_WritePin(LED_3_GPIO_Port, LED_3_Pin | LED_4_Pin, 1);
        HAL_Delay(200);
        LED_All_Off();
        HAL_Delay(200);
    }
}

void Debug_Log(char* mensaje) {
    HAL_UART_Transmit(&huart3, (uint8_t*)mensaje, strlen(mensaje), 100);
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
  // Semilla aleatoria basada en el tiempo de inicio
      srand(HAL_GetTick());

      Debug_Log("\r\n======================================\r\n");
      Debug_Log("   SIMON DICE: VERSION HARDWARE\r\n");
      Debug_Log("======================================\r\n");
      Debug_Log("Presiona cualquier boton para empezar...\r\n");

      while(!Simon_CualquierBotonPresionado()); // Espera a que toques algo para arrancar
      HAL_Delay(500);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

	  	  	  // 1. Aumentar nivel
	          if (nivel_actual < MAX_NIVEL) {
	              secuencia[nivel_actual] = (rand() % 4) + 1;
	              nivel_actual++;
	          }

	          char buffer[50];
	          sprintf(buffer, "\r\n--- NIVEL %d ---\r\n", nivel_actual);
	          Debug_Log(buffer);

	          HAL_Delay(800);

	          // 2. Mostrar la secuencia con los LEDs
	          Simon_ReproducirSecuencia();

	          // 3. Fase de respuesta del usuario
	          uint8_t fallo = 0;
	          Debug_Log("Tu turno...");

	          for (int i = 0; i < nivel_actual; i++) {
	              // Esperar a que el usuario presione un botón
	              uint8_t botonUser = Simon_EsperarBoton();

	              // Feedback: Encender el LED que corresponde al botón apretado
	              Simon_EncenderUno(botonUser);

	              // Log del botón presionado
	              char b_log[15];
	              sprintf(b_log, " [BTN %d]", botonUser);
	              Debug_Log(b_log);

	              // IMPORTANTE: Esperar a que suelte el botón (Antirrepetidor)
	              while(Simon_CualquierBotonPresionado());
	              HAL_Delay(50); // Debounce
	              LED_All_Off();

	              // 4. Validar contra la secuencia
	              if (botonUser != secuencia[i]) {
	                  fallo = 1;
	                  break;
	              }
	          }

	          // 5. Resultado
	          if (fallo) {
	              Debug_Log("\r\nERROR: Secuencia incorrecta!\r\n");
	              Game_Over_Anim();
	              nivel_actual = 0; // Reiniciar progreso
	              HAL_Delay(2000);
	              Debug_Log("Reiniciando juego...");
	          }
	          else {
	              // Si no falló, verificamos si alcanzó la meta
	              if (nivel_actual == NIVEL_VICTORIA) {
	                  Win_Animation(); // ¡La coreografía de luces!
	                  nivel_actual = 0; // Reiniciar después de la gloria
	                  Debug_Log("\r\nReiniciando nuevo desafio...");
	                  HAL_Delay(3000);
	              }
	              else {
	                  // Solo pasó de nivel, seguimos jugando
	                  Debug_Log("\r\nCORRECTO! Siguiente nivel.");
	                  HAL_Delay(800);
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
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, LD1_Pin|LED_3_Pin|LED_4_Pin|LD3_Pin
                          |LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOE, LED_1_Pin|LED_2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(USB_PowerSwitchOn_GPIO_Port, USB_PowerSwitchOn_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : USER_Btn_Pin */
  GPIO_InitStruct.Pin = USER_Btn_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(USER_Btn_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : BTN_3_Pin BTN_4_Pin */
  GPIO_InitStruct.Pin = BTN_3_Pin|BTN_4_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : LD1_Pin LED_3_Pin LED_4_Pin LD3_Pin
                           LD2_Pin */
  GPIO_InitStruct.Pin = LD1_Pin|LED_3_Pin|LED_4_Pin|LD3_Pin
                          |LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : LED_1_Pin LED_2_Pin */
  GPIO_InitStruct.Pin = LED_1_Pin|LED_2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

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

  /*Configure GPIO pins : BTN_1_Pin BTN_2_Pin */
  GPIO_InitStruct.Pin = BTN_1_Pin|BTN_2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

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
