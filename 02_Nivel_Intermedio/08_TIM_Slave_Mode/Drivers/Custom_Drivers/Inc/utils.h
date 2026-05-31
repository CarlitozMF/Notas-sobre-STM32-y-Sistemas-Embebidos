/**
 * @file utils.h
 * @author Mamani Flores Carlos (UTN FRT)
 * @brief Driver de utilidades y temporización de alta resolución para Cortex-M4.
 * @details Provee funciones de inicialización del módulo DWT y retrasos
 * precisos a nivel de microsegundos exclusivos para la plataforma.
 * @version 1.0
 * @date 2026
 */

#ifndef INC_UTILS_H_
#define INC_UTILS_H_

#include "main.h" // Incluye las definiciones del núcleo y CMSIS (CoreDebug, DWT)
#include <stdint.h>

/* --- API Pública de Utilidades --- */

/**
 * @brief Inicializa y habilita el contador de ciclos del bloque DWT.
 * @note Debe llamarse al inicio del main antes de usar retrasos en microsegundos.
 */
void UTILS_DWT_Init(void);

/**
 * @brief Obtiene el valor instantáneo de tiempo del sistema en microsegundos.
 * @return Cantidad de microsegundos transcurridos desde el reset.
 */
uint32_t UTILS_GetUs(void);

/**
 * @brief Genera un retardo bloqueante preciso en microsegundos.
 * @param us Cantidad de microsegundos a esperar.
 */
void UTILS_DelayUs(uint32_t us);

#endif /* INC_UTILS_H_ */
