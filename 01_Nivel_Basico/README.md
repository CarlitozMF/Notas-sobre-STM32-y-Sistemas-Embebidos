# 🏗️ Nivel Básico: Fundamentos y Control de Flujo

Este nivel marca el inicio del camino en los sistemas embebidos profesionales. El enfoque principal es dominar el lenguaje **C aplicado al hardware**, la gestión eficiente de memoria y la transición del código secuencial hacia la **programación no bloqueante**. 

El objetivo es dejar de escribir código lineal para empezar a construir bloques de software modulares y reutilizables.

---

## 🧠 Competencias a Desarrollar
Al finalizar este nivel, se habrán consolidado los cimientos técnicos necesarios para interactuar con cualquier arquitectura de microcontroladores:

* **Dominio de C Embebido:** Uso estricto de tipos de ancho fijo (`stdint.h`) y manipulación de registros mediante **Lógica de Bits** (Máscaras y desplazamientos).
* **Gestión de Memoria y Organización:** Aplicación de estructuras y arreglos para optimizar el uso de la RAM y organizar objetos de hardware.
* **Pensamiento No Bloqueante:** Sustitución definitiva de esperas activas (`delay`) por gestión de tiempos basada en conteo de ticks de sistema.
* **Arquitectura de Drivers:** Capacidad de encapsular hardware en archivos `.h` y `.c` (APIs propias), aplicando el principio de **Encapsulamiento**.
* **Abstracción de Hardware:** Comprensión de las capas de software (Capa de Aplicación vs. Capa de Driver) mediante el uso del framework HAL.

---

## 🚀 Pilares del Nivel Básico

### 1. Manipulación de Datos y Bits 🔢
El lenguaje del microcontrolador. Sin el dominio del bit, no hay control real sobre el hardware.
* **Bitwise Logic:** Uso de operadores `&`, `|`, `^` y `~` para modificar registros de periféricos sin alterar bits adyacentes.
* **Portabilidad:** Implementación de `uint8_t`, `int16_t`, etc., para asegurar que el código se comporte igual en un STM32 que en cualquier otro procesador.

### 2. Gestión de GPIO (General Purpose I/O) 🔌
El primer puente con el mundo físico.
* **Modos de Configuración:** Pull-up, Pull-down, Push-Pull y Open-Drain.
* **Lectura por Polling:** Monitoreo constante del estado de pines de entrada y gestión del fenómeno físico del **Rebote (Bounce)**.

### 3. Interfaz de Usuario y Multiplexación 💡
Optimización de recursos y visualización de datos.
* **Look-up Tables:** Creación de tablas de búsqueda para decodificación eficiente (BCD a 7 segmentos).
* **Persistencia de la Visión (POV):** Implementación de multiplexación temporal para controlar múltiples dispositivos con el mínimo de pines posibles.

### 4. Arquitectura de Drivers y Encapsulamiento 🏗️
El paso definitivo hacia la programación profesional. Aquí el enfoque cambia de "hacer que ande" a "hacer que sea mantenible".
* **Modularidad (Archivos .h / .c):** Separación de responsabilidades. El archivo de cabecera (`.h`) define la interfaz (qué hace el driver), mientras que el archivo fuente (`.c`) oculta la implementación (cómo lo hace).
* **Encapsulamiento de Hardware:** Uso de estructuras y descriptores para manejar periféricos como "objetos", evitando el uso de variables globales dispersas y protegiendo el estado interno del hardware.
* **Abstracción de Capas:** Creación de una capa de software propia sobre la HAL, permitiendo que la lógica de aplicación sea independiente del pinout o del hardware específico.
* **Uso de `static` y `extern`:** Control estricto del alcance (*scope*) de las funciones y variables para evitar colisiones de nombres y accesos no autorizados a registros críticos.

---

## 🏗️ Arquitectura de Software Aplicada
En esta etapa, se establecen las bases de la organización del código profesional:

* **Máquinas de Estado Finitos (FSM):** Uso del modelo de Moore para estructurar programas complejos mediante estados lógicos claros, evitando el código "espagueti".
* **Modularidad (API Design):** Separación de la lógica de usuario de los detalles del hardware mediante la creación de drivers propios.
* **Temporización Asíncrona:** Uso de `HAL_GetTick()` para ejecutar tareas en paralelo sin detener el flujo del procesador.

