/**
 * @file servo_sg90.h
 * @author CarlitozMF (UTN FRT)
 * @brief Driver avanzado para controlar Servo SG90 con PWM.
 * @details Soporta N servos.
 * @version 1.0
 * @date 2026
 */
#ifndef SERVO_SG90_H
#define SERVO_SG90_H

#include "main.h" //Con esta linea me idependizo del micro stm32?

/**
 * @brief Estructura de control para una instancia de Servo.
 */
typedef struct {
    TIM_HandleTypeDef *htim;      // Puntero al Timer (ej. &htim3)
    uint32_t channel;             // Canal del Timer (ej. TIM_CHANNEL_1)

    // Calibración (permite usar diferentes servos con un mismo driver)
    uint32_t min_pulse;           // CCR para 0 grados
    uint32_t max_pulse;           // CCR para 180 grados

    // Estado del movimiento
    float current_angle;          // Ángulo real (acumulador de precisión flotante)
    int target_angle;             // Ángulo objetivo ordenado
    float step_per_tick;          // Grados por milisegundo (velocidad)
    uint32_t last_time_ms;        // Timestamp para control asíncrono
} Servo_t;

/**
 * @brief Inicializa el servo con sus parámetros de hardware y calibración.
 */
void SERVO_SG90_Init(Servo_t *servo, TIM_HandleTypeDef *htim, uint32_t channel, uint32_t min, uint32_t max);

/**
 * @brief Posicionamiento instantáneo.
 */
void SERVO_SG90_SetAngle(Servo_t *servo, int angle);

/**
 * @brief Control de trayectoria suave (Interpolación lineal).
 */
void SERVO_SG90_SetSpeedAngle(Servo_t *servo, int target_angle, float speed_dps);

/**
 * @brief Máquina de estados de movimiento. Llamar en el while(1).
 */
void SERVO_SG90_Update(Servo_t *servo);

#endif /* SERVO_SG90_H */
