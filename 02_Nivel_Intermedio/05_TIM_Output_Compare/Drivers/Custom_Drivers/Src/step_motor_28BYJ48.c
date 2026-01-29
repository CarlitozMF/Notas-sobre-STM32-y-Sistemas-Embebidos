/**
 * @file step_motor_28BYJ48.c
 * @brief Implementación de la lógica de secuenciamiento para el 28BYJ-48.
 */

#include "step_motor_28BYJ48.h"

/**
 * @brief Tabla de pasos para el modo Half-Step (8 pasos).
 * Mapeo: Bit0=IN1, Bit1=IN2, Bit2=IN3, Bit3=IN4.
 */
/*
static const uint8_t HALF_STEP_TABLE[8] = {
    0x01, // IN1
    0x03, // IN1+IN2
    0x02, // IN2
    0x06, // IN2+IN3
    0x04, // IN3
    0x0C, // IN3+IN4
    0x08, // IN4
    0x09  // IN4+IN1
};*/
static const uint8_t HALF_STEP_TABLE[8] = {
    0x08, // B1000 (Paso 0)
    0x0C, // B1100 (Paso 1)
    0x04, // B0100 (Paso 2)
    0x06, // B0110 (Paso 3)
    0x02, // B0010 (Paso 4)
    0x03, // B0011 (Paso 5)
    0x01, // B0001 (Paso 6)
    0x09  // B1001 (Paso 7)
};

/** * @brief Tabla para Full-Step (4 pasos).
 * Corregida para Bit0=IN1 -> Bit3=IN4.
 */
static const uint8_t FULL_STEP_TABLE[4] = {
    0x03, // IN1+IN2 (Paso 1)
    0x06, // IN2+IN3 (Paso 2)
    0x0C, // IN3+IN4 (Paso 3)
    0x09  // IN4+IN1 (Paso 4)
};

void Stepper_Init(Stepper_t* hstepper, GPIO_TypeDef* ports[], uint16_t pins[], Step_Mode_t mode) {
    for(int i = 0; i < 4; i++) {
        hstepper->GPIO_Ports[i] = ports[i];
        hstepper->GPIO_Pins[i] = pins[i];
    }
    hstepper->mode = mode;
    hstepper->current_step = 0;
    hstepper->direction = STEP_CW;
    hstepper->is_active = 0;
    Stepper_Stop(hstepper);
}

void Stepper_Set_Direction(Stepper_t* hstepper, Step_Dir_t dir) {
    hstepper->direction = dir;
}

void Stepper_Step_Sequential(Stepper_t* hstepper) {
    uint8_t pattern;
    uint8_t max_steps = (hstepper->mode == MODE_HALF_STEP) ? 8 : 4;

    // 1. Cálculo del siguiente paso según dirección
    if (hstepper->direction == STEP_CW) {
        hstepper->current_step++;
        if (hstepper->current_step >= max_steps) hstepper->current_step = 0;
    } else {
        hstepper->current_step--;
        if (hstepper->current_step < 0) hstepper->current_step = max_steps - 1;
    }

    // 2. Selección del patrón de bits según el modo
    if (hstepper->mode == MODE_HALF_STEP) {
        pattern = HALF_STEP_TABLE[hstepper->current_step];
    } else {
        pattern = FULL_STEP_TABLE[hstepper->current_step];
    }

    // 3. Escritura física en los pines (Lógica corregida tipo Arduino)
    for (int i = 0; i < 4; i++) {
        // Arduino lee bit 0 para Pin 1, bit 1 para Pin 2...
        // Antes hacíamos (0x08 >> i), ahora hacemos (0x01 << i)
        HAL_GPIO_WritePin(hstepper->GPIO_Ports[i], hstepper->GPIO_Pins[i],
                          (pattern & (0x01 << i)) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    }
    hstepper->is_active = 1;
}

void Stepper_Stop(Stepper_t* hstepper) {
    for (int i = 0; i < 4; i++) {
        HAL_GPIO_WritePin(hstepper->GPIO_Ports[i], hstepper->GPIO_Pins[i], GPIO_PIN_RESET);
    }
    hstepper->is_active = 0;
}
