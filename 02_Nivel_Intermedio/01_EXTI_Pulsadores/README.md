# 01_EXTI_Pulsadores: Contador de 3 Dígitos con Control EXTI 📑

Este proyecto implementa un contador digital de 0 a 999 sobre tres displays de 7 segmentos multiplexados, **integrando interrupciones externas** para el control de flujo.

# 🚀 Características

- Multiplexación por División de Tiempo: Control de 3 dígitos con bus compartido.
- Tiempo No Bloqueante: Uso de HAL_GetTick() para el avance del contador.
- Control por Interrupciones (EXTI):
    1. Botón Reset: Reinicio instantáneo a 000.
    2. Botón Pause/Play: Congela/Reanuda el conteo.
    3. Filtrado de Hardware: Uso de capacitores de 100nF para eliminar el bounce mecánico.

# 🛠️ Configuración de Hardware

- Displays: 3x 7-Segmentos (Cátodo Común).
- Pines de Datos: SEG_A a SEG_G (GPIO Output).
- Pines de Control: EN1, EN2, EN3 (Transistores NPN).
- Pin de Salida Led: usr_led
- Botones Externos: Conectados a GND con configuración Pull-Up interna.
    1. btn_rst (PB10) - Falling Edge.
    2. btn_pp (PB11) - Falling Edge.

## 📌 Asignación de Pines (Pinout)

| Componente | Pin STM32 | Etiqueta (User Label) | Modo GPIO |
| :--- | :--- | :--- | :--- |
| Segmento A | PB8 | `SEG_A` | Output Push-Pull |
| Segmento B | PB9 | `SEG_B` | Output Push-Pull |
| Segmento C | PA5| `SEG_C` | Output Push-Pull |
| Segmento D | PA6 | `SEG_D` | Output Push-Pull |
| Segmento E | PA7 | `SEG_E` | Output Push-Pull |
| Segmento F | PD14 | `SEG_F` | Output Push-Pull |
| Segmento G | PD15 | `SEG_G` | Output Push-Pull |
| Habilitador 1 | PC8 | `EN1` | Output Push-Pull |
| Habilitador 2 | PC9 | `EN2` | Output Push-Pull |
| Habilitador 3 | PC10 | `EN3` | Output Push-Pull |
| Led Usuario | PF13 | `usr_led` | Output Push-Pull |
| Botón Reset | PB10 | `btn_rst` | EXTI (Falling Edge) |
| Botón Pause | PB11 | `btn_pp` | EXTI (Falling Edge) |

# 🧠 Lógica de Software

El sistema utiliza el NVIC para priorizar las acciones de los botones sobre el bucle principal. Gracias a los capacitores físicos, el código de la ISR (Interrupt Service Routine) permanece limpio y sin necesidad de delays por software.

## 🔄 Flujo de Control
El sistema opera bajo dos estados principales controlados por la interrupción del botón de pausa:
- **Estado PLAY:** El contador incrementa cada 100ms validando la diferencia de `HAL_GetTick()`.
- **Estado PAUSE:** El incremento se detiene, pero la función de barrido `DisplayMux_Scan()` se sigue ejecutando para mantener la persistencia visual de los últimos datos.

## ⚡ Solución al Rebote (Debounce) Hardware
Para evitar disparos múltiples de la interrupción EXTI, se implementó un filtro RC básico:
1. Se activó la resistencia de **Pull-Up interna** del STM32.
2. Se colocó un **capacitor de 100nF** en paralelo con cada pulsador.
Esta configuración actúa como un filtro pasa-bajos que elimina el ruido mecánico del pulsador, permitiendo que la ISR se ejecute exactamente una vez por cada pulsación real.

## 🕹️ Instrucciones de Uso
1. Al iniciar, el contador comenzará automáticamente de 000 a 999.
2. Presione `BTN_PAUSE` para congelar el tiempo. El LED azul de la placa indicará el estado de pausa.
3. Presione `BTN_RESET` en cualquier momento para volver el conteo a 000.

# Notas sobre Interrupciones Externas "El Callback"

## 🧠 ¿Qué es realmente el Callback?

En la arquitectura de ST, el hardware salta primero a un archivo llamado stm32fxx_it.c (donde están las ISR reales). Esas funciones son cortas y llaman a una función general de la HAL (HAL_GPIO_EXTI_IRQHandler).
Esa función de la HAL hace el trabajo sucio: limpia las banderas de interrupción (para que no se repitan) y finalmente llama al Callback.
- Dato Clave: El Callback está definido como una función __weak (débil). Esto significa que la HAL tiene una versión vacía, pero si vos escribís una con el mismo nombre en tu main.c, el compilador usa la tuya. Es como un "enchufe" esperando que conectes tu lógica.

## 🚦 Manejo de múltiples EXTI en el mismo Callback
A diferencia de otros periféricos, todas las interrupciones de GPIO (sin importar si es el pin 0 o el 15) terminan en la misma función de Callback. Por eso es obligatorio usar el parámetro GPIO_Pin para identificar quién llamó. Para este ejemplo el callback es el que sigue:

```c
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    // BOTÓN DE RESET
    if (GPIO_Pin == btn_rst_Pin)
    {
        contador_global = 0;
        valores_display[0] = 0;
        valores_display[1] = 0;
        valores_display[2] = 0;
    }

    // BOTÓN DE PAUSE / PLAY
    if (GPIO_Pin == btn_pp_Pin)
    {
        pausa_activada = !pausa_activada; // Cambia el estado (Toggle)
        HAL_GPIO_TogglePin(usr_led_GPIO_Port, usr_led_Pin); // LED azul para indicar pausa
    }
}
```
# ⚠️ Reglas de Oro para el Callback

Para que tu código sea estable, el Callback debe respetar estas tres leyes:

1. Brevedad Extrema: El microcontrolador detiene todo el programa (incluyendo el barrido de tus displays) mientras está dentro del Callback. Si hacés cálculos largos o ponés un HAL_Delay, el display va a parpadear o el sistema se va a colgar.
2. No usar bloqueantes: Nunca uses HAL_Delay() ni funciones que esperen a que algo pase (como recibir un dato de UART).
3. Banderas (Flags): Si el botón debe disparar algo complejo (como escribir en una tarjeta SD o calcular una raíz cuadrada), hacé esto:
    1. En el Callback: Ponés una variable ejecutar_tarea = 1;.
    2. En el while(1): Preguntás if(ejecutar_tarea) y hacés el trabajo pesado ahí.

---
*En este proyecto dimos el salto al Nivel Intermedio introduciendo el uso de etiquetas (User Labels) en el .ioc, lo que nos permite escribir un código más genérico, legible y fácil de migrar entre diferentes pines.*
*Sin embargo, es importante notar que la función Display_Mux_Scan() todavía vive en el bucle principal (while(1)). Esto significa que la persistencia visual del display depende totalmente de la velocidad del CPU: si el procesador se ocupa en una tarea pesada o bloqueante, el barrido se detendrá y el display parpadeará.*
*El siguiente desafío será independizar el hardware mediante el uso de Timers, logrando que el display sea totalmente autónomo.*