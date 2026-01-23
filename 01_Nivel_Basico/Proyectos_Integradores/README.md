# 📂 Proyectos Integradores (Nivel Básico)

Este directorio contiene los proyectos finales del **Nivel Básico** de aprendizaje en sistemas embebidos con STM32. El objetivo de esta sección es consolidar los conocimientos atómicos (GPIO, Bits, Delays) en aplicaciones reales y funcionales.

## 🚀 Filosofía de Diseño
Cada proyecto en esta carpeta cubre los siguientes aspectos fundamentales:
1. **Modularidad:** Uso de funciones para separar la lógica de control del hardware.
2. **Robustez:** Implementación de antirrebotadores (debounce) y gestión de estados de error.
3. **Feedback de Usuario (HMI):** Comunicación constante con el usuario a través de LEDs, Displays y Terminal Serial (UART).
4. **Modularidad Total:** Drivers `API_` (Delay, LED, Debounce) que separan la aplicación del hardware.
5. **Programación No Bloqueante:** Sustitución definitiva de retardos por lógica asíncrona de Ticks.
6. **MEFs Jerárquicas:** Una máquina de estados de driver (bajo nivel) alimentando a una máquina de estados de aplicación (alto nivel).

---

## 🛠️ Proyectos Incluidos

| Proyecto | Descripción | Conceptos Clave |
| :--- | :--- | :--- |
| **01_Contador_Up_Down_7Seg** | Contador UP/DOWN (0-9) con detección de límites. | *Active-Low, Ventana de Reset, Feedback Visual.* |
| **02_Semaforo_Smart** | Sistema de tráfico automático con transición de seguridad. | *Máquina de Estados Finitos (MEF), Sincronización.* |
| **03_Simon_Dice** | Juego de memoria con 4 botones y 4 LEDs. | *Arreglos, Generación Aleatoria, Lógica de Juego.* |
| **04_Integracion_APIs** | Controlador maestro de 6 efectos visuales. | *Multitarea Cooperativa y Lógica de Grupos.* |

---

## 🧠 Competencias Adquiridas

Al completar estos tres proyectos, se han dominado las siguientes áreas:

### 1. Máquinas de Estados Finitos (MEF/FSM)
Capacidad para organizar el flujo del programa en estados lógicos (`enum` + `switch-case`), permitiendo que el microcontrolador tome decisiones complejas de forma ordenada.

### 2. Abstracción de Hardware
Uso de estructuras (`struct`) para definir periféricos como el Display de 7 segmentos, facilitando la portabilidad del código entre diferentes pines o placas.

### 3. Comunicación y Depuración
Implementación de drivers básicos de UART (`Debug_Log`) para monitorear variables y estados internos del sistema en tiempo real desde una PC.

### 4. Gestión de Tiempos y Eventos
Diferenciación entre el uso de bloqueos controlados (`HAL_Delay`) y la necesidad de escaneo constante de entradas (Polling activo) para interfaces de usuario fluidas.

### 5. Multitarea Cooperativa (Async Logic)
Dominio del tiempo sin bloqueos del CPU. El sistema puede procesar una UART, filtrar el ruido de un botón y ejecutar secuencias de 50ms simultáneamente sin que un proceso interfiera con el otro.

### 6. Lógica de Abstracción de Grupos
Capacidad para diseñar APIs que manejan colecciones de hardware. El uso de punteros y arreglos de estructuras permite escalar el sistema (ej. pasar de 3 a 8 LEDs) modificando una sola línea de código.

### 7. Drivers Reentrantes y Modulares
Creación de librerías propias (`API_debounce`, `API_delay`) que son independientes del proyecto. Estos drivers son "cajas negras" que pueden copiarse y pegarse en cualquier nuevo desarrollo de STM32.

### 8. Diagnóstico por Telemetría (UART)
Uso de la terminal serie no solo para mensajes, sino como una herramienta de rastreo de estados de la MEF, permitiendo "ver" qué está pensando el microcontrolador en cada microsegundo.
---

## 🔧 Requisitos del Laboratorio
- **Hardware:** Nucleo-F439ZI (STM32F4) + Pulsadores externos en Pull-Up/Down + Leds diversos colores con su respectiva resistencia.
- **Software:** STM32CubeIDE + Doxygen para la documentación de las APIs.
- **Protocolo:** Comunicación Serial a 115200 baudios.

---
# **🏁 NIVEL BÁSICO COMPLETADO.** 
*Los cimientos están listos. El siguiente paso es el **Nivel Intermedio**, donde introduciremos **Interrupciones (EXTI)** y **Timers por Hardware** para alcanzar una respuesta en tiempo real determinística. Además, profundizaremos en el manejo avanzado de los periféricos integrados de la placa Nucleo, integrando sensores y actuadores mediante protocolos de comunicación industrial, preparándonos para el diseño de sistemas embebidos de alta complejidad.*