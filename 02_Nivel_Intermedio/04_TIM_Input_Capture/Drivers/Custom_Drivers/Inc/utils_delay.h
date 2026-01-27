/**
 * @file utils_delay.h
 * @author CarlitozMF (UTN FRT)
 * @brief Servicio de temporización de alta precisión para STM32F4.
 * @details Utiliza el registro DWT (Data Watchpoint and Trace) del núcleo
 * Cortex-M4 para generar retardos determinísticos.
 * @version 1.0
 * @date 2026
 */

#ifndef UTILS_DELAY_H
#define UTILS_DELAY_H

#include "stm32f4xx_hal.h"

/**
 * @brief Inicializa el contador de ciclos del núcleo (DWT).
 * @note Debe llamarse una sola vez al inicio del programa, después de HAL_Init().
 */
void Utils_Delay_Init(void);

/**
 * @brief Genera un retardo bloqueante en microsegundos.
 * @param us Cantidad de microsegundos a esperar.
 * @warning Esta función es bloqueante y utiliza ciclos de CPU. No usar para
 * retardos largos (>10ms) si se requiere multitarea estricta.
 */
void delay_us(uint32_t us);

#endif /* UTILS_DELAY_H */
