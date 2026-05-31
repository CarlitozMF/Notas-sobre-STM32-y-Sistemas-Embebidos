/**
 * @file servo_sg90.h
 * @author Mamani Flores Carlos (UTN FRT)
 * @brief Driver genérico y multiplataforma para el control de Servo SG90 con PWM.
 * @details Contiene la lógica cinemática y las estructuras necesarias para operar
 *          múltiples instancias de servos mediante la inyección de dependencias (PAL).
 * @version 2.0
 * @date 2026
 */

#ifndef SERVO_SG90_H
#define SERVO_SG90_H

#include "hal_interface.h"
#include <stddef.h>

/**
 * @struct Servo_t
 * @brief Estructura de control para una instancia de Servo agnóstica al hardware.
 */
typedef struct {
    generic_pwm_t   pwm_chan;       /**< Descriptor genérico del canal PWM (Timer y canal) */
    hal_interface_t pal;            /**< Súper-objeto PAL con servicios de la plataforma */

    // Calibración (permite usar diferentes servos con un mismo driver)
    uint32_t min_pulse;           /**< Valor asignado para 0 grados (en microsegundos o cuentas) */
    uint32_t max_pulse;           /**< Valor asignado para 180 grados (en microsegundos o cuentas) */

    // Estado del movimiento
    float current_angle;          /**< Ángulo real (acumulador de precisión flotante) */
    int target_angle;             /**< Ángulo objetivo ordenado */
    float step_per_tick;          /**< Grados por milisegundo (velocidad) */
    uint32_t last_time_ms;        /**< Timestamp para control asíncrono */
} Servo_t;

/**
 * @brief Inicializa el servo con sus parámetros de hardware genéricos y calibración.
 */
void SERVO_SG90_Init(Servo_t *servo, generic_pwm_t pwm_ch, hal_interface_t pal_io, uint32_t min, uint32_t max);

/**
 * @brief Posicionamiento instantáneo.
 */
void SERVO_SG90_SetAngle(Servo_t *servo, int angle);

/**
 * @brief Control de trayectoria suave (Interpolación lineal).
 */
void SERVO_SG90_SetSpeedAngle(Servo_t *servo, int target_angle, float speed_dps);

/**
 * @brief Máquina de estados de movimiento. Llamar en el lazo principal while(1).
 */
void SERVO_SG90_Update(Servo_t *servo);

#endif /* SERVO_SG90_H */
