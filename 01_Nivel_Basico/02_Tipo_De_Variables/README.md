# 02_Tipo_De_Variables - Tipos de Datos y Memoria 🧠

Este módulo se centra en entender cómo el microcontrolador gestiona la memoria RAM y cómo elegir el tipo de dato correcto para optimizar recursos y evitar errores críticos.

## 📍 Objetivos
- Verificar el tamaño real de los tipos de datos en una arquitectura **ARM Cortex-M4 (32-bit)**.
- Implementar una función de abstracción para telemetría (`Debug_Log`).
- Observar y documentar el fenómeno de **Overflow** (desbordamiento).

## 📊 Tipos de Datos en Sistemas Embebidos
En lugar de usar `int` o `long`, utilizamos la librería `<stdint.h>` para asegurar la portabilidad del código entre diferentes arquitecturas.

| Tipo de Dato | Tamaño (sizeof) | Rango de Valores | Uso Común |
| :--- | :--- | :--- | :--- |
| `uint8_t` | 1 Byte | 0 a 255 | Banderas, estados lógicos, buffers. |
| `uint16_t` | 2 Bytes | 0 a 65,535 | Valores de ADC, contadores de Timers. |
| `uint32_t` | 4 Bytes | 0 a 4,294,967,295 | Marcas de tiempo (`HAL_GetTick()`). |
| `float` | 4 Bytes | Decimales | Cálculos de sensores analógicos. |

## 📊 Tabla de Especificadores de Formato (printf/sprintf)
Para mostrar datos por consola, debemos indicar al compilador cómo interpretar los bytes. Usar el especificador incorrecto puede generar datos erróneos o *Warnings*.

| Especificador | Tipo de Dato C | Descripción |
| :--- | :--- | :--- |
| `%u` | `uint8_t`, `uint16_t` | Entero sin signo (pequeño). |
| `%d` | `int8_t`, `int16_t` | Entero con signo. |
| **`%lu`** | **`uint32_t`** | **Unsigned Long**: Indispensable para `HAL_GetTick()`. |
| **`%zu`** | **`size_t`** | Especificador estándar para el operador `sizeof`. |
| `%f` | `float` | Punto flotante (decimales). |
| `%x` / `%X` | `uint32_t` | Valor en **Hexadecimal** (ideal para debugear registros). |
| `%p` | `void*` | Dirección de memoria (Puntero). |

## 🔧 Solución de Problemas: Soporte para Floats
Al usar `sprintf` con variables tipo `float` en STM32CubeIDE, es posible que no se muestren los datos o se reciba un error. 
**Solución:**
1. Ir a `Project Properties` > `C/C++ Build` > `Settings`.
2. En `Tool Settings` > `MCU Settings`, marcar la casilla: **"Use float with printf from newlib-nano (-u _printf_float)"**.


## ⚠️ El Fenómeno del Overflow
En este ejemplo, observamos qué sucede cuando una variable supera su valor máximo permitido por su tamaño de bits:
- Un `uint8_t` que vale **255** y recibe un incremento (`++`), vuelve automáticamente a **0**.
- **Impacto:** Si se usa un tipo de dato pequeño para una variable que crece constantemente (como el tiempo), el sistema fallará o tendrá comportamientos erráticos al "dar la vuelta".

## 📏 La Importancia de sizeof en Sistemas Embebidos

💡 Breve Explicación Técnica

sizeof es un operador en tiempo de compilación. Esto significa que no consume ciclos de reloj del procesador mientras tu programa corre; el compilador calcula el tamaño y sustituye el sizeof por el número constante antes de grabar el código en la memoria Flash del microcontrolador.

El uso de sizeof es una de las mejores prácticas en la programación de microcontroladores por tres razones fundamentales:

- Portabilidad del Código: A diferencia de la programación en PC, en el mundo de los sistemas embebidos el tamaño de un int o un long no es estándar; depende de la arquitectura del procesador (8, 16, 32 o 64 bits). sizeof permite que el código se adapte automáticamente al hardware donde se compila.

- Seguridad en el Manejo de Buffers: Cuando usamos funciones como HAL_UART_Transmit o memcpy, necesitamos indicar cuántos bytes vamos a procesar. Usar sizeof(mi_variable) en lugar de un número fijo (como "4") evita errores de desbordamiento de memoria (Buffer Overflow) si en el futuro decidimos cambiar el tipo de la variable.

- Cálculo de Elementos en Arreglos: Es la forma más segura de saber cuántos elementos tiene un arreglo sin contarlos a mano: int num_elementos = sizeof(mi_arreglo) / sizeof(mi_arreglo[0]);

El operador `sizeof` devuelve un tipo de dato llamado `size_t`. 
- **Por qué usarlo:** Permite que el código sea portable. No importa si el micro es de 8 o 32 bits, `sizeof` siempre devolverá el tamaño correcto.
- **En el código:** Al imprimirlo, lo ideal es usar `%zu` o `%lu` para evitar que el compilador emita advertencias sobre el tamaño de los argumentos.

## 🛠️ Herramientas de Desarrollo
### Abstracción: `Debug_Log`
Se implementó una función personalizada para simplificar el envío de datos por la **UART3** (conectada al puerto USB de la Nucleo-F439ZI).

```c
void Debug_Log(const char *msg) {
    HAL_UART_Transmit(&huart3, (uint8_t*)msg, strlen(msg), 100);
}
```