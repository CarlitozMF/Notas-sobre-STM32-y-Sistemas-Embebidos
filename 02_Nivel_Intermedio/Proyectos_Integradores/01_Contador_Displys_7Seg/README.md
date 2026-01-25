# 🚀 Proyecto Integrador: Contador de Personas Real-Time con Scheduler Cooperativo

Este proyecto representa un hito en el **Nivel Intermedio** de mi formación en sistemas embebidos. Se ha diseñado un sistema de conteo bidireccional robusto, modular y dirigido por eventos, utilizando la potencia de la plataforma **STM32F439ZI (Nucleo-144)**.

## 🎯 Objetivos
- **Implementar una arquitectura orientada a tareas** mediante un despachador (*Task Scheduler*) cooperativo.
- **Desarrollar un Middleware (Driver)** para la gestión autónoma de displays de 7 segmentos.
- **Sincronizar eventos asíncronos (EXTI)** con tareas de refresco periódico (TIM) y telemetría (UART).
- **Garantizar el determinismo** del sistema mediante el uso de interrupciones para tareas de tiempo crítico.

---

## 🧠 Arquitectura de Software: El Kernel Cooperativo

El sistema evoluciona del flujo secuencial tradicional hacia una **Arquitectura Basada en Tareas**, estructurada bajo los siguientes pilares:

### 1. Planificador (Task Scheduler)
Se implementó un despachador de tareas no bloqueante en el `while(1)`. Las tareas se ejecutan basándose en una tabla de control (`task_t`) que compara el tiempo transcurrido mediante el `SysTick` de la HAL.

```mermaid
graph TD
    A[Inicio del Programa] --> B{HAL_Init & Periféricos}
    B --> C[Driver Display Init]
    C --> D[Mensaje Inicial HI]
    D --> E{Loop Principal: while 1}
    E --> F[currentTick = HAL_GetTick]
    F --> G{Para cada Tarea en la Tabla}
    G --> H{¿Tiempo de Tarea Cumplido?}
    H -- Sí --> I[Ejecutar Tarea ej: Heartbeat, UART]
    I --> J[Actualizar lastTick de la Tarea]
    J --> G
    H -- No --> G
    G -- Fin de Tareas --> E

    subgraph Interrupciones de Hardware
        K[Timer 2 ISR - Refresh Display]
        L[EXTI ISR - Sensores HW-201]
    end
    E -.-> K
    E -.-> L
```
### 2. Driver Display_7Seg_stm32 (Middleware)

Se desarrolló un driver genérico desacoplado del hardware con las siguientes características:

* **Multiplexación por Hardware:** Utiliza el Timer 2 para el refresco automático de los dígitos en segundo plano.
* **Buffer de Video:** Implementa una variable de memoria que actúa como buffer, separando la lógica del contador de la visualización física.
* **Interfaz Avanzada:** Soporte para control de brillo, efectos de parpadeo (Flash) y mapeo de caracteres alfanuméricos básicos.

---

## ⚙️ Configuración Técnica y Sincronismo
### Temporización (Timer 2)

Configurado sobre el bus **APB1 a 90 MHz** para el refresco de los displays:

* **Prescaler:** 8999 (Frecuencia de conteo de 10 kHz).
* **ARR (Period):** 54 (Evento de interrupción cada ~5.5 ms).
* **Frecuencia de Refresco:** ~181 Hz globales (~60 Hz por dígito), garantizando una imagen estable y libre de parpadeo (Flicker-free).

### Interrupciones Externas (EXTI)

Los sensores de proximidad HW-201 operan mediante interrupciones para garantizar latencia cero en la detección:
* **Modo:** *Falling Edge* (Detección por flanco de bajada).
* **Debounce por Software:** Filtro de tiempo de 600ms para evitar falsos disparos por ruido o rebotes mecánicos.

---

## 🛠️ Diagrama de Bloques Lógico

```mermaid
graph LR
    subgraph Entradas
        S1[Sensor 1 - Entrada]
        S2[Sensor 2 - Salida]
    end

    subgraph Procesamiento STM32
        EXTI[EXTI Callback]
        SCH[Scheduler Cooperativo]
        T2[Timer 2 ISR]
        BUF[Display Buffer]
    end

    subgraph Salidas
        D7[Display 7 Segmentos]
        UART[Terminal Serial UART]
        LED[LED Heartbeat]
    end

    S1 & S2 --> EXTI
    EXTI --> SCH
    SCH --> BUF
    BUF --> T2
    T2 --> D7
    SCH --> UART
    SCH --> LED
```

---

## 🔬 Conceptos de Programación Aplicados

* **Modificador `volatile`:** Aplicado a las variables compartidas entre el `main` y las ISR (como el contador de personas), asegurando que el compilador no optimice lecturas erróneamente.
* **Variables `static` locales:** Utilizadas dentro de las funciones de tarea para persistir estados de tiempo sin exponer variables al ámbito global.
* **Encapsulamiento:** La lógica de multiplexado está contenida en el driver, permitiendo que la aplicación solo se preocupe por actualizar el valor numérico.

---
*“Nivel Intermedio: La maestría reside en orquestar el hardware para que el sistema parezca inteligente y autónomo, mientras el CPU permanece eficiente y disponible para la siguiente tarea.”*