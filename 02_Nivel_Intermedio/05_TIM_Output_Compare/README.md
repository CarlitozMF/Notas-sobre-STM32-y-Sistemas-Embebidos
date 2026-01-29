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

El sistema delega tareas críticas a tres periféricos independientes, permitiendo que el núcleo Cortex-M4 se libere de procesos repetitivos y garantice el determinismo temporal:

#### A. TIM5: Generador de Pasos (Output Compare)
El **TIM5** actúa como el "corazón" del movimiento. A diferencia de un PWM convencional, aquí utilizamos el modo **Output Compare (OC)** para generar una base de tiempo elástica y ultra-precisa.

* **El Pin "Fantasma":** Aunque el canal 1 (`TIM5_CH1`) está configurado en el microcontrolador, no se utiliza para conmutar un pin físico de salida. Funciona exclusivamente como una **alarma interna** de alta resolución.
* **Determinismo por Software:** Cada vez que el contador del Timer alcanza el valor de comparación, el hardware dispara de forma asíncrona la interrupción `HAL_TIM_OC_DelayElapsedCallback`. Es allí donde se ejecutan los pasos lógicos del motor.
* **Técnica de Acumulador de Fase:** Para eliminar errores de redondeo y jitter, el siguiente evento de comparación se programa sumando el `step_delay` al valor de captura actual, asegurando una velocidad constante.

```c
/**
 * @brief Lógica interna en el Callback de TIM5 (Output Compare)
 * Genera el pulso de temporización para el motor sin bloquear el CPU.
 */
void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef *htim) {
    if (htim->Instance == TIM5) {
        // 1. Reprogramación del evento de comparación (Fase de acumulador)
        uint32_t current_capture = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
        __HAL_TIM_SET_COMPARE(htim, TIM_CHANNEL_1, current_capture + step_delay);

        // 2. Ejecución del paso físico si el motor está habilitado
        if (current_state == MOTOR_RUNNING) {
            Stepper_Step_Sequential(&motor1); 
        }
    }
}
```

#### B. TIM4: Estado Cromático (PWM)
Gestiona el LED RGB de ánodo común mediante tres canales independientes, transformando estados lógicos en una interfaz visual intuitiva:

* **Modulación de Ancho de Pulso:** Genera señales de $1 \text{ kHz}$ para controlar la intensidad de cada componente (R, G, B) con una resolución de 1000 niveles.
* **Corrección Gamma:** Se implementa un mapeo logarítmico para que la transición de colores y el brillo sean percibidos de forma lineal por el ojo humano, compensando la respuesta no lineal de la visión.
* **Estados Visuales Definidos:** * **Rojo:** Sistema en `STOP` (Seguridad).
    * **Verde:** Giro horario (`CW`).
    * **Azul:** Giro antihorario (`CCW`).



#### C. TIM2: Base de Tiempo para Display
Funciona como el "latido" de la interfaz visual. Genera una interrupción pura por desbordamiento (Update Event) cada **1 ms**.

* **Multiplexado Asíncrono:** En cada interrupción, la ISR conmuta el cátodo común (o ánodo) correspondiente y actualiza los segmentos del siguiente dígito. Este proceso se delega al hardware para liberar al `while(1)`.
* **Persistencia de Visión:** Al ejecutarse a una frecuencia de refresco efectiva de $1 \text{ kHz}$, se garantiza una imagen estable y libre de parpadeos (*flicker*). 
* **Independencia de Tareas:** La calidad visual del display es totalmente inmune a la velocidad del motor, a la carga de la telemetría UART o a la latencia de otros procesos del sistema.



```c
/**
 * @brief Callback de refresco del Display (TIM2)
 * Se ejecuta cada 1ms para garantizar la persistencia de visión.
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
    if (htim->Instance == TIM2) {
        // Barrido del siguiente dígito en el display
        Display7Seg_Refresh_ISR(&Display);
    }
}
```

### 4. El Misterio del Pin "Fantasma" (OC sin Pin Físico)
Una duda común es por qué el pin **PA0 (TIM5_CH1)** no está conectado físicamente al motor. La respuesta reside en la arquitectura de interrupciones del STM32:

* **El Timer como Alarma:** En este proyecto, el periférico Output Compare no se usa para mover un pin externo, sino como un **despertador de alta precisión**. 
* **Evento Interno:** Cada vez que el contador del Timer alcanza el valor de comparación (`CCR`), se genera un evento interno que dispara una interrupción (`IT`).
* **Acción por Software:** En lugar de que el hardware mueva un pin, el CPU salta a la función `HAL_TIM_OC_DelayElapsedCallback`. Es allí donde nuestro código ejecuta la lógica del motor y conmuta los pines reales (**IN1 a IN4**).

