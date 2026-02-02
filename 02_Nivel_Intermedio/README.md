# 📂 Nivel Intermedio: Sistemas Reactivos y Periféricos Avanzados ⚙️

Bienvenido al **Nivel Intermedio**. Tras dominar los fundamentos de la programación modular en el Nivel Básico, aquí evolucionamos hacia una arquitectura de software **basada en eventos y tareas**. El objetivo central es optimizar el uso del CPU mediante el aprovechamiento de los periféricos de hardware integrados en el **Cortex-M4**.

El objetivo principal es explotar el hardware interno del microcontrolador para lograr autonomía en los periféricos, aplicando el paradigma de **"CPU-Offloading"**: liberar al procesador de tareas repetitivas para que se enfoque exclusivamente en la lógica de alto nivel y toma de decisiones.

## 🧠 Competencias a Desarrollar
Al finalizar este nivel, se habrán consolidado las siguientes capacidades críticas para el diseño de sistemas de tiempo real:

* **Gestión del NVIC y Determinismo:** Configuración avanzada del controlador de interrupciones para garantizar tiempos de respuesta predecibles. Manejo estricto de **Prioridades** y **Sub-prioridades** para mitigar condiciones de carrera (*Race Conditions*) en sistemas multi-evento.
* **Diseño de ISR (Interrupt Service Routines) de Alto Rendimiento:** Aplicación del paradigma **"Short & Fast"**: las rutinas de interrupción deben limitarse a gestionar banderas y datos críticos, delegando el procesamiento pesado al flujo principal para mantener la latencia del sistema al mínimo.
* **Orquestación de Hardware Periférico:** Capacidad de interconectar Timers, ADC y Comunicaciones de forma autónoma (Hardware Synchronization). Los Timers actúan como disparadores (*Triggers*) de otros periféricos, minimizando la carga del CPU (**CPU-Offloading**).
* **Gestión de Memoria y Scope:** Uso profesional de calificadores de almacenamiento como `static` para encapsulamiento de datos y `volatile` para garantizar la coherencia de memoria en contextos de interrupción.
* **Sincronización de Datos (Atomicidad):** Implementación de secciones críticas y barreras de acceso para asegurar que la transferencia de datos entre las ISR y el `main` sea atómica y libre de corrupciones.

---

## 🚀 Pilares del Nivel Intermedio

### 1. Gestión de Eventos (NVIC & EXTI) ⚡
Implementación de interrupciones para lograr una latencia mínima de respuesta ante estímulos externos.
* **Determinismo:** Respuesta inmediata a eventos críticos sin depender del ciclo del `main`.
* **Priorización:** Gestión del **NVIC (Nested Vectored Interrupt Controller)** para resolver conflictos de ejecución mediante niveles de prioridad y sub-prioridad.

### 2. Control de Tiempo y Señales (Timers) ⏱️
El "corazón" del sistema. Los Timers dejan de ser simples contadores para convertirse en unidades de procesamiento de señales especializadas y autónomas:

* **PWM (Pulse Width Modulation):** Control de potencia, actuadores y posicionamiento de precisión (Servos).
* **Input Capture:** Digitalización de parámetros temporales externos mediante la captura de marcas de tiempo (frecuencia, período y ancho de pulso).
* **Output Compare:** Generación de eventos determinísticos, síntesis de frecuencias de audio y control de motores paso a paso.
* **Slave, Gated & Reset Mode:** Orquestación y sincronización de periféricos por hardware (Master/Slave) para automatizar tareas sin intervención del CPU.
* **One-Pulse Mode (OPM):** Generación de pulsos monostables únicos disparados por eventos externos, garantizando una duración exacta e ininterrumpible.
* **Encoder Mode:** Procesamiento nativo de señales de cuadratura mediante lógica combinacional interna para odometría y control de posición (Base del proyecto **Micromouse**).

