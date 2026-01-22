# 09_Intro_MEF: Máquinas de Estado y Monitoreo Serial 🚀

Este proyecto marca un hito fundamental en el **Nivel Básico**. Aquí evolucionamos de la programación secuencial simple a una arquitectura basada en **Máquinas de Estado Finita (FSM/MEF)**, integrando por primera vez la comunicación serie (UART) para depuración y diagnóstico del sistema.

## 🧠 El Corazón del Diseño: Máquina de Estados Finita (MEF)

En este proyecto, la lógica de control se basa en el modelo de **Máquina de Moore**, donde las salidas dependen únicamente del estado actual. Implementar una MEF en sistemas embebidos ofrece ventajas críticas:

* **Determinismo:** El sistema siempre se encuentra en un estado conocido y responde de manera predecible a las entradas.
* **Escalabilidad:** Es mucho más sencillo agregar nuevos modos de operación añadiendo un `case` adicional que intentando anidar múltiples estructuras `if-else`.
* **Mantenimiento:** Facilita la depuración, ya que si el sistema falla en una acción específica, sabemos exactamente en qué bloque del `switch-case` se encuentra el error.

### Modos de Operación:
1.  **ESTADO_APAGADO**: El LED se mantiene en reposo.
2.  **ESTADO_ENCENDIDO**: El LED emite luz constante.
3.  **ESTADO_PARPADEO**: El LED oscila de forma asíncrona (usando `HAL_GetTick()`).

## 🏗️ Implementación y Hardware

### 1. Interfaz de Usuario (PB11)
Se utiliza un pulsador externo conectado al pin **PB11** y a **GND**. 
* **Lógica Negativa**: El pin está configurado con **Pull-Up interna**, por lo que el evento se dispara cuando el micro lee un `0` lógico.
* **Detección de Flanco**: Se implementó una lógica de "flag" para asegurar que la transición de estado ocurra solo una vez por pulsación, evitando que el sistema salte de estados infinitamente mientras se mantiene el botón presionado.

### 2. Diagnóstico por UART
Se implementó una función personalizada `Debug_Log` para enviar mensajes de estado a una terminal serial (**115200 baudios**).
* **Log de Inicio**: Mensaje de bienvenida que confirma que el Clock Tree y la UART están bien configurados.
* **Log de Transición**: El sistema informa cada vez que ocurre un cambio de estado.

💻 Fragmento Clave: La Estructura de la MEF

El siguiente código es el núcleo del proyecto. Observa cómo el switch-case permite que el microcontrolador tenga "memoria" de su estado actual:

```c
/* 1. LÓGICA DE TRANSICIÓN: Decidimos el próximo estado basado en la entrada */
if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_11) == GPIO_PIN_RESET) {
    if (!botonPresionado) {
        botonPresionado = true; 
        
        switch (estadoActual) {
            case ESTADO_APAGADO:   estadoActual = ESTADO_ENCENDIDO; break;
            case ESTADO_ENCENDIDO: estadoActual = ESTADO_PARPADEO;  break;
            case ESTADO_PARPADEO:  estadoActual = ESTADO_APAGADO;   break;
        }
        // El log se dispara solo en la transición, evitando saturar la UART
        Debug_Log("FSM: Cambio de estado detectado\r\n");
    }
} else {
    botonPresionado = false; // Reset del flag (detección de flanco)
}

/* 2. LÓGICA DE ESTADO: Ejecutamos la acción correspondiente al estado actual */
switch (estadoActual) {
    case ESTADO_APAGADO:
        HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET);
        break;
        
    case ESTADO_ENCENDIDO:
        HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);
        break;
        
    case ESTADO_PARPADEO:
        // Acción temporal no bloqueante
        if (HAL_GetTick() - lastTick >= 200) {
            HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin);
            lastTick = HAL_GetTick();
        }
        break;
}
```

## 🛠️ Configuración Técnica
- **Microcontrolador**: STM32F439ZI (Nucleo-144).
- **Periféricos**: GPIO (Salida para LED, Entrada para Botón) y UART3 (ST-Link Virtual COM Port).
- **Herramientas**: STM32CubeIDE.

## ⚠️ Observación de Ingeniería: El Problema del Bloqueo
Durante las pruebas, se nota un pequeño retraso en la reacción del LED tras presionar el botón. 

**Análisis del Cuello de Botella:**
La función `Debug_Log` utiliza `HAL_UART_Transmit`, la cual es una función **bloqueante**. El procesador detiene toda ejecución (incluyendo la actualización del LED) hasta terminar de enviar cada carácter por el cable. Este fenómeno justifica la necesidad de evolucionar hacia **Drivers No Bloqueantes**, tema central del siguiente módulo en este repositorio.

---

### 📂 Cómo utilizar este ejemplo
1. Conecte un pulsador entre **PB11** y **GND**.
2. Abra un monitor serial (TeraTerm, PuTTY o el monitor de STM32CubeIDE) a **115200 bps**.
3. Presione el botón y observe la consola para verificar las transiciones de la MEF.

---
*En este proyecto nos introducimos al diseño de Máquinas de Estado Finita (MEF) para dar orden a la lógica y utilizamos la UART para monitorear el sistema. Sin embargo, notarás que al enviar mensajes largos, el LED "se congela" un instante. Esto sucede porque la función Debug_Log es bloqueante; mientras el CPU transmite datos, no puede procesar nada más. En el próximo nivel, aprenderemos a crear Drivers No Bloqueantes para que la comunicación y la lógica fluyan en paralelo.*