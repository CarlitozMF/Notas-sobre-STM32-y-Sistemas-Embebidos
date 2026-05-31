/**
 * @file hal_interface.h
 * @brief Interfaz de Abstracción de Hardware (HAL) Universal.
 * @author Mamani Flores Carlos (UTN FRT)
 * @details Este archivo define el "contrato" entre los drivers agnósticos y el hardware
 * específico (STM32, AVR, NXP, etc.). Utiliza punteros a función para implementar
 * inyección de dependencias, permitiendo que un mismo driver funcione en cualquier
 * plataforma sin necesidad de recompilar el núcleo del código.
 * @version 2.0
 * @date 2026
 */

#ifndef HAL_INTERFACE_H
#define HAL_INTERFACE_H

#include <stdint.h>
#include <stdbool.h>

/* --- 1. Definiciones de Tipos Genéricos --- */

/**
 * @struct generic_gpio_t
 * @brief Descriptor universal para un pin de propósito general (GPIO).
 * @note El campo 'port' es un puntero genérico (void*) para soportar estructuras
 *       como GPIO_TypeDef en STM32 o punteros de registro en arquitecturas de 8 bits.
 */
typedef struct {
	void* port;        /**< Puntero al registro base del puerto (ej: GPIOA) */
	uint16_t pin;      /**< Identificador numérico del pin (ej: GPIO_PIN_5 o 5) */
} generic_gpio_t;

/**
 * @struct generic_pwm_t
 * @brief Descriptor universal para un canal de Modulación por Ancho de Pulso (PWM).
 */
typedef struct {
	void* timer_handle; /**< Puntero al controlador del periférico (ej: &htim2) */
	uint32_t channel;   /**< Canal específico asignado (ej: TIM_CHANNEL_1) */
} generic_pwm_t;

/**
 * @struct generic_ic_t
 * @brief Descriptor para un canal de Input Capture.
 */
typedef struct {
	void* timer_handle; /**< Puntero al timer (ej: &htim3) */
	uint32_t channel;   /**< Canal (ej: TIM_CHANNEL_1) */
} generic_ic_t;

/* --- 2. Definiciones de Callbacks (Punteros a Función) --- */

/** @defgroup PAL_Callbacks Tipos de punteros para servicios de plataforma
 *  @brief Define las firmas de las funciones que la capa física debe implementar.
 *  @{
 */

/** Puntero para escritura digital: (pin, estado) */
typedef void (*gpio_write_ptr)(generic_gpio_t gpio, bool state);

/** Puntero para lectura digital: retorna el estado del pin */
typedef bool (*gpio_read_ptr)(generic_gpio_t gpio);

/** Puntero para control de duty cycle: (canal, valor) */
typedef void (*pwm_write_ptr)(generic_pwm_t ch, uint16_t value);

/** Puntero para obtener el tiempo del sistema en milisegundos */
typedef uint32_t (*tick_get_ptr)(void);

/** Puntero para funciones de retardo bloqueante */
typedef void (*delay_ms_ptr)(uint32_t ms);

/* --- Nuevos Callbacks para el Ultrasónico --- */

/** Puntero para obtener ticks en microsegundos (Aquí inyectaremos el DWT) */
typedef uint32_t (*get_us_ptr)(void);

/** Puntero para leer el valor capturado en el registro CCR */
typedef uint32_t (*ic_read_ptr)(generic_ic_t ic);

/** Puntero para cambiar la polaridad de captura (subida/bajada) */
typedef void (*ic_set_edge_ptr)(generic_ic_t ic, bool rising);

/* --- Nuevos Callbacks para el Motor Paso a Paso (Output Compare) --- */

/** Puntero para leer el valor de coincidencia actual en el registro de comparación (CCR) */
typedef uint32_t (*oc_read_ptr)(generic_pwm_t ch);

/** Puntero para escribir/reprogramar el próximo punto de disparo de la alarma (Acumulador elástico) */
typedef void (*oc_write_ptr)(generic_pwm_t ch, uint32_t value);

/** @} */

/* --- 3. El Super-Objeto PAL (Platform Abstraction Layer) --- */

/**
 * @struct hal_interface_t
 * @brief Estructura de servicios de la Plataforma (PAL).
 *
 * @details Actúa como una tabla de despacho de funciones. Durante la inicialización,
 * el desarrollador vincula las funciones específicas de la HAL (ej: HAL_GPIO_WritePin)
 * con estos punteros. El driver agnóstico consume estos servicios sin conocer la
 * implementación interna.
 */
typedef struct {
	gpio_write_ptr  gpio_write; /**< Servicio de salida digital */
	gpio_read_ptr   gpio_read;  /**< Servicio de entrada digital */
	pwm_write_ptr   pwm_write;  /**< Servicio de generación PWM */
	tick_get_ptr    get_tick;   /**< Servicio de base de tiempo (Systick) */
	delay_ms_ptr    delay_ms;   /**< Servicio de temporización bloqueante */
	get_us_ptr      get_us;      /**< El driver usará esto en lugar de utils_delay.h directamente */
	ic_read_ptr     ic_read;     /**< Abstracción de HAL_TIM_ReadCapturedValue */
	ic_set_edge_ptr ic_set_edge; /**< Abstracción de __HAL_TIM_SET_CAPTUREPOLARITY */
	oc_read_ptr     oc_read;    /**< Abstracción de HAL_TIM_ReadCapturedValue */
	oc_write_ptr    oc_write;   /**< Abstracción de __HAL_TIM_SET_COMPARE */
} hal_interface_t;

#endif /* HAL_INTERFACE_H */
