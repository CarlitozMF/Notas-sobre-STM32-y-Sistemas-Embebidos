# 05_TIM_Output_Compare: Control de Movimiento y Generación de Eventos de Tiempo Real

Este proyecto documenta la implementación del periférico de **Output Compare (OC)** para el control preciso de un motor paso a paso **28BYJ-48**. Se integra una arquitectura de control asíncrona que combina la generación de pulsos determinísticos, modulación **PWM** para retroalimentación cromática y una interfaz visual multiplexada, todo orquestado bajo la plataforma **Nucleo-F439ZI**.

## 🎯 Objetivos
- **Dominar el modo Output Compare** para la generación de señales de temporización precisas e independientes del CPU.
- **Implementar un Driver Modular para Motores PAP** basado en estructuras de estado y tablas de secuencia (Half-Step/Full-Step).
- **Integrar un Sistema HMI Completo:** Control bidireccional mediante interrupciones externas (`EXTI`), visualización en display de 7 segmentos y estados mediante LED RGB.
- **Desarrollar un Scheduler Cooperativo No Bloqueante** en el lazo principal para la gestión de telemetría y lógica de negocio.

---

## 🔩 Teoría de Operación: Output Compare y Generación de Pulsos

### 1. El Concepto de Output Compare (OC)
A diferencia del modo PWM estándar, el **Output Compare** permite disparar una acción (o una interrupción) en el instante exacto en que el contador del Timer (`CNT`) coincide con un valor predefinido en el registro de comparación (`CCR`). 

* **Determinismo:** La generación de la base de tiempos para los pasos del motor no depende de bucles de software (`delay`), eliminando el *jitter* causado por otras tareas.
* **Frecuencia Variable:** Al modificar dinámicamente el valor de comparación, podemos ajustar la velocidad del motor (RPM) con una resolución de microsegundos.

### 2. El Motor 28BYJ-48: Secuenciamiento y Torque
El motor utilizado requiere una secuencia lógica de activación de 4 bobinas (IN1 a IN4). La precisión del movimiento depende de la regularidad de estos pulsos. Para este proyecto se optó por el modo **Half-Step** (secuencia de 8 estados) para lograr un movimiento más fluido y preciso.

### 3. Orquestación de Hardware: Triple Timer Workflow

El sistema delega tareas críticas a tres periféricos independientes:

#### A. TIM5: Generador de Pasos (Output Compare)
El **TIM5** actúa como el "corazón" del movimiento. Genera una interrupción periódica donde el microcontrolador ejecuta el siguiente paso de la tabla de estados. Esto garantiza que el motor mantenga su velocidad constante independientemente de la carga del procesador.

#### B. TIM4: Estado Cromático (PWM)
Gestiona el LED RGB de ánodo común. Utiliza **PWM** con **Corrección Gamma** para que la intensidad percibida sea lineal, asociando colores a los estados: Rojo (STOP), Verde (Giro Horario) y Azul (Giro Antihorario).

#### C. TIM2: Base de Tiempo para Display
Genera una interrupción cada $1 \text{ ms}$ para el multiplexado del display de 7 segmentos, garantizando una imagen estable y sin parpadeos mediante un barrido asíncrono.

---

## 🏗️ Arquitectura de Software: Scheduler No Bloqueante

Se implementó un esquema de **Polling No Bloqueante** basado en `HAL_GetTick()`, permitiendo la ejecución concurrente de múltiples tareas sin utilizar `HAL_Delay()`:

1.  **Tarea de Telemetría (1000ms):** Envío de RPM, dirección e índice de paso actual por UART.
2.  **Tarea de Refresco de Display (200ms):** Actualización de la cadena de caracteres visualizada.
3.  **Tarea de Efectos RGB:** Procesamiento de transiciones y parpadeos del LED de estado.

```c
// Estructura del Scheduler en el main loop
while (1) {
    uint32_t current_tick = HAL_GetTick();

    if (current_tick - last_telemetry_tick >= 1000) {
        last_telemetry_tick = current_tick;
        Update_UART_Telemetry();
    }
    
    if (flag_update_display) {
        Handle_HMI_Events();
    }
}
```

## ⚠️ Lecciones Aprendidas: Sincronía y Hardware

### 1. Alineación de Secuencias (Software vs Hardware)
Se identificó que el motor **28BYJ-48** presenta un comportamiento errático si la tabla de pasos no coincide estrictamente con la disposición física de las bobinas en el driver **ULN2003**. 
* **La Solución:** Radicó en unificar la lógica de bits (Bit 0 = IN1) y ajustar la secuencia de la tabla de pasos basándose en pruebas de torque real y validación de hardware previa.



### 2. Gestión de Prioridades en el NVIC
Para garantizar un movimiento suave y evitar que el motor pierda pasos por latencia de CPU, se configuró la jerarquía de interrupciones de la siguiente manera:

| Prioridad | Periférico / Evento | Función | Nota Técnica |
| :--- | :--- | :--- | :--- |
| **0 (Máxima)** | **TIM5** | Pasos del Motor | Tiempo Crítico (Determinismo) |
| **1** | **EXTI** | Botones | Respuesta de Usuario (Reactividad) |
| **2 (Mínima)** | **TIM2** | Display | Tarea Cosmética (Multiplexado) |

---

## 🗺️ Mapeo de Hardware: Nucleo-F439ZI

| Periférico | Pin | Función | Nota Técnica |
| :--- | :--- | :--- | :--- |
| **TIM5_CH1** | **PA0** | **Motor Step** | Output Compare Mode |
| **GPIO Out** | **PB0, PB7, PB14**| **Motor IN1-IN4** | Secuencia de Potencia |
| **TIM4_CH2-4**| **PD13-15** | **LED RGB** | PWM (Gamma Corrected) |
| **TIM2** | **Interno** | **Multiplexado** | Base de tiempo 1ms |
| **EXTI10** | **PB10** | **Botón Giro** | Interrupción por Flanco |
| **EXTI11** | **PB11** | **Botón Start** | Interrupción por Flanco |

---

## 🏁 Conclusión

Este laboratorio consolida la capacidad de generar eventos precisos mediante hardware. El uso de **Output Compare** permite abstraer el control de movimiento de la lógica de aplicación, demostrando que un microcontrolador de alto rendimiento como el **STM32** puede gestionar múltiples lazos de control de tiempo real (Motor, Display, PWM y UART) de forma simultánea y eficiente.

> *"Nivel Intermedio: Generación de eventos y control de potencia. El Output Compare transforma el conteo de ciclos en acciones físicas, permitiendo que el software gobierne el movimiento con precisión quirúrgica."*