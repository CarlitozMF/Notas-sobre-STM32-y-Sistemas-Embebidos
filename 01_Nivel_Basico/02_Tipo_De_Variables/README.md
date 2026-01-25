# 02_Tipo_De_Variables - Tipos de Datos y Gestión de Memoria 🧠

Este módulo analiza cómo el microcontrolador gestiona la memoria RAM y la importancia crítica de elegir el tipo de dato correcto para optimizar recursos y asegurar la robustez del sistema en una arquitectura **ARM Cortex-M4 (32-bit)** como la de la **Nucleo-F439ZI**.

## 📍 Objetivos
- Verificar el tamaño real de los datos en una arquitectura de 32 bits.
- Implementar una capa de abstracción para telemetría vía UART (`Debug_Log`).
- Documentar y mitigar el fenómeno de **Overflow** (desbordamiento aritmético).

---

## 📊 Tipos de Datos en Sistemas Embebidos
En sistemas profesionales, abandonamos el uso de `int` o `long` (cuyo tamaño depende del compilador) y adoptamos la librería `<stdint.h>` para garantizar la **portabilidad** absoluta entre diferentes arquitecturas.

| Tipo de Dato | Tamaño (Bytes) | Rango de Valores | Aplicación Típica |
| :--- | :--- | :--- | :--- |
| **`uint8_t`** | 1 | 0 a 255 | Banderas (flags), estados de MEF, buffers de comunicación. |
| **`uint16_t`** | 2 | 0 a 65,535 | Resolución de ADC (12-bit), registros de Timer (16-bit). |
| **`uint32_t`** | 4 | 0 a 4,294,967,295 | Marcas de tiempo (`HAL_GetTick()`), direcciones de memoria. |
| **`float`** | 4 | ±1.18e-38 a ±3.4e38 | Procesamiento de señales (la F439ZI tiene FPU integrada). |

---

## 📊 Tabla de Especificadores de Formato (`printf`/`sprintf`)

Para la telemetría por consola, debemos ser estrictos con el compilador. Usar el especificador incorrecto puede generar datos erróneos o *Warnings* de compilación.

| Especificador | Tipo de Dato C | Descripción Técnica |
| :--- | :--- | :--- |
| **`%u`** | `uint16_t` | Entero sin signo de 16 bits. |
| **`%d`** | `int16_t` | Entero con signo de 16 bits. |
| **`%lu`** | **`uint32_t`** | **Long Unsigned**: Obligatorio para variables de tiempo (`HAL_GetTick()`). |
| **`%zu`** | **`size_t`** | El estándar para el operador `sizeof`. Evita errores de portabilidad. |
| **`%f`** | `float` | Punto flotante (requiere activar el soporte en el IDE). |
| **`%x` / `%X`** | `uint32_t` | Representación **Hexadecimal**. Esencial para leer registros de periféricos. |
| **`%p`** | `void*` | Dirección de memoria (Puntero). Útil para verificar la ubicación en RAM. |

---

## 🔧 Configuración: Soporte para Punto Flotante
Por defecto, la librería *newlib-nano* de STM32CubeIDE no incluye soporte para imprimir `floats` para ahorrar memoria Flash. 

**Para activarlo:**
1. Click derecho en el proyecto > `Properties`.
2. `C/C++ Build` > `Settings` > `Tool Settings`.
3. `MCU Settings` > Marcar la casilla: **"Use float with printf from newlib-nano (-u _printf_float)"**.

---

## ⚠️ El Fenómeno del Overflow (Aritmética Circular)
En sistemas embebidos, los registros tienen límites físicos. Cuando una variable supera su valor máximo, ocurre un desbordamiento.



* **Comportamiento:** Si un `uint8_t` vale **255** y recibe un incremento (`++`), vuelve automáticamente a **0**.
* **Peligro Crítico:** Si utilizas un tipo de dato pequeño para una marca de tiempo que crece constantemente, el sistema fallará o tendrá comportamientos erráticos al "dar la vuelta". Siempre se debe dimensionar la variable para el escenario del peor caso (*Worst Case Scenario*).

---

## 📏 El Operador `sizeof` y el tipo `size_t`

El uso de `sizeof` no es opcional en código de alta calidad. Es un operador que se evalúa en **tiempo de compilación**, por lo que no consume ciclos de reloj del procesador.

1. **Cálculo de Arreglos:** Permite determinar el número de elementos de forma dinámica:
   `uint8_t num_elementos = sizeof(mi_arreglo) / sizeof(mi_arreglo[0]);`
2. **Seguridad en Buffers:** Al usar funciones como `HAL_UART_Transmit`, siempre usamos `sizeof` para evitar leer memoria fuera de los límites (Buffer Overflow).
3. **Portabilidad:** `sizeof` devuelve un `size_t`, adaptándose automáticamente si el microcontrolador es de 8, 16 o 32 bits.

---

## 🛠️ Herramientas de Telemetría: `Debug_Log`
Se implementó una función personalizada para simplificar el envío de datos por la **UART3** (Virtual COM Port de la Nucleo), actuando como una capa de abstracción sobre la HAL.

```c
void Debug_Log(const char *msg) {
    // Envío de cadena de texto mediante punteros y cálculo de longitud dinámica
    HAL_UART_Transmit(&huart3, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
}
```
 ---
 *Notas creadas durante el estudio de gestión eficiente de recursos*