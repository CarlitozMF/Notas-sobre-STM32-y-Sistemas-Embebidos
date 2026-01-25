# ⚙️ Nivel Intermedio: Sistemas de Tiempo Real y Periféricos

Bienvenido a la segunda etapa de mi bitácora de desarrollo sobre la **STM32F439ZI**. En este nivel, el enfoque cambia radicalmente: abandonamos el flujo lineal para dominar el **Procesamiento Basado en Eventos y Tareas**.

El objetivo principal es explotar el hardware interno del microcontrolador para lograr autonomía en los periféricos, aplicando el paradigma de **"CPU-Offloading"**: liberar al procesador de tareas repetitivas para que se enfoque exclusivamente en la lógica de alto nivel y toma de decisiones.

---

## 🚀 Pilares del Nivel Intermedio

### 1. Gestión de Eventos (NVIC & EXTI) ⚡
Implementación de interrupciones para lograr una latencia mínima de respuesta ante estímulos externos.
* **Determinismo:** Respuesta inmediata a eventos críticos sin depender del ciclo del `main`.
* **Priorización:** Gestión del **NVIC (Nested Vectored Interrupt Controller)** para resolver conflictos de ejecución mediante niveles de prioridad y sub-prioridad.

### 2. Control de Tiempo y Señales (Timers) ⏱️
El "corazón" del sistema. Los Timers dejan de ser simples contadores para convertirse en unidades de procesamiento de señales.
* **PWM (Pulse Width Modulation):** Control de potencia, actuadores y síntesis de señales analógicas.
* **Input Capture & Encoder Mode:** Medición de parámetros temporales externos y procesamiento por hardware de señales de cuadratura para odometría.

### 3. Digitalización de Señales (ADC) 📊
El puente crítico entre el mundo físico (analógico) y el procesamiento digital.
* **Muestreo por Interrupción:** Captura de datos en segundo plano sin detener la ejecución del código.
* **Modo Scan y Secuenciador:** Configuración de múltiples canales (sensores de línea, potenciómetros, shunts de corriente) para lecturas automatizadas.
* **Triggering:** Sincronización de la captura con eventos de Timer para obtener muestras en intervalos de tiempo exactos (esencial para análisis de señales).

### 4. Comunicación y Conectividad 📬
Estandarización del diálogo con el mundo exterior mediante protocolos industriales.
* **Arquitectura No Bloqueante:** Uso de UART/I2C/SPI con interrupciones para evitar cuellos de botella.
* **Middleware:** Creación de drivers portables y reutilizables basados en estructuras y punteros.

---

## 🏗️ Arquitectura de Software Aplicada
En esta etapa, los proyectos migran hacia un esquema de **Kernel Cooperativo (Task Scheduler)**:

* **Task Table:** Organización y ejecución de tareas basadas en períodos de tiempo específicos.
* **Estado Volátil:** Uso correcto del calificador `volatile` y gestión de secciones críticas para asegurar la consistencia de datos entre ISR y Main.
* **Abstracción:** Uso de *Handles* ,punteros y drivers para desacoplar el hardware de la lógica de aplicación.

---

## 🛠️ Roadmap de Laboratorios
1. **[01_EXTI_Pulsadores](./01_EXTI_Pulsadores):** Gestión de flancos y prioridades en el NVIC.
2. **[02_TIM_Basic](./02_TIM_Basic):** Creación de bases de tiempo precisas para tareas cíclicas.
3. **[03_TIM_PWM](./03_TIM_PWM/):** Control de potencia y efectos visuales.
4. **[04_TIM_InputCapture](./04_TIM_InputCapture):** Medición de frecuencia y sensores ultrasónicos. *(-Proximamente-)*
5. **[05_TIM_Encoder](./05_TIM_Encoder):** Lectura de encoders para control de motores. *(-Proximamente-)*
6. **[06_ADC_Multichannel](./06_ADC_Multichannel):** Digitalización de señales mediante interrupciones y secuenciadores. *(-Proximamente-)*
7. **[07_COM_UART_IT](./07_COM_UART_IT):** Comunicación bidireccional y parsing de comandos. *(-Proximamente-)*

---

## 🚀 Proyectos Integradores (EI)
*Proyectos de síntesis donde se combinan múltiples periféricos bajo una arquitectura robusta.*

### 01. [Contador de Personas Real-Time (Doble Barrera IR)](./Proyectos_Integradores/01_Contador_Displys_7Seg/)
* **Hito:** Implementación del primer **Scheduler Cooperativo**.
* **Técnicas:** EXTI Falling Edge, Timer-IT (181Hz) y Driver con soporte ASCII para 7-Segmentos.

### 02. [Smart Power Meter (Fase I: Adquisición)](./Proyectos_Integradores/02_Smart_Power_Meter/) *(-Proximamente-)*
* **Hito:** Uso de **ADC + Timers** para medir variables eléctricas (voltaje y corriente) de forma sincronizada.

---
## 📚 Referencias Técnicas
* **MCU:** STM32F439ZI (Cortex-M4 @ 180MHz).
* **Documentación:** Reference Manual (RM0090), Datasheet F439.
* **IDE:** STM32CubeIDE.

---
*💻 Orquestando el silicio: del código secuencial a la autonomía del hardware.*