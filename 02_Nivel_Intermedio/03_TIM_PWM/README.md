# 03_TIM_PWM: Dominando la energía: del control binario a la modulación por ancho de pulso (PWM) 

Este proyecto documenta el uso de los **Timers de hardware** para generar señales de Modulación por Ancho de Pulso (**PWM**). El objetivo es controlar la entrega de energía a periféricos de potencia de forma eficiente y asíncrona mediante la **Nucleo-F439ZI**, integrando un **Planificador Cooperativo (Scheduler)** para la gestión de tareas concurrentes.

## 🎯 Objetivos
- **Dominar la generación de señales PWM** mediante los registros de comparación (CCR) de los Timers.
- **Implementar un Kernel Cooperativo** no bloqueante para la gestión de múltiples tareas (*Tasks*).
- **Aplicar conceptos de colorimetría** (Modelo HSV y Corrección Gamma) para el control avanzado de LEDs RGB.
- **Calcular la resolución y frecuencia** del PWM para optimizar el control de potencia y evitar el parpadeo visual (*Flicker*).

---

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

# 🚀 Arquitectura del Sistema

El firmware está diseñado bajo un esquema de planificación cooperativa, donde cada tarea se ejecuta en intervalos de tiempo predefinidos sin bloquear el procesador. Esto garantiza que el control de los periféricos sea fluido y estable.

## 🧠 Arquitectura: Kernel Cooperativo

El "cerebro" del proyecto es un planificador basado en una arquitectura orientada a tareas. Utiliza aritmética de tiempo para decidir cuándo ejecutar cada función, optimizando el uso de la CPU al eliminar funciones bloqueantes como `HAL_Delay()`.

### 1. Estructura de Tareas (`Task_t`)
Se define una estructura que permite gestionar cada tarea de forma independiente:

```c
typedef struct {
    void (*pTask)(void);    // Puntero a la función de la tarea
    uint32_t period;        // Período de ejecución en ms
    uint32_t lastTick;      // Último tiempo de ejecución
} Task_t;
```

### 2. Tabla de Planificación

Las tareas se organizan con diferentes prioridades implícitas según su frecuencia:
|   Tarea   |   Período |   Frecuencia  |   Propósito   |
|BreathingLED   |   15 ms   |   ~66.6 Hz    |   Fluidez visual en el efecto de respiración. |
|RGBHandler |   30 ms   |   ~33.3 Hz    |   Control de un LED RGB con driver de contro. |
|Heartbeat  |   500 ms  |   2 Hz    |   Monitoreo de salud del Kernel.  |

Se define una tabla de tareas con su tiempo de ejecucion.

 ```c
 /* --- Tabla de Tareas --- */
Task_t taskTable[] = {
    { Task_Heartbeat,    500, 0 }, // Tarea 1: Cada 500ms
    { Task_BreathingLED,  15, 0 }, // Tarea 2: Cada 15ms
    { Task_RGBHandler,    30, 0 }  // Tarea 3: Cada 30ms
};
#define NUM_TASKS (sizeof(taskTable)/sizeof(Task_t)
```

#### 1. **Task_Heartbeat (GPIO Toggle)**
Funciona como el monitor de salud del sistema.
* Periférico: LED externo conectado al pin PB8 (usr_ledRojo).
* Función: Realiza un toggle (cambio de estado) del pin en un intervalo largo (ej. 500ms).
* Propósito: Si el LED deja de parpadear, indica que una de las otras tareas bloqueó el Kernel o hubo un fallo en el procesador.

#### 2. **Task_Breathing (Timer 3 - PWM)**
Controla un LED para generar un efecto de "respiración".
* Periférico: TIM3_CH4.
* Lógica: Modifica el ciclo de trabajo de forma incremental en cada llamada, invirtiendo el sentido al alcanzar los límites (0-1000).
* Frecuencia de ejecución: 20ms (típico para suavidad visual).

#### 3. **Task_RGBHandler (Timer 4 - Multi-Channel PWM)**
Es el núcleo visual del proyecto. Encapsula la lógica compleja de color para un LED RGB.
* Periférico: TIM4 (Canales 2, 3 y 4).
* Funcionalidad:

    **1. Abstracción:** El driver permite manejar el hardware independientemente de si el LED es de ánodo o cátodo común.

    **2. Modelo HSV:** Permite transiciones de color naturales (cambio de tono) sin saltos bruscos.

    **3. Corrección Gamma:** Mapea los valores de brillo para compensar la respuesta no lineal del ojo humano.

### 3. Ejecución del Planificador

El loop principal (while(1)) despacha las tareas comparando el currentTick contra el lastTick de cada una, asegurando que el sistema sea determinista y eficiente:
```c
uint32_t currentTick = HAL_GetTick();

for (int i = 0; i < NUM_TASKS; i++) {
    if (currentTick - taskTable[i].lastTick >= taskTable[i].period) {
        taskTable[i].lastTick = currentTick;
        taskTable[i].pTask(); // Ejecución de la tarea
    }
}
```
## 🏁 Conclusión

Este proyecto representa:

- La consolidación de dos mundos: la precisión cronométrica de los Timers y la flexibilidad de la Modulación por Ancho de Pulso (PWM).
- Se logró la transición de una señal digital binaria a una señal analógica emulada, permitiendo un control granular sobre la energía entregada a los periféricos.
- La abstracción de hardware a través de un **Driver RGB** con corrección logarítmica
- La implementación del **Planificador Cooperativo** demuestra que la eficiencia no depende de la velocidad del reloj, sino de la arquitectura del código. Al eliminar el uso de funciones bloqueantes, permitimos que el hardware de la **STM32F439ZI** despliegue su verdadero potencial multitarea.

---
*Nivel Intermedio: Desacoplamiento del software y optimización del CPU mediante hardware reactivo. El PWM es el puente definitivo entre el mundo digital del microcontrolador y la potencia del mundo físico.*