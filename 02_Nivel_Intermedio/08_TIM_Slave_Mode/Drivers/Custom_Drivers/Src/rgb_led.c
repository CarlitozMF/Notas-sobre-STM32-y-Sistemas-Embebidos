/**
 * @file rgb_led.c
 * @brief Implementación de la lógica agnóstica para el control de LEDs RGB.
 * @author Mamani Flores Carlos (UTN FRT)
 * @details Este archivo desarrolla la lógica de bajo nivel para la manipulación
 * de señales PWM a través de una PAL, aplicando transformaciones matemáticas
 * para mejorar la fidelidad del color y la percepción visual humana.
 *
 * @version 2.0
 * @date 2026
 */

#include "rgb_led.h"
#include <math.h>

/* --- Funciones Privadas (Internal Helpers) --- */

/**
 * @brief Aplica la corrección Gamma para linealizar la percepción lumínica humana.
 * @details Los LEDs no responden de forma lineal al ojo humano. Esta función aplica
 *          una curva de potencia (Exponente 2.2) para que los incrementos de brillo
 *          se perciban de manera uniforme.
 * @param value Valor de intensidad lógica (0 - 1000).
 * @param max_limit Límite de brillo máximo configurado para el LED.
 * @return uint16_t Valor corregido escalado para el duty cycle.
 */
static uint16_t apply_gamma(uint16_t value, uint16_t max_limit) {
    if (value == 0) return 0;

    // Saturación de seguridad por software
    uint16_t val = (value > max_limit) ? max_limit : value;

    // Normalización a rango [0.0, 1.0] y aplicación de la curva Gamma
    float normalized = (float)val / (float)RGB_MAX_PWM_VAL;
    float gamma_corrected = powf(normalized, RGB_GAMMA_COEFF);

    return (uint16_t)(gamma_corrected * RGB_MAX_PWM_VAL);
}

/**
 * @brief Escribe en la capa física mediante la abstracción PWM de la PAL.
 * @details Gestiona la inversión lógica necesaria para hardware de Ánodo Común
 *          y utiliza la inyección de dependencias para comunicarse con el silicio.
 * @param led Puntero a la instancia del LED.
 * @param ch Descriptor del canal PWM (R, G o B).
 * @param brightness_value Valor de brillo ya procesado (0-1000).
 */
static void write_hardware_raw(rgb_led_t *led, generic_pwm_t ch, uint16_t brightness_value) {
    if (!led->pal.pwm_write) return;

    uint16_t final_duty;

    /* Gestión de polaridad eléctrica */
    if (led->type == RGB_CATHODE_COMMON) {
        // Cátodo Común: Duty Cycle directamente proporcional (High enciende)
        final_duty = brightness_value;
    } else {
        // Ánodo Común: Duty Cycle inversamente proporcional (Low enciende)
        final_duty = (brightness_value == 0) ? RGB_MAX_PWM_VAL : (RGB_MAX_PWM_VAL - brightness_value);
    }

    // Ejecución del servicio inyectado por la plataforma
    led->pal.pwm_write(ch, final_duty);
}

/* --- API Pública --- */

/**
 * @brief Inicializa el objeto LED y vincula los servicios de la plataforma.
 */
void RGB_LED_Init(rgb_led_t *led, generic_pwm_t r, generic_pwm_t g, generic_pwm_t b,
                  rgb_type_t type, uint16_t max_br, hal_interface_t pal) {
    if (!led) return;

    /* Mapeo de hardware y límites */
    led->ch_r = r;
    led->ch_g = g;
    led->ch_b = b;
    led->type = type;
    led->max_br = max_br;

    /* Inyección de la PAL (Platform Abstraction Layer) */
    led->pal = pal;

    /* Inicialización de estados de motor de efectos */
    led->current_r = 0;
    led->current_g = 0;
    led->current_b = 0;
    led->active_effect = RGB_EFFECT_NONE;

    if (led->pal.get_tick) {
        led->last_tick = led->pal.get_tick();
    }

    /* Garantizar estado inicial seguro (OFF) */
    RGB_LED_SetColor(led, 0, 0, 0);
}

/**
 * @brief Establece color RGB aplicando procesamiento visual.
 */
