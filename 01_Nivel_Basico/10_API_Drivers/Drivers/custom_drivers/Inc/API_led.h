/**
 * @file API_led.h
 * @author CarlitozMF
 * @brief Driver de abstracción para el manejo genérico de LEDs.
 * @version 1.0
 * @date 2026-01-21
 * * @details Este driver permite controlar LEDs de forma independiente de la placa,
 * facilitando la portabilidad del código al pasar la configuración por referencia.
 */

#ifndef CUSTOM_DRIVERS_INC_API_LED_H_
#define CUSTOM_DRIVERS_INC_API_LED_H_

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include <stdbool.h>

/* Exported types ------------------------------------------------------------*/

/**
 * @brief Estructura que define un objeto LED mediante sus parámetros de hardware.
 */
typedef struct {
    GPIO_TypeDef* port; /**< Puntero al puerto GPIO (Ej: GPIOF). */
    uint16_t pin;       /**< Número del pin GPIO (Ej: GPIO_PIN_13). */
    bool inverted;      /**< Define si el LED es de lógica invertida (active-low). */
} led_t;

/* Exported functions prototypes ---------------------------------------------*/

/**
 * @brief Enciende el LED referenciado.
 * @param led Puntero a la estructura led_t.
 * @return void
 */
void LED_On(led_t * led);

/**
 * @brief Apaga el LED referenciado.
 * @param led Puntero a la estructura led_t.
 * @return void
 */
void LED_Off(led_t * led);

/**
 * @brief Conmuta el estado físico del LED.
 * @param led Puntero a la estructura led_t.
 * @return void
 */
void LED_Toggle(led_t * led);

#endif /* CUSTOM_DRIVERS_INC_API_LED_H_ */