* **Funciones Avanzadas de Robustez:**
    * **Dead-Time Insertion:** Generación de retardos de seguridad entre señales complementarias para evitar cortocircuitos en puentes H (Inversores de potencia).
    * **Break Input:** Función de seguridad por hardware que fuerza la detención inmediata de las salidas PWM ante una señal de falla crítica (E-Stop).
    * **Update Event (UEV) & Repetition Counter:** Control preciso de cuándo se actualizan los registros de sombra para evitar fallos en la generación de señales de alta frecuencia.

### 3. Visualización y HMI (Displays) 🖥️
El puente de comunicación visual con el usuario. En esta etapa se trasciende el uso de la UART para lograr autonomía total del hardware mediante la gestión de buses de datos.
* **Bus Paralelo (4/8-bits):** Implementación de protocolos de comunicación para controladores estándar (HD44780), gestionando señales de control críticas: **RS** (Register Select), **E** (Enable) y **RW** (Read/Write).
* **Modularidad y Abstracción:** Desarrollo de librerías propias para desacoplar los registros del microcontrolador de la lógica de visualización, facilitando la portabilidad del código.

### 4. Digitalización de Señales (ADC) 📊
Captura del mundo físico y conversión en datos procesables con precisión de nivel industrial. Es el pilar fundamental para el procesamiento de señales analógicas y el monitoreo de variables en tiempo real.

* **Modos de Operación Críticos:**
    * **Single Conversion:** Una única captura por cada disparo (software o hardware). Ideal para lecturas estáticas como sensores de temperatura.
    * **Continuous Conversion:** El ADC reinicia automáticamente una nueva conversión apenas termina la anterior, útil para monitoreo constante.
    * **Scan Mode:** Permite recorrer una lista de canales seleccionados en una sola secuencia, evitando configurar el periférico canal por canal.
    * **Injected Group:** Conversiones de alta prioridad que pueden "interrumpir" la secuencia regular (Regular Group), esencial para protecciones de sobrecorriente o eventos críticos.

* **Determinismo y Triggering:** Sincronización de capturas mediante eventos de hardware de un Timer (p. ej. *Update Event*). Esto garantiza un **muestreo isócrono** (intervalos de tiempo exactos), eliminando el *jitter* que produce el software y permitiendo análisis de frecuencia preciso (FFT).

* **Gestión de Datos:** Implementación de estrategias para evitar la pérdida de muestras y optimizar el uso del procesador:
    * **Polling:** Método básico por consulta de banderas (bloqueante).
    * **Interrupt (IT):** Notificación al CPU mediante el NVIC al completar una conversión o secuencia.
    * **Analog Watchdog:** Monitorización por hardware que dispara una interrupción solo si la señal sale de un umbral predefinido (Límite Superior/Inferior).

### 5. Comunicación y Conectividad 📬
Estandarización del diálogo con el mundo exterior mediante protocolos industriales. La conectividad es el pilar que permite la telemetría, el control remoto y la integración en redes de datos.

* **Protocolos Soportados:**
    * **UART/USART:** Comunicación serial asíncrona para depuración (printf), control de módems (AT commands) e interfaces RS-232/RS-485.
    * **I2C (Inter-Integrated Circuit):** Bus de dos cables para comunicación con sensores locales, memorias EEPROM y expansores de GPIO.
    * **SPI (Serial Peripheral Interface):** Bus de alta velocidad para dispositivos que requieren gran ancho de banda (Pantallas TFT, tarjetas SD, módulos Ethernet).

* **Estrategias de Gestión de Datos (Arquitectura No Bloqueante):**
    * **Interrupt-Driven (IT):** Uso del NVIC para procesar cada byte recibido o enviado en segundo plano, eliminando el uso de bucles de espera (`while`) y evitando cuellos de botella en el `main`.
    * **Manejo de Buffers Circulares:** Implementación de colas (FIFO) para la recepción de tramas, permitiendo que el sistema procese la información a su propio ritmo sin perder datos entrantes.
    * **Detección de Errores por Hardware:** Monitoreo de flags de *Overrun*, *Noise Error* y *Frame Error* para garantizar la integridad de la comunicación en entornos industriales con ruido eléctrico.

