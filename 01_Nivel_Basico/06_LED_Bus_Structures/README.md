# 06_GPIO_Generic_Structures - Estructuras, Bucles y Automatización 🚀

Este módulo representa la madurez técnica del Nivel Básico. Aplicamos **Abstracción de Hardware** para gestionar un bus de LEDs distribuidos, utilizando estructuras anidadas y metaprogramación con `sizeof` para crear un firmware robusto, escalable y profesional.

## 📍 Objetivos
- Implementar un **Hardware Mapping** (Mapeo de Hardware) desacoplado.
- Utilizar **estructuras anidadas** para abstraer los recursos físicos del microcontrolador.
- Automatizar la gestión de arreglos mediante el operador `sizeof` en tiempo de compilación.
- Dominar el recorrido de objetos complejos mediante punteros y bucles.

---

## 🧱 Arquitectura de Software: La Capa de Abstracción

Para que el firmware sea flexible, eliminamos el "Hardcoding" (direcciones fijas). En lugar de llamar a un pin directamente, creamos una jerarquía de datos:

1. **`GPIO_Config_t`**: Define la unidad mínima de hardware (Puerto + Pin).
2. **`LedBar_t`**: Actúa como un "Contenedor de Objetos", agrupando la tabla de configuración y su metadato de tamaño.



```c
typedef struct {
    GPIO_TypeDef* port;  // Puntero a la estructura de registros del puerto (GPIOA, GPIOB, etc.)
    uint16_t pin;       // Máscara del pin (GPIO_PIN_0, etc.)
} GPIO_Config_t;

typedef struct {
    GPIO_Config_t* leds; // Referencia al arreglo de configuración
    uint8_t count;       // Cantidad de elementos calculada dinámicamente
} LedBar_t;
```

## 📏 Automatización con `sizeof`

En el lenguaje C, el operador `sizeof(arreglo)` devuelve el tamaño total ocupado en memoria (en bytes). Para obtener la **cantidad exacta de elementos** de forma dinámica y evitar errores de desbordamiento de índice (*out of bounds*), aplicamos la siguiente fórmula:

$$\text{Cantidad de elementos} = \frac{\text{sizeof(arreglo completo)}}{\text{sizeof(un solo elemento)}}$$

### 🚀 Ventajas de este enfoque
* **Escalabilidad:** Si en el futuro decides añadir o quitar pines de la tabla de configuración, el resto del código (bucles `for`, lógica de control) se ajustará automáticamente sin tocar una sola línea de código extra.
* **Eficiencia (Cero costo de CPU):** Al ser un operador de tiempo de compilación, el compilador resuelve esta división y coloca el número constante en el binario antes de grabarlo en la memoria Flash del microcontrolador. No consume ciclos de reloj durante la ejecución.
* **Seguridad:** Elimina el riesgo de "Hardcoding" (usar números mágicos como `8` o `12`), garantizando que los bucles siempre recorran el rango de memoria exacto de la estructura.

## 🔄 Lógica de Control Genérica

Gracias a que la estructura LedBar_t conoce su propio tamaño, el bucle for se vuelve universal. No importa si los LEDs están en el Puerto A o en el F; la lógica de animación permanece intacta:

```c
for (int i = 0; i < miBarra.count; i++) {
    // La función HAL recibe los parámetros directamente de la estructura
    HAL_GPIO_WritePin(miBarra.leds[i].port, miBarra.leds[i].pin, GPIO_PIN_SET);
    HAL_Delay(100);
    HAL_GPIO_WritePin(miBarra.leds[i].port, miBarra.leds[i].pin, GPIO_PIN_RESET);
}
```
🔍 Por qué este enfoque es Profesional

* **Mantenibilidad:** Si el equipo de hardware decide cambiar los pines en la PCB, el programador solo cambia una línea en la tabla de configuración.
* **Portabilidad:** Este código es fácilmente migrado a cualquier otra familia STM32 (F1, F4, H7) ya que depende de la estructura HAL y no de registros específicos.
* **Legibilidad:** El código del main() se vuelve mucho más humano y descriptivo, enfocándose en qué hace el sistema y no en cómo están conectados los cables.

---
*El buen uso de las estructuras de datos transforma un conjunto disperso de pines en un sistema coherente, escalable y fácil de mantener.*