# 01_EXTI_Pulsadores: Contador de 3 Dígitos y Gestión de Eventos Asíncronos 📑

Este proyecto implementa un contador digital de 0 a 999 sobre una matriz de tres displays de 7 segmentos multiplexados, integrando por primera vez **Interrupciones Externas (EXTI)** para el control de flujo en tiempo real.

## 🎯 Objetivos
- **Comprender el funcionamiento del NVIC** (Nested Vectored Interrupt Controller) para la gestión de prioridades de hardware.
- **Implementar funciones de Callback** para responder a eventos externos de forma asíncrona.
- **Diferenciar el procesamiento reactivo (EXTI)** del sondeo constante (*Polling*).
- **Aplicar técnicas de acondicionamiento de señal** (Filtro RC) para resolver problemas de rebote mecánico (*Bounce*).

---

## 🚀 Características Técnicas
- **Multiplexación por División de Tiempo (TDM):** Control eficiente de 3 dígitos mediante bus de datos compartido.
- **Concurrencia Básica:** Uso de `HAL_GetTick()` para el avance del tiempo sin detener el barrido visual.
- **Control Reactivo (EXTI):**
    1. **Reset Inmediato:** Reinicio a 000 con prioridad de hardware.
    2. **Control de Flujo (Pause/Play):** Congelación y reanudación del proceso lógico.
- **Acondicionamiento de Señal:** Filtrado analógico (Hardware Debounce) para ISRs limpias.

---

## 🛠️ Configuración de Hardware y Pinout

Se utilizan etiquetas personalizadas (**User Labels**) en el archivo `.ioc` para garantizar que el código sea independiente de los cambios físicos en el PCB.

| Periférico | Pin | Etiqueta | Modo GPIO | Función |
| :--- | :--- | :--- | :--- | :--- |
| **Segmentos** | PB8-PD15 | `SEG_A`..`SEG_G` | Output | Bus de datos paralelo |
| **Habilitadores**| PC8-PC10 | `EN1`..`EN3` | Output | Control de transistores NPN |
| **Reset** | PB10 | `btn_rst` | **EXTI** | Detección Falling Edge |
| **Pause/Play** | PB11 | `btn_pp` | **EXTI** | Detección Falling Edge |

---

## ⚡ Solución al Rebote: Debounce por Hardware
En sistemas basados en interrupciones, el ruido mecánico de un pulsador es crítico, ya que dispararía la ISR múltiples veces en microsegundos. Para mantener una **ISR limpia y eficiente**, se implementó un filtro RC:
1. **Pull-Up Interna:** Garantiza un estado lógico '1' estable.
2. **Capacitor de 100nF:** Actúa como un filtro pasa-bajos, absorbiendo los transitorios del contacto mecánico.
Esto permite que el Callback se ejecute exactamente una vez por pulsación, eliminando la necesidad de delays bloqueantes dentro del código de interrupción.

---

## 🧠 Arquitectura de Software: El Callback

El sistema utiliza el **NVIC (Nested Vectored Interrupt Controller)** para priorizar las acciones de los botones sobre el bucle principal. 

### ¿Qué es realmente el Callback?
En la arquitectura de ST, el hardware salta a una ISR genérica en `stm32fxx_it.c`. La HAL gestiona la limpieza de banderas (*flags*) y finalmente invoca a la función `HAL_GPIO_EXTI_Callback`.
> **Dato de Ingeniería:** Esta función está definida como `__weak`. Esto permite que el programador la redefina en el `main.c`, actuando como un "enchufe" donde conectamos nuestra lógica de respuesta.

```c
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    /* Identificación del origen de la interrupción */
    if (GPIO_Pin == btn_rst_Pin) {
        contador_global = 0;
        memset(valores_display, 0, sizeof(valores_display)); // Limpieza de buffer
    }

    if (GPIO_Pin == btn_pp_Pin) {
        pausa_activada = !pausa_activada; // Toggle de estado lógico
        HAL_GPIO_TogglePin(usr_led_GPIO_Port, usr_led_Pin); 
    }
}
```
## ⚠️ Reglas de Oro para ISR (Interrupt Service Routines)

Para garantizar la estabilidad del sistema, el Callback debe seguir estas leyes fundamentales:

* **Brevedad Extrema:** El microcontrolador detiene el `while(1)` (y por ende, tu barrido de displays) mientras está en el Callback.
* **No Bloqueante:** Está estrictamente prohibido usar `HAL_Delay()` o funciones de espera.
* **Uso de Flags:** Para tareas pesadas (UART, Cálculos), solo se debe activar una bandera en el Callback y procesarla en el bucle principal.

## 🔍 Observación

Aunque el control es reactivo, la persistencia visual `(DisplayMux_Scan)` todavía depende de la velocidad del CPU en el `while(1)`. Si el procesador se ocupa en una tarea pesada, el display **parpadeará**.

**Próximo Desafío:** Independizar el barrido mediante **Timers**, logrando un hardware autónomo que no dependa de la carga del software principal.

---
*“Módulo de Nivel Intermedio: Transición hacia una arquitectura orientada a eventos y gestión de latencia. El uso de EXTI es el primer paso para transformar un programa secuencial en un sistema embebido profesional de respuesta inmediata.”*