/**
 * @file hcsr04_driver.c
 * @brief Implementación del driver no bloqueante para N sensores HC-SR04.
 */

#include "hcsr04_driver.h"
#include "utils_delay.h"
#include <string.h>
#include <math.h>

#define SOUND_SPEED_CMS 34300.0f

// NOTA: La variable s_current_hcsr y la función HAL_TIM_IC_CaptureCallback
// se han eliminado de este archivo para permitir que la aplicación (main.c)
// gestione la interrupción y el enrutamiento de N sensores.

// ===============================================
// 1. INICIALIZACIÓN Y CONFIGURACIÓN
// ===============================================

/**
 * @brief Inicializa el handle del sensor ultrasónico.
 * @details El driver ya no almacena un handle global estático.
 */
void HCSR04_Init(HCSR04_Handle* hcsr, TIM_HandleTypeDef* htim, uint32_t channel, GPIO_TypeDef* trig_port, uint16_t trig_pin) {
    // Almacena la configuración en la estructura
    hcsr->htim = htim;
    hcsr->channel = channel;
    hcsr->trig_port = trig_port;
    hcsr->trig_pin = trig_pin;

    // Inicializar estados a 0 (Esperando flanco de subida)
    hcsr->edge_state = 0;
    hcsr->rising_time = 0;
    hcsr->falling_time = 0;
    hcsr->pulse_duration = 0;

    // Configurar el pin Trigger en estado bajo (LOW)
    HAL_GPIO_WritePin(hcsr->trig_port, hcsr->trig_pin, GPIO_PIN_RESET);
}


// ===============================================
// 2. DISPARO (TRIGGER)
// ===============================================

/**
 * @brief Genera el pulso de 10us en el pin Trigger para iniciar la medición.
 */
void HCSR04_Trigger(HCSR04_Handle* hcsr) {
    hcsr->edge_state = 0;

    // 1. Mapeo dinámico del flag según el canal
    uint32_t it_flag = (hcsr->channel == TIM_CHANNEL_1) ? TIM_IT_CC1 :
                       (hcsr->channel == TIM_CHANNEL_2) ? TIM_IT_CC2 :
                       (hcsr->channel == TIM_CHANNEL_3) ? TIM_IT_CC3 : TIM_IT_CC4;

    // 2. Limpiar interrupción pendiente y forzar polaridad inicial de subida
    __HAL_TIM_CLEAR_IT(hcsr->htim, it_flag);
    __HAL_TIM_SET_CAPTUREPOLARITY(hcsr->htim, hcsr->channel, TIM_INPUTCHANNELPOLARITY_RISING);

    // 3. Resetear contador y arrancar antes del pulso físico
    __HAL_TIM_SET_COUNTER(hcsr->htim, 0);
    HAL_TIM_IC_Start_IT(hcsr->htim, hcsr->channel);

    // 4. Generar pulso Trigger (DWT garantiza exactitud)
    HAL_GPIO_WritePin(hcsr->trig_port, hcsr->trig_pin, GPIO_PIN_SET);
    delay_us(10);
    HAL_GPIO_WritePin(hcsr->trig_port, hcsr->trig_pin, GPIO_PIN_RESET);
}

// ===============================================
// 3. LÓGICA DE CAPTURA (Llamada desde main.c)
// ===============================================

/**
 * @brief Lógica de medición del pulso Echo. Llamada desde el callback de HAL.
 */
void HCSR04_InputCapture_Callback(HCSR04_Handle* hcsr) {

    if (hcsr->edge_state == 0) {
        // Estado 0: Capturar flanco de subida (Inicio del pulso)
        hcsr->rising_time = HAL_TIM_ReadCapturedValue(hcsr->htim, hcsr->channel);

        // Cambiar la polaridad para capturar el flanco de bajada
        __HAL_TIM_SET_CAPTUREPOLARITY(hcsr->htim, hcsr->channel, TIM_INPUTCHANNELPOLARITY_FALLING);
        hcsr->edge_state = 1;
    }
    else if (hcsr->edge_state == 1) {
        // Estado 1: Capturar flanco de bajada (Fin del pulso)
        hcsr->falling_time = HAL_TIM_ReadCapturedValue(hcsr->htim, hcsr->channel);

        // 1. Calcular la duración del pulso (Manejo de Overflow)
        if (hcsr->falling_time > hcsr->rising_time) {
            hcsr->pulse_duration = hcsr->falling_time - hcsr->rising_time;
        } else {
            // Manejar overflow usando el ARR del Timer
            uint32_t arr_value = __HAL_TIM_GET_AUTORELOAD(hcsr->htim);
            hcsr->pulse_duration = (arr_value - hcsr->rising_time) + hcsr->falling_time;
        }

        // 2. Detener la captura y marcar como lista la medición
        HAL_TIM_IC_Stop_IT(hcsr->htim, hcsr->channel);

        // 3. Restablecer la polaridad para el siguiente ciclo
        __HAL_TIM_SET_CAPTUREPOLARITY(hcsr->htim, hcsr->channel, TIM_INPUTCHANNELPOLARITY_RISING);
        hcsr->edge_state = 2; // Estado final: medición lista
    }
}


// ===============================================
// 4. LECTURA DE DISTANCIA
// ===============================================

/**
 * @brief Calcula y devuelve la distancia en centímetros.
 * @note Asume que el contador del Timer está configurado a 1 MHz (1 tick = 1 µs).
 */
float HCSR04_ReadDistance_cm(HCSR04_Handle* hcsr) {
    if (hcsr->edge_state != 2) {
        return -1.0f;
    }

    const float F_CNT_HZ = 1000000.0f; // 1 MHz = 1 µs/tick

    float time_s = (float)hcsr->pulse_duration / F_CNT_HZ;
    float distance = (time_s * SOUND_SPEED_CMS) / 2.0f;

    // Restablecer el estado para la próxima medición
    hcsr->edge_state = 0;

    return distance;
}
