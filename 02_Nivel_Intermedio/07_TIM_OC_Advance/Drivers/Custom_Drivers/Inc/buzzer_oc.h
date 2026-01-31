/**
 * @file buzzer_oc.h
 * @brief Driver encapsulado para la generación de tonos mediante Output Compare.
 * @author Carlos (CarlitozMF)
 * @date 2026
 *
 * Este driver permite generar frecuencias precisas utilizando el modo Toggle
 * de los temporizadores del STM32, permitiendo un control asíncrono y
 * determinístico del sonido.
 */

#ifndef BUZZER_OC_H_
#define BUZZER_OC_H_

#include "stm32f4xx_hal.h"

/** * @struct Buzzer_t
 * @brief Estructura de control para la instancia del Buzzer.
 */
typedef struct {
    TIM_HandleTypeDef *htim;    /*!< Puntero al manejador del Timer de la HAL */
    uint32_t channel;           /*!< Canal del Timer configurado en modo OC */
    uint32_t timer_clk_freq;    /*!< Frecuencia de entrada al Timer (f_clk) */
    uint32_t current_ticks;     /*!< Ticks necesarios para el semiperiodo actual */
    uint8_t  is_active;         /*!< Indicador de estado: 1 = Sonando, 0 = Silencio */
} Buzzer_t;

/**
 * @brief Inicializa el objeto Buzzer vinculándolo al hardware.
 * @param buzzer Puntero a la estructura del buzzer.
 * @param htim Puntero al Timer (ej. &htim5).
 * @param channel Canal del Timer (ej. TIM_CHANNEL_1).
 * @param bus_freq Frecuencia del bus de reloj en Hz (considerar multiplicadores de Timers).
 */
void Buzzer_Init(Buzzer_t *buzzer, TIM_HandleTypeDef *htim, uint32_t channel, uint32_t bus_freq);

/**
 * @brief Establece una frecuencia específica en el Buzzer.
 * @param buzzer Puntero a la estructura del buzzer.
 * @param freq Frecuencia deseada en Hz. Si freq = 0, se detiene el sonido.
 * @note La precisión depende de la configuración del Prescaler del Timer.
 */
void Buzzer_SetFrequency(Buzzer_t *buzzer, uint32_t freq);

/**
 * @brief Detiene la generación de señal (Silencio).
 * @param buzzer Puntero a la estructura del buzzer.
 */
void Buzzer_Stop(Buzzer_t *buzzer);

/**
 * @brief Manejador de la interrupción de Output Compare.
 * @param buzzer Puntero a la estructura del buzzer.
 * @warning Debe ser llamado dentro de HAL_TIM_OC_DelayElapsedCallback.
 */
void Buzzer_IRQ_Handler(Buzzer_t *buzzer);

#endif /* BUZZER_OC_H_ */
