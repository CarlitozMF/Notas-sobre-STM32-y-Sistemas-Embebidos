# 06_TIM_PWM_Advance: Control de Posición, Interpolación de Velocidad y HMI Multimodal

Este laboratorio documenta la implementación avanzada de señales **PWM (Pulse Width Modulation)** para el control de posición de un servomotor **SG90**. El proyecto integra una arquitectura orientada a eventos que combina el protocolo de comunicación PPM, el procesamiento de cuadratura mediante **EXTI**, y una interfaz HMI compuesta por un **LED RGB** (PWM) y un **Display 7 Segmentos** multiplexado, todo coordinado bajo la plataforma **Nucleo-F439ZI**.

## 🎯 Objetivos
- **Dominar el Protocolo PPM:** Configurar Timers para generar señales de 50Hz con resolución de microsegundos para el control de servos analógicos.
- **Implementar Interpolación de Velocidad:** Desarrollar un algoritmo de movimiento suave (Slew Rate Control) para evitar picos de corriente y estrés mecánico.
- **Procesamiento de Cuadratura por EXTI:** Decodificar señales de un encoder rotativo (**KY-040**) mediante interrupciones externas para un control de usuario fluido.
- **Gestión de HMI Dinámica:** Mapear variables físicas (ángulo) a variables visuales (color RGB y dígitos) de forma asíncrona.

---

## 🔩 Teoría de Operación: PWM de Posición vs. PWM de Potencia

### 1. El Protocolo de Control de Servos (PPM)
A diferencia de un PWM para un LED, donde el ciclo de trabajo varía de 0% a 100%, el servomotor SG90 utiliza el ancho del pulso como un **mensaje de posición**. 

* **Determinismo Temporal:** La señal debe ser estrictamente de **50 Hz** (20ms de periodo). Una desviación en esta frecuencia causa inestabilidad y vibraciones (jitter) en el actuador.
* **Resolución de Ángulo:** El rango útil se limita a pulsos entre **0.5ms (0°)** y **2.5ms (180°)**. En este proyecto, se configuró el Timer para obtener una resolución de **1 μs por paso**, permitiendo un control sub-grado.

### 2. Interpolación y Control de Velocidad
Para evitar que el servo intente alcanzar el ángulo objetivo de forma instantánea (causando golpes mecánicos), se implementó un driver que calcula pasos intermedios:
* **Target vs. Current:** El sistema diferencia entre la posición deseada (marcada por el encoder) y la posición actual del eje.
* **Update Rate:** En cada iteración del lazo principal, el ángulo actual se acerca al objetivo según una tasa de velocidad definida en grados por segundo.

---

## 🏗️ Orquestación de Hardware: Triple Timer Workflow

El sistema delega las tareas críticas a tres periféricos independientes para garantizar que el núcleo Cortex-M4 se enfoque en la lógica de control:

#### A. TIM3: Generador de Posición (PWM Avanzado)
Es el encargado de generar la señal de control para el servo. Se configuró para maximizar la resolución en el rango útil de 1ms a 2ms.



**Configuración Técnica:**
| Parámetro | Valor | Justificación Técnica |
| :--- | :--- | :--- |
| **Prescaler (PSC)** | 89 | Divide el reloj de 90MHz (APB1) para obtener **1 tick = 1 μs**. |
| **Period (ARR)** | 19999 | Define el periodo exacto de **20ms (50Hz)** requerido por el servo. |
| **Pulse (CCR)** | 520 - 2540 | Mapea el rango de 0.5ms a 2.5ms para cubrir los 180° de giro. |

#### B. TIM4: Feedback Cromático (PWM)
Controla el LED RGB mediante PWM de alta frecuencia ($1 \text{ kHz}$), permitiendo una mezcla de colores suave que indica la zona de trabajo del servo.

* **Lógica por Segmentos:** El color cambia dinámicamente:

    * **Rojo:** Posición mínima (0°).
    * **Verde:** Zona central (90°).
    * **Azul:** Posición máxima (180°).

#### C. TIM2: Multiplexado de Display
Genera una interrupción pura cada **2 ms** para el refresco del display de 7 segmentos. Al usar interrupciones, el brillo y la estabilidad del display son independientes de la carga de trabajo del procesador.

---

---

## 🏗️ Arquitectura de Software: Flujo de Datos y Concurrencia

El sistema opera bajo un esquema de **Multitarea Cooperativa** y **Gestión por Interrupciones**, eliminando el uso de retardos bloqueantes (`HAL_Delay`). La arquitectura se divide en tres dominios que interactúan de forma asíncrona:

