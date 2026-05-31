/**
 * @file tone_generator.h
 * @author Mamani Flores Carlos (UTN FRT)
 * @brief Driver de Capa 2 para la generación de frecuencias precisas por Output Compare.
 * @details Utiliza la inyección de dependencias de la PAL Universal para operar de forma
 * asíncrona mediante un acumulador de fase, desacoplándose del silicio nativo.
 * @version 1.0
 * @date 2026
 */

#ifndef TONE_GENERATOR_H_
#define TONE_GENERATOR_H_

#include "hal_interface.h"
#include <stddef.h>

/**
 * @struct tone_gen_t
 * @brief Estructura de control para la instancia del generador de tonos.
 */
typedef struct {
    generic_pwm_t   oc_chan;        /**< Descriptor genérico del canal de Output Compare */
    generic_gpio_t  pin_out;        /**< Descriptor genérico del pin físico del transductor */
    hal_interface_t pal;            /**< Tabla de despacho con servicios de plataforma */
    uint32_t        timer_clk_freq; /**< Frecuencia de conteo real de la entrada del Timer (f_clk/prescaler) */
    uint32_t        current_ticks;  /**< Ticks calculados para el semiperíodo de la nota activa */
    uint8_t         is_active;      /**< Bandera de estado: 1 = Reproduciendo, 0 = Silencio */
} tone_gen_t;

/**
 * @brief Inicializa el objeto generador de tonos inyectando sus interfaces de hardware.
 */
void TONE_GENERATOR_Init(tone_gen_t *tg, generic_pwm_t oc_ch, generic_gpio_t gpio_ch, hal_interface_t pal_io, uint32_t timer_freq);

/**
 * @brief Modifica la frecuencia de salida en Hz. Si la frecuencia es 0, detiene la señal.
 */
void TONE_GENERATOR_SetFrequency(tone_gen_t *tg, uint32_t freq);

/**
 * @brief Detiene de forma inmediata la oscilación del pin.
 */
void TONE_GENERATOR_Stop(tone_gen_t *tg);

/**
 * @brief Manejador asíncrono de interrupción por Output Compare.
 */
void TONE_GENERATOR_IRQ_Handler(tone_gen_t *tg);

#endif /* TONE_GENERATOR_H_ */
