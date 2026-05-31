/**
 * @file servo_sg90.c
 * @author Mamani Flores Carlos (UTN FRT)
 * @brief Implementación del driver para servomotores con control asíncrono y PAL.
 * @details Contiene la lógica cinemática y de interpolación para el posicionamiento
 *          suave del servomotor, utilizando inyección de dependencias para lograr
 *          un desacoplamiento absoluto de la arquitectura del silicio.
 * @version 1.0
 * @date 2026
 */

#include "servo_sg90.h"

/* --- Funciones Privadas del Módulo --- */

/**
 * @brief Función interna de mapeo (Privada).
 * @details Convierte el ángulo físico (0-180°) en una señal de tiempo o cuentas del registro.
 *          La relación es lineal: Valor = min + (ángulo * factor_escala).
 * @param[in] servo Puntero a la instancia del servo.
 * @param[in] angle Ángulo en formato float para mantener la precisión de la interpolación.
 * @retval uint32_t Valor calculado listo para ser inyectado en el periférico de salida.
 */
static uint32_t map_angle_to_ccr(Servo_t *servo, float angle) {
    /* 1. Restricción (Clamping): Evita que se calculen valores fuera de los límites de calibración */
    if (angle < 0.0f) angle = 0.0f;
    if (angle > 180.0f) angle = 180.0f;

    /* 2. Cálculo del factor de escala: Unidades de registro (cuentas/us) por cada grado físico */
    float range = (float)(servo->max_pulse - servo->min_pulse);
    float factor = range / 180.0f;

    /* 3. Resultado: Desplazamiento proporcional sumado al offset del pulso mínimo */
    return (uint32_t)((float)servo->min_pulse + (factor * angle));
}

/* --- Funciones Públicas del Módulo --- */

/**
 * @brief Inicializa el servo con sus parámetros de hardware genéricos y calibración.
 * @details Almacena los descriptores de hardware, vincula la tabla de servicios de la PAL
 *          y establece el estado inicial cinemático, forzando al servo a su posición de origen.
 * @param[out] servo Puntero a la estructura de control de la instancia.
 * @param[in]  pwm_ch Descriptor genérico del canal PWM asignado.
 * @param[in]  pal_io Contrato de servicios de plataforma (VTable).
 * @param[in]  min    Valor de comparación/tiempo asignado para los 0 grados.
 * @param[in]  max    Valor de comparación/tiempo asignado para los 180 grados.
 */
void SERVO_SG90_Init(Servo_t *servo, generic_pwm_t pwm_ch, hal_interface_t pal_io, uint32_t min, uint32_t max) {
    /* Validación de Robustez: Aborta si los punteros esenciales inyectados son nulos */
    if (servo == NULL || pal_io.pwm_write == NULL || pal_io.get_tick == NULL) return;

    /* Inyección de dependencias de hardware y servicios de plataforma */
    servo->pwm_chan = pwm_ch;
    servo->pal = pal_io;
    servo->min_pulse = min;
    servo->max_pulse = max;

    /* Inicialización del estado cinemático latente */
    servo->current_angle = 0.0f;
    servo->target_angle = 0;
    servo->step_per_tick = 0.0f; /* Movimiento instantáneo por defecto */
    servo->last_time_ms = servo->pal.get_tick();

    /* Forzar posicionamiento inicial seguro en el origen del sistema (0 grados) */
    SERVO_SG90_SetAngle(servo, 0);
}

/**
 * @brief Posicionamiento instantáneo.
 * @details Configura el ángulo de destino de manera inmediata, omitiendo cualquier perfil
 *          de velocidad activo y actualizando directamente el periférico de hardware.
 * @param[in,out] servo Puntero a la instancia del servo.
 * @param[in]     angle Ángulo entero destino (0 a 180).
 */
