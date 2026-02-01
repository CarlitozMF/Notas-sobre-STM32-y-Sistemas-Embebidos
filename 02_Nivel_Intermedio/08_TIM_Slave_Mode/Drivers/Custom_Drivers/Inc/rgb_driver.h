/**
 * @file rgb_driver.h
 * @brief Driver modular y encapsulado para el control de LEDs RGB mediante PWM.
 * @author CarlitozMF - UTN FRT
 * @date 2026
 * * @details Este driver permite gestionar LEDs RGB (Ánodo o Cátodo Común) de forma
 * no bloqueante. Utiliza estructuras de configuración para facilitar la
 * inicialización y soporta efectos de color avanzados como HSV y Rainbow Fade.
 */

#ifndef __RGB_DRIVER_H
#define __RGB_DRIVER_H

#include "stm32f4xx_hal.h"
#include <stdint.h>

/** * @defgroup RGB_Config_Constants Constantes de Configuración
 * @{
 */
#define MAX_PWM_VALUE       1000   /**< Resolución del Timer (corresponde al ARR) */
#define GAMMA_CORRECTION    2.2f   /**< Coeficiente para corrección de brillo visual */
/** @} */

/**
 * @enum RGB_LED_Type_t
 * @brief Define la arquitectura de conexión del LED RGB.
 */
typedef enum {
    LED_TYPE_CATHODE_COMMON, /**< Cátodo a GND: Duty Cycle alto = Más brillo */
    LED_TYPE_ANODE_COMMON    /**< Ánodo a VCC: Duty Cycle bajo = Más brillo (Inverso) */
} RGB_LED_Type_t;

/**
 * @enum RGB_Effect_t
 * @brief Lista de efectos disponibles para el motor de efectos.
 */
typedef enum {
    EFFECT_NONE,          /**< Color estático, sin procesamiento de efectos */
    EFFECT_FADE_RAINBOW,  /**< Transición automática por todo el círculo cromático */
} RGB_Effect_t;

/**
 * @enum RGB_FadeState_t
 * @brief Estados internos de la máquina de estados para el efecto Rainbow.
 */
typedef enum {
    FADE_G_IN_RED_TO_YELLOW,   /**< Incrementando Verde sobre Rojo */
    FADE_R_OUT_YELLOW_TO_GREEN,/**< Decrementando Rojo sobre Amarillo */
    FADE_B_IN_GREEN_TO_CYAN,   /**< Incrementando Azul sobre Verde */
    FADE_G_OUT_CYAN_TO_BLUE,   /**< Decrementando Verde sobre Cian */
    FADE_R_IN_BLUE_TO_MAGENTA, /**< Incrementando Rojo sobre Azul */
    FADE_B_OUT_MAGENTA_TO_RED  /**< Decrementando Azul sobre Magenta */
} RGB_FadeState_t;

/**
 * @enum RGB_PresetColor_t
 * @brief Colores preestablecidos para acceso rápido.
 */
typedef enum {
    COLOR_RED, COLOR_GREEN, COLOR_BLUE, COLOR_YELLOW,
    COLOR_CYAN, COLOR_MAGENTA, COLOR_WHITE, COLOR_OFF
} RGB_PresetColor_t;

/**
 * @struct RGB_Config_t
 * @brief Paquete de configuración inicial (Encapsulamiento de Hardware).
 * * Esta estructura se utiliza únicamente durante la inicialización para
 * pasar todos los parámetros de hardware de una sola vez.
 */
typedef struct {
    TIM_HandleTypeDef *htim;  /**< Handle del Timer configurado en CubeMX */
    uint32_t R_channel;       /**< Canal asignado al Rojo (ej. TIM_CHANNEL_1) */
    uint32_t G_channel;       /**< Canal asignado al Verde (ej. TIM_CHANNEL_2) */
    uint32_t B_channel;       /**< Canal asignado al Azul (ej. TIM_CHANNEL_3) */
    RGB_LED_Type_t led_type;  /**< Polaridad del LED (Ánodo o Cátodo común) */
    uint16_t max_brightness;  /**< Límite de brillo máximo de 0 a 1000 */
} RGB_Config_t;

