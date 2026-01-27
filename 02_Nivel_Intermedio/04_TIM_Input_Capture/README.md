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

### 2. Principio de Funcionamiento: El Sensor como Evento Temporal

El sensor **HC-SR04** no entrega un valor de distancia directo, sino una **señal de duración temporal**. El proceso se rige por la siguiente secuencia:
1. El microcontrolador genera un pulso de excitación (*Trigger*) de $10 \mu s$.
2. El transductor emite una ráfaga ultrasónica y eleva su pin **Echo** a estado lógico alto.
3. El pin Echo permanece activo hasta que el receptor detecta el rebote de la señal acústica.

La precisión de la telemetría depende críticamente de la capacidad del sistema para cronometrar el tiempo exacto entre el flanco de subida y el de bajada de dicha señal.

### 3. El Rol del Input Capture: Cronometría por Hardware

En sistemas embebidos de tiempo real, medir este pulso mediante *polling* (lectura constante del pin) introduciría errores aleatorios debido a la latencia de ejecución del CPU y la prioridad de otras tareas concurrentes. Por ello, se utiliza el modo **Input Capture**:

* **Determinismo:** El hardware del Timer "congela" y copia el valor del contador (`CNT`) al registro de captura (`CCR`) en el instante exacto del flanco, sin intervención del procesador.
* **Gestión de Eventos:** Permite al microcontrolador dedicarse a otras tareas (como el refresco del display) y solo atender la medición mediante una interrupción una vez que el evento físico ya ha sido registrado por el silicio.
* **Abstracción por Software:** Mediante la estructura `HCSR04_Handle`, el driver encapsula los tiempos de captura (`rising_time` y `falling_time`) y gestiona una **Máquina de Estados** interna que garantiza la integridad del dato antes de ser procesado por la aplicación.

### 4. Configuración del Periférico (TIM3)

Para la digitalización del pulso, el **TIM3** se configura como un cronómetro de alta resolución con los siguientes parámetros:

| Parámetro | Valor | Justificación Técnica |
| :--- | :--- | :--- |
| **Prescaler (PSC)** | 89 | Divide el reloj de 90MHz (APB1) para obtener una frecuencia de conteo de 1 MHz ($1 \mu s$ por incremento). |
| **Counter Period (ARR)** | 65535 | Maximiza el rango de medición (aprox. 65ms), suficiente para cubrir el alcance máximo del sensor sin desbordamientos. |
| **Input Capture** | Direct TI1 | Asigna el canal 1 directamente al pin físico PA6 mediante el multiplexor de funciones alternativas. |
| **IC Filter** | 4 - 8 | Implementa un filtrado digital por hardware que requiere estabilidad en el flanco durante N ciclos antes de validar la captura, eliminando ruidos eléctricos. |
| **NVIC Interrupt** | Enabled | Permite el procesamiento asíncrono de los datos capturados mediante la rutina de servicio de interrupción (ISR). |

### 5. Base de Tiempo Determinística: Precisión mediante DWT

Para que el sensor inicie la ráfaga ultrasónica, requiere un pulso de excitación (*Trigger*) de exactamente $10 \mu s$. En sistemas de alto rendimiento, el uso de `HAL_Delay()` es inviable debido a su resolución de milisegundos y su dependencia de la interrupción del *SysTick*.

Para resolver esto, se utiliza el periférico **Data Watchpoint and Trace (DWT)** del núcleo ARM Cortex-M4:

* **Acceso a bajo nivel:** El DWT cuenta ciclos de reloj del CPU ($180 \text{ MHz}$ en la Nucleo-F439ZI), permitiendo resoluciones de nanosegundos.
* **Independencia de Interrupciones:** A diferencia de los retardos basados en software, el contador `DWT->CYCCNT` es un registro de hardware que incrementa de forma monótona, garantizando un determinismo absoluto en la generación del pulso de disparo.

```c
/**
 * @brief Retardo de alta precisión en microsegundos.
 * @note Utiliza el contador de ciclos del núcleo (DWT) para evitar bloqueos por interrupción.
 */
void delay_us(uint32_t us) {
    uint32_t startTick = DWT->CYCCNT;
    uint32_t delayTicks = us * (SystemCoreClock / 1000000);
    
    // Bucle de espera basado en ciclos reales de CPU
    while (DWT->CYCCNT - startTick < delayTicks);
}
```
### 7. Orquestación de Hardware: Especialización de Timers

