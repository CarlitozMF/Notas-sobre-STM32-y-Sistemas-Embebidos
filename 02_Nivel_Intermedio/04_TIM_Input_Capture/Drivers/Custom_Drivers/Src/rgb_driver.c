/**
 * @file rgb_driver.c
 * @brief Implementación lógica del driver RGB con encapsulamiento y efectos.
 * @author CarlitozMF - UTN FRT
 * @date 2026
 * * @details Este archivo contiene la lógica de bajo nivel para la manipulación
 * de registros PWM, conversión de modelos de color HSV a RGB, corrección
 * Gamma y la máquina de estados para efectos no bloqueantes.
 */

#include "rgb_driver.h"
#include <math.h>

/* --- Funciones Privadas (Internal Helpers) --- */

/**
 * @brief Aplica la corrección Gamma para linealizar la percepción lumínica humana.
 * @details Los LEDs no se perciben de forma lineal. Esta función aplica una curva
 * de potencia (exponente 2.2) para que los cambios de brillo se vean uniformes.
 * @param value Valor de intensidad lógica (0 - 1000).
 * @param max_limit Límite de brillo máximo configurado para el LED.
 * @return uint16_t Valor corregido listo para el registro CCR del Timer.
 */
static uint16_t RGB_Apply_Gamma(uint16_t value, uint16_t max_limit) {
    if (value == 0) return 0;
    if (value > max_limit) value = max_limit;

    // Normalización (0.0 a 1.0) y aplicación de la curva Gamma
    float normalized = (float)value / (float)MAX_PWM_VALUE;
    float gamma_corrected = powf(normalized, GAMMA_CORRECTION);

    return (uint16_t)(gamma_corrected * MAX_PWM_VALUE);
}

/**
 * @brief Escribe directamente en el registro CCR del hardware.
 * @details Abstrae la capa física gestionando la inversión lógica necesaria
 * para LEDs de Ánodo Común.
 * @param led_ptr Puntero a la instancia del LED.
 * @param channel Canal del Timer (TIM_CHANNEL_x).
 * @param brightness_value Valor de brillo corregido por Gamma (0-1000).
 */
static void RGB_Set_Duty_Cycle_Raw(RGB_LED_t *led_ptr, uint32_t channel, uint16_t brightness_value) {
    if (!led_ptr || !led_ptr->htim) return;

    uint16_t final_duty;

    if (led_ptr->led_type == LED_TYPE_CATHODE_COMMON) {
        // Cátodo Común: Duty Cycle directamente proporcional al brillo
        final_duty = brightness_value;
    } else {
        // Ánodo Común: Duty Cycle inversamente proporcional al brillo
        // Se asegura el apagado total forzando el 100% del ciclo en 0 lógico
        final_duty = (brightness_value == 0) ? MAX_PWM_VALUE : (MAX_PWM_VALUE - brightness_value);
    }

    __HAL_TIM_SET_COMPARE(led_ptr->htim, channel, final_duty);
}

/* --- API Pública --- */

/**
 * @brief Inicializa el LED RGB y asocia la configuración de hardware.
 * @param led_ptr Puntero al objeto de estado del LED.
 * @param config Puntero a la estructura con los parámetros de hardware.
 */
void RGB_Init_Single(RGB_LED_t *led_ptr, RGB_Config_t *config) {
    if (!led_ptr || !config) return;

    // Desencapsulado: Se copian los parámetros estáticos al objeto dinámico
    led_ptr->htim        = config->htim;
    led_ptr->R_channel   = config->R_channel;
    led_ptr->G_channel   = config->G_channel;
    led_ptr->B_channel   = config->B_channel;
    led_ptr->led_type    = config->led_type;
    led_ptr->max_brightness = config->max_brightness;

    // Reset de estados internos
    led_ptr->current_r = 0;
    led_ptr->current_g = 0;
    led_ptr->current_b = 0;
    led_ptr->active_effect = EFFECT_NONE;
    led_ptr->last_tick_time = HAL_GetTick();

    // Activación de la generación PWM por hardware
    HAL_TIM_PWM_Start(led_ptr->htim, led_ptr->R_channel);
    HAL_TIM_PWM_Start(led_ptr->htim, led_ptr->G_channel);
    HAL_TIM_PWM_Start(led_ptr->htim, led_ptr->B_channel);

    // Estado inicial seguro (OFF)
    RGB_Set_Color_Direct(led_ptr, 0, 0, 0);
}

/**
 * @brief Establece un color RGB aplicando Gamma y brillo máximo.
 * @param led_ptr Puntero al LED.
 * @param r Intensidad roja (0-1000).
 * @param g Intensidad verde (0-1000).
 * @param b Intensidad azul (0-1000).
 */
void RGB_Set_Color_Direct(RGB_LED_t *led_ptr, uint16_t r, uint16_t g, uint16_t b) {
    if (!led_ptr) return;

    // Procesamiento visual y de límites
    uint16_t rf = RGB_Apply_Gamma(r, led_ptr->max_brightness);
    uint16_t gf = RGB_Apply_Gamma(g, led_ptr->max_brightness);
    uint16_t bf = RGB_Apply_Gamma(b, led_ptr->max_brightness);

    // Escritura en hardware
    RGB_Set_Duty_Cycle_Raw(led_ptr, led_ptr->R_channel, rf);
    RGB_Set_Duty_Cycle_Raw(led_ptr, led_ptr->G_channel, gf);
    RGB_Set_Duty_Cycle_Raw(led_ptr, led_ptr->B_channel, bf);

    // Actualización de variables de seguimiento de estado
    led_ptr->current_r = r;
    led_ptr->current_g = g;
    led_ptr->current_b = b;
}

