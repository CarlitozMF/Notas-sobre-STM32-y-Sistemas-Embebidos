/**
 * @file Display_7Seg.c
 * @author Mamani Flores Carlos (UTN FRT)
 * @brief Implementación del driver de 7 segmentos independiente del hardware.
 * @details Contiene la lógica de conversión, manejo de buffer y multiplexación
 *          por PWM de software.
 * @version 1.0
 * @date 2026
 */

#include "Display_7Seg.h"
#include <string.h>

/**
 * @brief Tabla de conversión BCD a 7-Segmentos para ánodo/cátodo común.
 * @details Mapea los números 0-9 y caracteres especiales (E, r) a patrones de bits.
 *          Formato del byte: [DP][G][F][E][D][C][B][A]
 */
static const uint8_t segment_map[] = {
    0x3F, /**< 0 */
    0x06, /**< 1 */
    0x5B, /**< 2 */
    0x4F, /**< 3 */
    0x66, /**< 4 */
    0x6D, /**< 5 */
    0x7D, /**< 6 */
    0x07, /**< 7 */
    0x7F, /**< 8 */
    0x6F, /**< 9 */
    0x79, /**< E (Error) */
    0x50  /**< r (error) */
};

/**
 * @brief Diccionario ASCII para representación alfanumérica limitada.
 * @details Traduce caracteres a patrones de 7 segmentos. Soporta letras que
 *          tienen una representación legible en el display.
 */
static const uint8_t ascii_map[256] = {
    // Letras Mayúsculas
    ['H'] = 0x76, ['E'] = 0x79, ['L'] = 0x38, ['P'] = 0x73,
    ['O'] = 0x3F, ['A'] = 0x77, ['C'] = 0x39, ['U'] = 0x3E,
    ['S'] = 0x6D, ['F'] = 0x71, ['G'] = 0x3D, ['J'] = 0x1E,
    ['Y'] = 0x6E, ['J'] = 0x1E,

    // Letras Minúsculas (las que son legibles)
    ['t'] = 0x78, ['r'] = 0x50, ['b'] = 0x7C, ['n'] = 0x54,
    ['d'] = 0x5E, ['i'] = 0x04, ['o'] = 0x5C, ['u'] = 0x1C,

    // Símbolos y Números
    ['-'] = 0x40, ['_'] = 0x08, [' '] = 0x00,
    ['0'] = 0x3F, ['1'] = 0x06, ['2'] = 0x5B, ['3'] = 0x4F,
    ['4'] = 0x66, ['5'] = 0x6D, ['6'] = 0x7D, ['7'] = 0x07,
    ['8'] = 0x7F, ['9'] = 0x6F
};

/**
 * @brief Inicializa la estructura de control del display.
 * @param hdisplay Puntero al handle del display.
 * @param pal Estructura de funciones de abstracción (Capa 1).
 * @param segments Arreglo de pines para los segmentos A-G+DP.
 * @param digits Arreglo de pines para los comunes de cada dígito.
 * @param count Número de dígitos físicos.
 * @param buffer Puntero a la memoria donde se guardarán los patrones.
 */
void Display7Seg_Init(display_7seg_t* hdisplay, display_7seg_pal_t pal,
                      display_gpio_t* segments, display_gpio_t* digits,
                      uint8_t count, uint8_t* buffer, display_type_t type) {

    if (!hdisplay || !buffer) return;

    /* Inyección de la PAL: Desacopla el driver de la HAL del fabricante */
    hdisplay->pal = pal;

    /* Configuración de Hardware Mapping y Memoria */
    hdisplay->segments = segments;
    hdisplay->digits = digits;
    hdisplay->digitsCount = count;
    hdisplay->buffer = buffer;
    hdisplay->type = type;

    /* Inicialización de estado: Brillo máximo y sin parpadeo */
    hdisplay->currentDigit = 0;
    hdisplay->brightness = 100;
    hdisplay->showLeadingZeros = false;
    hdisplay->decimalPointPos = 0;
    hdisplay->isEnabled = true;
    hdisplay->isFlashing = false;
    hdisplay->flashState = true;

    /* Limpieza inicial de la memoria del buffer */
    memset(hdisplay->buffer, 0, count);
}

/**
 * @brief Descompone un número entero en dígitos y los mapea al buffer.
 * @param hdisplay Puntero al handle.
 * @param number Valor numérico a mostrar.
 */
void Display7Seg_WriteNumber(display_7seg_t* hdisplay, uint32_t number) {
    uint32_t temp = number;

    /* Recorrido de derecha a izquierda (unidades, decenas, etc.) */
    for (int8_t i = hdisplay->digitsCount - 1; i >= 0; i--) {
        /* Lógica para supresión de ceros no significativos */
        if (temp == 0 && i < hdisplay->digitsCount - 1 && !hdisplay->showLeadingZeros) {
            hdisplay->buffer[i] = 0x00; /* Dígito apagado */
        } else {
            hdisplay->buffer[i] = segment_map[temp % 10];
        }
        temp /= 10;

        /* Aplicación del Punto Decimal mediante máscara OR sobre el bit 7 */
        if (hdisplay->decimalPointPos == (hdisplay->digitsCount - i)) {
            hdisplay->buffer[i] |= 0x80;
        }
    }
}

/**
 * @brief Traduce una cadena ASCII al buffer de segmentos.
 * @param hdisplay Puntero al handle.
 * @param str Cadena de caracteres a mostrar.
 */