* **Abstracción de Mensajería:** Implementación de protocolos de capa de aplicación simples (Parsing de comandos) para estructurar el intercambio de información entre el microcontrolador y aplicaciones externas (Serial Monitor, Python scripts, etc.).

---

## 🏗️ Arquitectura de Software Aplicada
En esta etapa, los proyectos migran hacia un esquema de **Arquitectura Orientada a Eventos**, escalando en complejidad y robustez según los requerimientos del sistema:

* **Jerarquía de Control de Estados (de menor a mayor complejidad):**
    1. **FSM Básica (Switch-Case):** Implementación de Máquinas de Estados Finitos dentro del bucle principal para gestionar lógica secuencial simple.
    2. **MEF No Bloqueante:** Integración de estados con temporización basada en `Systick` o Timers, eliminando esperas activas.
    3. **FSM Dirigida por Eventos (Event-Driven):** Los cambios de estado no se consultan activamente, sino que son disparados exclusivamente por interrupciones (EXTI, Timer-IT), minimizando el consumo de energía.
    4. **Máquinas de Estados Jerárquicas:** Estructuras donde un estado puede contener sub-estados, ideal para sistemas con menús complejos o múltiples modos de operación.

* **Estado Volátil y Secciones Críticas:** Uso estricto del calificador `volatile` para variables compartidas entre el flujo principal y las ISR. Implementación de barreras de acceso para garantizar la **atomicidad** de los datos y evitar condiciones de carrera.

* **Abstracción y Encapsulamiento:** Uso de *Handles* (estructuras de configuración), punteros y drivers modulares. Esta técnica permite desacoplar la lógica de aplicación del hardware específico, facilitando la escalabilidad hacia otros microcontroladores.

* **Task Scheduler (Kernel Cooperativo):** Ejecución de tareas basadas en períodos de tiempo específicos. El CPU orquesta "cuándo" se ejecuta cada función sin necesidad de un RTOS completo, manteniendo un flujo de programa limpio y predecible.

---

## 🛠️ Roadmap de Laboratorios *(-Actualizado 01/02/2026-)*

### ⏱️ Fase 1: Control de Tiempo y Señales (Timers)
1.  **[01_EXTI_Pulsadores](./01_EXTI_Pulsadores):** Gestión de flancos y prioridades en el NVIC.
2.  **[02_TIM_Basic](./02_TIM_Basic):** Creación de bases de tiempo precisas para tareas cíclicas.
3.  **[03_TIM_PWM](./03_TIM_PWM/):** Control de potencia y efectos visuales asíncronos.
4.  **[04_TIM_Input_Capture](./04_TIM_Input_Capture):** Telemetría ultrasónica y digitalización de tiempo.
5.  **[05_TIM_Output_Compare](./05_TIM_Output_Compare):** Control de precisión para motor paso a paso (28BYJ-48).
6.  **[06_TIM_PWM_Advance](./06_TIM_PWM_Advance):** Control de posición mediante resolución temporal (Servo SG90).
7.  **[07_TIM_OC_Advance](./07_TIM_OC_Advance):** Generación de tonos de audio mediante frecuencia variable (Buzzer Pasivo).
8.  **[08_TIM_Slave_Mode](./08_TIM_Slave_Mode):** Sincronización de periféricos y sensor de color TCS3200.
9.  **[09_TIM_One_Pulse](./09_TIM_One_Pulse):** Alarma Monostable Determinística disparada por hardware (HW-201). 🚀 *(-En curso-)*
10. **[10_TIM_Encoder](./10_TIM_Encoder):** Odometría y control de motores de cuadratura (Base Micromouse). *(-Proximamente-)*

### 🖥️ Fase 2: Interfaz Visual y HMI
11. **[11_LCD_Parallel_8bit](./11_LCD_Parallel_8bit):** Manejo de buses de datos y protocolo HD44780 (8 bits). *(-Proximamente-)*
12. **[12_LCD_Parallel_4bit](./12_LCD_Parallel_4bit):** Optimización de pines y creación de caracteres personalizados (CGRAM). *(-Proximamente-)*

