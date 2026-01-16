# 07_Display_7_Segmentos - Control de Display de 7 Segmentos 🔢

Este módulo cierra el **Nivel Básico** aplicando todas las herramientas de programación en C (Estructuras, Arreglos, Lógica de Bits y Bucles) para controlar un display de 7 segmentos de **Cátodo Común**.

## 📍 Objetivos
- Implementar una **Lookup Table** (Tabla de Búsqueda) para decodificar números.
- Utilizar **Bit Masking** y **Bit Shifting** para extraer información de un byte.
- Reutilizar la estructura genérica de hardware para mapear los segmentos (A-G).

## 💡 Concepto Físico: Cátodo Común
En un display de Cátodo Común (CC), todos los terminales negativos de los LEDs internos están unidos a un punto común conectado a GND.
- **Encendido:** Se requiere un `1` lógico (HIGH) en el pin del segmento.
- **Apagado:** Se requiere un `0` lógico (LOW).

## 🛠️ La Tabla de Búsqueda (Lookup Table)
En lugar de usar múltiples sentencias `if` o un `switch-case` de 10 casos, utilizamos un arreglo constante. El número que deseamos mostrar sirve como el **índice** para obtener el patrón de bits necesario.

| Dígito | Binario (gfedcba) | Hexadecimal |
| :---: | :---: | :---: |
| 0 | `0011 1111` | `0x3F` |
| 1 | `0000 0110` | `0x06` |
| 8 | `0111 1111` | `0x7F` |

## 💻 Lógica de Extracción de Bits
Para enviar el patrón de un byte a los 7 pines físicos, utilizamos un bucle `for` combinado con operaciones de bits. Esto permite que el código sea independiente de los pines utilizados:

```c
uint8_t patron = segmento_map[numero];

for (int i = 0; i < 7; i++) {
    // 1. Desplazamos el bit deseado a la posición 0
    // 2. Aplicamos una máscara AND 0x01 para aislarlo
    uint8_t estado = (patron >> i) & 0x01;
    
    // 3. Escribimos el estado en el pin correspondiente de la estructura
    HAL_GPIO_WritePin(miDisplay.leds[i].port, miDisplay.leds[i].pin, estado);
}
```

## 🔧 Hardware Setup

    Microcontrolador: STM32F439ZI (Nucleo).

    Display: 7 Segmentos Cátodo Común.

    Resistencias: Es fundamental colocar resistencias de limitación (220-330 Ω) en cada segmento para proteger los pines del STM32.

# 🚀 Conclusión del Nivel Básico

Con este ejemplo finalizamos los fundamentos. Hemos pasado de un simple parpadeo de LED a la creación de sistemas de hardware abstraídos mediante software.

Logros alcanzados:

- Manejo de GPIO (In/Out/Polling).

- Tipos de datos y gestión de memoria (sizeof, estructuras anidadas).

- Lógica de bits profesional (Máscaras y desplazamientos).

- Automatización de tareas con bucles dinámicos.

---
*Con este ejemplo finalizamos la fase de aprendizaje atómico. Este nivel concluye con una serie de Ejemplos Integradores (ver carpeta **Proyectos_Integradores**) donde aplicamos todo lo visto hasta ahora para resolver problemas reales de control lógico.*
*Una vez superados estos desafíos, estaremos listos para avanzar al Nivel Intermedio. Allí abordaremos la creación de Drivers profesionales, Multiplexación de displays, y la configuración profunda de periféricos avanzados (ADC, Timers, UART, etc.).*

*¡Nos vemos en los proyectos integradores!*