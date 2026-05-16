/**
 * @file sensor_hcsr04.c
 * @author Mamani Flores Carlos (UTN FRT)
 * @brief Implementación del driver generico para el sensor ultrasónico HC-SR04.
 * @details Procesa la telemetría mediante una máquina de estados no bloqueante.
 *          Implementa un filtro EMA (Exponential Moving Average) para suavizar
 *          las lecturas y un control de timeout para evitar bloqueos por pérdida de eco.
 * @version 2.0
 * @date 2026
 */

#include "sensor_hcsr04.h"

/* --- Definiciones de Control de Robustez --- */

/** @brief Velocidad del sonido a nivel del mar (cm/us). */
#define HCSR04_SPEED_SOUND_CM_US 0.0343f

/** @brief Tiempo máximo de espera para el eco (30ms = ~5.1 metros). */
#define HCSR04_TIMEOUT_US        30000

/**
 * @brief Factor de suavizado para el filtro EMA (0.0 a 1.0).
 * @details Valores menores dan más estabilidad; valores mayores dan más velocidad de respuesta.
 */
#define HCSR04_FILTER_ALPHA      0.35f

/* --- 1. Inicialización de la Instancia --- */

/**
 * @brief Inicializa el objeto del sensor inyectando la PAL.
 */
void SENSOR_HCSR04_Init(sensor_hcsr04_t* dev, generic_gpio_t trig, generic_ic_t echo, hal_interface_t pal) {
    if (!dev) return;

    dev->trig = trig;
    dev->echo = echo;
    dev->pal = pal;

    dev->t_rise = 0;
    dev->t_fall = 0;
    dev->last_t_us = 0;
    dev->dist_cm = 0.0f;
    dev->state = HCSR04_STATE_IDLE;

    /* Asegurar que el pin de trigger inicie en un estado conocido (bajo) */
    dev->pal.gpio_write(dev->trig, false);
}

/* --- 2. Gestión del Disparo (Trigger) --- */

/**
 * @brief Inicia una nueva medición mediante un pulso de 10us.
 * @details Incluye un mecanismo de recuperación de errores: si el sensor se queda
 *          trabado esperando un eco, esta función detecta el timeout y resetea el estado.
 */
void SENSOR_HCSR04_Trigger(sensor_hcsr04_t* dev) {
    if (!dev) return;

    /* Verificación de Timeout: Evita bloqueos si se pierde el eco anterior */
    if (dev->state == HCSR04_STATE_BUSY || dev->state == HCSR04_STATE_MEASURING) {
        if ((dev->pal.get_us() - dev->last_t_us) < HCSR04_TIMEOUT_US) {
            return; /* Todavía estamos dentro de la ventana de tiempo del eco anterior */
        }
        /* Si excedió el tiempo, marcamos error y forzamos el reinicio */
        dev->state = HCSR04_STATE_ERROR;
    }

    /* Preparación para nueva captura */
    dev->state = HCSR04_STATE_IDLE;
    dev->pal.ic_set_edge(dev->echo, true); /* Configurar flanco de subida inicial */

    /* Generación del pulso Trigger (Determinístico mediante get_us de la PAL) */
    dev->pal.gpio_write(dev->trig, true);
    uint32_t start_pulse = dev->pal.get_us();
    while ((dev->pal.get_us() - start_pulse) < 10); /* Bloqueo mínimo de 10us */
    dev->pal.gpio_write(dev->trig, false);

    /* Registro de timestamp de inicio para monitoreo de Timeout */
    dev->last_t_us = dev->pal.get_us();
    dev->state = HCSR04_STATE_BUSY;
}

/* --- 3. Lógica de Captura (Contexto ISR) --- */

/**
 * @brief Maneja la transición de estados durante la captura de flancos.
 * @details Esta función se ejecuta en contexto de interrupción. Minimiza el jitter
 *          leyendo directamente el registro CCR del hardware vía ic_read.
 */
void SENSOR_HCSR04_OnCapture(sensor_hcsr04_t* dev) {
    if (!dev) return;

    switch (dev->state) {
        case HCSR04_STATE_BUSY:
            /* Detectado flanco de subida (Echo High) */
            dev->t_rise = dev->pal.ic_read(dev->echo);
            dev->pal.ic_set_edge(dev->echo, false); /* Cambiar a detección de flanco de bajada */
            dev->state = HCSR04_STATE_MEASURING;
            break;

        case HCSR04_STATE_MEASURING:
            /* Detectado flanco de bajada (Echo Low) */
            dev->t_fall = dev->pal.ic_read(dev->echo);
            dev->state = HCSR04_STATE_READY;
            break;

        default:
            /* Ignorar capturas fuera de secuencia (ruido) */
            break;
    }
}

/* --- 4. Procesamiento de Datos y Filtrado --- */

/**
 * @brief Obtiene la distancia aplicando un filtro de mediana de 3 muestras y EMA.
 * @details Esta combinación elimina los outliers por superficies irregulares.
 */
float SENSOR_HCSR04_GetDistance(sensor_hcsr04_t* dev) {
    if (dev->state != HCSR04_STATE_READY) return dev->dist_cm;

    static float window[3] = {0};
    static uint8_t idx = 0;

    // 1. Cálculo de la muestra actual
    uint32_t diff = (dev->t_fall >= dev->t_rise) ?
                    (dev->t_fall - dev->t_rise) :
                    (0xFFFF - dev->t_rise + dev->t_fall);
    float raw = (diff * HCSR04_SPEED_SOUND_CM_US) / 2.0f;

    // 2. Filtro de Mediana simple (3 muestras)
    window[idx] = raw;
    idx = (idx + 1) % 3;

    float a = window[0], b = window[1], c = window[2];
    float median = (a < b) ? ((b < c) ? b : ((a < c) ? c : a))
                           : ((a < c) ? a : ((b < c) ? c : b));

    // 3. Aplicar EMA sobre la mediana para máxima suavidad
    if (median > 2.0f && median < 400.0f) {
        dev->dist_cm = (0.3f * median) + (0.7f * dev->dist_cm);
    }

    dev->state = HCSR04_STATE_IDLE;
    return dev->dist_cm;
}
