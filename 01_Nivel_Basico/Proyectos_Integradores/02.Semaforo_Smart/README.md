# EI_2: Semáforo Automático con Máquina de Estados y Temporizador Visual 🚦

Este proyecto integrador del **Nivel Básico** demuestra el control de un sistema secuencial complejo mediante una arquitectura de software robusta. Se coordina la iluminación vial con un contador regresivo de seguridad para peatones.

## 📍 Objetivos del Proyecto
- Implementar una **Máquina de Estados Finitos (MEF)** para el control de flujo.
- Sincronizar periféricos de salida simple (LEDs) con salidas multiplexadas (Display 7-Seg).
- Monitorear el comportamiento del sistema en tiempo real mediante **Debug Logs** (UART).
- Aplicar secuencias de seguridad vial avanzadas (Transición Pre-Rojo).

## 🧠 ¿Qué es una Máquina de Estados Finitos (MEF)?
La MEF es un modelo de comportamiento que consiste en un conjunto de **estados**, **transiciones** y **acciones**. En sistemas embebidos, es fundamental porque:
1. **Elimina la lógica "Espagueti":** Evita el uso excesivo de `if-else` anidados.
2. **Determinismo:** El sistema siempre sabe en qué estado está y cuál es el siguiente paso lógico.
3. **Escalabilidad:** Permite añadir nuevas funciones (como un modo intermitente nocturno) simplemente agregando un nuevo estado.

### Estados implementados en este diseño:
- **VERDE:** Flujo vehicular habilitado.
- **AMARILLO:** Transición de advertencia (Precaución).
- **PRE-ROJO:** Seguridad reforzada (Amarillo + Rojo simultáneos).
- **ROJO:** Alto total con temporizador visual para cruce peatonal.



## 🔌 Conexión de Hardware
- **Leds de Tráfico:** Conectados a pines GPIO configurados como `Output`.
- **Display 7 Segmentos:** Utiliza la estructura genérica `LedBar_t` definida en módulos anteriores.
- **Terminal Serial:** Configurada a 115200 bps para recibir los logs de estado.



## 📊 Lógica de Sincronización
El sistema utiliza el `ESTADO_ROJO` no solo como una señal de pare, sino como una sub-rutina de control de tiempo. Mientras los vehículos están detenidos, se ejecuta un bucle `for` que actualiza el display en reversa, proporcionando información clara al usuario antes de reiniciar el ciclo.

```c
// Ejemplo de transición controlada en el estado Rojo
case ESTADO_ROJO:
    HAL_GPIO_WritePin(Rojo_Port, Rojo_Pin, SET);
    for (int i = 5; i >= 0; i--) {
        Display_Write(i);  // Sincronización con el display
        HAL_Delay(1000);   // Base de tiempo de 1 segundo
    }
    estadoActual = ESTADO_VERDE;
    break;
```

---
*Este proyecto marca la transición de "encender luces" a "diseñar sistemas de control". La implementación de la MEF garantiza que el semáforo sea predecible, seguro y profesional, sentando las bases para proyectos de automatización más ambiciosos en el Nivel Intermedio.*