void RGB_LED_SetColor(rgb_led_t *led, uint16_t r, uint16_t g, uint16_t b) {
    if (!led) return;

    /* Procesamiento de salida para los tres canales */
    write_hardware_raw(led, led->ch_r, apply_gamma(r, led->max_br));
    write_hardware_raw(led, led->ch_g, apply_gamma(g, led->max_br));
    write_hardware_raw(led, led->ch_b, apply_gamma(b, led->max_br));

    /* Sincronización de estado interno */
    led->current_r = r;
    led->current_g = g;
    led->current_b = b;
}

/**
 * @brief Algoritmo de conversión HSV a RGB para transiciones cromáticas naturales.
 */
void RGB_LED_SetHSV(rgb_led_t *led, float h, float s, float v) {
    if (!led) return;

    float r, g, b, c, x, m;

    h = fmodf(h, 360.0f);
    if (h < 0) h += 360.0f;

    c = v * s;
    x = c * (1.0f - fabsf(fmodf(h / 60.0f, 2.0f) - 1.0f));
    m = v - c;

    if      (h < 60)  { r = c; g = x; b = 0; }
    else if (h < 120) { r = x; g = c; b = 0; }
    else if (h < 180) { r = 0; g = c; b = x; }
    else if (h < 240) { r = 0; g = x; b = c; }
    else if (h < 300) { r = x; g = 0; b = c; }
    else              { r = c; g = 0; b = x; }

    RGB_LED_SetColor(led, (uint16_t)((r + m) * RGB_MAX_PWM_VAL),
                          (uint16_t)((g + m) * RGB_MAX_PWM_VAL),
                          (uint16_t)((b + m) * RGB_MAX_PWM_VAL));
}

/**
 * @brief Tarea periódica del motor de efectos (No bloqueante).
 * @details Utiliza aritmética de timers para ejecutar transiciones sin detener el CPU.
 */
void RGB_LED_Task(rgb_led_t *led, uint32_t interval_ms, uint16_t step) {
    if (!led || led->active_effect == RGB_EFFECT_NONE || !led->pal.get_tick) return;

    /* Aritmética de Ticks para multitasking cooperativo */
    if ((led->pal.get_tick() - led->last_tick) < interval_ms) return;
    led->last_tick = led->pal.get_tick();

    if (led->active_effect == RGB_EFFECT_RAINBOW) {
        uint16_t max = led->max_br;

        switch (led->fade_state) {
            case FADE_G_IN:
                led->current_g += step;
                if (led->current_g >= max) { led->current_g = max; led->fade_state = FADE_R_OUT; }
                break;
            case FADE_R_OUT:
                if (led->current_r <= step) { led->current_r = 0; led->fade_state = FADE_B_IN; }
                else led->current_r -= step;
                break;
            case FADE_B_IN:
                led->current_b += step;
                if (led->current_b >= max) { led->current_b = max; led->fade_state = FADE_G_OUT; }
                break;
            case FADE_G_OUT:
                if (led->current_g <= step) { led->current_g = 0; led->fade_state = FADE_R_IN; }
                else led->current_g -= step;
                break;
            case FADE_R_IN:
                led->current_r += step;
                if (led->current_r >= max) { led->current_r = max; led->fade_state = FADE_B_OUT; }
                break;
            case FADE_B_OUT:
                if (led->current_b <= step) { led->current_b = 0; led->fade_state = FADE_G_IN; }
                else led->current_b -= step;
                break;
        }
        RGB_LED_SetColor(led, led->current_r, led->current_g, led->current_b);
    }
}

/**
 * @brief Activa el efecto Rainbow configurando los puntos de inicio.
 */
void RGB_LED_StartRainbow(rgb_led_t *led) {
    if (!led) return;

    led->current_r = led->max_br;
    led->current_g = 0;
    led->current_b = 0;
    led->fade_state = FADE_G_IN;
    led->active_effect = RGB_EFFECT_RAINBOW;

    RGB_LED_SetColor(led, led->current_r, led->current_g, led->current_b);
}

/**
 * @brief Detiene cualquier proceso dinámico en la instancia.
 */
void RGB_LED_StopEffect(rgb_led_t *led) {
    if (led) led->active_effect = RGB_EFFECT_NONE;
}
