#include "siren_service.h"

/* Punteros privados */
static Buzzer_t *pBuzzer;
static TIM_HandleTypeDef *phLedTimer;
static SirenMode_t current_mode = MODE_OFF;

/* Variables de estado globales al archivo (Static) */
static uint32_t last_siren_tick = 0;
static uint32_t last_led_tick = 0;   // <--- Aquí se define y se usa abajo
static uint32_t current_freq = 600;
static int16_t  freq_step = 10;      // int16 para permitir valores negativos

void Siren_Init(Buzzer_t *buzzer, TIM_HandleTypeDef *htim_leds) {
    pBuzzer = buzzer;
    phLedTimer = htim_leds;

    HAL_TIM_PWM_Start(phLedTimer, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(phLedTimer, TIM_CHANNEL_2);
}

void Siren_SetMode(SirenMode_t new_mode) {
    current_mode = new_mode;

    // Reset de seguridad al cambiar de modo
    last_siren_tick = HAL_GetTick();
    last_led_tick = HAL_GetTick();

    if (new_mode == MODE_OFF) {
        Buzzer_Stop(pBuzzer);
        __HAL_TIM_SET_COMPARE(phLedTimer, TIM_CHANNEL_1, 0);
        __HAL_TIM_SET_COMPARE(phLedTimer, TIM_CHANNEL_2, 0);
    } else {
        current_freq = 600;
        // Ajustamos el paso inicial según el modo
        freq_step = (new_mode == MODE_YELP) ? 40 : 12;
    }
}

void Siren_Update(void) {
    if (current_mode == MODE_OFF) return;

    uint32_t now = HAL_GetTick();

    /* --- LOGICA DE AUDIO (Barrido de Frecuencia) --- */
    if (now - last_siren_tick >= 20) {
        last_siren_tick = now;

        switch (current_mode) {
            case MODE_WAIL: // Ambulancia lenta
                current_freq += freq_step;
                if (current_freq >= 1400 || current_freq <= 600) freq_step *= -1;
                Buzzer_SetFrequency(pBuzzer, current_freq);
                break;

            case MODE_YELP: // Patrulla rápida
                current_freq += freq_step;
                if (current_freq >= 1600 || current_freq <= 700) freq_step *= -1;
                Buzzer_SetFrequency(pBuzzer, current_freq);
                break;

            case MODE_HI_LO: // Bitonal
                if ((now / 500) % 2 == 0) Buzzer_SetFrequency(pBuzzer, 700);
                else Buzzer_SetFrequency(pBuzzer, 1100);
                break;

            default: break;
        }
    }

    /* --- LOGICA DE LEDS (Sincronización) --- */
    // Determinamos la velocidad del parpadeo según el modo
    uint32_t led_interval = (current_mode == MODE_WAIL) ? 400 : 150;

    if (now - last_led_tick >= led_interval) {
        last_led_tick = now; // <--- AQUÍ SE USA, eliminando el Warning
        static uint8_t toggle = 0;
        toggle = !toggle;

        if (toggle) {
            __HAL_TIM_SET_COMPARE(phLedTimer, TIM_CHANNEL_1, 800);
            __HAL_TIM_SET_COMPARE(phLedTimer, TIM_CHANNEL_2, 0);
        } else {
            __HAL_TIM_SET_COMPARE(phLedTimer, TIM_CHANNEL_1, 0);
            __HAL_TIM_SET_COMPARE(phLedTimer, TIM_CHANNEL_2, 800);
        }
    }
}