**Ventaja Técnica:** Esto nos permite desacoplar la base de tiempo (el "cuándo" se da el paso) de la lógica de potencia (el "cómo" se activan las bobinas), permitiendo que el motor gire con una precisión de microsegundos sin bloquear el resto del sistema.

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

```c
/**
 * @brief Callback de comparación de salida (Output Compare) del Timer.
 * @details Responsable de la generación de pasos del motor PAP. Si el sistema
 * está en RUNNING, ejecuta un paso. Independientemente del estado,
 * reprograma la próxima comparación para mantener la base de tiempo
 * constante y evitar latencias al arrancar.
 * @param htim Puntero a la estructura del Timer que generó el evento.
 */
void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef *htim) {
	if (htim->Instance == TIM5) {
		if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1) {

			/* 1. Generación del paso lógico si el motor está habilitado */
			if (current_state == MOTOR_RUNNING) {
				Stepper_Step_Sequential(&motor1);
			}

			/* 2. Reprogramación del evento de comparación (Fase de acumulador) */
			uint32_t current_capture = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
			__HAL_TIM_SET_COMPARE(htim, TIM_CHANNEL_1, current_capture + step_delay);
		}
	}
}
```

### 2. El Refresco del Display: `HAL_TIM_PeriodElapsedCallback` (TIM2)
Gestiona el multiplexado de los 3 dígitos. Se configuró para dispararse periódicamente, asegurando que el barrido de los segmentos sea constante y libre de parpadeos (*flicker*), incluso bajo alta carga de telemetría UART.

```c
/**
 * @brief Callback del Timer para la multiplexación del Display.
 * @note Se ejecuta periódicamente (recomendado cada 2-5ms) para refrescar
 * un dígito a la vez del display de 7 segmentos.
 * @param htim Puntero a la estructura del Timer que generó la interrupción.
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
	if (htim->Instance == TIM2) {
		Display7Seg_Refresh_ISR(&Display);
	}
}
```

### 3. La Reactividad del Usuario: `HAL_GPIO_EXTI_Callback` (PB10/PB11)
Implementa la lógica de control de marcha/parada y sentido de giro. 
* **Debouncing por Software:** Se utiliza una guarda de tiempo ($250 \text{ ms}$) para ignorar los rebotes mecánicos de los pulsadores, garantizando transiciones limpias entre estados.

```c
/**
 * @brief Callback de Interrupción Externa para pines GPIO.
 * @details Gestiona el pulsador en PB10 y PB11 (lógica negativa) para alternar giros y entre
 * los estados de marcha y parada del motor (Toggle). Incluye un
 * mecanismo de debouncing por software.
 * @param GPIO_Pin Pin que disparó la interrupción.
 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
	uint32_t interrupt_time = HAL_GetTick();
	static uint32_t last_btn_start_time = 0;
	static uint32_t last_btn_dir_time = 0;

	/* --- Botón START/STOP (PB11) --- */
	if (GPIO_Pin == usr_btn_PS_Pin) {
		if (interrupt_time - last_btn_start_time > 250) {
			if (current_state == MOTOR_STOPPED) {
				current_state = MOTOR_RUNNING;
			} else {
				current_state = MOTOR_STOPPED;
				Stepper_Stop(&motor1);
			}
			flag_update_display = 1;
			last_btn_start_time = interrupt_time;
		}
	}

	/* --- Botón CAMBIO DE SENTIDO (PB10) --- */
	if (GPIO_Pin == usr_btn_G_Pin) {
			if (interrupt_time - last_btn_dir_time > 250) {
				// Invertimos el sentido para que coincida con el display
				if (motor1.direction == STEP_CW) {
					Stepper_Set_Direction(&motor1, STEP_CCW); // Ahora dirá 'r' y girará CCW
				} else {
					Stepper_Set_Direction(&motor1, STEP_CW);  // Ahora dirá 'C' y girará CW
				}
				flag_update_display = 1;
				last_btn_dir_time = interrupt_time;
			}
		}
}
```

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
* **Segmentos (A-G):** Mapeados en los Puertos E, F y G (Ej: `SEG_A` en PE10).
* **Habilitadores (EN1-EN3):** Mapeados en el Puerto C (`EN1_Pin` en PC8).

---

## 🏁 Conclusión

El Laboratorio 05 demuestra cómo el modo **Output Compare** transforma al microcontrolador en un generador de eventos físicos de alta precisión. Al separar la lógica de negocio (en el `while`) de la generación de señales críticas (en los Callbacks), se logra un sistema embebido profesional capaz de controlar potencia y tiempo de forma simultánea y determinística.

---
*"Nivel Intermedio: La maestría de los Timers permite que el software gobierne el hardware no solo con lógica, sino con una precisión temporal quirúrgica."*