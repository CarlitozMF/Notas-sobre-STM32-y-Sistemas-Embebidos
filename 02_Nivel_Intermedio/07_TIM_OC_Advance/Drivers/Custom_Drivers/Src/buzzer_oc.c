/**
 * @file buzzer_oc.c
 * @brief Implementación del driver de audio por Output Compare.
 */

#include "buzzer_oc.h"

void Buzzer_Init(Buzzer_t *buzzer, TIM_HandleTypeDef *htim, uint32_t channel, uint32_t bus_freq) {
    if (buzzer == NULL || htim == NULL) return;

    buzzer->htim = htim;
    buzzer->channel = channel;
    buzzer->timer_clk_freq = bus_freq;
    buzzer->is_active = 0;
    buzzer->current_ticks = 0;
}

void Buzzer_SetFrequency(Buzzer_t *buzzer, uint32_t freq) {
    if (buzzer == NULL) return;

    if (freq == 0) {
        Buzzer_Stop(buzzer);
        return;
    }

    uint32_t timer_base_freq = buzzer->timer_clk_freq / (buzzer->htim->Init.Prescaler + 1);
    buzzer->current_ticks = timer_base_freq / (2 * freq);

    if (!buzzer->is_active) {
        buzzer->is_active = 1;

        /* --- AGREGAR ESTO: Sincronización inicial --- */
        // Leemos el valor actual del contador y le sumamos el primer delay
        uint32_t now = __HAL_TIM_GET_COUNTER(buzzer->htim);
        __HAL_TIM_SET_COMPARE(buzzer->htim, buzzer->channel, now + buzzer->current_ticks);
        /* -------------------------------------------- */

        HAL_TIM_OC_Start_IT(buzzer->htim, buzzer->channel);
    }
}

void Buzzer_Stop(Buzzer_t *buzzer) {
    if (buzzer == NULL) return;

    buzzer->is_active = 0;
    HAL_TIM_OC_Stop_IT(buzzer->htim, buzzer->channel);
}

/**
 * @brief Manejador de la interrupción OC.
 * @details Modificado para conmutar el pin GPIO manualmente.
 */
void Buzzer_IRQ_Handler(Buzzer_t *buzzer) {
    if (buzzer == NULL || !buzzer->is_active) return;

    /* 1. Conmutar el pin físico (Toggle) */
    HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_0);

    /* 2. Programar el próximo evento (Acumulador de fase) */
    uint32_t pulse = HAL_TIM_ReadCapturedValue(buzzer->htim, buzzer->channel);
    __HAL_TIM_SET_COMPARE(buzzer->htim, buzzer->channel, pulse + buzzer->current_ticks);
}