/**
 * @struct RGB_LED_t
 * @brief Objeto de estado del LED RGB.
 * * Contiene tanto los parámetros de hardware como las variables de estado
 * necesarias para la ejecución de efectos en tiempo real.
 */
typedef struct {
    // Copia de los parámetros de configuración
    TIM_HandleTypeDef *htim;
    uint32_t R_channel;
    uint32_t G_channel;
    uint32_t B_channel;
    RGB_LED_Type_t led_type;
    uint16_t max_brightness;

    // Variables de estado dinámico
    uint16_t current_r;       /**< Intensidad actual del Rojo (0-1000) */
    uint16_t current_g;       /**< Intensidad actual del Verde (0-1000) */
    uint16_t current_b;       /**< Intensidad actual del Azul (0-1000) */

    uint32_t last_tick_time;  /**< Registro de tiempo para control de velocidad de efectos */
    RGB_Effect_t active_effect; /**< Efecto activo actualmente */
    RGB_FadeState_t fade_state; /**< Estado actual de la máquina de estados de color */
} RGB_LED_t;

/* --- API del Driver (Prototipos) --- */

/**
 * @brief Inicializa el hardware y el objeto de estado mediante un paquete de configuración.
 * @param led_ptr Puntero al objeto de estado del LED.
 * @param config Puntero a la estructura con los parámetros de hardware.
 */
void RGB_Init_Single(RGB_LED_t *led_ptr, RGB_Config_t *config);

/**
 * @brief Establece un color RGB directo aplicando corrección Gamma.
 * @param led_ptr Puntero al LED.
 * @param r Componente Rojo (0-1000).
 * @param g Componente Verde (0-1000).
 * @param b Componente Azul (0-1000).
 */
void RGB_Set_Color_Direct(RGB_LED_t *led_ptr, uint16_t r, uint16_t g, uint16_t b);

/**
 * @brief Establece un color a partir de la lista de presets.
 * @param led_ptr Puntero al LED.
 * @param color Color predefinido (ej. COLOR_MAGENTA).
 */
void RGB_Set_Preset(RGB_LED_t *led_ptr, RGB_PresetColor_t color);

/**
 * @brief Establece el color mediante el modelo HSV (Tono, Saturación, Valor).
 * @param led_ptr Puntero al LED.
 * @param h Hue o Tono (0.0 - 360.0).
 * @param s Saturation o Saturación (0.0 - 1.0).
 * @param v Value o Brillo (0.0 - 1.0).
 */
void RGB_Set_Color_HSV(RGB_LED_t *led_ptr, float h, float s, float v);

/**
 * @brief Motor de efectos no bloqueante. Debe llamarse periódicamente en el Scheduler.
 * @param led_ptr Puntero al LED.
 * @param fade_interval_ms Tiempo en ms entre cada paso de actualización.
 * @param fade_step Cantidad de unidades (0-1000) que cambia el color por paso.
 */
void RGB_Effects_Handler(RGB_LED_t *led_ptr, uint32_t fade_interval_ms, uint16_t fade_step);

/**
 * @brief Inicia el efecto de arcoíris (Rainbow Fade).
 * @param led_ptr Puntero al LED.
 * @param initial_brightness Brillo máximo con el que se ejecutará el efecto.
 */
void RGB_Start_Effect_Rainbow(RGB_LED_t *led_ptr, uint16_t initial_brightness);

/**
 * @brief Detiene cualquier efecto activo y mantiene el color actual.
 * @param led_ptr Puntero al LED.
 */
void RGB_Stop_Effect(RGB_LED_t *led_ptr);

#endif /* __RGB_DRIVER_H */
