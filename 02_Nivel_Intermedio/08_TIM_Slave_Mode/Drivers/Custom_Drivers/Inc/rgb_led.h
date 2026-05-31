/**
 * @file rgb_led.h
 * @brief Driver agnóstico y modular para el control de LEDs RGB mediante PWM.
 * @author Mamani Flores Carlos (UTN FRT).
 *
 * @details Este driver implementa una lógica de control de color para LEDs RGB
 * utilizando una capa de abstracción (PAL). Soporta corrección Gamma,
 * conversión de espacio de color HSV a RGB y efectos no bloqueantes.
 * Permite gestionar múltiples instancias de LEDs de forma independiente.
 *
 * @version 2.0
 * @date 2026
 */

#ifndef RGB_LED_H
#define RGB_LED_H

#include "hal_interface.h"
#include <stdint.h>

/**
 * @defgroup RGB_Constants Parámetros de Diseño Lumínico
 * @{
 */
#define RGB_MAX_PWM_VAL     1000   /**< Resolución máxima lógica para el Duty Cycle (0-1000) */
#define RGB_GAMMA_COEFF     2.2f   /**< Coeficiente de corrección Gamma para linealidad visual */
/** @} */

/**
 * @enum rgb_type_t
 * @brief Define la configuración eléctrica del hardware.
 */
typedef enum {
    RGB_CATHODE_COMMON, /**< Cátodo común: Nivel ALTO enciende el segmento */
    RGB_ANODE_COMMON    /**< Ánodo común: Nivel BAJO enciende el segmento (Lógica inversa) */
} rgb_type_t;

/**
 * @enum rgb_effect_t
 * @brief Modos de operación del motor de efectos.
 */
typedef enum {
    RGB_EFFECT_NONE,    /**< Modo estático: El color se mantiene fijo */
    RGB_EFFECT_RAINBOW, /**< Modo dinámico: Ciclo automático por el círculo cromático */
} rgb_effect_t;

/**
 * @enum rgb_fade_state_t
 * @brief Estados internos para la máquina de estados del efecto Rainbow.
 */
typedef enum {
    FADE_G_IN,  /**< Transición: Rojo -> Amarillo */
    FADE_R_OUT, /**< Transición: Amarillo -> Verde */
    FADE_B_IN,  /**< Transición: Verde -> Cian */
    FADE_G_OUT, /**< Transición: Cian -> Azul */
    FADE_R_IN,  /**< Transición: Azul -> Magenta */
    FADE_B_OUT  /**< Transición: Magenta -> Rojo */
} rgb_fade_state_t;

/**
 * @struct rgb_led_t
 * @brief Descriptor de objeto para una instancia de LED RGB.
 *
 * @details Contiene el mapeo de hardware, los punteros a servicios de la plataforma (PAL)
 * y las variables de estado necesarias para el control de efectos y brillo.
 */
typedef struct {
    /** @name Mapeo de Hardware */
    /**@{*/
    generic_pwm_t ch_r;      /**< Descriptor de canal PWM para Rojo */
    generic_pwm_t ch_g;      /**< Descriptor de canal PWM para Verde */
    generic_pwm_t ch_b;      /**< Descriptor de canal PWM para Azul */
    rgb_type_t    type;      /**< Configuración de polaridad (Ánodo/Cátodo) */
    uint16_t      max_br;    /**< Límite de brillo máximo de la instancia (0-1000) */
    /**@}*/

    /** @name Abstracción de Plataforma */
    hal_interface_t pal;     /**< Interfaz con las funciones específicas del silicio */

    /** @name Estado Dinámico Interno */
    /**@{*/
    uint16_t current_r;      /**< Valor actual de intensidad Roja (0-1000) */
    uint16_t current_g;      /**< Valor actual de intensidad Verde (0-1000) */
    uint16_t current_b;      /**< Valor actual de intensidad Azul (0-1000) */
    uint32_t last_tick;      /**< Marca de tiempo para control de velocidad de efectos */
    rgb_effect_t active_effect;   /**< Efecto que se está ejecutando actualmente */
    rgb_fade_state_t fade_state;  /**< Estado de la máquina de color actual */
    /**@}*/
} rgb_led_t;

/* --- API Pública --- */

/**
 * @brief Inicializa una instancia de LED RGB y la vincula con la plataforma.
 * @param led Puntero a la estructura del LED.
 * @param r Configuración del canal PWM para el Rojo.
 * @param g Configuración del canal PWM para el Verde.
 * @param b Configuración del canal PWM para el Azul.
 * @param type Polaridad eléctrica del hardware.
 * @param max_br Brillo máximo permitido para esta instancia.
 * @param pal Estructura de servicios de la plataforma (punteros a funciones).
 */
void RGB_LED_Init(rgb_led_t *led, generic_pwm_t r, generic_pwm_t g, generic_pwm_t b,
                  rgb_type_t type, uint16_t max_br, hal_interface_t pal);

/**
 * @brief Establece un color RGB directo con corrección Gamma.
 * @param led Puntero a la instancia.
 * @param r Intensidad roja (0-1000).
 * @param g Intensidad verde (0-1000).
 * @param b Intensidad azul (0-1000).
 */
void RGB_LED_SetColor(rgb_led_t *led, uint16_t r, uint16_t g, uint16_t b);

/**
 * @brief Establece el color utilizando el modelo HSV.
 * @param led Puntero a la instancia.
 * @param h Tono (0.0 - 360.0).
 * @param s Saturación (0.0 - 1.0).
 * @param v Valor/Brillo (0.0 - 1.0).
 */
void RGB_LED_SetHSV(rgb_led_t *led, float h, float s, float v);

/**
 * @brief Tarea de actualización de efectos. Debe llamarse periódicamente.
 * @param led Puntero a la instancia.
 * @param interval_ms Tiempo entre pasos del efecto (ms).
 * @param step Salto de intensidad por cada paso.
 */
void RGB_LED_Task(rgb_led_t *led, uint32_t interval_ms, uint16_t step);

/**
 * @brief Inicia el efecto de arcoíris (Rainbow).
 * @param led Puntero a la instancia.
 */
void RGB_LED_StartRainbow(rgb_led_t *led);

/**
 * @brief Detiene el efecto actual y mantiene el color último color.
 * @param led Puntero a la instancia.
 */
void RGB_LED_StopEffect(rgb_led_t *led);

#endif /* RGB_LED_H */