/**
 * @brief Establece un color predefinido.
 * @param led_ptr Puntero al LED.
 * @param color Identificador del color (RGB_PresetColor_t).
 */
void RGB_Set_Preset(RGB_LED_t *led_ptr, RGB_PresetColor_t color) {
    uint16_t max = led_ptr->max_brightness;
    switch (color) {
        case COLOR_RED:     RGB_Set_Color_Direct(led_ptr, max, 0, 0); break;
        case COLOR_GREEN:   RGB_Set_Color_Direct(led_ptr, 0, max, 0); break;
        case COLOR_BLUE:    RGB_Set_Color_Direct(led_ptr, 0, 0, max); break;
        case COLOR_YELLOW:  RGB_Set_Color_Direct(led_ptr, max, max, 0); break;
        case COLOR_CYAN:    RGB_Set_Color_Direct(led_ptr, 0, max, max); break;
        case COLOR_MAGENTA: RGB_Set_Color_Direct(led_ptr, max, 0, max); break;
        case COLOR_WHITE:   RGB_Set_Color_Direct(led_ptr, max, max, max); break;
        case COLOR_OFF:     RGB_Set_Color_Direct(led_ptr, 0, 0, 0); break;
        default: break;
    }
}

/**
 * @brief Convierte el espacio de color HSV a RGB para transiciones naturales.
 * @param led_ptr Puntero al LED.
 * @param h Tono (0.0 - 360.0).
 * @param s Saturación (0.0 - 1.0).
 * @param v Valor/Brillo (0.0 - 1.0).
 */
void RGB_Set_Color_HSV(RGB_LED_t *led_ptr, float h, float s, float v) {
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

    RGB_Set_Color_Direct(led_ptr, (uint16_t)((r + m) * MAX_PWM_VALUE),
                                  (uint16_t)((g + m) * MAX_PWM_VALUE),
                                  (uint16_t)((b + m) * MAX_PWM_VALUE));
}

/**
 * @brief Orquestador de efectos de tiempo real (No bloqueante).
 * @details Gestiona la máquina de estados del color y el timing independiente.
 * @param led_ptr Puntero al LED.
 * @param fade_interval_ms Velocidad del efecto (ms por paso).
 * @param fade_step Cantidad de unidades de incremento por paso.
 */
void RGB_Effects_Handler(RGB_LED_t *led_ptr, uint32_t fade_interval_ms, uint16_t fade_step) {
    if (!led_ptr || led_ptr->active_effect == EFFECT_NONE) return;

    // Control de flujo temporal mediante aritmética de ticks
    if ((HAL_GetTick() - led_ptr->last_tick_time) < fade_interval_ms) return;
    led_ptr->last_tick_time = HAL_GetTick();

    if (led_ptr->active_effect == EFFECT_FADE_RAINBOW) {
        uint16_t max = led_ptr->max_brightness;

        switch (led_ptr->fade_state) {
            case FADE_G_IN_RED_TO_YELLOW:
                led_ptr->current_g += fade_step;
                if (led_ptr->current_g >= max) { led_ptr->current_g = max; led_ptr->fade_state = FADE_R_OUT_YELLOW_TO_GREEN; }
                break;
            case FADE_R_OUT_YELLOW_TO_GREEN:
                if (led_ptr->current_r <= fade_step) { led_ptr->current_r = 0; led_ptr->fade_state = FADE_B_IN_GREEN_TO_CYAN; }
                else led_ptr->current_r -= fade_step;
                break;
            case FADE_B_IN_GREEN_TO_CYAN:
                led_ptr->current_b += fade_step;
                if (led_ptr->current_b >= max) { led_ptr->current_b = max; led_ptr->fade_state = FADE_G_OUT_CYAN_TO_BLUE; }
                break;
            case FADE_G_OUT_CYAN_TO_BLUE:
                if (led_ptr->current_g <= fade_step) { led_ptr->current_g = 0; led_ptr->fade_state = FADE_R_IN_BLUE_TO_MAGENTA; }
                else led_ptr->current_g -= fade_step;
                break;
            case FADE_R_IN_BLUE_TO_MAGENTA:
                led_ptr->current_r += fade_step;
                if (led_ptr->current_r >= max) { led_ptr->current_r = max; led_ptr->fade_state = FADE_B_OUT_MAGENTA_TO_RED; }
                break;
            case FADE_B_OUT_MAGENTA_TO_RED:
                if (led_ptr->current_b <= fade_step) { led_ptr->current_b = 0; led_ptr->fade_state = FADE_G_IN_RED_TO_YELLOW; }
                else led_ptr->current_b -= fade_step;
                break;
        }
        RGB_Set_Color_Direct(led_ptr, led_ptr->current_r, led_ptr->current_g, led_ptr->current_b);
    }
}

/**
 * @brief Configura e inicia el efecto Rainbow Fade.
 * @param led_ptr Puntero al LED.
 * @param initial_brightness Brillo máximo deseado para el efecto.
 */
void RGB_Start_Effect_Rainbow(RGB_LED_t *led_ptr, uint16_t initial_brightness) {
    if (!led_ptr) return;
    RGB_Stop_Effect(led_ptr);

    led_ptr->max_brightness = initial_brightness;
    led_ptr->current_r = initial_brightness;
    led_ptr->current_g = 0;
    led_ptr->current_b = 0;
    led_ptr->fade_state = FADE_G_IN_RED_TO_YELLOW;
    led_ptr->active_effect = EFFECT_FADE_RAINBOW;
}

/**
 * @brief Detiene cualquier efecto en curso.
 * @param led_ptr Puntero al LED.
 */
void RGB_Stop_Effect(RGB_LED_t *led_ptr) {
    if (led_ptr) led_ptr->active_effect = EFFECT_NONE;
}
