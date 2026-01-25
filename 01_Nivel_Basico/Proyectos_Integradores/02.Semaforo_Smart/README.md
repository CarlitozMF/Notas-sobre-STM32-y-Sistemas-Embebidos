# EI_2: Semáforo Automático con Arquitectura FSM y Temporizador Visual 🚦

Este proyecto integrador del **Nivel Básico** demuestra el control de un sistema secuencial crítico mediante una **Máquina de Estados Finitos (FSM)**. Se coordina la señalización vial vehicular con un contador regresivo de seguridad para el cruce peatonal, garantizando un flujo determinista y seguro.

## 📍 Objetivos del Proyecto
- Implementar una **Máquina de Estados Finitos (MEF)** robusta para la gestión de procesos.
- Sincronizar periféricos de salida simple (LEDs) con salidas decodificadas (Display 7-Seg).
- Implementar **Telemetría UART** para el monitoreo de transiciones y tiempos de ciclo.
- Aplicar secuencias de seguridad vial estándar (incluyendo la transición de advertencia Pre-Rojo).

---

## 🧠 Arquitectura de Control: La Máquina de Estados (FSM)

En ingeniería de sistemas embebidos, la FSM es el estándar para eliminar la "lógica espagueti". Al segmentar el programa en estados definidos, logramos:

1. **Determinismo:** El sistema siempre reside en un estado conocido, eliminando comportamientos erráticos.
2. **Seguridad:** Las transiciones están validadas; no es posible saltar de un estado de "Verde" a "Rojo" sin pasar por la advertencia del "Amarillo".
3. **Mantenibilidad:** Facilita la expansión (ej: agregar un botón de pedido de cruce peatonal) sin reescribir la lógica principal.



### Estados del Ciclo Vial:
* **VERDE:** Flujo vehicular habilitado (Prioridad de paso).
* **AMARILLO:** Transición de advertencia (Precaución y despeje de intersección).
* **PRE-ROJO:** Seguridad reforzada (Amarillo + Rojo simultáneos) según normativas específicas.
* **ROJO:** Alto total. Se activa el **Temporizador Visual** para informar al peatón/conductor el tiempo restante de espera.

---

## 🔌 Especificaciones de Hardware
- **Semáforo:** LEDs de alta luminosidad (R, A, V) con resistencias de limitación calculadas para operación continua.
- **Display 7-Seg:** Cátodo Común controlado mediante la librería genérica `LedBar_t`.
- **Telemetría:** Interfaz UART3 a **115200 bps** para logs de diagnóstico.



---

## 📊 Lógica de Sincronización y Bloques de Código

El sistema utiliza el estado **ROJO** no solo como una señal de pare, sino como una tarea de procesamiento temporal. Durante esta fase, el MCU ejecuta un conteo regresivo que sincroniza la lógica aritmética con la visualización física.

```c
/* Ejemplo de transición lógica en el motor de la FSM */
case ESTADO_ROJO:
    Debug_Log("FSM: Iniciando fase de espera activa (Rojo)\r\n");
    HAL_GPIO_WritePin(LED_ROJO_GPIO_Port, LED_ROJO_Pin, GPIO_PIN_SET);
    
    // Conteo regresivo visual para el usuario
    for (int i = 5; i >= 0; i--) {
        Display_Write(i);   // Actualiza el patrón en el display 7-seg
        HAL_Delay(1000);    // Base de tiempo de 1 segundo por dígito
    }
    
    estadoActual = ESTADO_VERDE; // Transición automática al reiniciar ciclo
    break;
```

## 🔍 Análisis sobre el Desafío del Tiempo

En esta versión básica, el conteo regresivo utiliza `HAL_Delay()`.

**Limitación técnica:** Mientras el display cuenta hacia atrás, el CPU está **"bloqueado"**. Si quisiéramos agregar un botón de emergencia que interrumpa el ciclo instantáneamente, el sistema tardaría hasta 1 segundo en responder.

*Hacia el **Nivel Intermedio:** Esta es la justificación perfecta para migrar el sistema a una **MEF No Bloqueante**, permitiendo que el semáforo sea interactivo y responda a eventos externos sin detener su temporización interna.*

---
*“Este proyecto marca la transición de 'encender luces' a 'diseñar sistemas de control'. La implementación de la MEF garantiza que el semáforo sea predecible y profesional, sentando las bases para la automatización industrial.”*