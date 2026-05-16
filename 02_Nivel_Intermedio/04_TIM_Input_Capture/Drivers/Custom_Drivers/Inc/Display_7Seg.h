/**
 * @file Display_7Seg.h
 * @author Mamani Flores Carlos (UTN FRT)
 * @brief Driver agnóstico para displays de 7 segmentos con multiplexación asíncrona.
 * @details Esta versión implementa una Capa de Abstracción de Plataforma (PAL)
 *          permitiendo su uso en STM32, AVR, PIC y otros MCUs sin modificar el driver.
 * @version 3.0
 * @date 2026
 */

#ifndef DISPLAY_7SEG_H
#define DISPLAY_7SEG_H

#include <Display_7Seg_Platform.h>
#include <stdint.h>
#include <stdbool.h>


/**
 * @enum display_type_t
 * @brief Define la lógica de control según el tipo de hardware.
 */
typedef enum {
    DISPLAY_CATHODE = 0, /**< Cátodo Común: Segmentos se activan con 'true' */
    DISPLAY_ANODE   = 1  /**< Ánodo Común: Segmentos se activan con 'false' */
} display_type_t;

/**
 * @struct display_7seg_t
 * @brief Handle de control principal para el display.
 * @details Contiene el estado del display, el buffer de datos y los punteros
 *          a la capa de abstracción de hardware.
 */
typedef struct {
    display_7seg_pal_t pal;        /**< Capa de Abstracción de Hardware (PAL) */
    display_gpio_t* segments;      /**< Arreglo de 8 pines para los segmentos (A..G, DP) */
    display_gpio_t* digits;        /**< Arreglo de N pines para los ánodos/cátodos comunes */
    uint8_t* buffer;               /**< Buffer que almacena los patrones de bits a mostrar */

    display_type_t type;
    uint8_t digitsCount;           /**< Cantidad total de dígitos físicos */
    uint8_t currentDigit;          /**< Índice del dígito que se está refrescando actualmente */
    uint8_t brightness;            /**< Nivel de brillo (0-100) gestionado por PWM software */
    uint8_t decimalPointPos;       /**< Posición del punto decimal (0 = apagado, 1 = derecha) */

    bool showLeadingZeros;         /**< Configuración: true para mostrar ceros a la izquierda */
    bool isEnabled;                /**< Control de encendido/apagado general del display */
    bool isFlashing;               /**< Estado: indica si el modo parpadeo está activo */
    uint32_t flashInterval;        /**< Tiempo de intervalo de parpadeo en milisegundos */
    uint32_t lastFlashTick;        /**< Auxiliar de tiempo para el control del flash */
    bool flashState;               /**< Estado interno (visible/invisible) durante el flash */
} display_7seg_t;

/* --- Funciones de Inicialización --- */

/**
 * @brief Configura e inicializa el handle del display.
 * @param hdisplay Puntero a la estructura de control.
 * @param pal Estructura de la plataforma (punteros a funciones de hardware).
 * @param segments Puntero al arreglo de 8 pines de segmentos.
 * @param digits Puntero al arreglo de N pines de comunes.
 * @param count Cantidad de dígitos físicos del display.
 * @param buffer Puntero a un arreglo de uint8_t de tamaño 'count'.
 * @param type acceso al tipo de display
 */
void Display7Seg_Init(display_7seg_t* hdisplay, display_7seg_pal_t pal,
                      display_gpio_t* segments, display_gpio_t* digits,
                      uint8_t count, uint8_t* buffer, display_type_t type);
/* --- Funciones de Escritura --- */

/**
 * @brief Convierte un número entero a patrones de 7 segmentos y los escribe en el buffer.
 * @param hdisplay Puntero al handle.
 * @param number Número a mostrar (0 a 99... según dígitos).
 */
void Display7Seg_WriteNumber(display_7seg_t* hdisplay, uint32_t number);

/**
 * @brief Escribe una cadena de texto permitida en el display.
 * @param hdisplay Puntero al handle.
 * @param str Cadena de caracteres (ej: "HELP", "Err", "A1").
 */
void Display7Seg_WriteString(display_7seg_t* hdisplay, const char* str);

/**
 * @brief Muestra un mensaje de error con un código específico (ej: "Err 01").
 * @param hdisplay Puntero al handle.
 * @param errorCode Código numérico del error.
 */
void Display7Seg_WriteError(display_7seg_t* hdisplay, uint8_t errorCode);

/**
 * @brief Apaga todos los segmentos del display.
 */
void Display7Seg_Clear(display_7seg_t* hdisplay);

/* --- Funciones de Configuración Visual --- */

/**
 * @brief Ajusta el ciclo de trabajo de los LEDs para controlar la intensidad lumínica.
 * @param hdisplay Puntero al handle.
 * @param level Porcentaje de brillo (0 a 100).
 */
void Display7Seg_SetBrightness(display_7seg_t* hdisplay, uint8_t level);

/**
 * @brief Activa o desactiva el efecto de parpadeo.
 * @param hdisplay Puntero al handle.
 * @param interval_ms Tiempo en milisegundos. 0 para desactivar parpadeo.
 */
void Display7Seg_SetFlash(display_7seg_t* hdisplay, uint16_t interval_ms);

/**
 * @brief Configura la posición del punto decimal fijo.
 * @param hdisplay Puntero al handle.
 * @param pos Posición desde la derecha (1 = primer dígito). 0 para desactivar.
 */
void Display7Seg_SetDecimalPoint(display_7seg_t* hdisplay, uint8_t pos);

/* --- Función de Sistema (Contexto Crítico) --- */

/**
 * @brief Función de refresco de hardware. DEBE ser llamada periódicamente.
 * @details Esta función realiza la multiplexación física. Se recomienda llamarla
 *          dentro de una interrupción de Timer (aprox. 1ms a 5ms) o un Scheduler.
 * @param hdisplay Puntero al handle.
 */
void Display7Seg_Refresh_ISR(display_7seg_t* hdisplay);

#endif /* DISPLAY_7SEG_H */
