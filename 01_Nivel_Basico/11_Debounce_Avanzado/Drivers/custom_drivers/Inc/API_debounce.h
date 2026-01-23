/**
 * @file API_debounce.h
 * @author CarlitozMF (UTN FRT)
 * @brief Driver de antirrebote (debounce) basado en una FSM para STM32.
 * @version 1.0
 * @date 2026-01-22
 */

#ifndef API_DEBOUNCE_H_
#define API_DEBOUNCE_H_

#include "stm32f4xx_hal.h"
#include <stdbool.h>
#include "API_delay.h"

/**
 * @brief Estructura que define un botón con su hardware y estado.
 * * Permite instanciar múltiples botones de forma independiente.
 */
typedef struct {
    GPIO_TypeDef* port;  /**< Puerto GPIO del botón (ej: GPIOB) */
    uint16_t pin;        /**< Pin GPIO del botón (ej: GPIO_PIN_11) */
    bool inverted;       /**< true: Lógica negativa (Active Low), false: Lógica positiva */
    bool keyPressed;     /**< Flag que indica que ocurrió una pulsación válida */
} button_t;

/**
 * @brief Inicializa la Máquina de Estados Finita (FSM) del antirrebote.
 * * Configura el tiempo de filtrado (debounce time) y el estado inicial.
 */
void debounceFSM_Init(void);

/**
 * @brief Actualiza la FSM del botón.
 * * Debe llamarse periódicamente en el bucle principal. Gestiona las
 * transiciones de estado y la validación temporal.
 * * @param btn Puntero a la estructura del botón a procesar.
 */
void debounceFSM_Update(button_t* btn);

/**
 * @brief Lee y limpia el flag de pulsación detectada.
 * * Esta función implementa una lógica de limpieza automática (clear on read),
 * asegurando que el evento se procese una sola vez.
 * * @param btn Puntero a la estructura del botón.
 * @return true Si se detectó una pulsación válida.
 * @return false Si no hay eventos nuevos.
 */
bool readKey(button_t* btn);

#endif /* API_DEBOUNCE_H_ */
