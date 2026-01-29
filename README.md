<div align="center">
  <img src="./assets/portada.png" alt="Banner STM32" width="100%">

  # Notas sobre STM32 y Sistemas Embebidos 🚀
  
  [![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
  [![STM32](https://img.shields.io/badge/Microcontroller-STM32F4-blue.svg)](https://www.st.com/en/microcontrollers-microprocessors/stm32f4-series.html)
  [![Language](https://img.shields.io/badge/Language-C-00599C.svg)](https://en.cppreference.com/w/c)

  Este repositorio es mi bitácora personal de aprendizaje sobre microcontroladores **STM32** y el desarrollo de sistemas embebidos profesionales.
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

### 📘 [Documentación y Guías](./Documentacion/)
*Fundamentos técnicos y buenas prácticas antes de programar:*
* **Guía de Supervivencia:** *Particularidades del IDE y regeneración de código.*
* **Arquitectura HAL:** *Estructura de funciones, Handles y Timeouts.*
* **Diccionario de Funciones:** *Sintaxis de WritePin, ReadPin y Delay.*
* **Troubleshooting:** *Solución de errores comunes y Debugging.*

### 🏗️ [01_Nivel Básico (GPIO & Lógica de Control)](./01_Nivel_Basico/) - ✅ ACTUALIZADO 22/01/2026 -
*Fundamentos de electrónica digital y programación estructurada:*
* **01 al 04:** *Variables de ancho fijo (`stdint.h`), Estructuras y Lógica de Bits.*
* **05 al 08:** *Gestión de GPIO, Polling, Buses de salida y Multiplexación.*
* **09 al 11:** *Máquinas de Estado Finitos (MEF), creación de Drivers (APIs) y Debounce No Bloqueante.*

#### **Proyectos Integradores Nivel 1**
* **Contador_Up_Down_7Seg** | **Semaforo_Smart** | **Simon_Dice** | **Integración de APIs y Modularidad.**

### ⚙️ [02_Nivel Intermedio (Periféricos y Eficiencia)](./02_Nivel_Intermedio/) - 🚀 EN CURSO -
*Transición hacia el procesamiento basado en eventos y autonomía del hardware:*

* **01_EXTI_Pulsadores:** *Gestión de flancos y prioridades en el NVIC para interfaces reactivas.*
* **02_TIM_Basic:** *Creación de bases de tiempo precisas para la ejecución de tareas cíclicas.*
* **03_TIM_PWM:** *Control de potencia y efectos visuales asíncronos mediante **Kernel Cooperativo (Task Scheduler)**.*
* **04_TIM_Input_Capture:** *Telemetría ultrasónica y digitalización de tiempo mediante captura de eventos externos.*
* **05_TIM_Output_Compare:** *Control de precisión para motor paso a paso (28BYJ-48) y generación de eventos determinísticos.*
* **06_TIM_Slave_Mode:** *Sincronización de periféricos y frecuencímetro de hardware con sensor de color TCS3200.*
* **07_TIM_One_Pulse:** *Generación de pulsos determinísticos disparados por hardware.*
* **08_TIM_Encoder:** *Odometría y control de motores de cuadratura mediante interfaz de encoder dedicada.*


#### **Proyectos Integradores Nivel 2**
* **01. Contador_RealTime_7Seg:** *Evolución hacia Arquitectura Basada en Tareas.*
* **02. Smart_Power_Meter:** *Adquisición sincronizada de tensión y corriente.* 🆕

### 🚀 03_Nivel Avanzado (Arquitectura de alto rendimiento) - PROXIMAMENTE -
* **DMA & RTOS:** *Transferencia de datos sin intervención de CPU y sistemas operativos de tiempo real.*
* **Low Power:** *Modos de bajo consumo y optimización energética (Sleep/Stop/Standby).*
* **Digital Signal Processing (DSP):** *Filtros digitales y procesamiento de señales en el Cortex-M4.*

---

## 🔬 Conceptos Clave Implementados
* **Non-blocking Code:** Eliminación total de `HAL_Delay()` en favor de temporización por software/hardware.
* **Encapsulamiento:** Drivers modulares que separan la lógica de aplicación del hardware.
* **Determinismo:** Gestión de latencias mediante prioridades en el NVIC.

---

<div align="center">
  <p><i>Notas creadas durante mi proceso de estudio y experimentación.</i></p>
</div>