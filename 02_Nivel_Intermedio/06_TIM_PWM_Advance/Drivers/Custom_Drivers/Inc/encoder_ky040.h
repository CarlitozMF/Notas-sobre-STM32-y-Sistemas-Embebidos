/**
 * @file encoder_ky040.h
 * @author Mamani Flores Carlos (UTN FRT)
 * @brief Driver genérico por software para encoder rotativo KY040 usando PAL.
 * @details Utiliza interrupciones externas (EXTI) mapeadas por la plataforma
 *          y filtrado por tiempo para mitigar el rebote mecánico (debouncing).
 * @version 2.0
 * @date 2026
 */

#ifndef ENCODER_KY040_H
#define ENCODER_KY040_H

#include "hal_interface.h"
#include <stddef.h>

/**
 * @struct KY040_t
 * @brief Estructura de control agnóstica para el encoder rotativo KY-040.
 */
typedef struct {
    /* --- Configuración de Hardware Genérico (Pines) --- */
    generic_gpio_t pin_A;          /**< Descriptor genérico del pin CLK */
    generic_gpio_t pin_B;          /**< Descriptor genérico del pin DT */
    generic_gpio_t pin_SW;         /**< Descriptor genérico del pin SW (Pulsador) */

    hal_interface_t pal;           /**< Súper-objeto PAL con servicios de la plataforma */

    /* --- Estados de Control y Posición --- */
    volatile int32_t position;     /**< Posición actual del contador */
    int32_t min_val;               /**< Límite inferior de conteo */
    int32_t max_val;               /**< Límite superior de conteo */
    uint8_t lastStateA;            /**< Memoria de estado lógico previo del canal CLK */
    volatile uint8_t sw_pressed;   /**< Bandera de pulsador oprimido */

    /* --- Temporización para Anti-rebote (Debounce) --- */
    uint32_t last_tick;            /**< Marca de tiempo para filtrar el giro de cuadratura */
    uint32_t last_tick_SW;         /**< Marca de tiempo para filtrar el rebote del botón */
} KY040_t;

/* --- Funciones Públicas --- */

/**
 * @brief Inicializa la estructura del encoder acoplando los canales físicos mediante descriptores.
 */
void KY040_Init(KY040_t *dev, hal_interface_t pal_io,
                generic_gpio_t pinA, generic_gpio_t pinB, generic_gpio_t pinSW,
                int32_t min, int32_t max);

/**
 * @brief Manejador unificado de interrupciones para el Encoder de forma agnóstica.
 * @param dev Puntero a la instancia del encoder.
 * @param triggered_gpio Descriptor del pin exacto que generó el flanco de interrupción.
 */
void KY040_IRQ_Handler(KY040_t *dev, generic_gpio_t triggered_gpio);

#endif /* ENCODER_KY040_H */
