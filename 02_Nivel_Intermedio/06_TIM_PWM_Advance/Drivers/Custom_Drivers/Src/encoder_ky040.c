/**
 * @file encoder_ky040.c
 * @author Mamani Flores Carlos (UTN FRT)
 * @brief Implementación del driver para encoder rotativo KY-040 acoplado a la PAL.
 * @details Desarrolla la lógica de decodificación por software para señales en
 *          cuadratura y el tratamiento del pulsador integrado. Utiliza comparación
 *          de descriptores genéricos para asegurar portabilidad e inmunidad al ruido.
 * @version 2.0
 * @date 2026
 */

#include "encoder_ky040.h"

/**
 * @brief Inicializa la estructura del encoder acoplando los canales físicos mediante descriptores.
 * @details Mapea los pines genéricos de la plataforma al objeto de control, inicializa
 *          los límites de conteo por saturación y realiza una lectura limpia del estado
 *          físico inicial del canal CLK a través de la PAL.
 * @param[out] dev    Puntero a la instancia del encoder (KY040_t).
 * @param[in]  pal_io Contrato de servicios de plataforma (VTable).
 * @param[in]  pinA   Descriptor genérico del canal CLK (puerto y pin).
 * @param[in]  pinB   Descriptor genérico del canal DT (puerto y pin).
 * @param[in]  pinSW  Descriptor genérico del pulsador SW (puerto y pin).
 * @param[in]  min    Límite inferior del rango operativo del contador.
 * @param[in]  max    Límite superior del rango operativo del contador.
 */
void KY040_Init(KY040_t *dev, hal_interface_t pal_io,
                generic_gpio_t pinA, generic_gpio_t pinB, generic_gpio_t pinSW,
                int32_t min, int32_t max) {
    /* Validación de Robustez: Aborta si la estructura o los servicios críticos son nulos */
    if (dev == NULL || pal_io.gpio_read == NULL || pal_io.get_tick == NULL) return;

    /* Vinculación de Hardware Abstraído */
    dev->pin_A = pinA;
    dev->pin_B = pinB;
    dev->pin_SW = pinSW;
    dev->pal = pal_io;

    /* Configuración de Rangos Operativos */
    dev->min_val = min;
    dev->max_val = max;

    /* Posición Inicial: Setea por defecto el centro del rango especificado (ej: 90 para el servo) */
    dev->position = (max + min) / 2;

    /* Inicialización de Estados y Marcas de Tiempo */
    dev->last_tick = 0;
    dev->last_tick_SW = 0;
    dev->sw_pressed = 0;

    /* Captura del estado lógico inicial del canal CLK empleando la PAL */
    dev->lastStateA = dev->pal.gpio_read(dev->pin_A) ? 1 : 0;
}

/**
 * @brief Manejador unificado de interrupciones para el Encoder de forma agnóstica.
 * @details Compara la totalidad del descriptor físico (tanto puntero de puerto como máscara de pin)
 *          para identificar el origen del disparo EXTI. Implementa filtros de tiempo independientes
 *          para debouncing de alta y baja frecuencia.
 * @param[in,out] dev            Puntero a la instancia del encoder.
 * @param[in]     triggered_gpio Descriptor del pin exacto que generó el flanco de interrupción.
 * @note Este manejador debe ser invocado desde la rutina de interrupción (ISR) de la plataforma.
 */
void KY040_IRQ_Handler(KY040_t *dev, generic_gpio_t triggered_gpio) {
    /* Validación preventiva de la tabla de servicios inyectada */
    if (dev == NULL || dev->pal.get_tick == NULL || dev->pal.gpio_read == NULL) return;

    uint32_t ahora = dev->pal.get_tick();

    /* --- PROCESAMIENTO DE GIRO (Verificación completa de Puerto y Pin del canal CLK) --- */
    if (triggered_gpio.port == dev->pin_A.port && triggered_gpio.pin == dev->pin_A.pin) {

        /* Debounce rápido de 5ms para mitigar ruido térmico/transitorios en la cuadratura */
        if (ahora - dev->last_tick < 5) return;

        /* Lectura simultánea y agnóstica de ambos canales lógicos */
        bool estadoA = dev->pal.gpio_read(dev->pin_A);
        bool estadoB = dev->pal.gpio_read(dev->pin_B);

        /* Algoritmo de detección por flanco de bajada en el Canal CLK */
        if (dev->lastStateA == 1 && estadoA == false) {

            /* Evaluación de fase: Si CLK != DT en flanco descendente, giro horario (CW) */
            if (estadoB != estadoA) {
                dev->position++;
            } else {
                dev->position--;
            }

            /* Aplicación estricta de límites por saturación */
            if (dev->position > dev->max_val) dev->position = dev->max_val;
            if (dev->position < dev->min_val) dev->position = dev->min_val;

            /* Actualización del registro de tiempo del giro */
            dev->last_tick = ahora;
        }

        /* Almacenamiento del estado actual como memoria para el próximo flanco */
        dev->lastStateA = estadoA ? 1 : 0;
    }

    /* --- PROCESAMIENTO DE PULSADOR SW (Verificación completa de Puerto y Pin del botón) --- */
    else if (triggered_gpio.port == dev->pin_SW.port && triggered_gpio.pin == dev->pin_SW.pin) {

        /* Debounce largo de 200ms para filtrar el rebote mecánico masivo del switch */
        if (ahora - dev->last_tick_SW > 200) {

            /* Confirmación física diferida: Verificamos nivel bajo estable (Active Low con Pull-Up) */
            if (dev->pal.gpio_read(dev->pin_SW) == false) {
                dev->sw_pressed = 1; /* Alza la bandera de evento para el loop de la aplicación */
            }
            dev->last_tick_SW = ahora;
        }
    }
}
