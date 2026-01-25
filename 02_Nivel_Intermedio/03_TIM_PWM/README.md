# 03_TIM_PWM: Dominando la energía: del control binario a la modulación por ancho de pulso (PWM) 

Este proyecto documenta el uso de **Timers** de hardware para crear modulación por ancho de pulsos (inglés Pulse width Modulation PWM) de una señal modificando el ciclo de trabajo de una señal periódica, para controlar la cantidad de energía que se envía a una carga con la **Nucleo-F439ZI**.

Se implementa un **Planificador Cooperativo (Scheduler)** no bloqueante, diseñado para gestionar múltiples periféricos de potencia de forma concurrente. El sistema integra efectos visuales avanzados, control de registros a bajo nivel y una arquitectura de software modular para el aprendizaje de Timers y PWM.

## 🔩 Teoría de Operación: Timers y PWM

### 1. Conceptos Fundamentales de PWM
El modo **PWM (Pulse Width Modulation)** permite emular una señal analógica variando el ancho de un pulso digital. Se utiliza para controlar la potencia entregada a los LEDs mediante la manipulación de tres registros clave del Timer:
* **Periodo (ARR):** Define el ciclo total de la señal.
* **Duty Cycle (CCR):** Define el tiempo que la señal permanece en estado ALTO.
* **Frecuencia ($f_{pwm}$):** Velocidad de conmutación (1 kHz para evitar *flicker*).

### 2. Cálculo de Frecuencia y Resolución
La frecuencia se determina mediante la relación entre el reloj del sistema ($f_{clk}$) y los divisores del Timer:

$$f_{pwm} = \frac{f_{clk}}{(PSC + 1) \cdot (ARR + 1)}$$

Donde:
* fclk​: Es la frecuencia del reloj que alimenta al Timer (en una F439ZI, suele ser 90MHz o 180MHz dependiendo de qué bus APB use el Timer).
* PSC (Prescaler): Divisor previo para "ralentizar" el reloj.
* ARR (Auto-Reload Register): Es el "Counter Period". Define cuántos pasos cuenta el timer antes de volver a cero.

**Configuración aplicada:**
* **Resolución:** 1000 pasos (ARR = 999).
* **Frecuencia de conteo:** 1 MHz (PSC ajustado según bus APB).
* **Resultado:** Señal de 1 kHz con precisión del 0.1% en el Duty Cycle.

### 3. Configuración en STM32CubeMX
Para replicar el comportamiento, los periféricos **TIM3** y **TIM4** deben configurarse con los siguientes parámetros:

| Parámetro | Valor | Descripción Técnica |
| :--- | :--- | :--- |
| **Clock Source** | Internal Clock | Uso del oscilador interno del sistema. |
| **Channel x** | PWM Generation | Activación del modo comparación para salida de pines. |
| **PWM Mode** | Mode 1 | Salida activa mientras `CNT < CCR`. |
| **CH Polarity** | High | Lógica directa (invertida por software si es Ánodo Común). |
| **Preload** | Enable | Garantiza cambios de brillo suaves sin glitches. |

### 4. Control de Registros y Eficiencia
El driver realiza escritura directa en memoria mediante macros de la HAL, permitiendo que el **Scheduler** gestione los efectos sin latencia:

```c
// Actualización instantánea del registro de comparación
__HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, valor_pwm);
```

## 🚀 Características Principales
* **Planificador de Tareas (Kernel Cooperativo):** Gestión de hilos de ejecución basada en aritmética de tiempos con `HAL_GetTick()`, evitando el uso de funciones bloqueantes.
* **Breathing LED (TIM3):** Efecto de "respiración" analógica mediante modulación PWM en el Canal 4 del Timer 3.
* **Driver RGB Encapsulado (TIM4):** Controlador modular con soporte para:
    * Arquitecturas de Ánodo y Cátodo común.
    * **Corrección Gamma (2.2):** Ajuste logarítmico para una percepción visual natural.
    * **Modelo de Color HSV:** Transiciones de color suaves mediante Tono, Saturación y Brillo.
* **Monitor de Estado (Heartbeat):** LED de señalización mediante Toggle para monitorear la salud del planificador.

---

