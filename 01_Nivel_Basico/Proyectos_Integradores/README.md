# 📂 Directorio: Proyectos Integradores (Nivel Básico)

Este directorio contiene los proyectos finales del **Nivel Básico** de aprendizaje en sistemas embebidos con STM32. El objetivo de esta sección es consolidar los conocimientos atómicos (GPIO, Bits, Delays) en aplicaciones reales y funcionales.

## 🚀 Filosofía de Diseño
Cada proyecto en esta carpeta sigue tres principios fundamentales:
1. **Modularidad:** Uso de funciones para separar la lógica de control del hardware.
2. **Robustez:** Implementación de antirrebotadores (debounce) y gestión de estados de error.
3. **Feedback de Usuario (HMI):** Comunicación constante con el usuario a través de LEDs, Displays y Terminal Serial (UART).

---

## 🛠️ Proyectos Incluidos

| Proyecto | Descripción | Conceptos Clave |
| :--- | :--- | :--- |
| **08_01_Contador_Pro** | Contador UP/DOWN (0-9) con detección de límites. | Active-Low, Ventana de Reset, Feedback Visual. |
| **08_02_Semaforo_MEF** | Sistema de tráfico automático con transición de seguridad. | Máquina de Estados Finitos (MEF), Sincronización. |
| **08_03_Simon_Says** | Juego de memoria con 4 botones y 4 LEDs. | Arreglos, Generación Aleatoria, Lógica de Juego. |

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

---

## 🔧 Requisitos Técnicos
- **Placa:** Nucleo-F439ZI (STM32F4).
- **IDE:** STM32CubeIDE.
- **Herramientas:** Terminal Serial (PuTTY, TeraTerm o el monitor de CubeIDE).

---
*Este es el cierre del Nivel Básico. Los siguientes desafíos en el Nivel Intermedio incluirán Interrupciones, Timers y ADC para eliminar las limitaciones de los retardos bloqueantes.*