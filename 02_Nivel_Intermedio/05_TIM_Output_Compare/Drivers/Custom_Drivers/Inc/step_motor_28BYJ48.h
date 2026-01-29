/**
 * @file step_motor_28BYJ48.h
 * @author CarlitozMF
 * @brief Driver genérico y encapsulado para el motor paso a paso 28BYJ-48.
 * @version 1.1
 * @date 2026-01-27
 * * @details Este driver permite controlar múltiples motores utilizando una estructura
 * de handle. Soporta modos Full-Step y Half-Step mediante tablas de estados.
 */

#ifndef STEP_MOTOR_28BYJ48_H_
#define STEP_MOTOR_28BYJ48_H_

#include "main.h"

/**
 * @enum Step_Mode_t
 * @brief Modos de excitación de las bobinas del motor.
 */
typedef enum {
    MODE_FULL_STEP = 0,    /*!< 4 pasos por secuencia. Mayor torque, mayor vibración. */
    MODE_HALF_STEP = 1     /*!< 8 pasos por secuencia. Mayor suavidad y precisión.    */
} Step_Mode_t;

/**
 * @enum Step_Dir_t
 * @brief Sentido de giro del motor.
 */
typedef enum {
    STEP_CW  = 0,          /*!< Sentido horario (Clockwise).        */
    STEP_CCW = 1           /*!< Sentido antihorario (Counter-Clockwise). */
} Step_Dir_t;

/**
 * @struct Stepper_Handle_t
 * @brief Estructura de control para el encapsulamiento de cada instancia del motor.
 */
typedef struct {
    GPIO_TypeDef* GPIO_Ports[4]; /*!< Puertos GPIO para las señales IN1, IN2, IN3, IN4. */
    uint16_t      GPIO_Pins[4];  /*!< Pines GPIO para las señales IN1, IN2, IN3, IN4.   */
    int8_t        current_step;  /*!< Índice del paso actual en la secuencia.           */
    Step_Mode_t   mode;          /*!< Modo de operación seleccionado.                   */
    Step_Dir_t    direction;     /*!< Sentido de giro actual.                           */
    uint8_t       is_active;     /*!< Flag de estado del motor (1: Activo, 0: Parado).  */
} Stepper_t;

/* --- Prototipos de Funciones --- */

/**
 * @brief Inicializa la estructura del motor y configura el estado inicial.
 * @param hstepper Puntero al handle del motor.
 * @param ports Arreglo de punteros a los puertos GPIO (orden IN1 a IN4).
 * @param pins Arreglo de pines GPIO correspondientes.
 * @param mode Modo de paso (Full o Half).
 */
void Stepper_Init(Stepper_t* hstepper, GPIO_TypeDef* ports[], uint16_t pins[], Step_Mode_t mode);

/**
 * @brief Ejecuta un paso de la secuencia basándose en el modo y la dirección.
 * @note Esta función debe ser llamada desde el Callback de Output Compare.
 * @param hstepper Puntero al handle del motor.
 */
void Stepper_Step_Sequential(Stepper_t* hstepper);

/**
 * @brief Configura el sentido de giro.
 * @param hstepper Puntero al handle del motor.
 * @param dir Sentido de giro (STEP_CW o STEP_CCW).
 */
void Stepper_Set_Direction(Stepper_t* hstepper, Step_Dir_t dir);

/**
 * @brief Detiene el motor apagando todas las bobinas (ahorro de energía).
 * @param hstepper Puntero al handle del motor.
 */
void Stepper_Stop(Stepper_t* hstepper);

#endif /* STEP_MOTOR_28BYJ48_H_ */
