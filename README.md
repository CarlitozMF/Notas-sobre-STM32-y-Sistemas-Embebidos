<div align="center">
  <img src="./assets/portada.png" alt="Banner STM32" width="100%">

  # Notas sobre STM32 y Sistemas Embebidos 🚀
  
  [![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
  [![STM32](https://img.shields.io/badge/Microcontroller-STM32F4-blue.svg)](https://www.st.com/en/microcontrollers-microprocessors/stm32f4-series.html)
  [![Language](https://img.shields.io/badge/Language-C-00599C.svg)](https://en.cppreference.com/w/c)


</div>

---

## 🛠️ Entorno de Desarrollo

Para este proyecto utilizo un stack de herramientas estándar de la industria:

* **Placa:** STM32 Nucleo-F439ZI (Cortex-M4 @ 180MHz)
* **IDE:** STM32CubeIDE
* **Framework:** STM32Cube HAL (Hardware Abstraction Layer)
* **Depuración:** ST-LINK/V2-1 (On-board) + STM32CubeMonitor

---

## 📂 Estructura del Aprendizaje

### 📘 [Documentación y Guías](./Documentacion/) - 🚀 EN CURSO -
*Bitácora teórica de ingeniería y análisis de hardware previa al desarrollo de firmware:*
* **Fundamentos del Silicio:** Análisis de la arquitectura Cortex-M4 del **STM32F439ZI**, mapa de memoria y árbol de relojes a **180 MHz**.
* **Entorno e Infraestructura:** Configuración profesional del **STM32CubeIDE**, inicialización estructurada de proyectos y anatomía de los bloques `USER CODE` en el `main.c`.
* **Capa 1 (Hardware Mapping):** Modularización semántica del hardware utilizando el archivo `main.h` y las *User Labels* como muro de contención ante cambios eléctricos.

### 🔌 [Drivers Propios (Capa 2)](./Drivers/) — 🚀 EN CURSO —
*Lógica de control soberana, modular y agnóstica al silicio mediante inyección de dependencias (PAL):*
* **Actuadores y Displays:** Implementación orientada a objetos para el control de **LEDs RGB**, **motores paso a paso** y multiplexación de **displays de 7 segmentos.**
* **Sensores y Captura:** Adquisición y acondicionamiento asíncrono de señales físicas, como la telemetría de distancia no bloqueante con el sensor de ultrasonido **HC-SR04**.
* **Infraestructura de Servicios:** El corazón portátil del firmware (`hal_interface.h`). Define las tablas de despacho y contratos lógicos que rompen el acoplamiento con la HAL de ST.

### 🏗️ [01_Nivel Básico (GPIO & Lógica de Control)](./01_Nivel_Basico/) - ✅ COMPLETADO -
*Fundamentos de electrónica digital y programación estructurada:*
* **Gestión de GPIO:** *Polling, Buses de salida y Multiplexación.*
* **Lógica de Control:** *Máquinas de Estado Finitos (MEF) y Debounce No Bloqueante.*
* **Modularidad:** *Creación de Drivers (APIs) para periféricos simples.*

#### **Proyectos Integradores Nivel Básico**
* **Contador_Up_Down_7Seg** | **Semaforo_Smart** | **Simon_Dice** | **Integración de APIs y Modularidad.**

### ⚙️ [02_Nivel Intermedio (Periféricos y Eficiencia)](./02_Nivel_Intermedio/) - 🚀 EN CURSO -
*Transición hacia el procesamiento basado en eventos y autonomía del hardware:*

* **Timers (Fase 1):** *PWM, Input Capture, Slave Mode y One-Pulse (OPM).*
* **HMI (Fase 2):** *Manejo de displays LCD en modo bus paralelo (4/8 bits).*
* **Adquisición (Fase 3):** *ADC avanzado: Scan Mode, IT y Sincronización por Timer.*
* **Comunicaciones (Fase 4):** *Protocolos industriales: UART (IT), I2C, SPI y CAN Bus.*

#### **Proyectos Integradores Nivel Intermedio**
* **Contador_RealTime_7Seg**  | **...** | **.** 

### 🚀 [03_Nivel Avanzado (Arquitectura de alto rendimiento)](./03_Nivel_Avanzado/) - PROXIMAMENTE -
*Arquitecturas de alto rendimiento: El salto hacia la gestión masiva de datos con DMA, procesamiento de señales (DSP) y la multiprogramación en tiempo real con RTOS.*

* **DMA Mastery:** *Transferencia masiva de datos con carga cero de CPU.*
* **RTOS:** *Sistemas Operativos de Tiempo Real (FreeRTOS) y Multitarea.*
* **DSP & Low Power:** *Procesamiento de señales y optimización energética extrema.*

---

## 🔬 Conceptos Clave Implementados

* **CPU-Offloading:** Delegación de tareas críticas al hardware (Timers/ADC) para maximizar la eficiencia.
* **Non-blocking Code:** Diseño reactivo eliminando esperas activas (`HAL_Delay`).
* **Determinismo:** Gestión de latencias y prioridades mediante configuración estricta del **NVIC**.
* **Encapsulamiento:** Drivers modulares que separan la lógica de aplicación del hardware (Capa de Abstracción).

---

## 💎 Filosofía del Repositorio

> "Diseñar sistemas embebidos no es solo hacer parpadear un LED; es estructurar el software para que el hardware sea un aliado modular, no una limitación. Este espacio documenta la evolución hacia un código predecible, eficiente y escalable, donde la abstracción es la herramienta clave para dominar la complejidad del silicio."

* **Precisión • Eficiencia • Autonomía**

---

### 🚀 Notas sobre STM32 y Sistemas Embebidos
  *Este repositorio es mi bitácora personal de aprendizaje sobre microcontroladores **STM32** y el desarrollo de sistemas embebidos de alto rendimiento.*

  🛠️ **Carlos** | Estudiante de Ing. Electrónica @UTN_FRT.  
  🚀 Apasionado Autodidacta por los Sistemas Embebidos.