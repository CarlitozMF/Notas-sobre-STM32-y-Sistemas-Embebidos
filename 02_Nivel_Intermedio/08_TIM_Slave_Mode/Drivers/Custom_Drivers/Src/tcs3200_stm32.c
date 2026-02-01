/**
 * @file    tcs3200_stm32.c
 * @author  Carlos
 * @brief   Implementación del driver TCS3200.
 */

#include "tcs3200_stm32.h"

void TCS3200_Init(TCS3200_t *sensor, TIM_HandleTypeDef *hgate, TIM_HandleTypeDef *hcount) {
    sensor->htim_gate = hgate;
    sensor->htim_count = hcount;
    sensor->measurement_ready = 0;

    // Inicializamos resultados en 0
    sensor->frequency_red = 0;
    sensor->frequency_green = 0;
    sensor->frequency_blue = 0;
    sensor->frequency_clear = 0;
}

void TCS3200_ConfigGPIO(TCS3200_t *sensor,
                        GPIO_TypeDef* s0_p, uint16_t s0, GPIO_TypeDef* s1_p, uint16_t s1,
                        GPIO_TypeDef* s2_p, uint16_t s2, GPIO_TypeDef* s3_p, uint16_t s3,
                        GPIO_TypeDef* led_p, uint16_t led) {
    // 1. Vinculación de pines físicos a la estructura lógica
    sensor->S0_Port = s0_p; sensor->S0_Pin = s0;
    sensor->S1_Port = s1_p; sensor->S1_Pin = s1;
    sensor->S2_Port = s2_p; sensor->S2_Pin = s2;
    sensor->S3_Port = s3_p; sensor->S3_Pin = s3;
    sensor->LED_Port = led_p; sensor->LED_Pin = led;

    // 2. Configuración Inicial de Escala de Frecuencia
/*
    // S0=HIGH, S1=LOW -> Escala del 20% (Ideal para STM32 y precisión media)
    HAL_GPIO_WritePin(sensor->S0_Port, sensor->S0_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(sensor->S1_Port, sensor->S1_Pin, GPIO_PIN_RESET);*/
    // S0=HIGH, S1=HIGH -> Escala del 100%
    HAL_GPIO_WritePin(sensor->S0_Port, sensor->S0_Pin, GPIO_PIN_SET);   // S0 = 1
    HAL_GPIO_WritePin(sensor->S1_Port, sensor->S1_Pin, GPIO_PIN_SET);   // S1 = 1 (100% Output Freq)

    // 3. Estado inicial del LED (Apagado para no calentar el sensor)
    TCS3200_ControlLED(sensor, TCS_LED_OFF);
}

void TCS3200_SetFilter(TCS3200_t *sensor, TCS_Filter_t filter) {
    // Configuración de pines S2 y S3 según tabla de verdad del Datasheet
    switch(filter) {
        case TCS_FILTER_RED:   // Rojo: L, L
            HAL_GPIO_WritePin(sensor->S2_Port, sensor->S2_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(sensor->S3_Port, sensor->S3_Pin, GPIO_PIN_RESET);
            break;
        case TCS_FILTER_BLUE:  // Azul: L, H
            HAL_GPIO_WritePin(sensor->S2_Port, sensor->S2_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(sensor->S3_Port, sensor->S3_Pin, GPIO_PIN_SET);
            break;
        case TCS_FILTER_CLEAR: // Sin Filtro: H, L
            HAL_GPIO_WritePin(sensor->S2_Port, sensor->S2_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(sensor->S3_Port, sensor->S3_Pin, GPIO_PIN_RESET);
            break;
        case TCS_FILTER_GREEN: // Verde: H, H
            HAL_GPIO_WritePin(sensor->S2_Port, sensor->S2_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(sensor->S3_Port, sensor->S3_Pin, GPIO_PIN_SET);
            break;
    }
    // Pequeño delay de estabilización (el fotodiodo tarda unos microsegundos en reaccionar)
    HAL_Delay(2);
}

void TCS3200_ControlLED(TCS3200_t *sensor, TCS_LedState_t state) {
    HAL_GPIO_WritePin(sensor->LED_Port, sensor->LED_Pin, (state == TCS_LED_ON) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void TCS3200_StartMeasurement(TCS3200_t *sensor) {
    // 1. Limpieza de seguridad del contador
    __HAL_TIM_SET_COUNTER(sensor->htim_count, 0);

    // 2. Iniciar el Timer Contador (Esclavo en PB6)
    // Este timer cuenta continuamente, no depende de nadie en este modo.
    HAL_TIM_Base_Start(sensor->htim_count);

    // 3. Iniciar el Timer Base de Tiempo (Maestro Interno) con Interrupción
    // Este timer generará una interrupción periódica (PeriodElapsed) cada TCS_MEASURE_WINDOW_MS.
    HAL_TIM_Base_Start_IT(sensor->htim_gate);
}

void TCS3200_ProcessCallback(TCS3200_t *sensor) {
    // ESTA ES LA FUNCIÓN CRÍTICA. Se llama desde la ISR del Timer Base.

    // 1. Capturar el valor actual de pulsos contados en PB6
    uint32_t pulses = __HAL_TIM_GET_COUNTER(sensor->htim_count);

    // 2. Resetear el contador inmediatamente para la siguiente ventana
    // Hacemos esto rápido para perder la menor cantidad de pulsos posible.
    __HAL_TIM_SET_COUNTER(sensor->htim_count, 0);

    // 3. Calcular Frecuencia en Hz
    // Fórmula: Frecuencia = Pulsos / Tiempo(segundos)
    // Ejemplo: Si contamos 1000 pulsos en 500ms (0.5s) -> 1000 * 2 = 2000 Hz.
    uint32_t multiplier = 1000 / TCS_MEASURE_WINDOW_MS;
    uint32_t frequency_hz = pulses * multiplier;

    // 4. Asignar el valor a la variable correcta
    // Leemos el estado físico de los pines para saber qué filtro estábamos midiendo.
    // Esto hace que el código sea robusto ante cambios manuales.
    uint8_t s2 = HAL_GPIO_ReadPin(sensor->S2_Port, sensor->S2_Pin);
    uint8_t s3 = HAL_GPIO_ReadPin(sensor->S3_Port, sensor->S3_Pin);

    if      (s2 == 0 && s3 == 0) sensor->frequency_red   = frequency_hz;
    else if (s2 == 0 && s3 == 1) sensor->frequency_blue  = frequency_hz;
    else if (s2 == 1 && s3 == 0) sensor->frequency_clear = frequency_hz;
    else if (s2 == 1 && s3 == 1) sensor->frequency_green = frequency_hz;

    // 5. Indicar al Main que hay nuevos datos disponibles
    sensor->measurement_ready = 1;
}
