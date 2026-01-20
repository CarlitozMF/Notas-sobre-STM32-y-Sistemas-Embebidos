# ⚙️ 02_Nivel Intermedio: Sistemas de Tiempo Real y Periféricos

Bienvenido al segundo nivel de mi bitácora de aprendizaje. En esta etapa, el enfoque cambia radicalmente: dejamos de usar el procesador de forma lineal (Polling) para dominar el **Procesamiento Basado en Eventos**.

El objetivo principal es aprender a utilizar el hardware interno del **STM32F439ZI** para que realice tareas de forma autónoma, liberando al CPU para procesos de mayor nivel.

---

## 🚀 Pilares del Nivel Intermedio

### 1. Interrupciones (NVIC & EXTI) ⚡
Es la capacidad del microcontrolador de "pausar" su tarea actual para atender un evento urgente de forma inmediata.
* **Concepto:** Adiós al Polling. El hardware nos avisa cuando ocurre un evento.
* **Aplicaciones:** Botones de emergencia, sensores de velocidad, detección de flancos.



### 2. Temporizadores (Timers) ⏱️
El "corazón" del sistema. Aprenderemos a gestionar el tiempo sin bloquear la ejecución del código.
* **Base de Tiempo:** Generar interrupciones cíclicas (ej: cada 1ms).
* **PWM (Pulse Width Modulation):** Control de intensidad lumínica y servomotores.
* **Encoder Mode:** Lectura de sensores de posición para robótica.



### 3. Protocolos de Comunicación (I2C, SPI, UART) 📬
Estableceremos diálogo con otros dispositivos y sensores externos.
* **UART con Interrupciones:** Recibir datos en segundo plano.
* **I2C:** Comunicación con sensores inerciales (MPU6050) y pantallas OLED.
* **SPI:** Transferencia de datos a alta velocidad.



---

## 🛠️ Roadmap de Proyectos
1. **[01_EXTI_Pulsadores](./01_EXTI_Pulsadores):** Contador de 3 Dígitos con Control EXTI.


---
*Notas creadas durante mi proceso de estudio y experimentación en San Miguel de Tucumán.*