---

## 🛠️ Roadmap de Laboratorios *(-✅ Completado 22/01/2026-)*

### 🔢 Fase 1: C aplicado y Lógica de Bits
1. **[01_Hola_Mundo_GPIO](./01_Hola_Mundo_GPIO):** Primer contacto con el hardware y control de LEDs de usuario.
2. **[02_Tipos_De_Variables](./02_Tipo_De_Variables):** Implementación de tipos de ancho fijo para control de memoria.
3. **[03_Estructuras](./03_Estructuras):** Organización de datos y creación de objetos de hardware virtuales.
4. **[04_Bitwise_Logic](./04_Bitwise_Logic):** Manipulación de registros mediante máscaras y operaciones binarias.

### 🔌 Fase 2: Periféricos de Entrada/Salida
5. **[05_GPIO_Input_Polling](./05_GPIO_Input_Polling):** Lectura de entradas y análisis del rebote de pulsadores.
6. **[06_LED_Bus_Structures](./06_LED_Bus_Structures):** Manejo de buses de datos y automatización de secuencias con arreglos.
7. **[07_Display_7_segmentos](./07_Display_7_Segmentos):** Creación de tablas de verdad para decodificación visual.
8. **[08_Multiplex_7Seg](./08_Multiplex_7Seg):** Multiplexación temporal y ahorro de recursos de hardware.

### 🏛️ Fase 3: Arquitectura y Abstracción
9. **[09_Intro_MEF](./09_Intro_MEF):** Implementación de Máquinas de Estado para lógica de control robusta.
10. **[10_API_Drivers](./10_API_Drivers):** Encapsulamiento de hardware y gestión de tiempo no bloqueante.
11. **[11_Debounce_Avanzado](./11_Debounce_Avanzado):** Integración técnica de MEF y temporización asíncrona.

---

## 🚀 Proyectos Integradores (EI)
*Desafíos que consolidan la transición hacia el Nivel Intermedio.*

### 01. [Contador Up-Down 7-Segmentos](./Proyectos_Integradores/01.Contador_Up_Down_7Seg)
* **Técnicas:** Aritmética básica, manejo de entradas Active-Low y decodificación.

### 02. [Semáforo Smart](./Proyectos_Integradores/02.Semaforo_Smart)
* **Técnicas:** Control de tiempos y secuencias críticas mediante FSM.

### 03. [Simon Dice](./Proyectos_Integradores/03.Simon_Dice)
* **Técnicas:** Arreglos dinámicos, números pseudo-aleatorios y lógica de juego modular.

### 04. [Integración de APIs](./Proyectos_Integradores/04.Integracion_APIs)
* **Técnicas:** Proyecto final de nivel. Un secuenciador maestro que demuestra la potencia de la arquitectura limpia.

---

## 📚 Referencias Técnicas
El desarrollo de este nivel se fundamenta en los estándares de la industria para sistemas críticos y la documentación oficial del fabricante:

* **MCU:** STM32F439ZI (Cortex-M4 @ 180MHz) - Nucleo-144.
* **Standard C (ISO/IEC 9899):** Uso de tipos de datos de ancho fijo definidos en `<stdint.h>` para garantizar la portabilidad y evitar desbordamientos de memoria.
* **Reference Manual (RM0090):** Consulta de registros base de los periféricos GPIO y RCC (Reset and Clock Control).
* **Datasheet F439:** Verificación de niveles lógicos de voltaje, corrientes de drenaje en pines y mapeo de funciones alternas.
* **HAL Driver User Manual (UM1725):** Referencia para la implementación de funciones de control de GPIO y gestión del tiempo de sistema.

**Herramientas de Software:**
* **IDE:** STM32CubeIDE (v1.13.0+).
* **Compiler:** GNU Arm Embedded Toolchain (GCC).
* **Linter/Format:** Verificación de sintaxis basada en buenas prácticas de programación modular.

---

<div align="center">
  <h3>💎 "Aprendiendo paso a paso el control del silicio: de la lógica binaria a la arquitectura de sistemas."</h3>
  <p><i>Hacia una base sólida en ingeniería de sistemas embebidos.</i></p>
</div>