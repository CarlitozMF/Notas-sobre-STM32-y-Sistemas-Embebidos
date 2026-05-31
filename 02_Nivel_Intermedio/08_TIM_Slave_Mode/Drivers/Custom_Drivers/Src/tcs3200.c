/**
 * @file tcs3200.c
 * @author Mamani Flores Carlos (UTN FRT)
 * @brief Implementación agnóstica de la lógica de control del TCS3200.
 * @details Contiene la lógica de secuenciamiento de filtros y cálculo de frecuencias
 * utilizando la inyección de dependencias de la PAL para desvincularse del silicio.
 * @version 2.0 (Sampling Genérico)
 * @date 2026
 */

#include "tcs3200.h"
#include <stddef.h>

/**
 * @brief Inicializa la estructura interna del objeto sensor.
 * @param sensor Puntero a la instancia del sensor TCS3200_t.
 * @param pal_io Estructura con la tabla de despacho de funciones de hardware (PAL).
 * @param count_ch Descriptor del canal/timer que operará como contador de pulsos.
 */
void TCS3200_Init(TCS3200_t *sensor, hal_interface_t pal_io, generic_pwm_t count_ch) {
    /* Protección básica contra punteros nulos */
    if (sensor == NULL) return;

    /* Inyección de dependencias y mapeo de periféricos */
    sensor->pal = pal_io;
    sensor->ic_count_ch = count_ch;
    sensor->measurement_ready = 0;

    /* Inicialización del vector de frecuencias en zona segura de reposo */
    sensor->frequency_red = 0;
    sensor->frequency_green = 0;
    sensor->frequency_blue = 0;
    sensor->frequency_clear = 0;
}

/**
 * @brief Configura y vincula los pines GPIO abstractos al objeto sensor.
 * @param sensor Puntero a la instancia del sensor.
 * @param s0 Pin abstracto de control de escala S0.
 * @param s1 Pin abstracto de control de escala S1.
 * @param s2 Pin abstracto de selección de filtro S2.
 * @param s3 Pin abstracto de selección de filtro S3.
 * @param led Pin abstracto para control de iluminación frontal.
 */
void TCS3200_ConfigGPIO(TCS3200_t *sensor,
                        generic_gpio_t s0, generic_gpio_t s1,
                        generic_gpio_t s2, generic_gpio_t s3,
                        generic_gpio_t led) {
    /* Verificación de seguridad en la inyección de dependencias críticas */
    if (sensor == NULL || sensor->pal.gpio_write == NULL) return;

    /* Enlace de descriptores abstractos de pines */
    sensor->pin_s0 = s0;
    sensor->pin_s1 = s1;
    sensor->pin_s2 = s2;
    sensor->pin_s3 = s3;
    sensor->pin_led = led;

    /* Configuración de escala al 100% (S0 = H, S1 = H) mediante la PAL.
     * Esto maximiza la resolución de conteo en la ventana de tiempo. */
    sensor->pal.gpio_write(sensor->pin_s0, true);
    sensor->pal.gpio_write(sensor->pin_s1, true);

    /* Estado inicial seguro: Iluminación apagada para mitigar Fatiga Térmica */
    TCS3200_ControlLED(sensor, TCS_LED_OFF);
}

/**
 * @brief Cambia el filtro óptico activo escribiendo en S2 y S3 a través de la PAL.
 * @param sensor Puntero a la instancia del sensor.
 * @param filter Filtro óptico seleccionado (TCS_FILTER_RED, GREEN, BLUE, CLEAR).
 */
