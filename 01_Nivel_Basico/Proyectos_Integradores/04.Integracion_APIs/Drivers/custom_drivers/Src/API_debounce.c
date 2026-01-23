/**
 * @file API_debounce.c
 * @author CarlitozMF (UTN FRT)
 * @brief Implementación de la lógica de filtrado para botones mecánicos.
 */

#include "API_debounce.h"

/**
 * @brief Estados internos de la FSM de antirrebote.
 */
typedef enum {
    BUTTON_UP,      /**< Botón en reposo (suelto) */
    BUTTON_FALLING, /**< Detectado posible flanco de bajada */
    BUTTON_DOWN,    /**< Botón presionado (estado estable) */
    BUTTON_RISING   /**< Detectado posible flanco de subida */
} debounceState_t;

/* Variables privadas */
static delay_t debounce_timer;      /**< Timer de validación asíncrono */
static debounceState_t estado_actual; /**< Estado actual de la FSM */

/**
 * @brief Lee el estado físico del pin y lo adapta a la lógica configurada.
 * @param btn Puntero a la estructura del botón.
 * @return true si el botón se considera presionado, false en caso contrario.
 */
static bool readButton_Physical(button_t* btn) {
    bool pin_state = HAL_GPIO_ReadPin(btn->port, btn->pin) == GPIO_PIN_SET;
    return btn->inverted ? !pin_state : pin_state;
}

void debounceFSM_Init() {
    delayInit(&debounce_timer, 40); // 40ms de tiempo de validación estándar
    estado_actual = BUTTON_UP;
}

void debounceFSM_Update(button_t* btn) {
    switch (estado_actual) {
        case BUTTON_UP:
            if (readButton_Physical(btn)) {
                delayReset(&debounce_timer);
                estado_actual = BUTTON_FALLING;
            }
            break;

        case BUTTON_FALLING:
            if (delayRead(&debounce_timer)) {
                if (readButton_Physical(btn)) {
                    btn->keyPressed = true; // Evento confirmado
                    estado_actual = BUTTON_DOWN;
                } else {
                    estado_actual = BUTTON_UP;
                }
            }
            break;

        case BUTTON_DOWN:
            if (!readButton_Physical(btn)) {
                delayReset(&debounce_timer);
                estado_actual = BUTTON_RISING;
            }
            break;

        case BUTTON_RISING:
            if (delayRead(&debounce_timer)) {
                if (!readButton_Physical(btn)) {
                    estado_actual = BUTTON_UP;
                } else {
                    estado_actual = BUTTON_DOWN;
                }
            }
            break;

        default:
            estado_actual = BUTTON_UP;
            break;
    }
}

bool readKey(button_t* btn) {
    bool status = btn->keyPressed;
    if (status) {
        btn->keyPressed = false; // Reset automático tras lectura
    }
    return status;
}