### 📊 Fase 3: Digitalización de Señales (ADC)
13. **[13_ADC_Single_Polling](./13_ADC_Single_Polling):** Lectura analógica básica por consulta de bandera (Bloqueante). *(-Proximamente-)*
14. **[14_ADC_Continuous_IT](./14_ADC_Continuous_IT):** Muestreo en segundo plano mediante interrupciones del NVIC. *(-Proximamente-)*
15. **[15_ADC_Scan_Mode](./15_ADC_Scan_Mode):** Digitalización secuencial de múltiples canales analógicos. *(-Proximamente-)*
16. **[16_ADC_Injected_Group](./16_ADC_Injected_Group):** Gestión de conversiones de alta prioridad para eventos críticos. *(-Proximamente-)*
17. **[17_ADC_Timer_Trigger](./17_ADC_Timer_Trigger):** Muestreo isócrono disparado por hardware. *(-Proximamente-)*
18. **[18_ADC_Analog_Watchdog](./18_ADC_Analog_Watchdog):** Monitoreo de umbrales por hardware sin intervención del CPU. *(-Proximamente-)*

### 📬 Fase 4: Comunicación y Conectividad (Buses Industriales)
19. **[19_UART_USART_IT](./19_UART_USART_IT):** Comunicación serial asíncrona mediante interrupciones y buffers circulares. *(-Proximamente-)*
20. **[20_I2C_Master_Mode](./20_I2C_Master_Mode):** Lectura de sensores inerciales o memorias EEPROM mediante bus de dos hilos. *(-Proximamente-)*
21. **[21_SPI_High_Speed](./21_SPI_High_Speed):** Interfaz periférica serial para manejo de periféricos de alta velocidad (Displays TFT / SD). *(-Proximamente-)*
22. **[22_CAN_Bus_Foundations](./22_CAN_Bus_Foundations):** Introducción a redes de control de área (Automotriz) y robustez diferencial. *(-Proximamente-)*

---

## 🚀 Proyectos Integradores (EI)

### 01. [Contador de Personas Real-Time](./Proyectos_Integradores/01_Contador_Displys_7Seg/)
* **Técnicas:** EXTI Falling Edge, Timer-IT y Scheduler Cooperativo.
* **Hardware:** Sensores IR, Display 7-Segmentos multiplexado.

---

## 📚 Referencias Técnicas
Para el desarrollo de este nivel, se consultan de forma recurrente los siguientes documentos oficiales de **STMicroelectronics**:

* **MCU:** STM32F439ZI (Cortex-M4 @ 180MHz) - Nucleo-144.
* **Reference Manual (RM0090):** El documento "maestro". Detalla el funcionamiento de todos los registros de los Timers, ADC y periféricos de comunicación.
* **Datasheet (STM32F437/439):** Especificaciones eléctricas, mapeo de pines y capacidades máximas del hardware.
* **Programming Manual (PM0214):** Guía específica del núcleo **Cortex-M4**, vital para entender el NVIC y las instrucciones DSP.
* **HAL Driver User Manual (UM1725):** Descripción detallada de todas las funciones de la capa de abstracción de hardware (HAL).
* **Application Note (AN4013):** Guía específica para el uso de Timers en modo Master/Slave y aplicaciones avanzadas.

**Herramientas de Software:**
* **IDE:** STM32CubeIDE (v1.13.0+).
* **STM32CubeMX:** Para la configuración visual de relojes (Clock Tree) y pinout.
* **STM32CubeMonitor:** Para la visualización de variables analógicas (ADC) en tiempo real sin pausar el CPU.

---

<div align="center">
  <h3>🚀 "En el nivel intermedio, el programador deja de controlar el flujo y empieza a orquestar los eventos del hardware."</h3>
  <p><i>Hacia una ingeniería de sistemas embebidos determinística y eficiente.</i></p>
</div>