1. **Dominio de Entrada (Eventos de Usuario):** El Encoder KY-040 genera interrupciones externas (**EXTI**) que son procesadas de inmediato. El software no "espera" al usuario; el hardware le avisa al procesador que la perilla se ha movido.
   - **Captura:** El Callback de EXTI detecta el flanco y actualiza la posición lógica.
   - **Acción de Reset:** Al presionar el switch del encoder, se dispara un evento que restablece el objetivo a 90°.

2. **Dominio de Procesamiento (Lógica de Control):** En el lazo principal (`while(1)`), el sistema actúa como un **Sincronizador de Estados**:
   - **Generación de Trayectoria:** El driver del servo calcula el siguiente paso basado en una velocidad de 150°/s (**Slew Rate**).
   - **Filtro de Actualización:** El display y el log de UART solo se refrescan si el ángulo entero ha cambiado (`current_angle != last_angle`).

3. **Dominio de Salida (Periféricos de Hardware):** El hardware gestiona las tareas de alta frecuencia mediante Timers:
   - **PWM de Posición (TIM3):** Señal constante de 50Hz para el servo.
   - **PWM Cromático (TIM4):** Mezcla de colores en el LED RGB según el ángulo.
   - **Multiplexado (TIM2):** Refresco secuencial de los 3 dígitos cada 2ms.

---

## 🔍 Análisis de Implementación: Puntos Críticos del `main.c`

### 1. El Paradigma de "Control por Eventos" (EXTI vs Polling)
El sistema optimiza los ciclos de CPU al no preguntar constantemente por el estado del encoder. 
- **Prioridad NVIC:** Se asignó una prioridad estratégica al `EXTI9_5_IRQn` para que el conteo de pasos sea preferente frente a tareas cosméticas.
- **Abstracción en Callback:** La lógica de decodificación se delega al `KY040_IRQ_Handler`, manteniendo el `main.c` limpio de lógica de bajo nivel.

### 2. Sincronización de Dominios: Encoder vs. Servo
Se implementa un **desacople de objetivos** para evitar movimientos bruscos que dañarían la mecánica:
- El **Encoder** define el `target_from_encoder` (intención del usuario).
- El **Driver del Servo** gestiona la `current_angle` (realidad física).
- **Interpolación Lineal:** La función `SERVO_SG90_SetSpeedAngle` permite que el servo "viaje" hacia el objetivo suavemente, imitando el comportamiento de sistemas industriales.

### 3. Máquina de Estados para Feedback Visual
Para optimizar el bus de datos y reducir el ruido electromagnético, la función `UI_Update_Feedback` utiliza una **guarda de estado**:

```c
if (current_led_state != last_led_state) {
    // Solo se actualiza el PWM del LED si el estado de color realmente cambió.
    // Esto evita re-escrituras innecesarias en los registros CCR del Timer.
}
```
Aquí tenés todo integrado en un solo bloque de Markdown profesional, listo para que lo copies y pegues en tu README.md. He unificado los conceptos para que la narrativa pase de la arquitectura general a los puntos críticos del código.
Markdown

---

## 🏗️ Arquitectura de Software: Flujo de Datos y Concurrencia

El sistema opera bajo un esquema de **Multitarea Cooperativa** y **Gestión por Interrupciones**, eliminando el uso de retardos bloqueantes (`HAL_Delay`). La arquitectura se divide en tres dominios que interactúan de forma asíncrona:

1. **Dominio de Entrada (Eventos de Usuario):** El Encoder KY-040 genera interrupciones externas (**EXTI**) que son procesadas de inmediato. El software no "espera" al usuario; el hardware le avisa al procesador que la perilla se ha movido.
   - **Captura:** El Callback de EXTI detecta el flanco y actualiza la posición lógica.
   - **Acción de Reset:** Al presionar el switch del encoder, se dispara un evento que restablece el objetivo a 90°.

2. **Dominio de Procesamiento (Lógica de Control):** En el lazo principal (`while(1)`), el sistema actúa como un **Sincronizador de Estados**:
   - **Generación de Trayectoria:** El driver del servo calcula el siguiente paso basado en una velocidad de 150°/s (**Slew Rate**).
   - **Filtro de Actualización:** El display y el log de UART solo se refrescan si el ángulo entero ha cambiado (`current_angle != last_angle`).

3. **Dominio de Salida (Periféricos de Hardware):** El hardware gestiona las tareas de alta frecuencia mediante Timers:
   - **PWM de Posición (TIM3):** Señal constante de 50Hz para el servo.
   - **PWM Cromático (TIM4):** Mezcla de colores en el LED RGB según el ángulo.
   - **Multiplexado (TIM2):** Refresco secuencial de los 3 dígitos cada 2ms.

---

## 🔍 Análisis de Implementación: Puntos Críticos del `main.c`

