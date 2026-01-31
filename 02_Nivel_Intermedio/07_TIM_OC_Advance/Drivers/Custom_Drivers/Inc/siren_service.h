/**
 * @file siren_service.h
 * @brief Servicio de gestión de sirenas vehiculares y balizas estroboscópicas.
 * @author Carlos (CarlitozMF)
 */

#ifndef SIREN_SERVICE_H_
#define SIREN_SERVICE_H_

#include "main.h"
#include "buzzer_oc.h"

/**
 * @enum SirenMode_t
 * @brief Modos de operación de la sirena.
 */
typedef enum {
    MODE_OFF,       /*!< Sistema apagado */
    MODE_WAIL,      /*!< Ambulancia: Barrido de frecuencia lento */
    MODE_YELP,      /*!< Patrulla: Barrido de frecuencia rápido */
    MODE_HI_LO,     /*!< Europeo: Alternancia de dos tonos fijos */
    MODE_HORN       /*!< Bocina de aire (Manual) */
} SirenMode_t;

/**
 * @brief Inicializa el servicio de sirena vinculando los periféricos.
 * @param buzzer Puntero a la instancia del driver del buzzer.
 * @param htim_leds Puntero al Timer que maneja los LEDs (PWM).
 */
void Siren_Init(Buzzer_t *buzzer, TIM_HandleTypeDef *htim_leds);

/**
 * @brief Cambia el modo actual de la sirena.
 * @param new_mode Modo seleccionado de SirenMode_t.
 */
void Siren_SetMode(SirenMode_t new_mode);

/**
 * @brief Actualiza la lógica de modulación y luces.
 * @note Debe llamarse periódicamente en el while(1).
 */
void Siren_Update(void);

#endif /* SIREN_SERVICE_H_ */
