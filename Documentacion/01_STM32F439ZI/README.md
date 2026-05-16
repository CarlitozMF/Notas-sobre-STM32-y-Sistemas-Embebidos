# 01. 🧠 STM32F439ZI: Arquitectura y Periféricos

Este documento profundiza en las capacidades del silicio **STM32F439ZI**. Comprender la organización interna es vital para desarrollar la **Capa 1 (Hardware Mapping)** sin depender de herramientas de generación de código, permitiendo un control total sobre el hardware.

---

## 1. 🏗️ Arquitectura del Sistema (Multi-AHB Matrix)
El STM32F439ZI utiliza una matriz de buses **Multi-AHB** de 32 bits. Según el **RM0090**, esta estructura permite que varios maestros (como el DMA o la CPU) accedan a diferentes esclavos simultáneamente sin colisiones.

* **Acelerador ART (Adaptive Real-Time):** Permite la ejecución de código desde la Flash con 0 estados de espera a **180 MHz**.
* **Memoria CCM (Core Coupled Memory):** $64\text{ KB}$ de RAM acoplada directamente al bus de datos del núcleo para algoritmos críticos (como el procesamiento DSP en mediciones).



---

## 2. 🗺️ Mapa de Memoria y Registros Críticos
Para programar a nivel de registros, estas son las direcciones base fundamentales extraídas del **Datasheet DS9484**:

| Periférico | Dirección Base | Bus |
| :--- | :--- | :--- |
| **GPIOA** | `0x4002 0000` | AHB1 |
| **GPIOB** | `0x4002 0400` | AHB1 |
| **GPIOC** | `0x4002 0800` | AHB1 |
| **ADC1 / ADC2 / ADC3** | `0x4001 2000` - `0x4001 2300` | APB2 |
| **LTDC (LCD-TFT)** | `0x4001 6800` | APB2 |
| **USART3** | `0x4000 4800` | APB1 |

---

## 3. 🔌 Periféricos de Alto Rendimiento

### 3.1 📊 Conversión Analógica-Digital (ADC)
El sistema analógico es clave para proyectos de instrumentación (Smart Power Meter).
* **Resolución:** 12 bits (4096 niveles).
* **Velocidad:** Hasta **2.4 MSPS** (Mega Samples Per Second).
* **Modo Triple Interleaved:** Al usar los 3 ADCs en paralelo, se puede alcanzar una tasa de muestreo combinada de **7.2 MSPS**.
* **Canales Internos:** Incluye sensores para monitoreo de temperatura interna, $V_{REFINT}$ y $V_{BAT}$.



### 3.2 🖼️ Controlador LCD-TFT (LTDC)
Periférico especializado en el manejo de pantallas gráficas de forma eficiente.
* **Capas:** Soporta 2 capas independientes con **Alpha Blending** (transparencias).
* **Resolución:** Soporta hasta XGA ($1024 \times 768$).
* **Sinergia con DMA2D (Chrom-ART):** Permite mover bloques de memoria gráfica (BitBlt) y convertir formatos de color sin usar ciclos de la CPU.

### 3.3 ⚡ Gestión de Energía (PWR)
Configuración de estados para optimizar el consumo de corriente según el **RM0090**:
* **Sleep:** Detiene la CPU, periféricos activos.
* **Stop:** Detiene todos los relojes, mantiene SRAM.
* **Standby:** Apagado casi total ($2.2 \mu A$), solo despierta por RTC o pin de Wakeup.

---

## 4. 📍 GPIO y Funciones Alternativas (AF)
Cada pin físico puede multiplexarse hasta en 16 funciones distintas mediante los registros `GPIOx_AFRL` y `GPIOx_AFRH`.

* **Digital:** Input (con/sin Pull-up/down), Output (Push-pull / Open-drain).
* **Analog:** Desconecta los buffers digitales para evitar ruido en el ADC/DAC.
* **Velocidad:** Configurable en 4 niveles (Low, Medium, Fast, High) para control de EMI.

---

## 5. 🔗 Gestión de Interrupciones (NVIC)
El controlador **NVIC** (Nested Vectored Interrupt Controller) gestiona la ejecución en tiempo real:
* **Prioridades:** 16 niveles de prioridad configurables por software.
* **Tail-chaining:** Tecnología que reduce la latencia entre interrupciones consecutivas, ideal para sistemas de control de alta velocidad.

---

**Siguiente paso:** [02_Arquitectura de Software](../02_Arquitectura_De_Software/README.md)

🛠️ **Carlos** | Estudiante de Ing. Electrónica @UTN_FRT.  
🚀 Apasionado Autodidacta por los Sistemas Embebidos.