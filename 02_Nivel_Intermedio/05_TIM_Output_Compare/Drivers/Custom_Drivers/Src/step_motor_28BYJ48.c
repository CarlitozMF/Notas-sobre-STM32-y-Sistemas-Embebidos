/**
 * @file step_motor_28BYJ48.c
 * @brief Implementación de la lógica de secuenciamiento agnóstica para el motor 28BYJ-48.
 * @author Mamani Flores Carlos (UTN FRT)
 * @details Desarrolla la máquina de estados lógicos para la excitación de bobinas de
 * un motor paso a paso unipolar. La manipulación de hardware se realiza mediante
 * inyección de dependencias (PAL), aislando las rutinas críticas de interrupción
 * y el acumulador de fase de las capas del silicio del fabricante.
 * @version 2.0
 * @date 2026
 */

#include "step_motor_28BYJ48.h"

/**
 * @brief Tabla de pasos para el modo Half-Step (8 estados).
 * @details Mapeo binario de conmutación: Bit 0 -> IN1, Bit 1 -> IN2, Bit 2 -> IN3, Bit 3 -> IN4.
 * Proporciona pasos de 5.625° por secuencia lógica (4096 pasos por vuelta de eje).
 */
static const uint8_t HALF_STEP_TABLE[8] = {
    0x08, /**< Paso 0: B1000 -> IN4 Activa */
    0x0C, /**< Paso 1: B1100 -> IN4 + IN3 Activas */
    0x04, /**< Paso 2: B0100 -> IN3 Activa */
    0x06, /**< Paso 3: B0110 -> IN3 + IN2 Activas */
    0x02, /**< Paso 4: B0010 -> IN2 Activa */
    0x03, /**< Paso 5: B0011 -> IN2 + IN1 Activas */
    0x01, /**< Paso 6: B0001 -> IN1 Activa */
    0x09  /**< Paso 7: B1001 -> IN1 + IN4 Activas */
};

/**
 * @brief Tabla de pasos para el modo Full-Step (4 estados).
 * @details Mapeo binario de conmutación bifásica (dos bobinas activas simultáneamente).
 * Maximiza el torque dinámico sacrificando suavidad en la resolución.
 */
static const uint8_t FULL_STEP_TABLE[4] = {
    0x03, /**< Estado 1: B0011 -> IN1 + IN2 Activas */
    0x06, /**< Estado 2: B0110 -> IN2 + IN3 Activas */
    0x0C, /**< Estado 3: B1100 -> IN3 + IN4 Activas */
    0x09  /**< Estado 4: B1001 -> IN4 + IN1 Activas */
};

/**
 * @brief Inicializa la estructura del motor inyectando las dependencias de la plataforma.
 * @param hstepper Puntero al handle de control del motor (Instancia Soberana).
 * @param pins Arreglo de 4 estructuras con el mapeo físico de los pines IN1 a IN4.
 * @param oc_ch Estructura descriptor del canal del Timer asignado al Output Compare.
 * @param mode Modo de excitación inicial (MODE_FULL_STEP o MODE_HALF_STEP).
 * @param initial_delay Valor delta inicial en ticks (us) para el acumulador elástico.
 * @param pal Estructura de servicios de plataforma que contiene las tablas de funciones virtuales.
 */
void Stepper_Init(Stepper_t* hstepper, generic_gpio_t pins[], generic_pwm_t oc_ch,
                  Step_Mode_t mode, uint32_t initial_delay, hal_interface_t pal) {
    if (!hstepper) return;

    /* Clonación del mapeo físico de pines hacia el contexto privado del objeto */
    for(int i = 0; i < 4; i++) {
        hstepper->pins[i] = pins[i];
    }

    /* Enlace de descriptores de hardware y constantes operacionales */
    hstepper->oc_channel = oc_ch;
    hstepper->mode = mode;
    hstepper->step_delay = initial_delay;
    hstepper->pal = pal;

    /* Forzado de condiciones iniciales de parada segura */
    hstepper->current_step = 0;
    hstepper->direction = STEP_CW;
    hstepper->is_active = false;

    Stepper_Stop(hstepper);
}

