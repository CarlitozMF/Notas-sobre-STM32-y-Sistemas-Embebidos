/**
 * @file utils_delay.c
 * @author CarlitozMF (UTN FRT)
 * @brief Implementación del servicio de delay por hardware.
 */

#include "utils_delay.h"

/**
 * @brief Configura los registros del núcleo para habilitar el contador CYCCNT.
 * @details Activa el bit TRCENA en el registro DEMCR y habilita el contador
 * de ciclos en el registro de control DWT.
 */
void Utils_Delay_Init(void) {
    // Verificar si el contador ya está activo
    if (!(CoreDebug->DEMCR & CoreDebug_DEMCR_TRCENA_Msk)) {
        CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
        DWT->CYCCNT = 0;
        DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
    }
}

/**
 * @brief Retardo basado en ciclos de instrucción.
 * @details Calcula la cantidad de ciclos necesarios basándose en SystemCoreClock.
 * Es inmune a las optimizaciones del compilador.
 */
void delay_us(uint32_t us) {
    uint32_t startTick = DWT->CYCCNT;
    // Cálculo: ciclos = microsegundos * (Frecuencia CPU / 1.000.000)
    uint32_t delayTicks = us * (SystemCoreClock / 1000000);

    while ((DWT->CYCCNT - startTick) < delayTicks);
}
