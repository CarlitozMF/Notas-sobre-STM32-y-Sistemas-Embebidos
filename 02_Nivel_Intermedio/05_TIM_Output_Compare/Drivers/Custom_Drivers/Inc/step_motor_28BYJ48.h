/**
 * @file step_motor_28BYJ48.h
 * @brief Driver agnóstico y modular para el control de motores paso a paso 28BYJ-48.
 * @author Mamani Flores Carlos (UTN FRT)
 * @details Este archivo define el "contrato" y las estructuras de control necesarias
 * para operar motores paso a paso unipolar mediante el arreglo Darlington ULN2003.
 * Utiliza una Capa de Abstracción de Hardware (PAL) para desvincular por completo
 * la lógica de secuenciamiento y el acumulador de fase elástico (Output Compare)
 * de los registros específicos del silicio (STM32, AVR, NXP, etc.).
 * @version 2.0
 * @date 2026
 */

#ifndef STEP_MOTOR_28BYJ48_H_
#define STEP_MOTOR_28BYJ48_H_

#include "hal_interface.h"
#include <stdint.h>
#include <stdbool.h>

typedef enum {
    MODE_FULL_STEP = 0,    /**< 4 pasos por secuencia. Mayor torque. */
    MODE_HALF_STEP = 1     /**< 8 pasos por secuencia. Mayor suavidad. */
} Step_Mode_t;

typedef enum {
    STEP_CW  = 0,          /**< Sentido horario (Clockwise). */
    STEP_CCW = 1           /**< Sentido antihorario (Counter-Clockwise). */
} Step_Dir_t;

/**
 * @struct Stepper_t
 * @brief Handle de control encapsulado e independiente del hardware.
 */
typedef struct {
    generic_gpio_t  pins[4];       /**< Mapeo agnóstico de IN1, IN2, IN3, IN4 */
    generic_pwm_t   oc_channel;    /**< Canal/Timer asociado a la alarma OC */
    hal_interface_t pal;           /**< Tabla de despacho de servicios de plataforma */

    int8_t          current_step;  /**< Índice del paso actual en la secuencia */
    Step_Mode_t     mode;          /**< Modo de operación (Full/Half) */
    Step_Dir_t      direction;     /**< Sentido de giro actual */
    uint32_t        step_delay;    /**< Delta de tiempo para el acumulador de fase (ticks) */
    bool            is_active;     /**< Flag de estado de movimiento (true/false) */
} Stepper_t;

/* --- API Pública de Control --- */

/**
 * @brief Inicializa la estructura del motor inyectando las dependencias de la plataforma.
 */
void Stepper_Init(Stepper_t* hstepper, generic_gpio_t pins[], generic_pwm_t oc_ch,
                  Step_Mode_t mode, uint32_t initial_delay, hal_interface_t pal);

/**
 * @brief Configura dinámicamente el sentido de giro.
 */
void Stepper_Set_Direction(Stepper_t* hstepper, Step_Dir_t dir);

/**
 * @brief Configura dinámicamente el delay entre pasos (Velocidad / RPM).
 */
void Stepper_Set_Delay(Stepper_t* hstepper, uint32_t delay);

/**
 * @brief Arranca el movimiento del motor.
 */
void Stepper_Start(Stepper_t* hstepper);

/**
 * @brief Detiene el motor desenergizando bobinas de forma inmediata (Protección térmica).
 */
void Stepper_Stop(Stepper_t* hstepper);

/**
 * @brief Manejador asíncrono de interrupción por Output Compare.
 * @details Debe ser llamado desde el Callback físico del Timer de la plataforma.
 * Resuelve la lógica de pasos y actualiza de forma elástica el acumulador de fase.
 */
void Stepper_OC_Handler(Stepper_t* hstepper);

#endif /* STEP_MOTOR_28BYJ48_H_ */
