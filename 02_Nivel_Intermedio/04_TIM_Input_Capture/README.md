# 04_TIM_Input_Capture: Telemetría de precisión y captura de eventos en tiempo real

Este proyecto documenta la implementación del periférico de **Input Capture (IC)** para la medición precisa de intervalos temporales. Se utiliza un sensor ultrasónico **HC-SR04** para validar la arquitectura, integrando una base de tiempos determinística mediante **DWT** y una interfaz visual multiplexada por interrupción, todo bajo la plataforma **Nucleo-F439ZI**.

## 🎯 Objetivos
- **Dominar el modo Input Capture** para la detección de flancos y medición de ancho de pulso.
- **Implementar una base de tiempo de microsegundos** de alta precisión utilizando el contador de ciclos de CPU (**DWT**).
- **Desarrollar un Driver No Bloqueante** para sensores ultrasónicos basado en una máquina de estados finitos (FSM).
- **Garantizar la concurrencia** entre la adquisición de datos (TIM3) y el refresco visual (TIM2) mediante la gestión jerárquica de prioridades en el **NVIC**.

---

## 🔩 Teoría de Operación: Input Capture y Telemetría

### 1. Conceptos Fundamentales de Input Capture (IC)

El modo **Input Capture** permite al Timer registrar ("capturar") el valor de su contador interno (`CNT`) en el instante exacto en que ocurre un evento en un pin (flanco de subida o bajada). Es la técnica estándar para medir frecuencias, periodos y anchos de pulso con error mínimo.

* **Registro de Captura (CCR):** Al detectar el flanco, el valor de `CNT` se copia automáticamente al registro `CCR`.
* **Diferencia de Ticks:** La duración de un evento se calcula restando el valor capturado en el flanco de bajada menos el valor del flanco de subida.
* **Resolución del Timer:** Define el "paso" mínimo de tiempo. Para este proyecto, se configura para obtener **1 tick = 1 μs**.

### 2. Medición de Distancia (Ultrasonido)

El sensor **HC-SR04** requiere un pulso de excitación (*Trigger*) de $10 \mu s$. El sensor responde emitiendo una ráfaga sónica y poniendo su pin *Echo* en estado ALTO durante un tiempo proporcional a la distancia del objeto.

La fórmula de conversión, asumiendo la velocidad del sonido a $343 \text{ m/s}$ ($0.0343 \text{ cm/μs}$), es:

$$Distancia [cm] = \frac{Ticks_{pulse} \cdot 0.0343}{2}$$



### 3. Configuración en STM32CubeMX (TIM3)

Para la captura del pulso, el **TIM3** se configura de la siguiente manera para obtener una resolución de microsegundos:

| Parámetro | Valor | Descripción Técnica |
| :--- | :--- | :--- |
| **Prescaler (PSC)** | 89 | Divide el reloj de 90MHz (APB1) para obtener 1 MHz (1 tick/μs). |
| **Counter Period (ARR)** | 65535 | Máximo rango para evitar desbordamientos prematuros. |
| **Input Capture** | Direct TI1 | El canal 1 se conecta físicamente al pin de entrada (PA6). |
| **IC Filter** | 4 - 8 | Filtro digital por hardware para eliminar ruido eléctrico en los flancos. |
| **NVIC Interrupt** | Enabled | Habilita la llamada al Callback cada vez que ocurre una captura. |

---

## 🚀 Arquitectura del Sistema

El firmware implementa un desacoplamiento total entre el disparo del sensor, la captura del hardware y la visualización de los datos en el display de 7 segmentos.

## 🧠 Arquitectura: Gestión Asíncrona de Eventos

### 1. Máquina de Estados del Driver (FSM)
El driver gestiona la medición mediante tres estados clave dentro del `CaptureCallback`, cambiando la polaridad del flanco dinámicamente para procesar la señal de *Echo*:

1.  **Estado 0 (Rising):** Se captura el inicio del pulso. Se cambia la polaridad a *Falling Edge* mediante registros.
2.  **Estado 1 (Falling):** Se captura el final del pulso. Se calcula el delta de tiempo (manejando el desbordamiento del Timer).
3.  **Estado 2 (Ready):** El dato está disponible para la aplicación principal y se detiene la interrupción hasta el próximo disparo.



### 2. Base de Tiempo Determinística (DWT)
Para generar el pulso de *Trigger* de $10 \mu s$ exactos, se utiliza el **Data Watchpoint and Trace (DWT)**. A diferencia de `HAL_Delay`, el DWT utiliza el contador de ciclos del núcleo ARM Cortex-M4, permitiendo retardos de alta resolución que no dependen de interrupciones.

```c
void delay_us(uint32_t us) {
    uint32_t startTick = DWT->CYCCNT;
    uint32_t delayTicks = us * (SystemCoreClock / 1000000);
    while (DWT->CYCCNT - startTick < delayTicks);
}
```

### 3. Integración del Display (Multiplexado por TIM2)

Mientras el TIM3 se encarga de la telemetría, el TIM2 genera interrupciones cada 1ms para el barrido de los dígitos del display de 7 segmentos. La convivencia de ambos se logra mediante la configuración de prioridades:

* **Prioridad 0:** SysTick (Base de tiempo del sistema).
* **Prioridad 5:** TIM3 (Captura crítica de datos).
* **Prioridad 7:** TIM2 (Visualización multiplexada).

## 🏁 Conclusión

Este laboratorio consolida el aprendizaje sobre la reactividad del hardware:

1. Se logró una precisión de microsegundos mediante el uso de Input Capture, eliminando el error de latencia del software.
2. El uso de DWT demuestra cómo acceder a registros internos del Cortex-M4 para tareas de temporización crítica que exceden las capacidades de la HAL estándar.
3. La implementación del multiplexado asíncrono garantiza que la interfaz de usuario no degrade el rendimiento de la adquisición de datos.

---
*"Nivel Intermedio: Reactividad de hardware y captura determinística de eventos. El Input Capture es la herramienta fundamental que permite al microcontrolador digitalizar el tiempo, transformando flancos de señales físicas en datos precisos para el análisis de sistemas en tiempo real."*