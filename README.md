<div align="center">
  <img src="./assets/portada.png" alt="Banner STM32" width="100%">

  # Notas sobre STM32 y Sistemas Embebidos 🚀
  
  [![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
  [![STM32](https://img.shields.io/badge/Microcontroller-STM32F4-blue.svg)](https://www.st.com/en/microcontrollers-microprocessors/stm32f4-series.html)
  [![Language](https://img.shields.io/badge/Language-C-00599C.svg)](https://en.cppreference.com/w/c)

  *Este repositorio es mi bitácora personal de aprendizaje sobre microcontroladores **STM32** y el desarrollo de sistemas embebidos de alto rendimiento.*
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

### 📘 [Documentación y Guías](./Documentacion/) -En Proceso-
*Fundamentos técnicos y buenas prácticas antes de programar:*


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

<div align="center">
  <h3>💎 Filosofía del Repositorio</h3>
  <p>
    <i>
      "Desde el control manual del bit hasta la orquestación autónoma del silicio. 
      Este espacio documenta la transición del programador secuencial al arquitecto de sistemas reactivos, 
      donde cada microsegundo cuenta y el determinismo es la regla de oro."
    </i>
  </p>
  
  <p>
    <b>Precisión • Eficiencia • Autonomía</b>
  </p>
  
  <p>
    🚀 <b>Notas-sobre-STM32-y-Sistemas-Embebidos creadas durante mi proceso de estudio</b> | <b>Carlos - UTN FRT 2026</b>
  </p>
</div>