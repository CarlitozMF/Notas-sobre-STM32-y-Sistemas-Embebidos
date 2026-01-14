# 03_Estructuras - Optimización de Memoria RAM 🏗️

Este módulo explora cómo el compilador organiza los datos en la memoria del microcontrolador y cómo podemos reducir el consumo de RAM mediante el orden estratégico de las variables.

## 📍 Objetivos
- Comprender el concepto de **Memory Alignment** (Alineación de memoria).
- Identificar y medir el **Padding** (relleno) generado por el compilador.
- Aprender la técnica de optimización por reordenamiento de miembros.

## 🧱 ¿Qué es el Padding?
En una arquitectura de 32 bits (como el Cortex-M4 de nuestra Nucleo), el procesador es más eficiente cuando lee datos de 4 bytes desde direcciones que son múltiplos de 4. 
Si declaramos un `uint8_t` (1 byte) seguido de un `uint32_t` (4 bytes), el compilador insertará **3 bytes de relleno** (padding) vacíos para que la variable de 32 bits quede alineada.



## 🚀 Técnica de Optimización: De Mayor a Menor
Para minimizar el desperdicio de memoria, la regla de oro es declarar los miembros de la estructura en orden descendente de tamaño:
1. `uint32_t`, `float`, `double` (4-8 bytes)
2. `uint16_t` (2 bytes)
3. `uint8_t`, `char` (1 byte)

### Comparación de resultados
En este ejemplo comparamos dos estructuras con los mismos datos:
- **Estructura Desordenada:** Genera huecos de padding entre variables pequeñas y grandes.
- **Estructura Optimizada:** Agrupa variables pequeñas al final, permitiendo que el compilador las "empaquete" mejor.

## 💻 Fragmento de Código Clave
```c
// Estructura optimizada para ahorrar RAM
typedef struct {
    uint32_t timestamp;   // 4 bytes
    float    lectura;     // 4 bytes
    uint8_t  id;          // 1 byte
    uint8_t  estado;      // 1 byte
    // Solo se añaden 2 bytes de padding al final para alinear la estructura completa
} Estructura_Flaca_t;