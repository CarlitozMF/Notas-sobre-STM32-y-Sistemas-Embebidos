# 05_GPIO_Input_Polling - Entradas Digitales y Debounce 🔘

En este módulo aprendemos a leer señales externas (botones) utilizando la técnica de **Polling** y a manejar los problemas físicos inherentes a los interruptores mecánicos.

## 📍 Objetivos
- Leer el estado de un pin de entrada con `HAL_GPIO_ReadPin`.
- Comprender y solucionar el fenómeno del **Rebote (Bounce)**.
- Implementar una lógica de control simple basada en el estado de un pulsador.

## ⚡ El Problema: Rebote Mecánico (Bounce)
Cuando presionas un botón, las láminas metálicas no hacen contacto de forma limpia. Durante unos pocos milisegundos, chocan y se separan varias veces antes de quedar fijas.

Para un microcontrolador que corre a MHz, estos rebotes parecen múltiples pulsaciones rápidas. Sin un sistema de **Debounce**, una sola presión del usuario podría encender y apagar un LED 10 veces seguidas.



## 🛠️ Técnica: Polling con Software Debounce
El **Polling** consiste en preguntar constantemente dentro del bucle `while(1)` si el botón ha cambiado de estado. 

Para limpiar la señal, aplicamos los siguientes pasos en el código:
1. **Detección:** ¿Se presionó el botón?
2. **Espera (Debounce):** Retraso de 20ms para ignorar los ruidos mecánicos.
3. **Confirmación:** ¿Sigue presionado? Si es así, la pulsación es válida.
4. **Antirrepetidor:** Esperar a que el usuario suelte el botón (`while` bloqueante) para evitar que la acción se repita infinitamente mientras el dedo sigue ahí.

## 💻 Implementación de Referencia

```c
if (HAL_GPIO_ReadPin(GPIOC, USER_Btn_Pin) == GPIO_PIN_SET) 
{
    HAL_Delay(20); // Debounce
    if (HAL_GPIO_ReadPin(GPIOC, USER_Btn_Pin) == GPIO_PIN_SET) 
    {
        HAL_GPIO_TogglePin(GPIOB, LD1_Pin); // Acción: Cambiar LED
        
        // Bloqueo hasta soltar: evita repetición no deseada
        while (HAL_GPIO_ReadPin(GPIOC, USER_Btn_Pin) == GPIO_PIN_SET);
    }
}
```

## ⚠️ Consideraciones Técnicas

- Resistencias Pull-Up / Pull-Down: Son necesarias para evitar que el pin quede en un "estado flotante" cuando el botón no está presionado. La Nucleo-F439ZI ya incluye estas resistencias en su diseño para el botón azul.

- Limitación del Polling: Esta técnica consume ciclos de CPU constantemente. Si el while(1) tuviera procesos muy largos, podríamos "perdernos" el momento exacto en que el usuario presiona el botón. (Esto se soluciona en niveles avanzados mediante Interrupciones).

---
*Notas sobre interfaces de usuario y control de entradas para STM32.*