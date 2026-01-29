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

## 🏗️ Arquitectura de Software: Scheduler No Bloqueante y Gestión de Tareas

El sistema implementa un esquema de **Multitarea Cooperativa** utilizando el temporizador del sistema (`HAL_GetTick()`). Esta arquitectura permite que el procesador gestione la telemetría, el refresco visual y los efectos del LED sin detener la ejecución de las tareas críticas de tiempo real (como el movimiento del motor).

### Organización de Tareas en el Lazo Principal:
1.  **Tarea de Efectos RGB:** Ejecuta el `RGB_Effects_Handler` para procesar transiciones y parpadeos asíncronos.
2.  **Tarea de Gestión de Eventos (Banderas):** Al activarse `flag_update_display` desde los botones, se reconfigura el estado del LED RGB y el parpadeo del display.
3.  **Tarea de Telemetría UART (Cada 1000ms):** Envía un reporte detallado incluyendo dirección, índice de paso del driver y RPM calculadas.
4.  **Tarea de Refresco Visual (Cada 200ms):** Actualiza el *string* del display 7 segmentos para reflejar el estado actual del motor.

---

## ⚠️ Lecciones Aprendidas: El Rol Crítico de los Callbacks

La robustez de este proyecto reside en la delegación de tareas a las **Rutinas de Servicio de Interrupción (ISR)** mediante los Callbacks de la HAL. Se definieron tres niveles de respuesta asíncrona:

### 1. El Latido del Motor: `HAL_TIM_OC_DelayElapsedCallback` (TIM5)
Es la tarea más crítica. Utiliza la técnica de **Acumulador de Fase**: en cada interrupción, se lee el valor capturado y se reprograma el siguiente evento sumando el `step_delay`. Esto garantiza que los pasos del motor se ejecuten con un determinismo absoluto, independiente de cuánto tarde el código en el `while(1)`.

### 2. El Refresco del Display: `HAL_TIM_PeriodElapsedCallback` (TIM2)
Gestiona el multiplexado de los 3 dígitos. Se configuró para dispararse periódicamente, asegurando que el barrido de los segmentos sea constante y libre de parpadeos (*flicker*), incluso bajo alta carga de telemetría UART.

### 3. La Reactividad del Usuario: `HAL_GPIO_EXTI_Callback` (PB10/PB11)
Implementa la lógica de control de marcha/parada y sentido de giro. 
* **Debouncing por Software:** Se utiliza una guarda de tiempo ($250 \text{ ms}$) para ignorar los rebotes mecánicos de los pulsadores, garantizando transiciones limpias entre estados.

---

## 🗺️ Mapeo de Hardware y Configuración de Pines

La asignación de pines se realizó utilizando etiquetas (*Labels*) en STM32CubeMX para facilitar la legibilidad del código y la portabilidad:

### Control de Movimiento (Motor Stepper)
| Señal | Pin (Nucleo-F439ZI) | Función | Puerto / Canal |
| :--- | :--- | :--- | :--- |
| **Paso Preciso** | **PA0** | TIM5_CH1 | Output Compare (Toggle) |
| **IN1 / IN2** | **PE9 / PE11** | GPIO Out | Secuencia de Bobinas |
| **IN3 / IN4** | **PE13 / PE14**| GPIO Out | Secuencia de Bobinas |

### Interfaz de Usuario y Telemetría
| Periférico | Pin | Etiqueta / Función | Configuración |
| :--- | :--- | :--- | :--- |
| **Botón Start/Stop** | **PB11** | `usr_btn_PS_Pin` | EXTI Line 11 (Pull-up) |
| **Botón Dirección** | **PB10** | `usr_btn_G_Pin` | EXTI Line 10 (Pull-up) |
| **LED RGB** | **PD13-15** | R, G, B | TIM4 (PWM CH2, CH3, CH4) |
| **UART Telemetry** | **PD8 / PD9** | ST-LINK VCP | USART3 (115200 bps) |

### Visualización (Display 7 Segmentos)
* **Segmentos (A-G):** Mapeados en los Puertos E, F y G (Ej: `SEG_A_Pin` en PE10).
* **Habilitadores (EN1-EN3):** Mapeados en el Puerto C (`EN1_Pin` en PC8).

---

## 🏁 Conclusión

El Laboratorio 05 demuestra cómo el modo **Output Compare** transforma al microcontrolador en un generador de eventos físicos de alta precisión. Al separar la lógica de negocio (en el `while`) de la generación de señales críticas (en los Callbacks), se logra un sistema embebido profesional capaz de controlar potencia y tiempo de forma simultánea y determinística.

---
*"Nivel Intermedio: La maestría de los Timers permite que el software gobierne el hardware no solo con lógica, sino con una precisión temporal quirúrgica."*