# 📂 Proyectos Integradores (Nivel Intermedio) ⚙️

Este directorio consolida las soluciones de ingeniería que marcan la transición de la programación secuencial hacia una **arquitectura basada en eventos**. El enfoque principal es la optimización del hardware autónomo de la **Nucleo-F439ZI** mediante la **Capa de Abstracción de Hardware (HAL)** para lograr determinismo y concurrencia sin la necesidad de un sistema operativo (RTOS).



## 🚀 Filosofía de Diseño: Sistemas Reactivos e Instrumentación Basados en HAL

Los proyectos integrados en esta sección demuestran que es posible alcanzar un alto grado de eficiencia, precisión y respuesta inmediata mediante el diseño estructural y el uso intensivo de los periféricos internos de la **STM32F439ZI**:

* **Determinismo y Respuesta Asíncrona (EXTI & Timers):** Uso mandatorio de interrupciones externas y de hardware para garantizar respuestas de baja latencia. El sistema deja de "esperar" eventos para empezar a "reaccionar" a ellos mediante funciones de *Callback* optimizadas.
* **Multitarea Cooperativa No Bloqueante:** Implementación de un **Scheduler manual por Time-Slicing**. El CPU distribuye su tiempo de cómputo basándose en deltas de tiempo reales, eliminando por completo funciones bloqueantes como `HAL_Delay()` y permitiendo la ejecución concurrente de múltiples tareas.
* **Adquisición y Digitalización de Señales (ADC):** Transformación de magnitudes físicas en datos digitales. Se aplican conceptos de muestreo, resolución y gestión de disparos (*triggers*) por software o hardware para obtener lecturas precisas del entorno analógico.
    
* **Interconectividad y Protocolos (UART, I2C, SPI):** Implementación de comunicación serie para el intercambio de datos con sensores inteligentes, actuadores y terminales de monitoreo. Se prioriza el manejo de tramas y buffers para asegurar la integridad de la información en el bus.
* **Independencia de Periféricos de Salida:** Delegación de tareas repetitivas (como el multiplexado de displays o la generación de **PWM**) a los periféricos autónomos, liberando el hilo principal (`while(1)`) para la toma de decisiones y lógica de control de alto nivel.
* **Optimización de ISR (Short & Fast):** Aplicación estricta del estándar de diseño para rutinas de interrupción, donde la ISR solo marca la ocurrencia del evento mediante banderas (*flags*) y delega el procesamiento pesado a la capa de aplicación.

---

## 🛠️ Proyectos Incluidos

| Proyecto | Descripción | Hito de Ingeniería |
| :--- | :--- | :--- |
| **[01. Contador_Displays_7Seg](./01_Contador_Displys_7Seg)** | Contador de 3 dígitos con refresco automático por Timer. | *Zero-Flicker Display via Hardware Timers.* |

---

## 🧠 Competencias de Ingeniería Consolidadas

### 1. Orquestación de Eventos (NVIC)
Configuración y jerarquización del controlador de interrupciones para priorizar eventos críticos (como paros de emergencia o señales de sensores de alta velocidad) sobre tareas secundarias. Se domina el concepto de **Preemption** y **Subpriority** para garantizar el determinismo del sistema.

### 2. Automatización de Tareas de Fondo
Capacidad para delegar tareas repetitivas al hardware autónomo del STM32. Esto permite que procesos como el refresco de displays mantengan una estabilidad absoluta (Zero-Flicker) independientemente de la carga computacional o bloqueos temporales en el hilo principal.

### 3. Modulación y Control de Energía (PWM)
Uso avanzado de Timers para emular salidas analógicas mediante la modificación del Duty Cycle. Se aplican modelos matemáticos (como la **Corrección Gamma**) para adaptar la entrega de potencia a la respuesta física de la carga o a la percepción del ojo humano.

### 4. Adquisición de Datos y Acondicionamiento (ADC)
Dominio del proceso de digitalización de señales del mundo físico. Se implementan técnicas de **muestreo y cuantización**, configurando resoluciones y tiempos de muestreo adecuados para evitar el *aliasing* y asegurar la fidelidad de la información analógica capturada.

### 5. Conectividad y Comunicación Industrial (UART, I2C, SPI)
Implementación de protocolos de comunicación para el intercambio de datos entre dispositivos. Se gestiona la integridad de la información mediante el uso de **Buffers circulares** y el manejo de errores de bus, permitiendo que el microcontrolador actúe como un nodo inteligente en una red de sensores o actuadores.

### 6. Planificación por Time-Slicing
Diseño de lógica de despacho de tareas (*Task Dispatcher*) que permite la ejecución concurrente de procesos con diferentes periodicidades. Esta técnica de **Multitarea Cooperativa** optimiza el uso del CPU al máximo, logrando un sistema fluido y eficiente sin la necesidad de un RTOS.

---

## 🔧 Requisitos del Laboratorio
- **Hardware:** Nucleo-F439ZI + Sensores/Actuadores externos + Analizador Lógico para validación de tiempos.
- **Clock Tree:** Sistema configurado a **180 MHz** para garantizar la máxima resolución y precisión en los periféricos de tiempo.

---
# **🏁 HACIA EL NIVEL AVANZADO** *Con estos proyectos, hemos alcanzado el límite de lo que se puede lograr con una arquitectura organizada sobre drivers de abstracción. El siguiente paso es el **Nivel Avanzado**, donde introduciremos **DMA (Direct Memory Access)** para mover datos sin intervención del CPU y, finalmente, la migración hacia un **RTOS (FreeRTOS)** para gestionar sistemas de misión crítica.*

---
*“La ingeniería embebida profesional comienza cuando el programador deja de preguntar qué pasó y diseña un sistema que reacciona cuando sucede.”*