/**
 * @file siren_service.h
 * @author Mamani Flores Carlos (UTN FRT)
 * @brief Servicio modular para la orquestación cinemática de sirenas y luces de advertencia.
 * @details Totalmente desacoplado de fabricantes. Opera coordinando objetos generadores
 * de tonos y descriptores PWM mediante servicios inyectados de plataforma.
 * @version 2.0
 * @date 2026
 */

#ifndef SIREN_SERVICE_H_
#define SIREN_SERVICE_H_

#include "tone_generator.h"

/**
 * @enum SirenMode_t
 * @brief Modos de operación de la sirena vehicular.
 */
typedef enum {
    MODE_OFF,       /**< Sistema en reposo absoluto */
    MODE_WAIL,      /**< Patrón Ambulancia: Barrido lineal lento */
    MODE_YELP,      /**< Patrón Patrulla: Barrido lineal rápido */
    MODE_HI_LO,     /**< Patrón Europeo: Alternancia bitonal discreta */
	MODE_WAR,       /**< Sirena de ataque aéreo (Rampa pesada de alta inercia) */
    MODE_HORN       /**< Bocina de aire manual */
} SirenMode_t;

/**
 * @struct siren_service_t
 * @brief Estructura de control que encapsula el estado dinámico de un sistema de sirena.
 */
typedef struct {
    tone_gen_t      *tone_gen;         /**< Puntero al objeto de Capa 2 encargado del audio */
    generic_pwm_t   led_ch1;           /**< Descriptor genérico del canal de baliza izquierdo */
    generic_pwm_t   led_ch2;           /**< Descriptor genérico del canal de baliza derecho */
    hal_interface_t pal;               /**< Interfaz de servicios del sistema */
    SirenMode_t     current_mode;      /**< Modo operativo actual */
    uint32_t        last_siren_tick;   /**< Historial de tiempo de modulación de audio */
    uint32_t        last_led_tick;     /**< Historial de tiempo del multiplexado estroboscópico */
    uint32_t        current_freq;      /**< Frecuencia de audio instantánea en ejecución */
    int16_t         freq_step;         /**< Delta de frecuencia para el perfil de barrido */
} siren_service_t;

/**
 * @brief Inicializa el servicio de sirena vinculando los periféricos abstractos.
 */
void SIREN_SERVICE_Init(siren_service_t *siren, tone_gen_t *tg, generic_pwm_t led1, generic_pwm_t led2, hal_interface_t pal_io);

/**
 * @brief Transiciona el servicio hacia un nuevo modo de operación de forma segura.
 */
void SIREN_SERVICE_SetMode(siren_service_t *siren, SirenMode_t new_mode);

/**
 * @brief Actualiza las máquinas de estado de audio y destellos lógicos. Invocar en el loop cooperativo.
 */
void SIREN_SERVICE_Update(siren_service_t *siren);

#endif /* SIREN_SERVICE_H_ */