void Display7Seg_WriteString(display_7seg_t* hdisplay, const char* str) {
    Display7Seg_Clear(hdisplay);
    /* Itera sobre el string y busca el patrón en la tabla ascii_map */
    for (uint8_t i = 0; i < hdisplay->digitsCount && str[i] != '\0'; i++) {
        hdisplay->buffer[i] = ascii_map[(uint8_t)str[i]];
    }
}

/**
 * @brief Muestra el mensaje de error "Err" en el display.
 * @param hdisplay Puntero al handle.
 * @param errorCode Código identificador del error.
 */
void Display7Seg_WriteError(display_7seg_t* hdisplay, uint8_t errorCode) {
    Display7Seg_Clear(hdisplay);
    if (hdisplay->digitsCount >= 3) {
        hdisplay->buffer[0] = 0x79; /**< Patrón de la 'E' */
        hdisplay->buffer[1] = 0x50; /**< Patrón de la 'r' */
        hdisplay->buffer[2] = 0x50; /**< Patrón de la 'r' */
        /* Nota: Se podría ampliar para imprimir errorCode en el último dígito */
    }
}

/**
 * @brief Limpia físicamente el buffer de datos (apaga todos los segmentos).
 * @param hdisplay Puntero al handle.
 */
void Display7Seg_Clear(display_7seg_t* hdisplay) {
    if (hdisplay->buffer) {
        memset(hdisplay->buffer, 0, hdisplay->digitsCount);
    }
}

/**
 * @brief Establece el nivel de brillo.
 * @param hdisplay Puntero al handle.
 * @param level Porcentaje de ciclo de trabajo (0-100).
 */
void Display7Seg_SetBrightness(display_7seg_t* hdisplay, uint8_t level) {
    hdisplay->brightness = (level > 100) ? 100 : level;
}

/**
 * @brief Configura el servicio de parpadeo asíncrono.
 * @param hdisplay Puntero al handle.
 * @param interval_ms Tiempo de toggle en ms. 0 desactiva el servicio.
 */
void Display7Seg_SetFlash(display_7seg_t* hdisplay, uint16_t interval_ms) {
    if (interval_ms == 0) {
        hdisplay->isFlashing = false;
        hdisplay->flashState = true;
    } else {
        hdisplay->isFlashing = true;
        hdisplay->flashInterval = interval_ms;
        if (hdisplay->pal.get_tick) {
            hdisplay->lastFlashTick = hdisplay->pal.get_tick();
        }
    }
}

/**
 * @brief Configura la posición del punto decimal.
 * @param hdisplay Puntero al handle.
 * @param pos Índice del dígito (1-N) donde se activará el DP.
 */
void Display7Seg_SetDecimalPoint(display_7seg_t* hdisplay, uint8_t pos) {
    hdisplay->decimalPointPos = pos;
}

/**
 * @brief Máquina de estados de multiplexación.
 * @details Realiza el refresco físico. Ejecuta el control de brillo por software
 *          y gestiona el parpadeo. Debe llamarse con alta prioridad.
 * @param hdisplay Puntero al handle.
 */
void Display7Seg_Refresh_ISR(display_7seg_t* hdisplay) {
    /* Validación de seguridad: Verifica que la PAL esté vinculada */
    if (!hdisplay->pal.write_pin) return;

    static uint8_t pwm_cnt = 0;

    /* 1. Gestión de Temporización (Flashing) */
    if (hdisplay->isFlashing && hdisplay->pal.get_tick) {
        if (hdisplay->pal.get_tick() - hdisplay->lastFlashTick >= hdisplay->flashInterval) {
            hdisplay->lastFlashTick = hdisplay->pal.get_tick();
            hdisplay->flashState = !hdisplay->flashState;
        }
    }

    /* 2. Anti-Ghosting: Apagar todos los comunes antes de cambiar segmentos */
    /* En Cátodo Común apagamos con 'false', en Ánodo Común con 'true' */
    bool common_off = (hdisplay->type == DISPLAY_CATHODE) ? false : true;
    for (uint8_t i = 0; i < hdisplay->digitsCount; i++) {
        hdisplay->pal.write_pin(hdisplay->digits[i], common_off);
    }

    /* 3. PWM por Software para control de Brillo */
    if (!hdisplay->isEnabled || !hdisplay->flashState || (pwm_cnt >= (hdisplay->brightness / 10))) {
        pwm_cnt = (pwm_cnt + 1) % 10;
        return;
    }
    pwm_cnt = (pwm_cnt + 1) % 10;

    /* 4. Escritura de Segmentos: Aplicar patrón con inversión lógica si aplica */
    uint8_t pattern = hdisplay->buffer[hdisplay->currentDigit];
    for (uint8_t j = 0; j < 8; j++) {
        bool segment_state = (pattern & (1 << j)) ? true : false;

        /* Si es Ánodo Común, invertimos el bit: 1 (encendido) pasa a ser nivel LOW */
        if (hdisplay->type == DISPLAY_ANODE) {
            segment_state = !segment_state;
        }
        hdisplay->pal.write_pin(hdisplay->segments[j], segment_state);
    }

    /* 5. Activación del Común: Enciende el dígito correspondiente */
    /* Cátodo se activa con HIGH (true), Ánodo se activa con LOW (false) */
    bool common_on = (hdisplay->type == DISPLAY_CATHODE) ? true : false;
    hdisplay->pal.write_pin(hdisplay->digits[hdisplay->currentDigit], common_on);

    /* Preparar el siguiente dígito para el próximo ciclo */
    hdisplay->currentDigit = (hdisplay->currentDigit + 1) % hdisplay->digitsCount;
}
