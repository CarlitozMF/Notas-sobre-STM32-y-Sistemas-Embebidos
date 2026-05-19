/**
 * @file display_7seg_platform.h
 * @author Mamani Flores Carlos (UTN FRT)
 * @brief Interfaz de Abstracción de Plataforma (PAL) para periféricos de salida.
 * @details Este archivo define los tipos de datos y contratos necesarios para 
 *          independizar la lógica del driver del hardware específico (STM32, AVR, PIC).
 * @version 3.0
 * @date 2026
 */

#ifndef DISPLAY_7SEG_PLATFORM_H
#define DISPLAY_7SEG_PLATFORM_H

#include <stdint.h>
#include <stdbool.h>

/**
 * @struct display_gpio_t
 * @brief Estructura genérica para la definición de un pin de E/S.
 * 
 * @details Utiliza un puntero opaco (void*) para almacenar la dirección del puerto,
 *          lo que permite compatibilidad con registros de 8, 16 o 32 bits según el MCU.
 */
typedef struct {
    void* port;      /**< Puntero al puerto de hardware (Ej: GPIOA, &PORTB, &LATA) */
    uint16_t pin;    /**< Máscara o número del pin (Ej: GPIO_PIN_5, (1<<2)) */
} display_gpio_t;

/**
 * @struct display_7seg_pal_t
 * @brief Contrato de funciones de abstracción para el hardware.
 * 
 * @details Esta estructura actúa como una vtable (tabla de funciones virtuales).
 *          El driver invocará estas funciones sin conocer su implementación interna,
 *          logrando un desacoplamiento total de la HAL del fabricante.
 */
typedef struct {
    /** 
     * @brief Puntero a función para escribir en un pin físico.
     * @param pin Estructura con el puerto y pin a modificar.
     * @param state Estado lógico deseado (true = SET, false = RESET).
     */
    void (*write_pin)(display_gpio_t pin, bool state);
    
    /** 
     * @brief Puntero a función para obtener el tiempo de sistema.
     * @return uint32_t Tiempo transcurrido en milisegundos (Uptime).
     */
    uint32_t (*get_tick)(void);
    
    /** 
     * @brief Puntero a función para leer el estado actual de un pin.
     * @param pin Estructura con el puerto y pin a consultar.
     * @return true si el pin está en nivel alto, false en caso contrario.
     */
    bool (*read_pin)(display_gpio_t pin);
} display_7seg_pal_t;

#endif /* DISPLAY_7SEG_PLATFORM_H */
