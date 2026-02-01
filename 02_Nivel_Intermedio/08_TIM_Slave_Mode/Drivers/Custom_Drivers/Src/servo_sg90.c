/**
 * @file servo_sg90.c
 * @brief Implementación del driver para servomotores con control asíncrono.
 */

#include "servo_sg90.h"

/**
 * @brief Función interna de mapeo (Privada).
 * @details Convierte el ángulo físico (0-180°) en una señal de tiempo (CCR).
 * La relación es lineal: CCR = min + (ángulo * factor_escala).
 * @param servo: Puntero a la instancia del servo.
 * @param angle: Ángulo en formato float para mantener la precisión de la interpolación.
 * @retval Valor calculado para el registro CCR del Timer.
 */
static uint32_t map_angle_to_ccr(Servo_t *servo, float angle) {
    // 1. Restricción (Clamping): Evita que el servo intente girar más allá de sus límites físicos
    if (angle < 0.0f) angle = 0.0f;
    if (angle > 180.0f) angle = 180.0f;

    // 2. Cálculo del factor de escala: (Diferencia de Ticks / Rango de Grados)
    // Esto nos dice cuántas unidades de CCR equivalen a 1 grado.
    float range = (float)(servo->max_pulse - servo->min_pulse);
    float factor = range / 180.0f;

    // 3. Resultado: Offset mínimo + desplazamiento proporcional
    return (uint32_t)((float)servo->min_pulse + (factor * angle));
}

void SERVO_SG90_Init(Servo_t *servo, TIM_HandleTypeDef *htim, uint32_t channel, uint32_t min, uint32_t max) {
    // Copiamos la configuración de hardware a la estructura de la instancia
    servo->htim = htim;
    servo->channel = channel;
    servo->min_pulse = min;
    servo->max_pulse = max;

    // Inicialización del estado de movimiento
    servo->current_angle = 0.0f;
    servo->target_angle = 0;
    servo->step_per_tick = 0.0f; // Por defecto, movimiento instantáneo
    servo->last_time_ms = HAL_GetTick();

    // Arrancamos el periférico PWM del STM32
    HAL_TIM_PWM_Start(servo->htim, servo->channel);

    // Posicionamos el servo en el punto de origen (0°)
    SERVO_SG90_SetAngle(servo, 0);
}

void SERVO_SG90_SetAngle(Servo_t *servo, int angle) {
    // Protección de rango
    if (angle < 0) angle = 0;
    if (angle > 180) angle = 180;

    // Actualizamos objetivos
    servo->target_angle = angle;
    servo->current_angle = (float)angle; // Sincronizamos float con el entero
    servo->step_per_tick = 0.0f;         // Desactivamos cualquier suavizado en curso

    // Aplicamos el cambio al registro CCR inmediatamente
    uint32_t ccr_val = map_angle_to_ccr(servo, servo->current_angle);
    __HAL_TIM_SET_COMPARE(servo->htim, servo->channel, ccr_val);
}

void SERVO_SG90_SetSpeedAngle(Servo_t *servo, int target_angle, float speed_dps) {
    if (target_angle < 0) target_angle = 0;
    if (target_angle > 180) target_angle = 180;

    servo->target_angle = target_angle;
    servo->last_time_ms = HAL_GetTick(); // Reiniciamos la base de tiempo para el cálculo de delta

    // Si la velocidad es 0 o negativa, el movimiento debe ser inmediato
    if (speed_dps <= 0.0f) {
        SERVO_SG90_SetAngle(servo, target_angle);
    } else {
        // Convertimos Grados/Segundo a Grados/Milisegundo (que es la unidad de HAL_GetTick)
        servo->step_per_tick = speed_dps / 1000.0f;
    }
}

void SERVO_SG90_Update(Servo_t *servo) {
    // Si ya estamos en el objetivo (comparación entera), no desperdiciamos ciclos de CPU
    if (servo->step_per_tick == 0.0f || (int)servo->current_angle == servo->target_angle) {
        return;
    }

    // 1. Cálculo del tiempo transcurrido (Delta Time)
    uint32_t now = HAL_GetTick();
    uint32_t delta = now - servo->last_time_ms;

    // Solo actualizamos si ha pasado al menos 1ms para evitar errores de punto flotante insignificantes
    if (delta > 0) {
        servo->last_time_ms = now;

        // 2. Calculamos cuánto debe avanzar el ángulo en este intervalo
        float move = servo->step_per_tick * (float)delta;

        // 3. Aproximación al objetivo (Hacia arriba o hacia abajo)
        if (servo->current_angle < (float)servo->target_angle) {
            servo->current_angle += move;
            // Evitamos sobrepasar el objetivo por el redondeo del float
            if (servo->current_angle > (float)servo->target_angle)
                servo->current_angle = (float)servo->target_angle;
        } else {
            servo->current_angle -= move;
            if (servo->current_angle < (float)servo->target_angle)
                servo->current_angle = (float)servo->target_angle;
        }

        // 4. Actualizamos el PWM con la nueva posición intermedia
        uint32_t ccr_val = map_angle_to_ccr(servo, servo->current_angle);
        __HAL_TIM_SET_COMPARE(servo->htim, servo->channel, ccr_val);
    }
}