### 1. El Paradigma de "Control por Eventos" (EXTI vs Polling)
El sistema optimiza los ciclos de CPU al no preguntar constantemente por el estado del encoder. 
- **Prioridad NVIC:** Se asignó una prioridad estratégica al `EXTI9_5_IRQn` para que el conteo de pasos sea preferente frente a tareas cosméticas.
- **Abstracción en Callback:** La lógica de decodificación se delega al `KY040_IRQ_Handler`, manteniendo el `main.c` limpio de lógica de bajo nivel.



### 2. Sincronización de Dominios: Encoder vs. Servo
Se implementa un **desacople de objetivos** para evitar movimientos bruscos que dañarían la mecánica:
- El **Encoder** define el `target_from_encoder` (intención del usuario).
- El **Driver del Servo** gestiona la `current_angle` (realidad física).
- **Interpolación Lineal:** La función `SERVO_SG90_SetSpeedAngle` permite que el servo "viaje" hacia el objetivo suavemente, imitando el comportamiento de sistemas industriales.

### 3. Máquina de Estados para Feedback Visual
Para optimizar el bus de datos y reducir el ruido electromagnético, la función `UI_Update_Feedback` utiliza una **guarda de estado**:

```c
if (current_led_state != last_led_state) {
    // Solo se actualiza el PWM del LED si el estado de color realmente cambió.
    // Esto evita re-escrituras innecesarias en los registros CCR del Timer.
}
```

### 4. Multiplexado de Display por Interrupción de Hardware

El refresco del display de 7 segmentos está orquestado por el **TIM2**, garantizando una **Persistencia de Visión** perfecta.

* **Refresco Asíncrono:** La función `Display7Seg_Refresh_ISR` se ejecuta en el background cada 2ms.
* **Determinismo:** Los números se mantienen estables y brillantes sin importar la carga de procesamiento o la latencia de los logs UART en el lazo principal.

---

## 🔄 Diagrama de Flujo del Sistema

graph TD
    A[Encoder KY-040] -- EXTI Interrupción --> B(Actualizar Target Angle)
    B --> C{Lazo Principal}
    C --> D[Driver Servo: Interpolación de Velocidad]
    D -- PWM TIM3 --> E[Actuador: Servo SG90]
    D --> F{¿Cambió el Ángulo?}
    F -- SI --> G[Actualizar Display 7-Seg]
    F -- SI --> H[Actualizar Color LED RGB]
    F -- SI --> I[Log por UART3]
    F -- NO --> C
    J[TIM2 Interrupción] -- Cada 2ms --> K[Refresco Físico Display]

---

## 🗺️ Mapeo de Hardware y Configuración de Pines

La asignación de recursos se ha diseñado para evitar conflictos entre los canales de los Timers y las líneas de interrupción externa, aprovechando el layout de la **Nucleo-F439ZI**:

| Periférico | Pin | Etiqueta | Función / Justificación Técnica |
| :--- | :--- | :--- | :--- |
| **TIM3_CH2** | **PB5** | `SERVO_PWM` | **Control de Posición:** Salida PWM con resolución de 1μs. |
| **TIM4_CH2-4**| **PD13-15** | `RGB_R/G/B` | **HMI Cromática:** Control de LED RGB mediante PWM de 1kHz. |
| **EXTI 9_5** | **PF12** | `ENC_CLK` | **Reloj Encoder:** Dispara la IT para el conteo de pasos. |
| **GPIO Input** | **PF13** | `ENC_DT` | **Sentido Encoder:** Determina dirección (CW/CCW). |
| **TIM2** | **Interno** | `DISP_REFRESH` | **Base de Tiempo:** Interrupción de 2ms para multiplexado. |
| **UART3** | **PD8/PD9** | `STLINK_VCP` | **Telemetría:** Debug Log a 115200 bps vía USB. |



### Notas de Conexión:
* **Servo SG90:** Se alimenta desde los 5V **externos**, pero la señal PWM es de 3.3V (compatible con el driver interno del servo).
* **Encoder KY-040:** Requiere resistencias de *Pull-Up* activas (configuradas internamente en el STM32) debido a que es un dispositivo de colector abierto.
* **Display 7 Segmentos:** Los pines de segmentos utilizan resistencias limitadoras de corriente para proteger los GPIOs del puerto E, F y G.

## 🏁 Conclusión

El **Laboratorio 06** demuestra la versatilidad del periférico Timer para ir más allá de la simple generación de señales, convirtiéndose en un protocolo de comunicación (PPM). La integración de algoritmos de interpolación y decodificación por interrupciones eleva el proyecto a un nivel de control industrial, donde la suavidad del movimiento y la respuesta del sistema son prioritarias.

---

*"Nivel Intermedio: Superar los retardos por software es el primer paso hacia la ingeniería; aquí el PWM deja de ser una señal de potencia para ser un protocolo de control determinístico."*