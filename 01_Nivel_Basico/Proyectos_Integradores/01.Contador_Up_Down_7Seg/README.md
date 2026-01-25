# EI_1: Contador Pro UP/DOWN con Gestión de Errores y Reset Dual 🔄

Este es el primer proyecto integrador del **Nivel Básico**. Representa la consolidación de los pilares fundamentales: manejo avanzado de GPIO, estructuras de datos reentrantes, lógica de bits y diseño de **Experiencia de Usuario (UX)** en sistemas embebidos.

## 📍 Objetivos del Proyecto
- Gestionar entradas digitales con **Lógica Invertida (Active-Low)**.
- Implementar una **Ventana de Sincronización** temporal para detección de eventos simultáneos (Reset Dual).
- Crear un sistema de **Feedback Visual** mediante patrones de parpadeo (*Blink*) para alertar sobre límites de conteo.
- Aplicar **Abstracción de Hardware** para desacoplar la lógica matemática del display físico.

---

## 🔌 Especificaciones de Hardware
Para optimizar el diseño, se utiliza la configuración de resistencias internas del **STM32F439ZI**, simplificando el BOM (*Bill of Materials*) del proyecto.

* **Pulsadores:** Conectados directamente a **GND**. 
* **Configuración del Micro:** Entradas con **Pull-Up interna** habilitada (evita estados flotantes).
* **Display 7 Segmentos:** Cátodo Común (CC) con resistencias de limitación de 220Ω por segmento para proteger el Fan-out de los pines del MCU.



---

## 🧠 Lógica y Algoritmos de Integración

### 1. Ventana de Tiempo para Reset (Detección Dual)
Debido a la velocidad de ejecución del Cortex-M4 (180MHz), es físicamente imposible que dos pulsaciones mecánicas ocurran en el mismo ciclo de instrucción. Implementamos un algoritmo de **coincidencia temporal**:
1. El sistema detecta el primer flanco descendente en cualquiera de los botones.
2. Abre una **ventana crítica de 50ms**.
3. Si el segundo botón se activa dentro de esa ventana, el sistema interpreta un comando de **RESET**.
4. Si expira el tiempo, se procesa como una pulsación individual (UP o DOWN).

### 2. Gestión de Límites y Error de Rango (Blink Feedback)
Para una UX profesional, el sistema informa cuando una acción es inválida. Si el usuario intenta decrementar por debajo de 0 o incrementar sobre 9:
* Se dispara la función `Display_Blink()`.
* El dígito parpadea a una frecuencia de **3.3Hz** durante 3 ciclos.
* Esto indica al usuario que el sistema está "vivo" pero ha alcanzado un **límite de software**.



---

### 3. Antirrepetidor y Debounce Robusto
Cada transición está protegida por una doble capa de seguridad:
1. **Debounce Temporal (20ms):** Filtrado de ruido transitorio mediante muestreo diferido.
2. **Lógica de Enclavamiento (`while`):** Implementación de un "espera-hasta-liberar". Una pulsación física garantiza exactamente un cambio lógico, evitando el "auto-disparo" del contador.

---

## 💻 Implementación de la Capa de Abstracción

El uso de la estructura `LedBar_t` permite que la lógica de parpadeo sea **agnóstica al hardware**. Si se cambian los puertos del display, la función sigue funcionando sin cambios:

```c
/**
 * @brief Genera un parpadeo en el display para alertar error/límite.
 * @param numero: Dígito a mostrar.
 * @param repeticiones: Cantidad de destellos.
 */
void Display_Blink(uint8_t numero, uint8_t repeticiones) {
    for (int i = 0; i < repeticiones; i++) {
        // Apagado total iterando sobre la abstracción de pines
        for (int j = 0; j < miDisplay.count; j++) {
            HAL_GPIO_WritePin(miDisplay.leds[j].port, miDisplay.leds[j].pin, GPIO_PIN_RESET);
        }
        HAL_Delay(150);
        
        Display_Write(numero); // Re-encendido del patrón LUT
        HAL_Delay(150);
    }
}
```
---
*“Este proyecto demuestra que la complejidad de un sistema embebido no reside solo en el flujo principal, sino en la gestión de las excepciones y en la sincronización precisa de los eventos del mundo físico.”*