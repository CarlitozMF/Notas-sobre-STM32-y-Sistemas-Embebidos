# 📂 Proyectos Integradores (Nivel Básico)

Este directorio consolida el **Nivel Básico** de aprendizaje en sistemas embebidos. Aquí, los conocimientos atómicos (GPIO, Bits, Delays) se transforman en aplicaciones reales mediante el uso de arquitecturas de software profesionales.

## 🚀 Filosofía de Diseño: El Estándar de API
Cada proyecto en esta carpeta se rige por un esquema de capas que separa el silicio de la aplicación:

1. **Modularidad Total:** Drivers `API_` (Delay, LED, Debounce) que actúan como capas de abstracción.
2. **Programación No Bloqueante:** Sustitución definitiva de `HAL_Delay()` por lógica asíncrona basada en Ticks.
3. **MEFs Jerárquicas:** Una máquina de estados de bajo nivel (Driver) alimentando a una máquina de estados de alto nivel (Aplicación).
4. **Escalabilidad:** Uso intensivo de punteros y estructuras para manejar hardware de forma genérica.

---

## 🛠️ Portafolio de Proyectos

| Proyecto | Descripción | Conceptos Clave de Ingeniería |
| :--- | :--- | :--- |
| **[01.Contador_Up-Down](./01.Contador_Up_Down_7Seg)** | Contador 0-9 con detección de límites y feedback visual. | *Active-Low, Edge Detection, Multiplexación.* |
| **[02.Semaforo_Smart](./02.Semaforo_Smart)** | Sistema de tráfico con transiciones de seguridad. | *MEF determinista, Temporización por estados.* |
| **[03.Simon_Dice](./03.Simon_Dice)** | Juego de memoria y destreza mental. | *Algoritmos de aleatoriedad, Manejo de arreglos.* |
| **[04.Integracion_APIs](./04.Integracion_APIs)** | Secuenciador maestro de efectos visuales. | *Multitarea Cooperativa, Abstracción de Grupos.* |

---

## 🧠 Competencias de Ingeniería Adquiridas

Al completar esta fase, se han dominado las siguientes áreas críticas para el desarrollo profesional:

### 1. Multitarea Cooperativa (Async Logic)
Dominio del tiempo sin bloqueos del CPU. El firmware puede procesar la UART, filtrar el ruido mecánico de un pulsador y ejecutar secuencias visuales de alta velocidad simultáneamente, garantizando una latencia mínima.

### 2. Máquinas de Estados Finitos (MEF)
Capacidad para modelar el comportamiento del sistema mediante `enum` + `switch-case`. Esto asegura un sistema determinista donde el microcontrolador siempre se encuentra en un estado conocido y seguro.

### 3. Drivers Reentrantes y Modulares
Desarrollo de librerías propias (`API_debounce`, `API_delay`) bajo un modelo de **objetos en C**. Estos drivers son "cajas negras" completamente reutilizables y **reentrantes**, lo que permite:
* **Multi-instancia:** Gestionar N periféricos de forma simultánea e independiente mediante el paso de punteros a estructuras.
* **Portabilidad:** Migrar la lógica de control a otras placas de la familia STM32 (o incluso otras arquitecturas) con un esfuerzo de re-mapeo mínimo en la capa de hardware.
* **Encapsulamiento:** Aislar la complejidad de los registros y timers internos, exponiendo únicamente una interfaz de usuario limpia y documentada bajo estándar **Doxygen**.

### 4. Telemetría y Diagnóstico (UART)
Implementación de canales de comunicación para monitorear variables internas y estados de la MEF en tiempo real, facilitando la depuración en sistemas donde el Debugger físico no es suficiente.

---

## 🔧 Requisitos del Laboratorio
* **Hardware:** Nucleo-F439ZI (Cortex-M4) + Pulsadores externos + Módulos de LEDs y Displays de 7 segmentos.
* **Protocolo:** Comunicación Serial a **115200 baudios**.
* **Entorno:** STM32CubeIDE con documentación técnica basada en comentarios de ingeniería.

---

# **🏁 NIVEL BÁSICO COMPLETADO.** 
**Los cimientos están listos. El sistema ya es capaz de gestionar lógica compleja de forma asíncrona mediante Polling.**

🛠️ **Carlos** | Estudiante de Ing. Electrónica @UTN_FRT.  
🚀 Apasionado Autodidacta por los Sistemas Embebidos.