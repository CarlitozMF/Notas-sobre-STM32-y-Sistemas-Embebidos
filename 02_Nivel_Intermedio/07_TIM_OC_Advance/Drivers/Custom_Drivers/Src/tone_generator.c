/**
 * @file tone_generator.c
 * @author Mamani Flores Carlos (UTN FRT)
 * @brief Implementación del generador de tonos agnóstico mediante Output Compare.
 * @details Contiene la lógica de cálculo elástico de fase y sincronización inicial
 * de hardware para la síntesis de frecuencias de audio sin bloqueos de CPU,
 * utilizando inyección de dependencias a través de la PAL Universal.
 * @version 1.0
 * @date 2026
 */

#include "tone_generator.h"
#include <stddef.h>

/**
 * @brief Inicializa la estructura de control del generador de tonos.
 * @param tg Puntero al descriptor del generador de tonos (objeto).
 * @param oc_ch Descriptor genérico del canal Output Compare del Timer.
 * @param gpio_ch Descriptor genérico del pin físico asignado.
 * @param pal_io Tabla de despacho con los servicios inyectados de la PAL.
 * @param timer_freq Frecuencia de conteo física del Timer maestro (en Hz).
 */
void TONE_GENERATOR_Init(tone_gen_t *tg, generic_pwm_t oc_ch, generic_gpio_t gpio_ch, hal_interface_t pal_io, uint32_t timer_freq) {
    if (tg == NULL) return;

    tg->oc_chan = oc_ch;
    tg->pin_out = gpio_ch;
    tg->pal = pal_io;
    tg->timer_clk_freq = timer_freq;
    tg->is_active = 0;
    tg->current_ticks = 0;
}

/**
 * @brief Configura la frecuencia de salida del tono dinámicamente.
 * @details Si la frecuencia es mayor a cero, calcula el delta de ticks necesario para el semiperiodo.
 * Si el oscilador estaba apagado, realiza una captura del contador físico (CNT) para forzar
 * un enganche de fase inmediato y evitar latencias de arranque. Si ya estaba corriendo,
 * muta el valor en memoria de forma atómica para que la ISR aplique el cambio de forma síncrona.
 * @param tg Puntero al descriptor del generador de tonos.
 * @param freq Frecuencia objetivo expresada en Hz (0 para detener).
 */
void TONE_GENERATOR_SetFrequency(tone_gen_t *tg, uint32_t freq) {
    if (tg == NULL) return;

    if (freq == 0) {
        TONE_GENERATOR_Stop(tg);
        return;
    }

    /* Cálculo semiperiódico exacto: ticks = F_timer / (2 * F_salida) */
    uint32_t nuevos_ticks = tg->timer_clk_freq / (2 * freq);

    if (!tg->is_active) {
        tg->current_ticks = nuevos_ticks;
        tg->is_active = 1;

        /* Sincronización inicial atómica usando el CNT físico para evitar latencias de arranque */
        if (tg->pal.get_timer_cnt != NULL && tg->pal.oc_write != NULL) {
            uint32_t current_cnt = tg->pal.get_timer_cnt(tg->oc_chan);
            tg->pal.oc_write(tg->oc_chan, current_cnt + tg->current_ticks);
        }
    } else {
        /* Blindaje de registros en caliente: En ejecución (while(1)) solo mutamos la variable en memoria.
         * La ISR aplicará el cambio en el siguiente match de forma síncrona, eliminando ruidos de fondo (clipping). */
        tg->current_ticks = nuevos_ticks;
    }
}

/**
 * @brief Detiene la síntesis de audio de forma inmediata.
 * @param tg Puntero al descriptor del generador de tonos.
 */
void TONE_GENERATOR_Stop(tone_gen_t *tg) {
    if (tg == NULL) return;
    tg->is_active = 0;
}

/**
 * @brief Rutina de Servicio de Interrupción (ISR) de la Capa 2 para Output Compare.
 * @details Esta función debe ser invocada directamente desde el callback de coincidencia del hardware
 * (Capa 1). Implementa el acumulador elástico puro por registros leyendo el instante del match
 * previo (CCR) y reprogramando el canal hacia el futuro absoluto síncrono.
 * @param tg Puntero al descriptor del generador de tonos que ejecutó la interrupción.
 */
void TONE_GENERATOR_IRQ_Handler(tone_gen_t *tg) {
    if (tg == NULL || !tg->is_active) return;

    /* Reprogramación elástica pura por registros para garantizar una síntesis libre de jitter */
    if (tg->pal.oc_read != NULL && tg->pal.oc_write != NULL) {
        uint32_t pulse = tg->pal.oc_read(tg->oc_chan);
        tg->pal.oc_write(tg->oc_chan, pulse + tg->current_ticks);
    }
}
