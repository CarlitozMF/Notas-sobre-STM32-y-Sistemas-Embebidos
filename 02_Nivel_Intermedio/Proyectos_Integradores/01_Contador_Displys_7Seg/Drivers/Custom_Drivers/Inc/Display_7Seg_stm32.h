/**
 * @file Display_7Seg_stm32.h
 * @author CarlitozMF (UTN FRT)
 * @brief Driver avanzado para displays de 7 segmentos con multiplexación por Timer.
 * @details Soporta N dígitos, control de brillo, flashing y mensajes alfanuméricos.
 * @version 2.0
 * @date 2026
 */

#ifndef DISPLAY_7SEG_STM32_H
#define DISPLAY_7SEG_STM32_H

#include "stm32f4xx_hal.h"
#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Estructura para mapeo de pines GPIO.
 */
typedef struct {
    GPIO_TypeDef* port;  /**< Puerto GPIO (Ej: GPIOA) */
    uint16_t pin;       /**< Pin GPIO (Ej: GPIO_PIN_5) */
} display_pio_t;

/**
 * @brief Handle de control para el display.
 */
typedef struct {
    TIM_HandleTypeDef* htim;      /**< Timer para la multiplexación */
    display_pio_t* segments;      /**< Arreglo de 8 pines (A-G + DP) */
    display_pio_t* digits;        /**< Arreglo de N pines comunes */
    uint8_t* buffer;              /**< Buffer de patrones de bits */
    uint8_t digitsCount;          /**< Cantidad de dígitos físicos */
    uint8_t currentDigit;         /**< Índice del dígito activo */
    uint8_t brightness;           /**< Brillo 0-100 (PWM software) */
    uint8_t decimalPointPos;      /**< Posición del punto (0=off) */
    bool showLeadingZeros;        /**< Relleno con ceros a la izquierda */
    bool isEnabled;               /**< Control de encendido general */
    bool isFlashing;              /**< Estado de parpadeo activo */
    uint32_t flashInterval;       /**< Intervalo de parpadeo en ms */
    uint32_t lastFlashTick;       /**< Auxiliar de tiempo para flash */
    bool flashState;              /**< Estado interno del toggle de flash */
} display_7seg_t;

/* --- Funciones de Inicialización --- */

/**
 * @brief Inicializa el driver y arranca el Timer asociado.
 * @param hdisplay Puntero al handle.
 * @param count Cantidad de dígitos.
 * @return true si se inició correctamente.
 */
bool Display7Seg_Init(display_7seg_t* hdisplay, TIM_HandleTypeDef* htim,
                      display_pio_t* segments, display_pio_t* digits,
                      uint8_t count, uint8_t* buffer);

/* --- Funciones de Escritura --- */

/**
 * @brief Escribe un número entero en el display.
 */
void Display7Seg_WriteNumber(display_7seg_t* hdisplay, uint32_t number);

/**
 * @brief Escribe una cadena de caracteres (H, E, L, P, O, A, C, U, S, t, r, b, -, ' ').
 */
void Display7Seg_WriteString(display_7seg_t* hdisplay, const char* str);

/**
 * @brief Muestra un mensaje de error "Err" seguido de un código.
 */
void Display7Seg_WriteError(display_7seg_t* hdisplay, uint8_t errorCode);

/**
 * @brief Limpia el display (lo pone en blanco).
 */
void Display7Seg_Clear(display_7seg_t* hdisplay);

/* --- Funciones de Configuración Visual --- */

/**
 * @brief Configura el brillo del display.
 * @param level 0 a 100 por ciento.
 */
void Display7Seg_SetBrightness(display_7seg_t* hdisplay, uint8_t level);

/**
 * @brief Configura el parpadeo del display.
 * @param interval_ms Tiempo en ms. 0 para desactivar.
 */
void Display7Seg_SetFlash(display_7seg_t* hdisplay, uint16_t interval_ms);

/**
 * @brief Configura el punto decimal (1 es el dígito más a la derecha).
 */
void Display7Seg_SetDecimalPoint(display_7seg_t* hdisplay, uint8_t pos);

/* --- Función de Sistema --- */

/**
 * @brief Función de refresco. Llamar en HAL_TIM_PeriodElapsedCallback.
 */
void Display7Seg_Refresh_ISR(display_7seg_t* hdisplay);

#endif