En este sistema, el microcontrolador actúa como un director de orquesta, delegando tareas críticas de tiempo a tres periféricos de hardware independientes. Esta especialización permite que el núcleo Cortex-M4 se libere de tareas repetitivas y se enfoque exclusivamente en la lógica de control y procesamiento de señales.

#### A. TIM4: Modulación de Energía (PWM)
El **TIM4** se encarga de la interfaz visual de advertencia mediante un LED RGB de ánodo común. 
* **Función:** Genera tres señales PWM independientes (Canales 2, 3 y 4) con una frecuencia de $1 \text{ kHz}$ y una resolución de 1000 niveles de Duty Cycle.
* **Ventaja Técnica:** Al ser un periférico de salida asíncrono, una vez que el software define el valor en el registro de comparación (`CCR`), el hardware mantiene la intensidad del LED de forma autónoma, sin consumir ciclos de CPU.

#### B. TIM2: Base de Tiempo para Multiplexado
El **TIM2** funciona como el "latido" de la interfaz de usuario.
* **Función:** Configurado como una base de tiempo pura, genera una interrupción periódica cada $1 \text{ ms}$.
* **Justificación:** Este intervalo es crítico para el barrido de los 3 dígitos del display de 7 segmentos. Al ejecutarse por hardware, garantizamos una tasa de refresco constante que elimina el *flicker* (parpadeo) visual, independientemente de la carga de procesamiento que exista en el lazo principal.



---

### 8. La Importancia de los Callbacks: Arquitectura Reactiva

El éxito de este proyecto reside en el uso correcto de las **Rutinas de Servicio de Interrupción (ISR)** a través de los *Callbacks* de la capa HAL. En el desarrollo de sistemas embebidos profesionales, entender la separación entre el código de "primer plano" (main loop) y el de "segundo plano" (callbacks) es vital para mantener el determinismo.

#### El Rol Crítico de la ISR
Cuando un Timer completa su tarea (ya sea por desbordamiento de tiempo o por captura de un evento físico), el hardware levanta una bandera de interrupción y el CPU salta inmediatamente a ejecutar la función de callback correspondiente:

1. **`HAL_TIM_PeriodElapsedCallback` (TIM2):** Es el motor del display. Cada milisegundo, el micro pausa brevemente su ejecución para conmutar al siguiente dígito. Es una tarea de **alta frecuencia y corta duración**.
2. **`HAL_TIM_IC_CaptureCallback` (TIM3):** Es el núcleo de la telemetría. Solo se ejecuta cuando hay un cambio físico en el pin de Echo. Aquí es donde la **Máquina de Estados** del driver registra los valores de los registros de captura para procesar la información del sensor.



> **Regla de Diseño en Callbacks:** Las funciones dentro de un callback deben ser atómicas y eficientes. En este proyecto, los callbacks no calculan distancias ni envían datos por UART; simplemente registran valores de hardware y actualizan estados. El procesamiento pesado (como el filtro de mediana o el formateo de datos para la telemetría serie) se delega al `while(1)`.

### 9. Lógica de Control y Seguridad del Sistema

El `main.c` integra estas piezas mediante una lógica de seguridad que garantiza la estabilidad del sistema:
* **Feedback de Captura:** Se utiliza un LED de diagnóstico (`usr_ledAzul`) dentro del callback del TIM3. Si el LED conmuta pero no hay datos en la UART, se identifica rápidamente un problema de lógica de software y no un fallo en la etapa de captura física.
* **Gestión de Prioridades:** Mediante el **NVIC**, se asigna una prioridad superior al TIM3 sobre el TIM2. Esto asegura que la medición del sensor (tiempo crítico) nunca sea retrasada por el refresco del display (tarea cosmética), minimizando el *jitter* en la medición.

## 🏁 Conclusión

Este laboratorio consolida el aprendizaje sobre la reactividad del hardware:

1. Se logró una precisión de microsegundos mediante el uso de Input Capture, eliminando el error de latencia del software.
2. El uso de DWT demuestra cómo acceder a registros internos del Cortex-M4 para tareas de temporización crítica que exceden las capacidades de la HAL estándar.
3. La implementación del multiplexado asíncrono garantiza que la interfaz de usuario no degrade el rendimiento de la adquisición de datos.

---
*"Nivel Intermedio: Reactividad de hardware y captura determinística de eventos. El Input Capture es la herramienta fundamental que permite al microcontrolador digitalizar el tiempo, transformando flancos de señales físicas en datos precisos para el análisis de sistemas en tiempo real."*