/**
 * @file tcs3200.h
 * @author Mamani Flores Carlos (UTN FRT)
 * @brief Driver genérico, instanciable y multiplataforma para el sensor TCS3200.
 * @details Implementa la digitalización de color mediante conteo de pulsos por ventana de tiempo.
 * Se independiza del silicio mediante inyección de dependencias con una PAL.
 * @version 2.0 (Sampling Genérico)
 * @date 2026
 */

#ifndef INC_TCS3200_H_
#define INC_TCS3200_H_

#include "hal_interface.h"
#include <stdint.h>

/**
 * @defgroup TCS3200_Config Constantes de Configuración del Driver
 * @{
 */
#define TCS_MEASURE_WINDOW_MS   100  /**< Ventana de tiempo de muestreo isócrono (ms) */
/** @} */

/**
 * @brief Selección de Filtros Ópticos (Control de Fotodiodos).
 */
typedef enum {
    TCS_FILTER_RED = 0,     /**< Filtro ROJO  (S2=0, S3=0) */
    TCS_FILTER_BLUE,        /**< Filtro AZUL  (S2=0, S3=1) */
    TCS_FILTER_CLEAR,       /**< Sin Filtro   (S2=1, S3=0) */
    TCS_FILTER_GREEN        /**< Filtro VERDE (S2=1, S3=1) */
} TCS_Filter_t;

/**
 * @brief Estado de la iluminación LED frontal.
 */
typedef enum {
    TCS_LED_OFF = 0,        /**< LEDs Apagados */
    TCS_LED_ON = 1          /**< LEDs Encendidos */
} TCS_LedState_t;

/**
 * @brief Estructura de control del objeto TCS3200 (Multiplataforma).
 */
typedef struct {
    /* --- Capa de Abstracción de Hardware (PAL) --- */
    hal_interface_t pal;            /**< Tabla de despacho de la PAL Universal */

    /* --- Descriptores de Hardware Genéricos (void*) --- */
    generic_pwm_t   ic_count_ch;    /**< Canal/Timer asignado como contador de pulsos */

    /* --- Pines GPIO Abstractos --- */
    generic_gpio_t pin_s0;          /**< Pin S0 (Escala de Frecuencia) */
    generic_gpio_t pin_s1;          /**< Pin S1 (Escala de Frecuencia) */
    generic_gpio_t pin_s2;          /**< Pin S2 (Selección de Filtro) */
    generic_gpio_t pin_s3;          /**< Pin S3 (Selección de Filtro) */
    generic_gpio_t pin_led;         /**< Pin LED (Iluminación Frontal) */

    /* --- Variables Atómicas de Frecuencia (Hz) --- */
    volatile uint32_t frequency_red;    /**< Frecuencia calculada Canal Rojo */
    volatile uint32_t frequency_green;  /**< Frecuencia calculada Canal Verde */
    volatile uint32_t frequency_blue;   /**< Frecuencia calculada Canal Azul */
    volatile uint32_t frequency_clear;  /**< Frecuencia calculada Canal Clear */

    /* --- Banderas de Estado / Sincronismo --- */
    uint8_t measurement_ready;          /**< Flag de ciclo completo para Capa 3 */
} TCS3200_t;

/* --- API Pública del Driver Agnóstico --- */

/**
 * @brief Inicializa la estructura interna del objeto sensor.
 * @param sensor Puntero a la instancia del sensor.
 * @param pal_io Estructura con la tabla de despacho de funciones de hardware.
 * @param count_ch Descriptor del canal/timer que operará como contador.
 */
void TCS3200_Init(TCS3200_t *sensor, hal_interface_t pal_io, generic_pwm_t count_ch);

/**
 * @brief Configura y vincula los pines GPIO abstractos al objeto sensor.
 */
void TCS3200_ConfigGPIO(TCS3200_t *sensor,
                        generic_gpio_t s0, generic_gpio_t s1,
                        generic_gpio_t s2, generic_gpio_t s3,
                        generic_gpio_t led);

/**
 * @brief Cambia el filtro óptico activo escribiendo en S2 y S3 a través de la PAL.
 */
void TCS3200_SetFilter(TCS3200_t *sensor, TCS_Filter_t filter);

/**
 * @brief Controla el bloque de iluminación frontal mediante la PAL.
 */
void TCS3200_ControlLED(TCS3200_t *sensor, TCS_LedState_t state);

/**
 * @brief Arranca la lógica de medición del contador sin invocar funciones nativas.
 */
void TCS3200_StartMeasurement(TCS3200_t *sensor);

/**
 * @brief Callback isócrono de procesamiento. Debe ser llamada desde el metrónomo (TIM3/Alarma).
 * @details Realiza la lectura del CNT, calcula los Hz y clasifica los datos de forma agnóstica.
 */
void TCS3200_ProcessCallback(TCS3200_t *sensor);

#endif /* INC_TCS3200_H_ */