void SERVO_SG90_SetAngle(Servo_t *servo, int angle) {
    /* Validación de seguridad perimetral */
    if (servo == NULL || servo->pal.pwm_write == NULL) return;

    /* Clamping preventivo sobre el argumento de entrada */
    if (angle < 0) angle = 0;
    if (angle > 180) angle = 180;

    /* Sincronización inmediata de los objetivos del sistema */
    servo->target_angle = angle;
    servo->current_angle = (float)angle;
    servo->step_per_tick = 0.0f; /* Se desactiva la interpolación por velocidad */

    /* Conversión matemática e inyección física de la orden mediante la PAL */
    uint32_t ccr_val = map_angle_to_ccr(servo, servo->current_angle);
    servo->pal.pwm_write(servo->pwm_chan, (uint16_t)ccr_val);
}

/**
 * @brief Control de trayectoria suave (Interpolación lineal).
 * @details Configura un ángulo objetivo y calcula la tasa de cambio de grados por milisegundo
 *          basándose en la velocidad Angular solicitada (DPS).
 * @param[in,out] servo        Puntero a la instancia del servo.
 * @param[in]     target_angle Ángulo entero final al que se desea arribar.
 * @param[in]     speed_dps    Velocidad de operación expresada en Grados por Segundo.
 */
void SERVO_SG90_SetSpeedAngle(Servo_t *servo, int target_angle, float speed_dps) {
    /* Validación de robustez en punteros */
    if (servo == NULL || servo->pal.get_tick == NULL) return;

    /* Control de límites del ángulo objetivo */
    if (target_angle < 0) target_angle = 0;
    if (target_angle > 180) target_angle = 180;

    servo->target_angle = target_angle;

    /* Reinicio de la base de tiempo para evitar saltos bruscos en el primer diferencial del Update */
    servo->last_time_ms = servo->pal.get_tick();

    /* Si la velocidad es nula o negativa, se asume comando de velocidad máxima (instantáneo) */
    if (speed_dps <= 0.0f) {
        SERVO_SG90_SetAngle(servo, target_angle);
    } else {
        /* Conversión de unidades: Grados/Segundo -> Grados/Milisegundo (acorde al latido de get_tick) */
        servo->step_per_tick = speed_dps / 1000.0f;
    }
}

/**
 * @brief Máquina de estados de movimiento. Llamar en el lazo principal while(1).
 * @details Ejecuta de forma asíncrona y no bloqueante la aproximación progresiva hacia
 *          el ángulo objetivo. Utiliza aritmética de tiempo diferencial para independizar
 *          la velocidad angular de la frecuencia de llamada de la función.
 * @param[in,out] servo Puntero a la instancia del servo.
 */
void SERVO_SG90_Update(Servo_t *servo) {
    /* Comprobación de seguridad en los servicios inyectados */
    if (servo == NULL || servo->pal.get_tick == NULL || servo->pal.pwm_write == NULL) return;

    /* Optimización de ciclos de CPU: Aborta si no hay suavizado activo o si ya alcanzó la meta */
    if (servo->step_per_tick == 0.0f || (int)servo->current_angle == servo->target_angle) {
        return;
    }

    /* 1. Captura de tiempo actual y cálculo del diferencial (Delta Time) */
    uint32_t now = servo->pal.get_tick();
    uint32_t delta = now - servo->last_time_ms;

    /* Se procesa únicamente si existió un avance real en el tiempo (mínimo 1 ms) */
    if (delta > 0) {
        servo->last_time_ms = now;

        /* 2. Cálculo del tramo angular proporcional al tiempo transcurrido */
        float move = servo->step_per_tick * (float)delta;

        /* 3. Algoritmo de aproximación lineal (dirección del paso) */
        if (servo->current_angle < (float)servo->target_angle) {
            servo->current_angle += move;
            /* Anti-overriding: Evita que el error numérico del float supere la meta */
            if (servo->current_angle > (float)servo->target_angle)
                servo->current_angle = (float)servo->target_angle;
        } else {
            servo->current_angle -= move;
            if (servo->current_angle < (float)servo->target_angle)
                servo->current_angle = (float)servo->target_angle;
        }

        /* 4. Refresh del periférico físico con la posición intermedia calculada */
        uint32_t ccr_val = map_angle_to_ccr(servo, servo->current_angle);
        servo->pal.pwm_write(servo->pwm_chan, (uint16_t)ccr_val);
    }
}
