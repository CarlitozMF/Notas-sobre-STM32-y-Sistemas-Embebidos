# 🏗️ Nivel Básico: Fundamentos y Control de Flujo

Este nivel marca el inicio del camino en los sistemas embebidos profesionales. El enfoque principal es dominar el lenguaje **C aplicado al hardware**, la gestión eficiente de memoria y la transición del código secuencial hacia la **programación no bloqueante**.

## 🛠️ Entorno de Desarrollo
* **MCU:** STM32F439ZI (Cortex-M4 @ 180MHz)
* **IDE:** STM32CubeIDE / HAL Driver
* **Concepto Clave:** Abstracción de registros mediante capas de software.

## 📂 Laboratorios de Fundamentación

1. **[01_Hola_Mundo_GPIO](./01_Hola_Mundo_GPIO):** Primer contacto con el hardware. Configuración de pines como salidas digitales y control de los LEDs de usuario (LD1, LD2, LD3).
2. **[02_Tipos_De_Variables](./02_Tipo_De_Variables):** Introducción a `stdint.h`. Uso de tipos de ancho fijo (`uint8_t`, `int32_t`, etc.) para asegurar la portabilidad y el control de memoria.
3. **[03_Estructuras](./03_Estructuras):** Organización de datos. Agrupación de variables relacionadas para crear objetos de hardware virtuales y optimizar la RAM.
4. **[04_Bitwise_Logic](./04_Bitwise_Logic):** Manipulación de registros a nivel de bit. Uso de máscaras, `AND`, `OR` y `XOR` para configurar periféricos sin afectar otros bits.
5. **[05_GPIO_Input_Polling](./05_GPIO_Input_Polling):** Lectura de entradas digitales. Análisis del fenómeno físico del rebote (Bounce) y lectura por escaneo constante (Polling).
6. **[06_LED_Bus_Structures](./06_LED_Bus_Structures):** Manejo de buses de datos. Automatización de secuencias utilizando arreglos y bucles `for` para el control de múltiples salidas.
7. **[07_Display_7_segmentos](./07_Display_7_Segmentos):** Interfaz con periféricos visuales. Creación de tablas de verdad (Look-up Tables) para decodificar BCD a 7 segmentos.
8. **[08_Multiplex_7Seg](./08_Multiplex_7Seg):** Optimización de pines. Nociones de persistencia de la visión (POV) y multiplexación temporal para manejar múltiples displays con pocos pines.
9. **[09_Intro_MEF](./09_Intro_MEF):** Arquitectura de software. Implementación de Máquinas de Estado Finitos (modelo Moore) para organizar la lógica de control compleja.
10. **[10_API_Drivers](./10_API_Drivers):** Abstracción de Hardware. Creación de archivos `.c` y `.h` propios para encapsular funciones y gestionar el tiempo de forma no bloqueante mediante `HAL_GetTick()`.
11. **[11_Debounce_Avanzado](./11_Debounce_Avanzado):** Integración técnica. Uso de MEF y temporización asíncrona para eliminar rebotes de pulsadores sin detener la ejecución del CPU.

---

## 🚀 Proyectos Integradores (EI)
*Desafíos prácticos que combinan todos los conceptos del nivel básico.*

* **[01. Contador_Up-Down_7Seg](./Proyectos_Integradores/01.Contador_Up_Down_7Seg):** Gestión de entradas Active-Low y lógica aritmética básica.
* **[02. Semaforo_Smart](./Proyectos_Integradores/02.Semaforo_Smart):** Control de tiempos y secuencias mediante una MEF robusta.
* **[03. Simon_Dice](./Proyectos_Integradores/03.Simon_Dice):** Manejo dinámico de arreglos, generación de números pseudo-aleatorios y lógica de juego.
* **[04. Integracion_Apis](./Proyectos_Integradores/04.Integracion_APIs):** El proyecto definitivo del nivel. Un secuenciador maestro que demuestra la potencia de la modularidad y la arquitectura de software limpia.

---
*💻 Aprendiendo paso a paso el control del silicio: de la lógica binaria a la arquitectura de sistemas.*