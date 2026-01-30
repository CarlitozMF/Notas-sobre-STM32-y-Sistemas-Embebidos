/**
 * @file encoder_ky040.h
 *  * @author CarlitozMF (UTN FRT)
 * @brief Driver genérico por software para encoder rotativo KY040.
 * @details Utiliza interrupciones externas (EXTI) y filtrado por tiempo para
 * manejar el rebote mecánico (debouncing) de encoders económicos.
 * @version 1.0
 * @date 2026
 */

#ifndef ENCODER_KY040_H
#define ENCODER_KY040_H

#include "main.h" // Mejor práctica para proyectos CubeIDE

/**
 * @struct KY040_t
 * @brief Estructura de control para el encoder rotativo KY-040.
 */
typedef struct {
    /* --- Configuración de Hardware (Pines) --- */
    GPIO_TypeDef* port_A;
    uint16_t pin_A;
    GPIO_TypeDef* port_B;
    uint16_t pin_B;
    GPIO_TypeDef* port_SW;
    uint16_t pin_SW;

    /* --- Estados de Control y Posición --- */
    volatile int32_t position;  /*!< Posición actual (volatile para uso en main) */
    int32_t min_val;
    int32_t max_val;
    uint8_t lastStateA;
    volatile uint8_t sw_pressed; /*!< Flag de pulsador (volatile) */

    /* --- Temporización para Anti-rebote (Debounce) --- */
    uint32_t last_tick;         /*!< Tiempo para filtrar el giro */
    uint32_t last_tick_SW;      /*!< Tiempo para filtrar el botón */

} KY040_t;

/* --- Funciones Públicas --- */

void KY040_Init(KY040_t *dev, GPIO_TypeDef* pA, uint16_t pinA,
                GPIO_TypeDef* pB, uint16_t pinB, GPIO_TypeDef* pSW, uint16_t pinSW,
                int32_t min, int32_t max);

/**
 * @brief Manejador unificado de interrupciones para el Encoder.
 * @details Procesa tanto el giro como el botón dependiendo del pin que disparó.
 */
void KY040_IRQ_Handler(KY040_t *dev, uint16_t triggered_pin);

#endif /* ENCODER_KY040_H */
