# 📂 Nivel Intermedio: Sistemas Reactivos y Periféricos Avanzados ⚙️

Bienvenido al **Nivel Intermedio**. Tras dominar los fundamentos de la programación modular en el Nivel Básico, aquí evolucionamos hacia una arquitectura de software **basada en eventos y tareas**. El objetivo central es optimizar el uso del CPU mediante el aprovechamiento de los periféricos de hardware integrados en el **Cortex-M4**.

El objetivo principal es explotar el hardware interno del microcontrolador para lograr autonomía en los periféricos, aplicando el paradigma de **"CPU-Offloading"**: liberar al procesador de tareas repetitivas para que se enfoque exclusivamente en la lógica de alto nivel y toma de decisiones.

---

## 🚀 Pilares del Nivel Intermedio

### 1. Gestión de Eventos (NVIC & EXTI) ⚡
Implementación de interrupciones para lograr una latencia mínima de respuesta ante estímulos externos.
* **Determinismo:** Respuesta inmediata a eventos críticos sin depender del ciclo del `main`.
* **Priorización:** Gestión del **NVIC (Nested Vectored Interrupt Controller)** para resolver conflictos de ejecución mediante niveles de prioridad y sub-prioridad.

### 2. Control de Tiempo y Señales (Timers) ⏱️
El "corazón" del sistema. Los Timers dejan de ser simples contadores para convertirse en unidades de procesamiento de señales especializadas:
* **PWM (Pulse Width Modulation):** Control de potencia, actuadores y posicionamiento de precisión (Servos).
* **Input Capture:** Digitalización de parámetros temporales externos (frecuencia, período y ancho de pulso).
* **Output Compare:** Generación de eventos, frecuencias de audio y control de motores paso a paso.
* **Slave & Gated Mode:** Interconexión de periféricos por hardware (Maestro/Esclavo) para orquestar tareas sin intervención del CPU.
* **Encoder Mode:** Procesamiento por hardware de señales de cuadratura para odometría (Base para el proyecto Micromouse).



### 3. Digitalización de Señales (ADC) 📊
El puente crítico entre el mundo físico (analógico) y el procesamiento digital.
* **Muestreo por Interrupción:** Captura de datos en segundo plano.
* **Triggering:** Sincronización de la captura con eventos de Timer para obtener muestras en intervalos de tiempo exactos.

### 4. Comunicación y Conectividad 📬
Estandarización del diálogo con el mundo exterior mediante protocolos industriales.
* **Arquitectura No Bloqueante:** Uso de UART/I2C/SPI con interrupciones para evitar cuellos de botella.

---

## 🏗️ Arquitectura de Software Aplicada
En esta etapa, los proyectos migran hacia un esquema de **Arquitectura Orientada a Eventos**:

* **Task Scheduler:** Ejecución de tareas basadas en períodos de tiempo específicos sin bloqueos.
* **Estado Volátil:** Uso correcto del calificador `volatile` y gestión de secciones críticas entre ISR y Main.
* **Abstracción:** Uso de *Handles*, punteros y drivers para desacoplar el hardware de la lógica de aplicación.

---

## 🛠️ Roadmap de Laboratorios *(-Actualizado 31/01/2026-)*
1. **[01_EXTI_Pulsadores](./01_EXTI_Pulsadores):** Gestión de flancos y prioridades en el NVIC.
2. **[02_TIM_Basic](./02_TIM_Basic):** Creación de bases de tiempo precisas para tareas cíclicas.
3. **[03_TIM_PWM](./03_TIM_PWM/):** Control de potencia y efectos visuales asíncronos.
4. **[04_TIM_Input_Capture](./04_TIM_Input_Capture):** Telemetría ultrasónica y digitalización de tiempo.
5. **[05_TIM_Output_Compare](./05_TIM_Output_Compare):** Control de precisión para motor paso a paso (28BYJ-48).
6. **[06_TIM_PWM_Advance](./06_TIM_PWM_Advance):** Control de posición mediante resolución temporal (Servomotor SG90). 
7. **[07_TIM_OC_Advance](./07_TIM_OC_Advance):** Generación de tonos de audio mediante frecuencia variable (Buzzer Pasivo).
8. **[08_TIM_Slave_Mode](./08_TIM_Slave_Mode):** Sincronización de periféricos y frecuencímetro con sensor TCS3200. *(-En Desarrollo-)*
9. **[09_TIM_One_Pulse](./09_TIM_One_Pulse):** Generación de pulsos determinísticos disparados por hardware. *(-Proximamente-)*
10. **[10_TIM_Encoder](./10_TIM_Encoder):** Odometría y control de motores de cuadratura (Micromouse). *(-Proximamente-)*

---

## 🚀 Proyectos Integradores (EI)

### 01. [Contador de Personas Real-Time](./Proyectos_Integradores/01_Contador_Displys_7Seg/)
* **Técnicas:** EXTI Falling Edge, Timer-IT y Scheduler Cooperativo.
* **Hardware:** Sensores IR, Display 7-Segmentos multiplexado.

---

## 🧠 Competencias a Desarrollar
- Configuración del **NVIC** y manejo estricto de prioridades para evitar condiciones de carrera.
- Diseño de **ISR (Interrupt Service Routines)** eficientes (Short & Fast).
- Orquestación de **Timers** como generadores de eventos y controladores de actuadores.
- Gestión de memoria mediante descriptores de hardware y calificador `static`.

---

## 📚 Referencias Técnicas
* **MCU:** STM32F439ZI (Cortex-M4 @ 180MHz).
* **Documentación:** Reference Manual (RM0090), Datasheet F439.
* **IDE:** STM32CubeIDE.

---
*💻 “En el nivel intermedio, el programador deja de controlar el flujo y empieza a orquestar los eventos del hardware.”*