void TCS3200_SetFilter(TCS3200_t *sensor, TCS_Filter_t filter) {
    /* Se requiere gpio_write para los pines de control y get_us para el retardo no bloqueante de la HAL */
    if (sensor == NULL || sensor->pal.gpio_write == NULL || sensor->pal.get_us == NULL) return;

    /* Tabla de verdad nativa del Datasheet del TCS3200 */
    switch(filter) {
        case TCS_FILTER_RED:   /* S2 = 0, S3 = 0 */
            sensor->pal.gpio_write(sensor->pin_s2, false);
            sensor->pal.gpio_write(sensor->pin_s3, false);
            break;
        case TCS_FILTER_BLUE:  /* S2 = 0, S3 = 1 */
            sensor->pal.gpio_write(sensor->pin_s2, false);
            sensor->pal.gpio_write(sensor->pin_s3, true);
            break;
        case TCS_FILTER_CLEAR: /* S2 = 1, S3 = 0 */
            sensor->pal.gpio_write(sensor->pin_s2, true);
            sensor->pal.gpio_write(sensor->pin_s3, false);
            break;
        case TCS_FILTER_GREEN: /* S2 = 1, S3 = 1 */
            sensor->pal.gpio_write(sensor->pin_s2, true);
            sensor->pal.gpio_write(sensor->pin_s3, true);
            break;
    }

    /* FILTRO DE ROBUSTEZ TEMPORAL: Retardo de estabilización agnóstico de 2 milisegundos (2000 us).
     * Evita capturar transitorios erráticos provocados por el tiempo de conmutación
     * interno de los fotodiodos al cambiar de canal óptico. */
    uint32_t start_us = sensor->pal.get_us();
    while ((sensor->pal.get_us() - start_us) < 2000);
}

/**
 * @brief Controla el bloque de iluminación frontal mediante la PAL.
 * @param sensor Puntero a la instancia del sensor.
 * @param state Estado deseado (TCS_LED_ON o TCS_LED_OFF).
 */
void TCS3200_ControlLED(TCS3200_t *sensor, TCS_LedState_t state) {
    if (sensor == NULL || sensor->pal.gpio_write == NULL) return;
    sensor->pal.gpio_write(sensor->pin_led, (state == TCS_LED_ON) ? true : false);
}

/**
 * @brief Prepara el hardware del contador borrando sus registros mediante la PAL.
 * @param sensor Puntero a la instancia del sensor.
 */
void TCS3200_StartMeasurement(TCS3200_t *sensor) {
    if (sensor == NULL || sensor->pal.oc_write == NULL) return;

    /* Reseteamos el registro de comparación/conteo a 0 mediante la PAL de manera portátil */
    sensor->pal.oc_write(sensor->ic_count_ch, 0);

    /* Nota de Arquitectura: El arranque físico de los periféricos (ej: HAL_TIM_Base_Start)
     * se delega intencionalmente a la Capa 1 en main.c para preservar la pureza del driver. */
}

/**
 * @brief Callback isócrono de procesamiento. Debe llamarse desde el metrónomo de Capa 3 (TIM3).
 * @details Realiza el vuelco de registros del contador, calcula la frecuencia real en Hz y
 * asigna el valor al canal correspondiente leyendo el estado físico de los pines por hardware.
 * @param sensor Puntero a la instancia del sensor.
 */
void TCS3200_ProcessCallback(TCS3200_t *sensor) {
    /* Verificación de los contratos necesarios para la lectura atómica de pulsos */
    if (sensor == NULL || sensor->pal.get_timer_cnt == NULL ||
        sensor->pal.oc_write == NULL || sensor->pal.gpio_read == NULL) return;

    /* 1. Capturar el acumulado de pulsos externos en la ventana de tiempo actual */
    uint32_t pulses = sensor->pal.get_timer_cnt(sensor->ic_count_ch);

    /* 2. Resetear el contador inmediatamente mediante la PAL para minimizar la pérdida de pulsos
     * durante el solapamiento de ventanas de muestreo (Garantiza Determinismo) */
    sensor->pal.oc_write(sensor->ic_count_ch, 0);

    /* 3. Extrapolación Matemática a Hz: Frecuencia = Pulsos / Tiempo_Ventana */
    uint32_t multiplier = 1000 / TCS_MEASURE_WINDOW_MS;
    uint32_t frequency_hz = pulses * multiplier;

    /* 4. Clasificación Inmune a Desfases: Leemos el estado físico real de los pines
     * para asegurar el almacenamiento en la variable correcta del filtro activo. */
    bool s2 = sensor->pal.gpio_read(sensor->pin_s2);
    bool s3 = sensor->pal.gpio_read(sensor->pin_s3);

    if      (!s2 && !s3) sensor->frequency_red   = frequency_hz;
    else if (!s2 &&  s3) sensor->frequency_blue  = frequency_hz;
    else if ( s2 && !s3) sensor->frequency_clear = frequency_hz;
    else if ( s2 &&  s3) sensor->frequency_green = frequency_hz;

    /* 5. Notificar a la máquina de estados de Capa 3 que la ráfaga de datos es válida */
    sensor->measurement_ready = 1;
}
