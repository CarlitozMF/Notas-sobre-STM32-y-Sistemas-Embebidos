# 03_Estructuras - Optimización de Memoria RAM 🏗️

Este módulo explora la organización interna de los datos en la memoria de la **STM32F439ZI** y cómo el diseño estratégico de estructuras permite reducir el consumo de RAM, evitando el desperdicio de bytes innecesarios.

## 📍 Objetivos
- Comprender el concepto de **Memory Alignment** (Alineación de memoria) en arquitecturas de 32 bits.
- Identificar y medir el **Padding** (relleno) generado automáticamente por el compilador.
- Aplicar la técnica de reordenamiento de miembros para optimizar el espacio.

---

## 🧱 El Fenómeno del Padding

En una arquitectura **ARM Cortex-M4**, el procesador accede a la memoria de forma más eficiente en bloques de 32 bits (4 bytes). Para mantener el rendimiento, el compilador exige que las variables residan en direcciones de memoria que sean múltiplos de su propio tamaño.

### ¿Por qué se desperdicia memoria?
Si declaramos un `uint8_t` seguido de un `uint32_t`, el microcontrolador no puede leer el dato de 32 bits si este comienza en una dirección impar. El compilador inserta **3 bytes de relleno (Padding)** para "empujar" la variable grande a la siguiente dirección alineada.



---

## 🚀 Técnica de Optimización: "De Mayor a Menor"

Para minimizar el desperdicio, la regla de oro en sistemas embebidos es declarar los miembros de la estructura en **orden descendente de tamaño**. Esto permite que el compilador agrupe las variables pequeñas en los huecos que dejan las grandes.

### Comparación de impacto en RAM:

Considerando variables de: 4 bytes (A), 1 byte (B) y 4 bytes (C).

1. **Orden Ineficiente (B, A, C):**
   - B (1 byte) + **Padding (3 bytes)** + A (4 bytes) + C (4 bytes) = **12 Bytes totales.**
2. **Orden Optimizado (A, C, B):**
   - A (4 bytes) + C (4 bytes) + B (1 byte) + **Padding (3 bytes al final)** = **12 Bytes totales.**
   *Nota: Si agregamos otro `uint8_t` al final del caso optimizado, ocuparía el padding existente y el tamaño seguiría siendo 12.*

---

## 💻 Implementación Técnica

```c
/* Estructura optimizada para ahorrar RAM */
typedef struct {
    uint32_t timestamp;   // 4 bytes - Alineación perfecta (divisible por 4)
    float    lectura;     // 4 bytes - Alineación perfecta
    uint16_t id_sensor;   // 2 bytes 
    uint8_t  estado;      // 1 byte
    uint8_t  error_code;  // 1 byte
    // Total: 12 bytes. Sin desperdicio interno.
} Estructura_Optimizada_t;
```

## El operador **sizeof** en estructuras

En este laboratorio comprobamos que sizeof(Estructura_t) no siempre es la suma de sus partes. Es una herramienta esencial para validar cuánta memoria está consumiendo realmente nuestra arquitectura de datos.

## 💡 Tip Avanzado: Atributo __packed

Existe una instrucción para indicarle al compilador que elimine todo el padding: struct __attribute__((__packed__)) MiEstructura { ... };

Advertencia: Aunque ahorra el máximo de RAM, el procesador tardará más ciclos de reloj en acceder a esos datos (desalineados), penalizando el rendimiento. En este curso priorizamos el reordenamiento manual por ser la práctica más equilibrada.

---
*Notas sobre Estructuras de datos y su aplicación eficiente en sistemas de 32 bits*