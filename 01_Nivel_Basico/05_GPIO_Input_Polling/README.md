# 05_GPIO_Input_Polling - Entradas Digitales y Debounce 🔘

Este módulo aborda la lectura de señales externas (periféricos de entrada) utilizando la técnica de **Polling** y la implementación de filtros para mitigar los problemas físicos inherentes a los interruptores mecánicos.

## 📍 Objetivos
- Configurar y leer pines en modo entrada con `HAL_GPIO_ReadPin`.
- Comprender el fenómeno físico del **Rebote (Bounce)** y su impacto en sistemas de alta velocidad.
- Implementar una lógica de control con **Software Debounce** y detección de flancos.

---

## ⚡ El Desafío Físico: Rebote Mecánico (Bounce)
Al presionar un botón, las láminas metálicas internas vibran y chocan entre sí durante unos milisegundos antes de establecer un contacto estable. 



Para una **STM32F439ZI** que ejecuta millones de instrucciones por segundo, estas vibraciones se interpretan como múltiples pulsaciones ultra rápidas. Sin un sistema de filtrado, una sola presión del usuario dispararía la lógica del programa decenas de veces de forma errática.

---

## 🛠️ Metodología: Polling y Filtrado
El **Polling** (muestreo) consiste en consultar cíclicamente el estado del registro `IDR` (Input Data Register) del GPIO. Para limpiar la señal, aplicamos un algoritmo de validación temporal:

1. **Detección de Flanco:** Se detecta un cambio de estado en el pin.
2. **Ventana de Confirmación (Debounce):** Se genera un retraso de ~20ms para ignorar el ruido transitorio.
3. **Re-muestreo:** Se verifica si la señal sigue activa. Si es así, se considera una pulsación legítima.
4. **Lazo de Espera (Antirrepetidor):** Se utiliza un bloqueo para esperar que el usuario suelte el botón, evitando que una pulsación larga se cuente como múltiples eventos.

## 💻 Implementación de Referencia

```c
// Verificamos si el botón azul de la Nucleo (PC13) está presionado
if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13) == GPIO_PIN_SET) 
{
    HAL_Delay(20); // Ventana de Debounce por software
    
    if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13) == GPIO_PIN_SET) 
    {
        HAL_GPIO_TogglePin(GPIOB, LD1_Pin); // Ejecución de la acción
        
        /* Bloqueo hasta soltar el pulsador */
        // Evita que el código siga ejecutándose mientras el dedo permanece en el botón
        while (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13) == GPIO_PIN_SET);
    }
}
```
## ⚠️ Consideraciones

* Estados Flotantes (Floating): Un pin de entrada nunca debe quedar "al aire". Si no hay una referencia de voltaje fija (VCC o GND), el ruido electromagnético ambiental provocará lecturas aleatorias. La Nucleo-F439ZI soluciona esto mediante resistencias de Pull-Down internas o externas.
* Eficiencia del CPU: La técnica del while al final del código es efectiva pero bloqueante. Durante ese tiempo, el microcontrolador no puede procesar nada más (como actualizar un display o leer otro sensor).
* Hacia el Nivel Intermedio: En etapas posteriores, reemplazaremos este esquema por Interrupciones Externas (EXTI) para lograr una respuesta inmediata y no bloqueante.

---
*En sistemas embebidos, el software debe ser capaz de filtrar la imperfección del mundo físico para transformarla en lógica digital confiable.*