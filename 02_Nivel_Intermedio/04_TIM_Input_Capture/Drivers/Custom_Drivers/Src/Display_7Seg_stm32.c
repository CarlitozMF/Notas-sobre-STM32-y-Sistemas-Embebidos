#include "Display_7Seg_stm32.h"
#include <string.h>

/** @brief Tabla BCD a 7-Segmentos (Cátodo Común) */
static const uint8_t segment_map[] = {
    0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F, 0x79, 0x50
};

static const uint8_t ascii_map[256] = {
    ['H'] = 0x76, ['E'] = 0x79, ['L'] = 0x38, ['P'] = 0x73,
    ['O'] = 0x3F, ['A'] = 0x77, ['C'] = 0x39, ['U'] = 0x3E,
    ['S'] = 0x6D, ['t'] = 0x78, ['r'] = 0x50, ['b'] = 0x7C,
    ['I'] = 0x06,
    ['n'] = 0x54, ['d'] = 0x5E, // Extras para "End" o "On"
    ['-'] = 0x40, [' '] = 0x00, ['0'] = 0x3F, ['1'] = 0x06,
    ['2'] = 0x5B, ['3'] = 0x4F, ['4'] = 0x66, ['5'] = 0x6D,
    ['6'] = 0x7D, ['7'] = 0x07, ['8'] = 0x7F, ['9'] = 0x6F
};

bool Display7Seg_Init(display_7seg_t* hdisplay, TIM_HandleTypeDef* htim,
                      display_pio_t* segments, display_pio_t* digits,
                      uint8_t count, uint8_t* buffer) {
    if (!hdisplay || !htim) return false;

    hdisplay->htim = htim;
    hdisplay->segments = segments;
    hdisplay->digits = digits;
    hdisplay->digitsCount = count;
    hdisplay->buffer = buffer;
    hdisplay->currentDigit = 0;
    hdisplay->brightness = 100;
    hdisplay->showLeadingZeros = false;
    hdisplay->decimalPointPos = 0;
    hdisplay->isEnabled = true;
    hdisplay->isFlashing = false;
    hdisplay->flashState = true;

    memset(hdisplay->buffer, 0, count);
    return (HAL_TIM_Base_Start_IT(hdisplay->htim) == HAL_OK);
}

void Display7Seg_WriteNumber(display_7seg_t* hdisplay, uint32_t number) {
    uint32_t temp = number;
    for (int8_t i = hdisplay->digitsCount - 1; i >= 0; i--) {
        if (temp == 0 && i < hdisplay->digitsCount - 1 && !hdisplay->showLeadingZeros) {
            hdisplay->buffer[i] = 0x00;
        } else {
            hdisplay->buffer[i] = segment_map[temp % 10];
        }
        temp /= 10;
        if (hdisplay->decimalPointPos == (hdisplay->digitsCount - i)) hdisplay->buffer[i] |= 0x80;
    }
}

void Display7Seg_WriteString(display_7seg_t* hdisplay, const char* str) {
    Display7Seg_Clear(hdisplay);
    for (uint8_t i = 0; i < hdisplay->digitsCount && str[i] != '\0'; i++) {
        hdisplay->buffer[i] = ascii_map[(uint8_t)str[i]];
    }
}

void Display7Seg_WriteError(display_7seg_t* hdisplay, uint8_t errorCode) {
    Display7Seg_Clear(hdisplay);
    if (hdisplay->digitsCount >= 3) {
        hdisplay->buffer[0] = segment_map[10]; // E
        hdisplay->buffer[1] = segment_map[11]; // r
        hdisplay->buffer[2] = segment_map[11]; // r
    }
}

void Display7Seg_Clear(display_7seg_t* hdisplay) {
    memset(hdisplay->buffer, 0, hdisplay->digitsCount);
}

void Display7Seg_SetBrightness(display_7seg_t* hdisplay, uint8_t level) {
    hdisplay->brightness = (level > 100) ? 100 : level;
}

void Display7Seg_SetFlash(display_7seg_t* hdisplay, uint16_t interval_ms) {
    if (interval_ms == 0) {
        hdisplay->isFlashing = false;
        hdisplay->flashState = true;
    } else {
        hdisplay->isFlashing = true;
        hdisplay->flashInterval = interval_ms;
        hdisplay->lastFlashTick = HAL_GetTick();
    }
}

void Display7Seg_SetDecimalPoint(display_7seg_t* hdisplay, uint8_t pos) {
    hdisplay->decimalPointPos = pos;
}

/**
 * @brief ISR de Multiplexación con PWM de Brillo y Lógica de Flash.
 */
void Display7Seg_Refresh_ISR(display_7seg_t* hdisplay) {
    static uint8_t pwm_cnt = 0;

    // 1. Lógica de Parpadeo (Flash)
    if (hdisplay->isFlashing) {
        if (HAL_GetTick() - hdisplay->lastFlashTick >= hdisplay->flashInterval) {
            hdisplay->lastFlashTick = HAL_GetTick();
            hdisplay->flashState = !hdisplay->flashState;
        }
    }

    // 2. Apagado de todos los comunes (Antigun ghosting)
    for (uint8_t i = 0; i < hdisplay->digitsCount; i++) {
        HAL_GPIO_WritePin(hdisplay->digits[i].port, hdisplay->digits[i].pin, GPIO_PIN_RESET);
    }

    // 3. Salida por PWM (Software) y Estado General
    if (!hdisplay->isEnabled || !hdisplay->flashState || (pwm_cnt >= (hdisplay->brightness / 10))) {
        pwm_cnt = (pwm_cnt + 1) % 10;
        return;
    }
    pwm_cnt = (pwm_cnt + 1) % 10;

    // 4. Escribir segmentos
    uint8_t p = hdisplay->buffer[hdisplay->currentDigit];
    for (uint8_t j = 0; j < 8; j++) {
        HAL_GPIO_WritePin(hdisplay->segments[j].port, hdisplay->segments[j].pin, (p & (1 << j)) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    }

    // 5. Encender común actual
    HAL_GPIO_WritePin(hdisplay->digits[hdisplay->currentDigit].port, hdisplay->digits[hdisplay->currentDigit].pin, GPIO_PIN_SET);

    hdisplay->currentDigit = (hdisplay->currentDigit + 1) % hdisplay->digitsCount;
}
