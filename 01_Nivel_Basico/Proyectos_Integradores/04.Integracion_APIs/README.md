# Proyecto Integrador Final: Secuenciador Maestro de 6 Efectos 🚀

Este proyecto representa la culminación del **Nivel Básico** en mi formación sobre sistemas embebidos. En él se integran de forma sinérgica los pilares de la arquitectura de software profesional: modularidad por capas, abstracción de periféricos y máquinas de estados finitas (FSM) sobre la plataforma **STM32 (Nucleo-F439ZI)**.

## 🎯 Objetivo del Proyecto
Diseñar un controlador de efectos lumínicos de grado industrial que gestione 6 secuencias complejas mediante un único pulsador de usuario. El sistema utiliza una **arquitectura asíncrona no bloqueante**, garantizando que el reporte por UART y la detección de eventos ocurran en tiempo real, independientemente del efecto visual activo.

---

## 🧱 Arquitectura del Firmware: El Modelo de Capas
El software se ha diseñado siguiendo el estándar de la industria para maximizar la portabilidad y el mantenimiento futuro:



1. **Capa de Aplicación (`main.c`)**: Orquestador de alto nivel que gestiona la lógica de la FSM y la selección de modos.
2. **Capa de Abstracción (API_Drivers)**: 
   - `API_debounce`: Motor de estados para el filtrado de ruido mecánico en el pulsador.
   - `API_delay`: Temporización asíncrona basada en el contador `SysTick`.
   - `API_led`: Gestión genérica de instancias individuales y grupos de LEDs mediante estructuras.
3. **Capa HAL (STMicroelectronics)**: Interfaz de bajo nivel que interactúa directamente con los registros del Cortex-M4.

---

## 🎨 Modos de Operación (FSM de Secuencias)
El sistema cicla entre los siguientes estados mediante una pulsación validada en el pin **PB11**:

| ID | Modo | Lógica Visual | Temporización |
| :---: | :--- | :--- | :---: |
| **0** | **MODO_SYNC** | Parpadeo síncrono de todo el grupo. | 500ms |
| **1** | **MODO_CARRERA** | Desplazamiento circular (1-2-3-1...). | 150ms |
| **2** | **MODO_REBOTE** | Efecto *Knight Rider* (ida y vuelta). | 100ms |
| **3** | **MODO_STREAK** | Acumulación progresiva de LEDs encendidos. | 200ms |
| **4** | **MODO_ALARMA** | Estroboscopio asimétrico (Rojo vs Azul). | 80ms |
| **5** | **MODO_RANDOM** | Selección estocástica (pseudo-aleatoria). | 120ms |

---

## 💻 Abstracción y Escalabilidad de Grupos
Una innovación clave de este integrador es el manejo de **Lógica de Grupos**. Esto permite que las funciones de la API actúen sobre un arreglo de estructuras, haciendo que el sistema sea escalable a cualquier número de LEDs mediante metaprogramación con `sizeof`.

```c
/* Definición del grupo de LEDs como objetos de software */
led_t misLeds[] = {
    {LD1_GPIO_Port, LD1_Pin, false}, // LED Verde
    {LD2_GPIO_Port, LD2_Pin, false}, // LED Azul
    {LD3_GPIO_Port, LD3_Pin, false}  // LED Rojo
};
const uint8_t CANT_LEDS = sizeof(misLeds) / sizeof(led_t);

/* Ejecución mediante API de Grupo en el motor de la FSM */
case MODO_SYNC:
    if (API_Delay_Read(&timerSecuencia)) {
        API_LED_ToggleAll(misLeds, CANT_LEDS); // Abstracción total
        Debug_Log("EVENTO: Toggle Sincronizado\r\n");
    }
    break;
```

## 🔍 Conclusiones del Nivel Básico

* **Multitarea Cooperativa:** Se erradicó por completo el uso de HAL_Delay(), permitiendo un aprovechamiento superior de los ciclos de CPU para tareas concurrentes.
* **Determinismo:** La integración de la API_debounce con la FSM principal garantiza transiciones de estado limpias, eliminando comportamientos erráticos por ruido mecánico.
* **Calidad de Código:** La separación estricta en archivos .h y .c y el uso de punteros para el paso de estructuras definen un flujo de trabajo profesional y escalable.

# 🏁 NIVEL BÁSICO COMPLETADO

Con la entrega de este secuenciador maestro, los fundamentos de entrada/salida, tiempo y arquitectura de software están consolidados. El sistema es robusto, pero depende de la velocidad del bucle while(1) (Polling).

> **Siguiente paso: Inmersión en el Nivel Intermedio.** > Tras consolidar la arquitectura modular, el próximo desafío es trascender los límites del *Polling*. Liberaremos al CPU de la vigilancia constante mediante el uso de **Interrupciones Externas (EXTI)** y **Timers por Hardware**, permitiendo una respuesta en tiempo real determinística. Además, evolucionaremos hacia el procesamiento de señales del mundo físico mediante el **ADC** y protocolos de comunicación robustos, transformando este firmware en un sistema reactivo de alto rendimiento.

---
*“La maestría en sistemas embebidos no nace de encender un LED, sino de diseñar la arquitectura que permite que mil procesos convivan en armonía sin bloquearse entre sí.”*