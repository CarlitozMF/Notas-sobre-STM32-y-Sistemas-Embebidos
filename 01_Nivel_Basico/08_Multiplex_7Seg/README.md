# 08_Multiplex_7Seg: Control Multiplexado y Persistencia de Visión 🔢

Este proyecto implementa la técnica de **Persistencia de Visión (POV)** para controlar un módulo de 3 o más displays de 7 segmentos compartiendo un único bus de datos, optimizando drásticamente el uso de pines GPIO en la **Nucleo-F439ZI**.

## 📍 Objetivos del Proyecto
- Implementar **Multiplexación por División de Tiempo** (TDM).
- Gestionar hardware mediante **Bus de Datos** compartido y pines de habilitación (*Enable*).
- Integrar **User Labels** en STM32CubeIDE para mejorar la portabilidad del firmware.
- Introducir la lógica de **Tiempo No Bloqueante** con `HAL_GetTick()`.

---

## 🧠 El Fenómeno POV (Persistence of Vision)
Dado que los segmentos (A-G) de todos los displays están unidos físicamente en el mismo bus, no es posible mostrar números distintos en cada dígito de forma estática. 



La solución es el **Barrido (Scan)**: encendemos un solo dígito a la vez, cargamos su valor, y pasamos al siguiente a una frecuencia superior a los **60 Hz**. El ojo humano no logra percibir el apagado intermedio, integrando la imagen como si todos los displays estuvieran encendidos simultáneamente.

---

## 🛠️ Particularidades Técnicas

### ⏱️ Gestión de Tiempo Asíncrono con `HAL_GetTick()`
Este proyecto marca un hito: el inicio del abandono de `HAL_Delay()`. Utilizamos el contador de milisegundos interno del microcontrolador para crear tareas con diferentes ritmos.



* **Tarea 1 (Rápida):** El barrido del display (debe ser constante para evitar parpadeos).
* **Tarea 2 (Lenta):** El incremento de un contador o proceso lógico (ej. cada 100ms).

```c
/* Ejemplo de temporización no bloqueante */
if (HAL_GetTick() - ultimo_tiempo >= 100) {
    ultimo_tiempo = HAL_GetTick();
    contador_global++; // La lógica avanza sin detener el refresco visual
}
```

## 🏷️ Abstracción con User Labels

Se configuraron etiquetas directamente en el archivo .ioc del CubeIDE. Esto permite que el código sea independiente de si el pin es el PA5 o el PB10:

* SEG_A ... SEG_G: Bus de datos para los segmentos.
* EN1, EN2, EN3: Control de transistores de habilitación (Dígitos).

## 📊 Algoritmo de Barrido (Display Scan)

Para un refresco limpio y sin "efecto fantasma" (Ghosting), se sigue este orden estrictamente:

    1. Apagar todos los habilitadores (EN1=0, EN2=0, EN3=0).
    2. Actualizar el bus de datos con el patrón del nuevo dígito.
    3. Encender el habilitador correspondiente al dígito actual.
    4. Pequeño delay (1-2ms) o salto de ciclo para permitir que el hardware conmute.

## ⚠️ Análisis de Limitaciones

Actualmente, la función Display_Mux_Scan() vive dentro del bucle principal while(1). Esto significa que si el CPU se ocupa en una tarea pesada o bloqueante, el display comenzará a parpadear o perderá brillo.
- Hacia el Nivel Intermedio: En el futuro, moveremos este barrido a una Interrupción de Timer (Timer-IT) para que el display brille de forma autónoma, independientemente de lo que haga el programa principal.

---
*La multiplexación no es solo ahorro de pines; es el arte de sincronizar el tiempo del software con los límites de la percepción humana.*