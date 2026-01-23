# Proyecto Integrador Final: Secuenciador Maestro de 6 Efectos 🚀

Este proyecto representa la culminación del **Nivel Básico** en mi formación sobre sistemas embebidos. En él se integran todos los conceptos de arquitectura de software, modularidad y máquinas de estados aprendidos hasta ahora sobre la plataforma **STM32 (Nucleo-F439ZI)**.

## 🎯 Objetivo del Proyecto
Diseñar un controlador de efectos lumínicos profesional que gestione 6 secuencias diferentes mediante un solo pulsador de usuario. El sistema utiliza una arquitectura no bloqueante para garantizar que el reporte por UART y la detección de pulsaciones ocurran en tiempo real, independientemente del efecto visual activo.



## 🧱 Arquitectura del Firmware
El software se ha diseñado bajo un modelo de capas para maximizar la reutilización del código:
1. **Capa de Aplicación (`main.c`)**: Orquestador principal que gestiona la FSM de 6 modos.
2. **Capa de Abstracción de Hardware (API_Drivers)**: 
   - `API_debounce`: Procesa la señal del botón eliminando ruidos mecánicos.
   - `API_delay`: Provee temporización asíncrona basada en el SysTick de la HAL.
   - `API_led`: Permite el manejo de LEDs individuales o grupos de forma genérica.
3. **Capa HAL (STMicroelectronics)**: Interfaz de bajo nivel con los registros del microcontrolador.

## 🎨 Modos de Operación (Secuencias)
El sistema cicla entre los siguientes efectos al presionar el pulsador en **PB11**:

| ID | Modo | Descripción | Velocidad |
| :--- | :--- | :--- | :--- |
| 0 | **MODO_SYNC** | Todos los LEDs del grupo parpadean al unísono. | 500ms |
| 1 | **MODO_CARRERA** | Desplazamiento secuencial LED por LED (1-2-3-1...). | 150ms |
| 2 | **MODO_REBOTE** | Efecto "Auto Fantástico" (ida y vuelta constante). | 100ms |
| 3 | **MODO_STREAK** | Los LEDs se acumulan encendidos hasta llenar el grupo. | 200ms |
| 4 | **MODO_ALARMA** | Estroboscopio asimétrico entre los LEDs Rojo y Azul. | 80ms |
| 5 | **MODO_RANDOM** | Selección pseudo-aleatoria de un LED del grupo. | 120ms |



## 💻 Abstracción de Grupos de LEDs
Una de las innovaciones de este integrador es el manejo de **Lógica de Grupos**. Esto permite que las funciones de la API actúen sobre un arreglo de estructuras, haciendo que el sistema sea escalable a cualquier número de LEDs sin modificar el `while(1)`.

```c
/* Definición del grupo de LEDs en main.c */
led_t misLeds[] = {
    {LD1_GPIO_Port, LD1_Pin, false}, // LED Verde
    {LD2_GPIO_Port, LD2_Pin, false}, // LED Azul
    {LD3_GPIO_Port, LD3_Pin, false}  // LED Rojo
};
const uint8_t CANT_LEDS = sizeof(misLeds) / sizeof(led_t);

/* Ejecución mediante API de Grupo */
case MODO_SYNC:
    delayWrite(&timerSecuencia, 500);
    API_LED_ToggleAll(misLeds, CANT_LEDS); // Procesa N LEDs dinámicamente
    Debug_Log("MODO: Sincronizado\r\n");
    break;
```
## 🛠️ Monitoreo y Debugging vía UART

Se implementó un sistema de logs profesional a través de UART3. Cada cambio de estado y cada paso crítico de las secuencias es reportado a la terminal serie, permitiendo un monitoreo exhaustivo de la lógica sin detener el procesador.

## 🔍 Conclusiones del Nivel Básico
1. Multitarea Cooperativa: Se evitó por completo el uso de HAL_Delay(), permitiendo que el CPU atienda múltiples procesos simultáneamente.
2. Robustez: La integración de la API_debounce garantiza una transición de estados limpia, ignorando ruidos eléctricos en el pulsador.
3. Portabilidad: Gracias a la capa de abstracción, los drivers desarrollados pueden migrarse a otras placas de la familia STM32 con cambios mínimos.

---
*Este proyecto cierra el Nivel Básico de mi formación en Sistemas Embebidos.*