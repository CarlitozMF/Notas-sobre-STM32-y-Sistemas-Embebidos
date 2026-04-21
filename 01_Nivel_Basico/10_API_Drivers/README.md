# 10_API_Drivers: Multitarea Cooperativa y Abstracción 🏗️

Este laboratorio eleva la complejidad del firmware para demostrar la potencia de la **programación modular**. El objetivo es transformar el código monolítico en componentes reutilizables (Drivers), permitiendo gestionar múltiples periféricos de forma asíncrona sobre la placa **Nucleo-F439ZI**.

## 🧱 Arquitectura del Sistema: Capas y "Objetos"

Hemos diseñado el software bajo un esquema de **Multitarea Cooperativa**. Cada periférico se trata como una "instancia" independiente, permitiendo que el CPU ejecute tareas con diferentes bases de tiempo sin que una bloquee a las demás.

* **Capa de Aplicación (`main.c`)**: Orquesta la lógica de alto nivel y el flujo del sistema.
* **Capa de Abstracción (`API_LED`, `API_Delay`)**: Oculta la complejidad de los registros y las funciones HAL de ST, exponiendo una interfaz humana y genérica.



### Configuración de Tareas Concurrentes:
- **LED Verde (LD1)**: Latido de alta frecuencia (200ms).
- **LED Azul (LD2)**: Latido de media frecuencia (500ms).
- **LED Rojo (LD3)**: Latido de baja frecuencia (1000ms).
- **UART (Telemetría)**: Reporte de estado del sistema cada 1 segundo.

---

## 🛠️ Conceptos de Ingeniería Implementados

### 1. Encapsulamiento con Estructuras
En lugar de variables sueltas, agrupamos la información del periférico en `structs`. Esto permite manejar los 3 LEDs con las mismas funciones de la API, simplemente pasando una referencia diferente. Esto reduce drásticamente la duplicación de código y el uso de globales.

### 2. Paso por Referencia (Punteros)
Las funciones de la API reciben punteros a las estructuras (`delayRead(delay_t * hdelay)`). Esta técnica es el estándar en la industria para optimizar el uso de memoria RAM y permitir que una sola función modifique el estado interno de una instancia específica.

### 3. Temporización No Bloqueante (Tick-Based)
Sustituimos definitivamente `HAL_Delay()` por una lógica de consulta de Ticks basada en el contador `SysTick`. Esto permite que el CPU "salte" de una tarea a otra instantáneamente si el tiempo de una no se ha cumplido, manteniendo el sistema siempre responsivo.

---

## 💻 El Corazón de la Multitarea

El bucle principal se transforma en un planificador de tareas legible y profesional, donde la complejidad operativa ha sido delegada a los drivers:

```c
/* Definición de instancias (Objetos de Software) */
delay_t delayL1, delayL2, delayL3;
led_t ledV = {LD1_Port, LD1_Pin, false};
led_t ledA = {LD2_Port, LD2_Pin, false};
led_t ledR = {LD3_Port, LD3_Pin, false};

int main(void) {
    // ... Inicialización de Periféricos ...

    /* Inicialización de timers con bases de tiempo independientes */
    delayInit(&delayL1, 200);
    delayInit(&delayL2, 500);
    delayInit(&delayL3, 1000);

    while (1) {
        // Tarea 1: Control asíncrono del LED Verde
        if (delayRead(&delayL1)) {
            API_LED_Toggle(&ledV);
        }

        // Tarea 2: Control asíncrono del LED Azul
        if (delayRead(&delayL2)) {
            API_LED_Toggle(&ledA);
        }

        // Tarea 3: Control asíncrono del LED Rojo + Telemetría UART
        if (delayRead(&delayL3)) {
            API_LED_Toggle(&ledR);
            Debug_Log("SISTEMA OK: Ciclo de 1 segundo completado\r\n");
        }
    }
}
```

## 🔍 ¿Qué estamos demostrando aquí?

* **Reutilización:** La misma lógica de delayRead sirve para N-instancias de tiempo sin escribir código extra.
* **Independencia:** El parpadeo del LED Verde es inmune a la latencia de las tareas más lentas, garantizando un comportamiento determinista.
* **Mantenibilidad:** Si el hardware cambia (ej: se reasignan pines), el while(1) permanece intacto; solo se actualiza la declaración de los objetos al inicio.

---

*La modularidad es la herramienta que nos permite gestionar la complejidad; al abstraer el hardware, transformamos el silicio en un sistema flexible, escalable y profesional.*

🛠️ **Carlos** | Estudiante de Ing. Electrónica @UTN_FRT.  
🚀 Apasionado Autodidacta por los Sistemas Embebidos.