/**
 * @file    tcs3200_stm32.h
 * @author  Carlos - Estudiante UTN FRT
 * @brief   Driver para el sensor de color TCS3200 en STM32.
 * @version 2.0 (Estrategia Sampling)
 * @date    2026-01-31
 *
 * @note    Estrategia de Medición: "Muestreo Periódico".
 * - TIM4 (Esclavo): Cuenta pulsos continuamente en PB6.
 * - TIM3 (Maestro): Genera una interrupción periódica (500ms) para leer el TIM4.
 */

#ifndef INC_TCS3200_STM32_H_
#define INC_TCS3200_STM32_H_

#include "main.h"

/**
 * @defgroup TCS3200_Config Constantes de Configuración
 * @{
 */
//#define TCS_MEASURE_WINDOW_MS   500   /*!< Ventana de tiempo de medición en milisegundos (Debe coincidir con el periodo de TIM3) */
// Antes: 500. Ahora: 100.
#define TCS_MEASURE_WINDOW_MS   100
/** @} */

/**
 * @brief Selección de Filtros de Color (Control de Fotodiodos).
 * Determina el estado de los pines S2 y S3.
 */
typedef enum {
    TCS_FILTER_RED = 0,     /*!< Filtro ROJO (S2=L, S3=L) */
    TCS_FILTER_BLUE,        /*!< Filtro AZUL (S2=L, S3=H) */
    TCS_FILTER_CLEAR,       /*!< Sin Filtro  (S2=H, S3=L) */
    TCS_FILTER_GREEN        /*!< Filtro VERDE (S2=H, S3=H) */
} TCS_Filter_t;

/**
 * @brief Estado de los LEDs de iluminación frontal.
 */
typedef enum {
    TCS_LED_OFF = 0,        /*!< LEDs Apagados */
    TCS_LED_ON = 1          /*!< LEDs Encendidos */
} TCS_LedState_t;

/**
 * @brief Estructura Principal del Objeto Sensor.
 */
typedef struct {
    /* --- Handles de Hardware (HAL) --- */
    TIM_HandleTypeDef *htim_gate;   /*!< Timer Base de Tiempo (TIM3) que marca el ritmo de lectura */
    TIM_HandleTypeDef *htim_count;  /*!< Timer Contador (TIM4) conectado al pin OUT del sensor */

    /* --- Pines GPIO de Control --- */
    GPIO_TypeDef *S0_Port; uint16_t S0_Pin; /*!< Puerto/Pin S0 (Escala de Frecuencia) */
    GPIO_TypeDef *S1_Port; uint16_t S1_Pin; /*!< Puerto/Pin S1 (Escala de Frecuencia) */
    GPIO_TypeDef *S2_Port; uint16_t S2_Pin; /*!< Puerto/Pin S2 (Selección Filtro) */
    GPIO_TypeDef *S3_Port; uint16_t S3_Pin; /*!< Puerto/Pin S3 (Selección Filtro) */
    GPIO_TypeDef *LED_Port; uint16_t LED_Pin; /*!< Puerto/Pin LED (Iluminación) */

    /* --- Variables de Resultado (Frecuencia en Hz) --- */
    volatile uint32_t frequency_red;    /*!< Resultado: Frecuencia componente ROJA */
    volatile uint32_t frequency_green;  /*!< Resultado: Frecuencia componente VERDE */
    volatile uint32_t frequency_blue;   /*!< Resultado: Frecuencia componente AZUL */
    volatile uint32_t frequency_clear;  /*!< Resultado: Intensidad total (CLEAR) */

    /* --- Banderas de Estado --- */
    uint8_t measurement_ready;          /*!< Flag: Se pone en 1 cuando se completa una lectura válida */

} TCS3200_t;

/* --- Prototipos de Funciones --- */

/**
 * @brief  Inicializa la estructura del sensor.
 * @param  sensor Puntero a la instancia TCS3200_t.
 * @param  hgate  Handle del Timer de Base de Tiempo (ej. &htim3).
 * @param  hcount Handle del Timer Contador de Pulsos (ej. &htim4).
 */
void TCS3200_Init(TCS3200_t *sensor, TIM_HandleTypeDef *hgate, TIM_HandleTypeDef *hcount);

/**
 * @brief  Configura los pines GPIO asociados al sensor.
 * @note   Esta función abstrae el hardware para facilitar la portabilidad.
 */
void TCS3200_ConfigGPIO(TCS3200_t *sensor,
                        GPIO_TypeDef* s0_p, uint16_t s0, GPIO_TypeDef* s1_p, uint16_t s1,
                        GPIO_TypeDef* s2_p, uint16_t s2, GPIO_TypeDef* s3_p, uint16_t s3,
                        GPIO_TypeDef* led_p, uint16_t led);

/**
 * @brief  Cambia el filtro de color activo.
 * @param  sensor Puntero al sensor.
 * @param  filter Opción del enum TCS_Filter_t (RED, GREEN, BLUE, CLEAR).
 */
void TCS3200_SetFilter(TCS3200_t *sensor, TCS_Filter_t filter);

/**
 * @brief  Enciende o apaga los LEDs frontales.
 * @param  sensor Puntero al sensor.
 * @param  state  TCS_LED_ON o TCS_LED_OFF.
 */
void TCS3200_ControlLED(TCS3200_t *sensor, TCS_LedState_t state);

/**
 * @brief  Arranca el proceso de medición continua.
 * @note   Inicia ambos Timers e habilita las interrupciones necesarias.
 * @param  sensor Puntero al sensor.
 */
void TCS3200_StartMeasurement(TCS3200_t *sensor);

/**
 * @brief  Callback de procesamiento de datos.
 * @note   **IMPORTANTE:** Llamar a esta función dentro de HAL_TIM_PeriodElapsedCallback() en main.c
 * cuando ocurra la interrupción del Timer Base (TIM3).
 * @param  sensor Puntero al sensor.
 */
void TCS3200_ProcessCallback(TCS3200_t *sensor);

#endif /* INC_TCS3200_STM32_H_ */
