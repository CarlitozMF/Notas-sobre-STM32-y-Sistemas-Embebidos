/**
 * @file sensor_hcsr04.h
 * @author Mamani Flores Carlos (UTN FRT)
 * @brief Driver generico y robusto para el sensor ultrasónico HC-SR04.
 * @details Este driver implementa una lógica de medición no bloqueante utilizando
 *          la interfaz PAL (hal_interface_t). Incluye gestión de estados,
 *          manejo de timeouts y está diseñado para ser integrado en arquitecturas
 *          de software de 3 capas.
 * @version 2.0
 * @date 2026
 */

#ifndef SENSOR_HCSR04_H
#define SENSOR_HCSR04_H

#include "hal_interface.h"

/* --- 1. Definiciones de Tipos y Enumeraciones --- */

/**
 * @enum hcsr04_state_t
 * @brief Estados internos de la máquina de estados del sensor.
 */
typedef enum {
    HCSR04_STATE_IDLE = 0,      /**< Sensor listo para un nuevo disparo */
    HCSR04_STATE_BUSY,          /**< Esperando flanco de subida (Echo Rising) */
    HCSR04_STATE_MEASURING,     /**< Midiendo duración del pulso (Echo Falling) */
    HCSR04_STATE_READY,         /**< Medición completada exitosamente */
    HCSR04_STATE_ERROR          /**< Error en la medición o Timeout detectado */
} hcsr04_state_t;

/**
 * @struct sensor_hcsr04_t
 * @brief Objeto descriptor del sensor HC-SR04.
 * @details Contiene los descriptores de hardware genéricos y los datos de la
 *          última medición. No depende de ninguna HAL específica de fabricante.
 */
typedef struct {
    generic_gpio_t  trig;       /**< Descriptor del pin de Trigger (Salida) */
    generic_ic_t    echo;       /**< Descriptor del canal de Input Capture (Entrada) */
    hal_interface_t pal;        /**< Tabla de servicios de hardware inyectada */

    uint32_t        t_rise;     /**< Marca de tiempo (ticks) del flanco de subida */
    uint32_t        t_fall;     /**< Marca de tiempo (ticks) del flanco de bajada */
    uint32_t        last_t_us;  /**< Timestamp del último trigger para control de timeout */
    float           dist_cm;    /**< Distancia calculada y filtrada en centímetros */
    hcsr04_state_t  state;      /**< Estado actual del proceso de medición */
} sensor_hcsr04_t;

/* --- 2. API del Driver (Funciones Públicas) --- */

/**
 * @brief Inicializa el objeto del sensor y limpia sus estados.
 * @param dev Puntero a la estructura del sensor.
 * @param trig Descriptor del pin GPIO para el Trigger.
 * @param echo Descriptor del canal de Timer para el Echo.
 * @param pal Estructura con los punteros a funciones de hardware.
 */
void SENSOR_HCSR04_Init(sensor_hcsr04_t* dev, generic_gpio_t trig, generic_ic_t echo, hal_interface_t pal);

/**
 * @brief Genera el pulso de disparo (Trigger) de 10us.
 * @details Utiliza el servicio @ref get_us de la PAL para garantizar precisión.
 *          Si el sensor está en estado BUSY, verifica internamente el timeout.
 * @param dev Puntero a la estructura del sensor.
 */
void SENSOR_HCSR04_Trigger(sensor_hcsr04_t* dev);

/**
 * @brief Procesa la captura de flancos del Echo.
 * @note Esta función DEBE ser llamada desde la interrupción (ISR) del Timer
 *       correspondiente al canal de Input Capture.
 * @param dev Puntero a la estructura del sensor.
 */
void SENSOR_HCSR04_OnCapture(sensor_hcsr04_t* dev);

/**
 * @brief Obtiene la última distancia medida procesada por filtros de robustez.
 * @details Realiza el cálculo físico basado en la velocidad del sonido (343 m/s)
 *          y aplica un filtro de suavizado para evitar inestabilidad.
 * @param dev Puntero a la estructura del sensor.
 * @return float Distancia en cm. Retorna el último valor válido en caso de error.
 */
float SENSOR_HCSR04_GetDistance(sensor_hcsr04_t* dev);

#endif /* SENSOR_HCSR04_H */
