# 03_TIM_PWM: Dominando la energía: del control binario a la modulación por ancho de pulso (PWM) 

Este proyecto documenta el uso de **Timers** de hardware para crear modulación por ancho de pulsos (inglés Pulse width Modulation PWM) de una señal modificando el ciclo de trabajo de una señal periódica, para controlar la cantidad de energía que se envía a una carga con la **Nucleo-F439ZI**.

Se implementa un **Planificador Cooperativo (Scheduler)** no bloqueante, diseñado para gestionar múltiples periféricos de potencia de forma concurrente. El sistema integra efectos visuales avanzados, control de registros a bajo nivel y una arquitectura de software modular para el aprendizaje de Timers y PWM.

## 🔩 Teoría de Operación: Timers y PWM

### 1. Conceptos Fundamentales de PWM

El modo PWM (Modulación por Ancho de Pulso) emula un voltaje analógico variable mediante la conmutación rápida de una señal digital. Su comportamiento en los Timers de STM32 se rige por tres parámetros críticos:

* **Periodo (ARR - Auto-Reload Register):** Es el valor máximo del contador. Define la duración total de un ciclo y, junto con el reloj del sistema, determina la frecuencia.
* **Duty Cycle (CCR - Capture Compare Register):** Define el "ancho" del pulso. Es el valor contra el cual se compara el contador para conmutar la salida entre estado ALTO y BAJO.
* **Frecuencia (fpwm​):** Es la inversa del periodo. Para control de LEDs, se utiliza una f≥1 kHz para eliminar el flicker (parpadeo) perceptible por el ojo humano.

### 2. Cálculo de Frecuencia y Resolución

La frecuencia de la señal PWM está determinada por la frecuencia de reloj del periférico ($f_{clk}$) y la configuración de los divisores del Timer. La relación matemática es:

$$f_{pwm} = \frac{f_{clk}}{(PSC + 1) \cdot (ARR + 1)}$$

#### Parámetros de Configuración:

* **$f_{clk}$ (Timer Clock):** Frecuencia de entrada al Timer. En la **STM32F439ZI**, esta frecuencia depende del bus al que está conectado el Timer (APB1 o APB2). Si el divisor del bus es mayor a 1, el reloj del Timer se multiplica automáticamente por 2.
    * *Ejemplo:* Con el sistema a 180 MHz, los Timers en APB2 suelen correr a **180 MHz**.
* **PSC (Prescaler):** Registro de 16 bits que divide la frecuencia de entrada. Se le suma `1` en la fórmula porque el conteo es base cero.
* **ARR (Auto-Reload Register):** Conocido como *Counter Period*. Define el valor máximo del contador y, por ende, la resolución del Duty Cycle.



#### Resolución del Duty Cycle
La resolución (o precisión del control) está ligada directamente al valor del **ARR**. Un valor de $ARR = 999$ proporciona una resolución de **1000 niveles** (0.1% por paso), mientras que un $ARR = 65535$ maximiza la precisión en Timers de 16 bits.

> **Regla de oro:** Para obtener una frecuencia exacta de 1 kHz con un reloj de 180 MHz, una combinación común es:
> * **PSC = 179**
> * **ARR = 999**

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

