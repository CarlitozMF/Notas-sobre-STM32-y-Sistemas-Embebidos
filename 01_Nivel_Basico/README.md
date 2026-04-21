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

## 🚀 Pilares del Nivel Básico (Consolidados)

### 1. Manipulación de Bits y Operaciones Atómicas 🔢
Dominio del lenguaje nativo del microcontrolador para interactuar con registros y periféricos sin efectos colaterales.
* **Máscaras de Bits:** Uso experto de operadores `&`, `|`, `^` y `~` para la manipulación selectiva de bits.
* **Serialización Manual:** Aplicación de desplazamientos (*bit-shifting*) para convertir datos lógicos en señales físicas, permitiendo el control de buses de datos dispersos.

### 2. Abstracción de Hardware y Funciones Agnósticas 🏗️
Separación total entre la lógica del problema y los pines específicos del silicio.
* **Hardware Mapping:** Implementación de etiquetas y descriptores que permiten cambiar el MCU o el conexionado sin modificar la lógica de aplicación.
* **Funciones Agnósticas:** Creación de subrutinas que operan sobre estructuras de datos en lugar de pines fijos, logrando un código portable, reutilizable y reentrante.

### 3. Arquitectura de Drivers y Encapsulamiento 🛠️
Evolución hacia módulos de software profesionales, escalables y fáciles de mantener.
* **Encapsulamiento mediante Estructuras:** Uso de `structs` para agrupar descriptores de hardware, tratando a los periféricos como "objetos" lógicos dentro de C puro.
* **Modularidad de Archivos:** Organización estricta en archivos de cabecera (`.h`) para la interfaz y fuente (`.c`) para la implementación, con control de visibilidad mediante `static` y `extern`.

### 4. Sistemas Reactivos y No Bloqueantes (MEF) ⏱️
El fin del uso de delays bloqueantes para permitir un multitasking real y eficiente.
* **Máquinas de Estados Finitos (MEF):** Modelado de la lógica del sistema mediante estados y transiciones determinísticas, eliminando el "código espagueti".
* **Gestión de Tiempos Asíncrona:** Sustitución de `HAL_Delay()` por comparaciones de tiempos y banderas de estado, liberando ciclos de CPU para procesar múltiples tareas en paralelo.

---

## 🏗️ Arquitectura de Software Aplicada
En esta etapa, se trasciende la programación secuencial para adoptar una organización de código profesional basada en la **Jerarquía de Responsabilidades**:

### 1. Modelo de Diseño en 3 Capas
El aprendizaje central reside en la capacidad de desacoplar el firmware mediante tres niveles de abstracción:
* **Capa de Aplicación (Lógica de Negocio):** Donde reside el `main.c` y las Máquinas de Estado. Esta capa es "ciega" al hardware; solo conoce procesos lógicos.
* **Capa de Driver (Abstracción del Periférico):** Módulos `.c/.h` que traducen comandos lógicos en secuencias de bits. Aquí se implementan funciones reentrantes y agnósticas.
* **Capa de Hardware Mapping (UHAL):** El diccionario del sistema. Mapea etiquetas lógicas a registros y pines físicos del MCU, permitiendo cambios de hardware sin tocar la lógica superior.

### 2. Máquinas de Estado Finitos (FSM)
Uso del **Modelo de Moore** para estructurar programas complejos. La lógica se define por estados determinísticos y transiciones claras, lo que elimina el código "espagueti" y garantiza que el sistema siempre se encuentre en un estado conocido y seguro.

### 3. Temporización Asíncrona y Multitarea Cooperativa
Sustitución definitiva de los bucles de espera por gestión de tiempos basada en `HAL_GetTick()`. 
* **Ejecución No Bloqueante:** Permite el flujo continuo del procesador, habilitando la ejecución de tareas en paralelo (multitasking cooperativo) y mejorando drásticamente la reactividad del sistema ante eventos externos.

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
  <p><i>"La eficiencia en sistemas embebidos reside en elegir las estructuras de datos que permitan al hardware hablar el lenguaje de la lógica con el menor costo de CPU posible."</i></p>
  <br>
  <h3>💎 Aprendiendo paso a paso el control del silicio: de la lógica binaria a la arquitectura de sistemas.</h3>
  <p><b>Hacia una base sólida en ingeniería de sistemas embebidos.</b></p>
</div>