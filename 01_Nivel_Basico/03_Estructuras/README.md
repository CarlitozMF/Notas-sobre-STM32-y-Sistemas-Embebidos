# 03_Estructuras - Optimización de Memoria RAM 🏗️

Este módulo explora la organización interna de los datos en la memoria de la **STM32F439ZI** y cómo el diseño estratégico de estructuras permite reducir el consumo de RAM, evitando el desperdicio de bytes innecesarios.

## 📍 Objetivos
- Comprender la importancia del uso de Enumeraciones `enum` y Estructuras `struct`.
- Comprender el concepto de **Memory Alignment** (Alineación de memoria) en arquitecturas de 32 bits.
- Identificar y medir el **Padding** (relleno) generado automáticamente por el compilador.
- Aplicar la técnica de reordenamiento de miembros para optimizar el espacio.

---

## Fundamentos de Programación: Estructuras.

En este proyecto, se utilizan `struct` para mejorar la legibilidad y mantenibilidad del código, evitando el uso de "números mágicos".

###  Estructuras (`struct`)
Permiten agrupar variables de diferentes tipos bajo un mismo nombre. En sistemas embebidos, las usamos poe ejemplo para empaquetar toda la configuración de un periférico o un objeto (como el display) en un solo bloque de memoria.
- **Definición:**  Se utiliza la palabra clave struct, seguida de un nombre (etiqueta) y los miembros entre llaves. Para simplificar la declaración de variables y no tener que escribir struct cada vez, se suele usar `typedef`.
- **Acceso a miembros"** Se utiliza el operador punto `(.)` para acceder a cada elemento de la estructura.
- **Flexibilidad:** Pueden contener diferentes tipos de datos, incluidos otros structs, punteros y arreglos.
- **Instanciación:** Se pueden crear múltiples instancias (variables) de una estructura definida.
- **Ventaja:** Permite pasar toda la configuración del hardware a una función usando un solo puntero.
* **Ejemplo:**
    ```c
   //Estructura 1 para definir los pines y puertos a controlar
   typedef struct {
      GPIO_TypeDef* port;  // GPIOA, GPIOB, etc.
      uint16_t pin;       // GPIO_PIN_0, etc.
   } GPIO_Config_t;

   // Estructura 2 para definir un arreglo de leds
   typedef struct {
      GPIO_Config_t* leds; // Puntero a un arreglo de configuraciones
   } Leds_t;
    ```

> **Nota Técnica:** El uso de `typedef` junto a estas estructuras permite definir tipos de datos personalizados que siguen el estándar de la arquitectura de capas, facilitando la portabilidad a otros microcontroladores.

* **Modo de Aplicación:**
Una vez definidas las estructuras, con *Estructura 1* preparamos en un array sin dimensión (debido a la alta capacidad de memoria en estos micros) los pines configurados como salida para poder usarlos. Con `#define LED_COUNT (sizeof(configuracion_leds) / sizeof(configuracion_leds[0]))` tenemos una forma automática de establecer la cantidad de elementos de la estructura. Con *Estructura 2* creamos otro array que encapsulará **toda** la configuración del display de 7 segmentos.

   ```c
   /* --- APLICACION DE LAS ESTRUCUTRAS DE CONTROL DE HARDWARE --- */
   // 1. Definimos los pines físicos (pueden ser de cualquier puerto) mediante un arreglo de estructuras
   GPIO_Config_t configuracion_leds[] = {
      {GPIOB, GPIO_PIN_0},
	   {GPIOB, GPIO_PIN_7},
      {GPIOB, GPIO_PIN_14}
   };

   // 2. Inicializamos la estructura usando la constante calculada
   Leds_t barra_leds = {configuracion_leds, LED_COUNT};

   // 3. Usamos sizeof para calcular la cantidad de elementos automáticamente
   // Fórmula: Tamaño total del arreglo / Tamaño de un solo elemento
   #define LED_COUNT (sizeof(configuracion_leds) / sizeof(configuracion_leds[0]))

   ```
* **Modo de Uso**
Sabiendo que `LED_COUNT` es un valor fijo calculado automáticamente que nos indica la cantidad de elementos, podemos usarlo como *tope* dentro de cualquier ciclo de **iteración**. Dentro de este ciclo accedemos mediante **(.)** a los puertos y pines mediante *Estructura 2* que contiene a *Estructura 1*, para poder escribir sobre un pin determinado con la función `HAL_GPIO_WritePin(GPIO_TypeDef* PUERTO, uint16_t PIN, GPIO_PinState ESTADO)`.
```c
  while (1)
  {
      for (int i = 0; i < LED_COUNT; i++)
      {
          HAL_GPIO_WritePin(barra_leds.leds[i].port, barra_leds.leds[i].pin, GPIO_PIN_SET);
          HAL_Delay(100);
          HAL_GPIO_WritePin(barra_leds.leds[i].port, barra_leds.leds[i].pin, GPIO_PIN_RESET);
      }
  }
```

## 🧱 El Fenómeno del Padding

En una arquitectura **ARM Cortex-M4**, el procesador accede a la memoria de forma más eficiente en bloques de 32 bits (4 bytes). Para mantener el rendimiento, el compilador exige que las variables residan en direcciones de memoria que sean múltiplos de su propio tamaño.

### ¿Por qué se desperdicia memoria?
Si declaramos un `uint8_t` seguido de un `uint32_t`, el microcontrolador no puede leer el dato de 32 bits si este comienza en una dirección impar. El compilador inserta **3 bytes de relleno (Padding)** para "empujar" la variable grande a la siguiente dirección alineada.

---

## 🚀 Técnica de Optimización: "De Mayor a Menor"

Para minimizar el desperdicio, la regla de oro en sistemas embebidos es declarar los miembros de la estructura en **orden descendente de tamaño**. Esto permite que el compilador agrupe las variables pequeñas en los huecos que dejan las grandes.🛠️ **Carlos** | Estudiante de Ing. Electrónica @UTN_FRT.  
🚀 Apasionado Autodidacta por los Sistemas Embebidos.

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

🛠️ **Carlos** | Estudiante de Ing. Electrónica @UTN_FRT.  
🚀 Apasionado Autodidacta por los Sistemas Embebidos.