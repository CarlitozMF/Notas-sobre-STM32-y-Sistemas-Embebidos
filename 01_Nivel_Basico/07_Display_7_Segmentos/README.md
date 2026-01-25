# 07_Display_7_Segmentos - Decodificación y Tablas de Búsqueda 🔢

Este módulo consolida el manejo de salidas digitales paralelas mediante la integración de Estructuras, Arreglos y Lógica de Bits para controlar un display de 7 segmentos de **Cátodo Común**.

## 📍 Objetivos
- Implementar una **Lookup Table** (LUT) para optimizar la decodificación numérica.
- Aplicar **Bit Masking** y **Bit Shifting** para la extracción de datos desde un bus virtual.
- Reutilizar la arquitectura de **Mapeo Genérico** de hardware definida en módulos anteriores.

---

## 💡 Concepto Físico: Cátodo Común (CC)
En un display de Cátodo Común, todos los terminales negativos (cátodos) de los segmentos están cortocircuitados internamente a un punto común conectado a **GND**.



* **Lógica Activa:** Para iluminar un segmento, el pin del STM32 debe entregar un **1 lógico (HIGH)**.
* **Protección de Hardware:** Es imperativo utilizar resistencias de limitación (220Ω - 330Ω) en cada segmento. La **STM32F439ZI** tiene un límite de corriente por pin que debe respetarse para evitar daños permanentes.

---

## 🛠️ La Tabla de Búsqueda (Lookup Table)
En lugar de procesar lógica pesada con `if` o `switch-case`, almacenamos los patrones en un arreglo `static const` en la memoria Flash. El número que deseamos mostrar actúa directamente como el **índice** del arreglo, reduciendo la latencia de ejecución.

| Dígito | Patrón Binario (gfedcba) | Hexadecimal |
| :---: | :---: | :---: |
| **0** | `0011 1111` | `0x3F` |
| **1** | `0000 0110` | `0x06` |
| **8** | `0111 1111` | `0x7F` |

---

## 💻 Lógica de Extracción de Bits (Bit-Banging)
Para enviar el patrón de un byte a los 7 pines físicos distribuidos en la estructura genérica, aplicamos una técnica de extracción bit a bit:



```c
uint8_t patron = segmento_map[numero]; // Acceso directo por índice

for (int i = 0; i < 7; i++) {
    // 1. Desplazamos el bit de interés (i) hacia la posición del LSB (derecha).
    // 2. Aplicamos una máscara AND 0x01 para aislar el estado de ese segmento.
    uint8_t estado = (patron >> i) & 0x01;
    
    // 3. Escribimos el estado en el hardware mapeado en nuestra estructura de driver.
    HAL_GPIO_WritePin(miDisplay.leds[i].port, miDisplay.leds[i].pin, estado);
}
```
## 🔬 Hacia la Arquitectura de Software

Aunque este laboratorio permite mostrar información numérica, todavía dependemos de la multiplexación manual y retardos bloqueantes.

---
*La eficiencia en sistemas embebidos reside en elegir las estructuras de datos que permitan al hardware hablar el lenguaje de la lógica con el menor costo de CPU posible*