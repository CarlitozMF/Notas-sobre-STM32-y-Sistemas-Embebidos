# 09_Intro_MEF: Máquinas de Estado y Diagnóstico Serial 🚀

Este proyecto marca un hito fundamental en el **Nivel Básico**. Aquí evolucionamos de la programación secuencial a una arquitectura basada en **Máquinas de Estado Finitas (FSM/MEF)**, integrando la comunicación serie (UART) para depuración y diagnóstico del sistema en tiempo real.

## 🧠 El Corazón del Diseño: Máquina de Moore

La lógica de control se basa en el modelo de **Máquina de Moore**, donde las salidas del sistema dependen únicamente del estado actual. Implementar una MEF en sistemas embebidos ofrece ventajas críticas para la ingeniería:

* **Determinismo:** El sistema siempre se encuentra en un estado conocido y responde de manera predecible a los eventos.
* **Escalabilidad:** Agregar nuevos modos de operación es tan simple como añadir un `case` al bloque principal, sin romper la lógica existente.
* **Mantenimiento:** Facilita la depuración al aislar comportamientos. Si el sistema falla en una acción, sabemos exactamente en qué bloque de la MEF buscar el error.

### Estados del Sistema:
1.  **`ESTADO_APAGADO`**: Reposo absoluto del actuador.
2.  **`ESTADO_ENCENDIDO`**: Salida constante a nivel alto.
3.  **`ESTADO_PARPADEO`**: Oscilación asíncrona controlada por tiempo no bloqueante (`HAL_GetTick()`).

---

## 🏗️ Implementación de Hardware y Telemetría

### 1. Interfaz de Usuario (HMI)
Se utiliza un pulsador externo conectado al pin **PB11** con configuración de **Pull-Up interna**.
* **Lógica Negativa:** El evento de presión se detecta con un `0` lógico (GND).
* **Detección de Flanco (Edge Detection):** Se implementó una lógica de "flag" para asegurar que la transición ocurra una sola vez por pulsación, evitando el salto infinito de estados mientras se mantiene el botón presionado.

### 2. Diagnóstico por UART (Universal Asynchronous Receiver-Transmitter)
Se implementó la función personalizada `Debug_Log` para telemetría a **115200 baudios** a través de la **UART3** (ST-Link Virtual COM Port).
* **Logs de Transición:** El sistema informa por consola cada vez que el motor de estados realiza un cambio de fase.

---

## 💻 Arquitectura del Código: El "Switch-Case" Motor

El núcleo del firmware se divide en dos secciones: **Lógica de Transición** (cuándo cambiar) y **Lógica de Estado** (qué hacer).

```c
/* 1. LÓGICA DE TRANSICIÓN: Basada en detección de flanco descendente */
if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_11) == GPIO_PIN_RESET) {
    if (!botonPresionado) {
        botonPresionado = true; 
        
        switch (estadoActual) {
            case ESTADO_APAGADO:   estadoActual = ESTADO_ENCENDIDO; break;
            case ESTADO_ENCENDIDO: estadoActual = ESTADO_PARPADEO;  break;
            case ESTADO_PARPADEO:  estadoActual = ESTADO_APAGADO;   break;
        }
        Debug_Log("FSM: Transicion a nuevo estado...\r\n");
    }
} else {
    botonPresionado = false; // Reset del flag para la próxima pulsación
}

/* 2. LÓGICA DE ESTADO: Ejecución según el estado actual de la MEF */
switch (estadoActual) {
    case ESTADO_APAGADO:
        HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET);
        break;
        
    case ESTADO_ENCENDIDO:
        HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);
        break;
        
    case ESTADO_PARPADEO:
        // Temporización no bloqueante: la MEF sigue "viva" mientras espera
        if (HAL_GetTick() - lastTick >= 200) {
            HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin);
            lastTick = HAL_GetTick();
        }
        break;
}
```

## ⚠️ Análisis sobre el Costo del Bloqueo

Durante la ejecución, se observa que al enviar mensajes largos por UART, el parpadeo del LED se **"congela"** momentáneamente.

**Diagnóstico:** La función *Debug_Log* utiliza **HAL_UART_Transmit** en modo **Polling**, la cual es **bloqueante**. Mientras el CPU está ocupado enviando caracteres bit a bit por el cable, no puede evaluar la condición de tiempo del parpadeo ni leer el botón.

* - Hacia el Nivel Intermedio: Este problema justifica la evolución hacia el uso de Interrupciones (IT) o DMA (Direct Memory Access) para que la comunicación serie ocurra en segundo plano sin afectar la latencia de la Máquina de Estados.

---
*Una Máquina de Estados bien diseñada es la base de un sistema embebido determinista; el manejo de la telemetría es lo que nos permite certificar su correcto funcionamiento.*