/**
 * @brief Configura dinámicamente el sentido de giro.
 * @param hstepper Puntero al handle del motor.
 * @param dir Dirección de giro deseada (STEP_CW o STEP_CCW).
 */
void Stepper_Set_Direction(Stepper_t* hstepper, Step_Dir_t dir) {
    if (hstepper) hstepper->direction = dir;
}

/**
 * @brief Configura dinámicamente el delay entre pasos (Modulación de RPM).
 * @param hstepper Puntero al handle del motor.
 * @param delay Tiempo delta en microsegundos (ticks del clock OC).
 * @note Valores por debajo de 900 us pueden provocar pérdida de sincronismo por inercia.
 */
void Stepper_Set_Delay(Stepper_t* hstepper, uint32_t delay) {
    if (hstepper) hstepper->step_delay = delay;
}

/**
 * @brief Habilita el despacho asíncrono de pasos del motor.
 * @param hstepper Puntero al handle del motor.
 */
void Stepper_Start(Stepper_t* hstepper) {
    if (hstepper) hstepper->is_active = true;
}

/**
 * @brief Detiene el motor desenergizando todas las bobinas (Ahorro térmico).
 * @param hstepper Puntero al handle del motor.
 * @note Corta el lazo magnético de retención para evitar sobrecalentamiento del estator.
 */
void Stepper_Stop(Stepper_t* hstepper) {
    if (!hstepper) return;

    hstepper->is_active = false;

    /* Escritura atómica a nivel de hardware usando la PAL inyectada */
    if (hstepper->pal.gpio_write) {
        for (int i = 0; i < 4; i++) {
            hstepper->pal.gpio_write(hstepper->pins[i], false);
        }
    }
}

/**
 * @brief Manejador asíncrono de interrupción por Output Compare.
 * @details Realiza la conmutación de fase basándose en la tabla de conmutación activa y
 * resuelve la ecuación elástica de tiempos del temporizador sin bloqueos de CPU.
 * @param hstepper Puntero al handle del motor.
 * @note Debe invocarse con alta prioridad desde el Callback físico del microcontrolador.
 */
void Stepper_OC_Handler(Stepper_t* hstepper) {
    if (!hstepper) return;

    /* Verificación de seguridad de existencia de vtable */
    if (!hstepper->pal.gpio_write || !hstepper->pal.oc_read || !hstepper->pal.oc_write) return;

    /* 1. Lógica de avance mecánico y secuenciamiento de bobinas */
    if (hstepper->is_active) {
        uint8_t pattern;
        uint8_t max_steps = (hstepper->mode == MODE_HALF_STEP) ? 8 : 4;

        /* Cálculo circular del índice de paso bajo aritmética modular implícita */
        if (hstepper->direction == STEP_CW) {
            hstepper->current_step++;
            if (hstepper->current_step >= max_steps) hstepper->current_step = 0;
        } else {
            hstepper->current_step--;
            if (hstepper->current_step < 0) hstepper->current_step = max_steps - 1;
        }

        /* Recuperación estática del patrón de bits binarios según el modo */
        if (hstepper->mode == MODE_HALF_STEP) {
            pattern = HALF_STEP_TABLE[hstepper->current_step];
        } else {
            pattern = FULL_STEP_TABLE[hstepper->current_step];
        }

        /* Despacho del patrón de estimulación física canal por canal */
        for (int i = 0; i < 4; i++) {
            /* Máscara de bit deslizante para extraer el estado de IN1 (i=0) a IN4 (i=3) */
            bool pin_state = (pattern & (0x01 << i)) ? true : false;
            hstepper->pal.gpio_write(hstepper->pins[i], pin_state);
        }
    }

    /* 2. Ecuación del Acumulador Elástico de Fase (Soberanía del tiempo de hardware) */
    /* Captura el contador actual de coincidencia del periférico remoto */
    uint32_t current_capture = hstepper->pal.oc_read(hstepper->oc_channel);

    /* Reprograma el próximo disparo de alarma sumando el delta lineal de tiempo */
    hstepper->pal.oc_write(hstepper->oc_channel, current_capture + hstepper->step_delay);
}
