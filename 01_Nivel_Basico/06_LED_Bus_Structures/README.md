# 06_GPIO_Generic_Structures - Estructuras, Bucles y Automatización 🚀

Este módulo representa la culminación del Nivel Básico. Aquí aplicamos **Abstracción de Hardware** para manejar un bus de 8 LEDs externos, utilizando estructuras anidadas y el cálculo automático de tamaño con `sizeof` para crear un código robusto y escalable.

## 📍 Objetivos
- Implementar un **Hardware Mapping** (Mapeo de Hardware) profesional.
- Utilizar **estructuras anidadas** para desacoplar la lógica del programa de los pines físicos.
- Automatizar el control de bucles mediante el operador `sizeof`.
- Dominar las estructuras de control (`for`, `while`) sobre arreglos de objetos.

## 🧱 La Arquitectura del Código

### Estructuras Anidadas (Hardware Abstraction)
Para que nuestro código sea flexible, no podemos asumir que todos los LEDs están en el mismo puerto. Hemos creado una jerarquía de datos:

1. **`GPIO_Config_t`**: Una estructura pequeña que define un "par" (Puerto + Pin).
2. **`LedBar_t`**: Una estructura maestra que contiene un arreglo de configuraciones y el tamaño del mismo.

```c
typedef struct {
    GPIO_TypeDef* port;  // Puerto (GPIOA, GPIOB, etc.)
    uint16_t pin;       // Pin (GPIO_PIN_0, etc.)
} GPIO_Config_t;

typedef struct {
    GPIO_Config_t* leds; // Puntero al arreglo de configuraciones
    uint8_t count;       // Cantidad de elementos
} LedBar_t;
```

### ¿Por qué es "Genérico"?
Gracias a este diseño, si cambias un LED del puerto `GPIOD` al `GPIOA`, **no necesitas tocar la lógica de tus bucles**. Solo actualizas la tabla de configuración inicial. Esto es la base de los **Drivers** modernos en sistemas embebidos.

### 📏 Automatización con sizeof

En C, sizeof(arreglo) devuelve el tamaño total en bytes. Como cada elemento de nuestra estructura GPIO_Config_t ocupa varios bytes (debido al puntero del puerto y al entero del pin), no podemos usar ese valor directamente.

La fórmula maestra:
$$\text{Cantidad de elementos} = \frac{\text{sizeof(arreglo completo)}}{\text{sizeof(un solo elemento)}}$$

En nuestro ejemplo:

#define LED_COUNT (sizeof(configuracion_leds) / sizeof(configuracion_leds[0]))

Al usar esta técnica en el archivo de configuración, logramos un código escalable:

* Si agregas un pin al arreglo configuracion_leds, LED_COUNT se actualiza solo.

* No hay riesgo de "Hardcoding" (escribir números mágicos como 8 o 10 en medio del código).

* El compilador resuelve este cálculo en tiempo de compilación, por lo que no consume CPU durante la ejecución.

### 🔄 Lógica de Control y Bucles
El Bucle for Dinámico

Al conocer el tamaño exacto a través de la estructura (miBarra.count), el bucle for recorre los LEDs sin importar en qué puerto físico se encuentren. Esto permite que la lógica de animación sea la misma para cualquier hardware.

```c
for (int i = 0; i < miBarra.count; i++) {
    HAL_GPIO_WritePin(miBarra.leds[i].port, miBarra.leds[i].pin, GPIO_PIN_SET);
    HAL_Delay(100);
    HAL_GPIO_WritePin(miBarra.leds[i].port, miBarra.leds[i].pin, GPIO_PIN_RESET);
}
```
### 🛠️ Hardware Setup

- Microcontrolador: STM32F439ZI (Nucleo-144).

- Periféricos: 8 LEDs externos.

- Conexión: Los LEDs están distribuidos en diferentes puertos (ej. GPIOD, GPIOA, GPIOG) para demostrar la versatilidad del mapeo genérico.

- Configuración: Pines en modo Output, Push-Pull, No pull-up/pull-down.

### 🔍 Por qué este enfoque es Profesional

- Escalabilidad: Si cambias un LED de puerto, solo editas la tabla de configuración. La lógica del main() no se toca.

- Portabilidad: Este mismo código puede llevarse a otra placa STM32 cambiando únicamente la definición del arreglo de pines.

- Mantenimiento: El uso de sizeof asegura que nunca habrá un desbordamiento de índice en el bucle si cambia el número de LEDs.

---
*Este ejemplo cierra la sección de Fundamentos de C para Embebidos, preparando el camino para el manejo de periféricos complejos en el Nivel Intermedio.*