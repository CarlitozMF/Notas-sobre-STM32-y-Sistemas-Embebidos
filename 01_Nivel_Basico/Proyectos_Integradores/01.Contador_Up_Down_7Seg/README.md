# EI_1: Contador Pro UP/DOWN con Gestión de Errores y Reset Dual 🔄

Este es el primer proyecto integrador del **Nivel Básico**. Representa la consolidación de todos los conceptos aprendidos anteriormente: manejo de GPIO, estructuras de datos, lógica de bits, control de rebotes (debounce) y diseño de interfaz de usuario (UX).

## 📍 Objetivos del Proyecto
- Gestionar entradas digitales con **Lógica Invertida (Active-Low)**.
- Implementar una **Ventana de Sincronización** para detectar la pulsación simultánea de dos botones (Reset).
- Crear un sistema de **Feedback Visual** mediante parpadeo para alertar sobre límites de conteo.
- Aplicar **Abstracción de Hardware** mediante estructuras para el manejo del display.

## 🔌 Conexión de Hardware
Para este proyecto, los pulsadores se conectan directamente a tierra (**GND**) para simplificar el cableado.

- **Microcontrolador:** STM32F439ZI (Nucleo-144).
- **Pulsadores:** Conectados a GND. Requieren configuración de **Pull-Up interna** en el microcontrolador.
- **Display 7 Segmentos:** Cátodo Común (CC) conectado mediante resistencias de 220Ω.



## 🧠 Lógica y Algoritmos Implementados

### 1. Ventana de Tiempo para Reset (Detección Dual)
Debido a la velocidad del microcontrolador, es físicamente imposible presionar dos botones exactamente al mismo tiempo. Para solucionar esto, el código detecta cuando *cualquiera* de los dos botones baja a `0` y abre una ventana de espera de **50ms**.
- Si en ese tiempo el segundo botón también baja, se ejecuta el **RESET** (contador = 0).
- Si transcurre el tiempo y solo hay uno presionado, se procesa como **UP** o **DOWN**.

### 2. Gestión de Límites y Feedback (Blink Error)
Para mejorar la experiencia de usuario (UX), el sistema no se queda "congelado" al llegar a los límites (0 o 9). Si el usuario intenta superar estos valores, el programa llama a una función `Display_Blink()` que hace parpadear el dígito actual 3 veces, indicando que la acción fue recibida pero no es permitida.



### 3. Antirrepetidor y Debounce
Cada acción de conteo está protegida por:
1. **Debounce (20ms):** Ignora el ruido mecánico de las láminas del pulsador.
2. **Bucle de Bloqueo (`while`):** Evita que el contador se dispare si el usuario deja el botón presionado. Una pulsación física = Un cambio lógico.

## 💻 Fragmento de Código Clave: Lógica Genérica
El uso de la estructura `LedBar_t` permite que la función de parpadeo sea totalmente independiente de los pines físicos:

```c
void Display_Blink(uint8_t numero, uint8_t repeticiones) {
    for (int i = 0; i < repeticiones; i++) {
        // Apagado total usando la estructura de pines
        for (int j = 0; j < miDisplay.count; j++) {
            HAL_GPIO_WritePin(miDisplay.leds[j].port, miDisplay.leds[j].pin, GPIO_PIN_RESET);
        }
        HAL_Delay(150);
        Display_Write(numero); // Re-encendido
        HAL_Delay(150);
    }
}