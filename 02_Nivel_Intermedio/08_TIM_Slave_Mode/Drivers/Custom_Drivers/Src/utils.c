/**
 * @file utils.c
 * @author Mamani Flores Carlos (UTN FRT)
 * @brief Implementación del driver de utilidades y base de tiempo de microsegundos.
 * @details Provee el control de bajo nivel para el periférico de rastreo DWT
 * (Data Watchpoint and Trace) embebido en el núcleo ARM Cortex-M4.
 * Permite obtener una base de tiempo independiente del SysTick para
 * temporizaciones críticas del firmware.
 * @version 1.0
 * @date 2026
 */

#include "utils.h"

/**
 * @brief Inicializa y habilita el contador de ciclos del bloque DWT.
 * @details Configura los registros del núcleo ARM de forma secuencial para
 * desbloquear y activar el contador de ciclos de hardware (CYCCNT).
 * Es mandatorio llamarla antes de realizar cualquier medición en microsegundos.
 */
void UTILS_DWT_Init(void) {
    /* 1. Activamos el bit de rastreo (TRCENA) en el registro de Control de Excepciones
     * y Monitoreo de Depuración (DEMCR) del bloque CoreDebug. Esto es un requisito
     * previo de hardware para poder acceder a los registros del DWT. */
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;

    /* 2. Reseteamos el contador de ciclos de hardware (CYCCNT) a cero para limpiar
     * cualquier valor residual post-reset del silicio. */
    DWT->CYCCNT = 0;

    /* 3. Habilitamos físicamente el contador de ciclos seteando el bit CYCCNTENA
     * en el registro de control del DWT (DWT_CTRL). A partir de esta línea,
     * el registro incrementa de forma síncrona con cada ciclo de reloj del sistema. */
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

/**
 * @brief Obtiene el valor instantáneo de tiempo del sistema en microsegundos.
 * @details Realiza la lectura atómica de los ciclos de clock actuales y los
 * escala dividiéndolos por la velocidad del procesador obtenida dinámicamente.
 * @return uint32_t Cantidad de microsegundos transcurridos (Base temporal elástica).
 */
uint32_t UTILS_GetUs(void) {
    /* Retorna los ciclos actuales divididos por la frecuencia del sistema expresada en MHz.
     * Ejemplo: Si SystemCoreClock es 180000000 Hz (180 MHz), la división da 180.
     * Por lo tanto, cada 180 ciclos del CPU se suma un microsegundo neto en el retorno. */
    return DWT->CYCCNT / (SystemCoreClock / 1000000);
}

/**
 * @brief Genera un retardo bloqueante preciso en microsegundos.
 * @details Mide el delta de tiempo directamente sobre los ciclos del silicio,
 * evitando el desborde o pérdida de ticks que suelen sufrir los bucles de software comunes.
 * @param us Cantidad de microsegundos a demorar.
 */
void UTILS_DelayUs(uint32_t us) {
    /* 1. Capturamos la marca de ciclos inicial para la referencia temporal */
    uint32_t start_cycles = DWT->CYCCNT;

    /* 2. Calculamos cuántos ciclos de clock reales representa el delay solicitado */
    uint32_t delay_cycles = us * (SystemCoreClock / 1000000);

    /* 3. Espera en lazo cerrado por hardware: la resta maneja de forma automática
     * el desborde nativo del registro de 32 bits (Aritmética de complemento a dos) */
    while ((DWT->CYCCNT - start_cycles) < delay_cycles);
}
