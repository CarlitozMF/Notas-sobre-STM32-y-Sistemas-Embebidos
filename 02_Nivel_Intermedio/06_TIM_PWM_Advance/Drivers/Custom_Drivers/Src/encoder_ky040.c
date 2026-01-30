/**
 * @file encoder_ky040.c
 * @author CarlitozMF (UTN FRT)
 * @brief Implementación del driver para encoder rotativo KY-040.
 */

#include "encoder_ky040.h"

/**
 * @brief Inicializa la estructura de control del encoder y configura los valores iniciales.
 * * Esta función vincula los periféricos GPIO físicos con la estructura de software y
 * establece el estado inicial de las variables de posición y filtros de tiempo.
 * * @param dev      Puntero a la instancia del encoder (KY040_t).
 * @param pA       Puerto GPIO asignado al canal CLK.
 * @param pinA     Pin GPIO asignado al canal CLK.
 * @param pB       Puerto GPIO asignado al canal DT.
 * @param pinB     Pin GPIO asignado al canal DT.
 * @param pSW      Puerto GPIO asignado al pulsador SW.
 * @param pinSW    Pin GPIO asignado al pulsador SW.
 * @param min      Valor mínimo del rango de posición.
 * @param max      Valor máximo del rango de posición.
 */
void KY040_Init(KY040_t *dev,
                GPIO_TypeDef* pA, uint16_t pinA,
                GPIO_TypeDef* pB, uint16_t pinB,
                GPIO_TypeDef* pSW, uint16_t pinSW,
                int32_t min, int32_t max) {

    /* Vinculación de Hardware: Asignación de puertos y pines */
    dev->port_A = pA;
    dev->pin_A = pinA;
    dev->port_B = pB;
    dev->pin_B = pinB;
    dev->port_SW = pSW;
    dev->pin_SW = pinSW;

    /* Configuración de Rango: Límites operativos del contador */
    dev->min_val = min;
    dev->max_val = max;

    /* Posición Inicial: Por defecto en la mitad del rango (ej. 90 para el servo) */
    dev->position = (max + min) / 2;

    /* Inicialización de Tiempos y Estados */
    dev->last_tick = 0;
    dev->last_tick_SW = 0;
    dev->sw_pressed = 0;

    /* Captura de Estado Inicial del Canal A para lógica de flancos */
    dev->lastStateA = (HAL_GPIO_ReadPin(pA, pinA) == GPIO_PIN_SET) ? 1 : 0;
}

/**
 * @brief Manejador unificado de interrupciones para el KY-040.
 * @details Gestiona tanto el giro (CLK) como el pulsador (SW) aplicando
 * debouncing independiente por software.
 */
void KY040_IRQ_Handler(KY040_t *dev, uint16_t triggered_pin) {
    uint32_t ahora = HAL_GetTick();

    /* --- PROCESAMIENTO DE GIRO (Pin CLK) --- */
    if (triggered_pin == dev->pin_A) {

        /* Debounce rápido para señales de cuadratura (5ms) */
        if (ahora - dev->last_tick < 5) return;

        GPIO_PinState estadoA = HAL_GPIO_ReadPin(dev->port_A, dev->pin_A);
        GPIO_PinState estadoB = HAL_GPIO_ReadPin(dev->port_B, dev->pin_B);

        /* Detectamos flanco de bajada en el Canal A */
        if (dev->lastStateA == 1 && estadoA == GPIO_PIN_RESET) {

            /* Lógica de dirección: si A != B en el flanco de bajada, sentido horario */
            if (estadoB != estadoA) {
                dev->position++;
            } else {
                dev->position--;
            }

            /* Aplicación de límites (Saturación) */
            if (dev->position > dev->max_val) dev->position = dev->max_val;
            if (dev->position < dev->min_val) dev->position = dev->min_val;

            dev->last_tick = ahora;
        }
        dev->lastStateA = (estadoA == GPIO_PIN_SET) ? 1 : 0;
    }

    /* --- PROCESAMIENTO DE PULSADOR (Pin SW) --- */
    else if (triggered_pin == dev->pin_SW) {

        /* Debounce largo para pulsador mecánico (200ms) */
        if (ahora - dev->last_tick_SW > 200) {

            /* Confirmación de estado: el botón cierra a GND (Active Low) */
            if (HAL_GPIO_ReadPin(dev->port_SW, dev->pin_SW) == GPIO_PIN_RESET) {
                dev->sw_pressed = 1;
            }
            dev->last_tick_SW = ahora;
        }
    }
}
