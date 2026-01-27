#ifndef HCSR04_DRIVER_H
#define HCSR04_DRIVER_H

#include "stm32f4xx_hal.h" // Incluye la librería HAL de STM32

/*
 * Estructura para almacenar el estado del driver.
 */
typedef struct {
    TIM_HandleTypeDef* htim; // Puntero al handle del timer
    uint32_t channel;         // Canal del timer
    GPIO_TypeDef* trig_port;  // Puerto GPIO del pin Trigger
    uint16_t trig_pin;        // Pin GPIO del pin Trigger
    uint32_t edge_state;      // Estado de la captura (0: Esperando flanco de subida, 1: Esperando flanco de bajada, 2: Medición lista)
    uint32_t rising_time;     // Valor del contador en el flanco de subida
    uint32_t falling_time;    // Valor del contador en el flanco de bajada
    uint32_t pulse_duration;  // Duración del pulso en ticks del timer
} HCSR04_Handle;

/*
 * Declaración de las funciones del driver.
 */
void HCSR04_Init(HCSR04_Handle* hcsr, TIM_HandleTypeDef* htim, uint32_t channel, GPIO_TypeDef* trig_port, uint16_t trig_pin);
void HCSR04_Trigger(HCSR04_Handle* hcsr);
float HCSR04_ReadDistance_cm(HCSR04_Handle* hcsr);

// La función que se llama desde HAL_TIM_IC_CaptureCallback()
void HCSR04_InputCapture_Callback(HCSR04_Handle* hcsr);

#endif // HCSR04_DRIVER_H
