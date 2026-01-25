# ⚙️ Nivel Intermedio: Sistemas de Tiempo Real y Periféricos

Bienvenido a la segunda etapa de mi bitácora de desarrollo. En este nivel, el enfoque cambia radicalmente: abandonamos el flujo lineal para dominar el **Procesamiento Basado en Eventos y Tareas**.

El objetivo principal es explotar el hardware interno del **STM32F439ZI** para lograr autonomía en los periféricos, liberando al CPU para procesos de lógica de alto nivel y toma de decisiones.

---

## 🚀 Pilares del Nivel Intermedio

### 1. Gestión de Eventos (NVIC & EXTI) ⚡
Implementación de interrupciones para lograr una latencia mínima de respuesta ante estímulos externos.
* **Determinismo:** Respuesta inmediata a eventos críticos sin depender del ciclo del `main`.
* **Debouncing:** Técnicas de filtrado de ruido y rebotes tanto por software (timers) como por hardware.
* **Contexto:** Dominio del controlador de interrupciones vectorizado (NVIC).

### 2. Control de Tiempo (Timers) ⏱️
El "corazón" del sistema. Gestión de bases de tiempo precisas para tareas asíncronas y concurrentes.
* **Multiplexación:** Control de periféricos en "background" (como el Driver de 7 Segmentos).
* **PWM (Pulse Width Modulation):** Generación de señales para control de potencia y actuadores.
* **Encoder Mode:** Lectura profesional de sensores de posición y velocidad.

### 3. Comunicación y Middleware 📬
Estandarización del diálogo con el mundo exterior y abstracción de hardware.
* **Arquitectura No Bloqueante:** Uso de interrupciones en UART para evitar cuellos de botella en la transmisión/recepción.
* **Middleware:** Creación de drivers portables, modulares y reutilizables basados en estructuras y punteros.

---

## 🏗️ Arquitectura de Software Aplicada
En esta etapa, los proyectos integradores migran hacia un esquema de **Planificador Cooperativo (Task Scheduler)**:

* **Task Table:** Organización y ejecución de tareas basadas en períodos de tiempo específicos.
* **Estado Volátil:** Gestión segura de memoria compartida entre las ISR (Rutinas de Servicio de Interrupción) y el lazo principal.
* **Encapsulamiento:** Uso de *Handles* y estructuras de datos para un código limpio y escalable.



---

## 🛠️ Roadmap de Laboratorios
1. **[01_EXTI_Pulsadores](./01_EXTI_Pulsadores):** Gestión de flancos y prioridades en el NVIC.
2. **[02_TIM_Basic](./02_TIM_Basic):** Creación de bases de tiempo precisas para tareas cíclicas.
3. **[03_TIM_PWM](./03_TIM_PWM/):** Modulación por Ancho de Pulso (PWM) y Planificador de Tareas


---

## 🚀 Proyectos Integradores (EI)
*Proyectos de síntesis donde se combinan múltiples periféricos bajo una arquitectura robusta.*

### 01. [Contador de Personas Real-Time (Doble Barrera IR)](./Proyectos_Integradores/01_Contador_Displys_7Seg/)
* **Hito:** Implementación exitosa del primer **Scheduler Cooperativo**.
* **Hardware:** Sensores HW-201 + Display 7-Segmentos multiplexado.
* **Técnicas:** EXTI Falling Edge, Timer-IT (181Hz), Software Debounce y Driver con soporte ASCII.

---
*💻 Orquestando el silicio: del código secuencial a la autonomía del hardware.*